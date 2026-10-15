#include "bm_app.h"

#include "bm_run.h"

#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_memory.h"
#include "bn_sram.h"

namespace bm
{
    namespace
    {
        constexpr int slot_offset[2] = { 0, 2048 };

        BN_DATA_EWRAM bm_test_interface test_interface;

        uint16_t keypad_bits(bool pressed)
        {
            uint16_t bits = 0;
            auto add = [&bits, pressed](bn::keypad::key_type key, uint16_t bit)
            {
                if(pressed ? bn::keypad::pressed(key) : bn::keypad::held(key))
                {
                    bits = uint16_t(bits | bit);
                }
            };
            add(bn::keypad::key_type::A, key_a);
            add(bn::keypad::key_type::B, key_b);
            add(bn::keypad::key_type::SELECT, key_select);
            add(bn::keypad::key_type::START, key_start);
            add(bn::keypad::key_type::RIGHT, key_right);
            add(bn::keypad::key_type::LEFT, key_left);
            add(bn::keypad::key_type::UP, key_up);
            add(bn::keypad::key_type::DOWN, key_down);
            add(bn::keypad::key_type::R, key_r);
            add(bn::keypad::key_type::L, key_l);
            return bits;
        }
    }

    app& the_app()
    {
        static app instance;
        return instance;
    }

    app::app()
    {
        const char magic[8] = { 'B', 'M', 'T', 'E', 'S', 'T', 'I', 'F' };
        for(int i = 0; i < 8; ++i)
        {
            test_interface.magic[i] = magic[i];
        }
        test_interface.version = BM_TI_VERSION;
        test = &test_interface;
        game = new session();
        run = new chaos_run();
    }

    void app::load_save()
    {
        save_data a;
        save_data b;
        bn::sram::read_offset(a, slot_offset[0]);
        bn::sram::read_offset(b, slot_offset[1]);
        const bm::save_status st = pick(a, b, save, _slot);
        _sequence = _slot >= 0 ? save.sequence : 0;
        save_status = int(st);
        test->save_loaded = st == bm::save_status::ok ? 1 : 2;
        test->worlds_unlocked = save.worlds_unlocked;
    }

    bool app::write_save()
    {
        ++_sequence;
        seal(save, _sequence);
        const int slot = _slot == 0 ? 1 : 0;
        bn::sram::write_offset(save, slot_offset[slot]);
        // Verify by reading back.
        save_data check;
        bn::sram::read_offset(check, slot_offset[slot]);
        if(validate(check) != bm::save_status::ok || check.sequence != _sequence)
        {
            test->error_code = 50;
            return false;
        }
        _slot = slot;
        ++test->save_writes;
        test->worlds_unlocked = save.worlds_unlocked;
        return true;
    }

    input_frame app::gameplay_input()
    {
        input_frame in;
        if(flow.autoplay)
        {
            in = autoplayer.next(*game);
            // START still pauses during autoplay so a human can take over.
            if(bn::keypad::pressed(bn::keypad::key_type::START))
            {
                in.pressed = uint16_t(in.pressed | key_start);
                in.held = uint16_t(in.held | key_start);
            }
            return in;
        }
        in.held = keypad_bits(false);
        in.pressed = keypad_bits(true);
        return in;
    }

    void app::tick()
    {
        ++frame;
        bm_test_interface& t = *test;
        t.frame = frame;
        const int usage = (bn::core::last_cpu_usage() * 1000).right_shift_integer();
        t.cpu_last = uint32_t(usage);
        if(uint32_t(usage) > t.cpu_max)
        {
            t.cpu_max = uint32_t(usage);
        }
        t.cpu_avg = (t.cpu_avg * t.cpu_frames + uint32_t(usage)) / (t.cpu_frames + 1);
        if(t.cpu_frames < 100000)
        {
            ++t.cpu_frames;
        }
        if(t.cmd && t.cmd != t.ack)
        {
            pending_command = t.cmd;
            pending_args[0] = t.arg0;
            pending_args[1] = t.arg1;
            pending_args[2] = t.arg2;
            pending_args[3] = t.arg3;
            if(t.cmd == BM_TI_CMD_RESET_CPU_STATS)
            {
                t.cpu_max = 0;
                t.cpu_avg = 0;
                t.cpu_frames = 0;
                pending_command = 0;
            }
            else if(t.cmd == BM_TI_CMD_SET_AUTOPLAY)
            {
                flow.autoplay = t.arg0 != 0;
                autoplayer.reset(bot_strategy::resource_aware, 77);
                pending_command = 0;
            }
            t.ack = t.cmd;
            t.cmd = 0;
        }
        t.autoplay = flow.autoplay ? 1 : 0;

        // Session status mirror.
        const session& s = *game;
        t.mode = uint32_t(s.config().mode);
        t.session_state = uint32_t(s.state());
        t.score = s.score();
        t.lines = uint32_t(s.lines());
        t.level = uint32_t(s.level());
        t.pieces = s.stats().pieces;
        t.heat = uint32_t(s.heat());
        t.overheat = s.overheat_active() ? 1 : 0;
        t.overheat_success = s.stats().overheat_success;
        t.overheat_fail = s.stats().overheat_fail;
        t.max_chain = s.stats().max_chain;
        t.bombs = s.stats().bombs;
        t.boss_hp = uint32_t(s.boss_hp());
        t.boss_max_hp = uint32_t(s.boss_max_hp());
        t.boss_phase = uint32_t(s.boss_phase());
        t.boss_attacks = uint32_t(s.boss_attacks());
        t.objective_done = s.objectives_complete() ? 1 : 0;
        t.stack_height = uint32_t(s.board_player(s.active_board()).grid().stack_height());
        t.active_board = uint32_t(s.active_board());
        t.stage = s.config().stage ? uint32_t(s.config().stage->world * 10 + s.config().stage->index) : 0;
        int cleared = 0;
        for(const stage_record& r : save.stages)
        {
            cleared += r.cleared ? 1 : 0;
        }
        t.stages_cleared = uint32_t(cleared);
        int unlocked = 0;
        for(int i = 0; i < 32; ++i)
        {
            unlocked += (save.achievements >> i) & 1;
        }
        t.achievements = uint32_t(unlocked);
        t.run_phase = uint32_t(run->phase());
        t.run_index = uint32_t(run->index());
        t.run_cleared = uint32_t(run->cleared());
        t.run_score = run->score();
        t.run_upgrades = uint32_t(run->build().upgrades_owned());
        t.rush_index = flow.boss_rush_index;
        t.rush_state = flow.boss_rush_state;
        t.rush_frames = flow.boss_rush_frames;
        const int v = s.validate();
        if(v && ! t.error_code)
        {
            t.error_code = uint32_t(v);
        }
    }

    bn::fixed app::music_volume() const
    {
        return bn::fixed(save.settings.music_volume) / 10;
    }

    bn::fixed app::sfx_volume() const
    {
        return bn::fixed(save.settings.sfx_volume) / 10;
    }
}
