#include "bm_story_scenes.h"

#include "bm_audio.h"
#include "bm_flow.h"

#include "bn_bg_palettes.h"
#include "bn_string.h"

#include "bn_sprite_items_spr_particles.h"

namespace bm
{
    namespace
    {
        struct beat_def
        {
            const char* big;
            const char* lines[3];
            uint16_t frames;
        };

        // PART 36.1: a peaceful factory, a destabilising signal, overload, falling blocks, the eye opens,
        // emergency, the Block Mayhem system comes online, then the title.
        constexpr beat_def intro_beats[] = {
            { nullptr, { "SECTOR 7 ARCADE FACTORY.", "ALL SYSTEMS NOMINAL.", nullptr }, 220 },
            { nullptr, { "06:00 - AN UNKNOWN SIGNAL", "FLOODS THE POWER GRID...", nullptr }, 190 },
            { nullptr, { "THE MACHINES OVERLOAD!", nullptr, nullptr }, 150 },
            { nullptr, { "STRANGE BLOCKS RAIN", "FROM THE SKY.", nullptr }, 190 },
            { nullptr, { "DEEP INSIDE THE REACTOR...", "SOMETHING OPENS ITS EYE.", nullptr }, 240 },
            { "EMERGENCY", { "FACTORY LOCKDOWN ENGAGED.", nullptr, nullptr }, 150 },
            { nullptr, { "BLOCK MAYHEM SYSTEM ONLINE.", "GOOD LUCK, OPERATOR.", nullptr }, 170 },
        };

        constexpr beat_def ending_beats[] = {
            { nullptr, { "THE CORE OF MAYHEM", "COLLAPSES!", nullptr }, 270 },
            { nullptr, { "THE REACTOR GOES QUIET.", "THE FACTORY HUMS BACK", "TO LIFE." }, 250 },
            { "SPICE PROTOCOL", { "COMPLETE.", nullptr, nullptr }, 200 },
            { "CHAMPION", { nullptr, nullptr, nullptr }, 320 },      // statistics are filled in at run time
        };

        constexpr const char* world_flavor[7][2] = {
            { "", "" },
            { "THE FACTORY FLOOR RUNS WILD.", "LEARN THE MACHINE FIRST." },
            { "COOLANT LEAKS FROZE THE LINES.", "BURN THE ICE, KEEP MOVING." },
            { "THE FOUNDRY SITS ON A VOLCANO.", "MIND THE PRESSURE." },
            { "A NEON CITY ON STOLEN POWER.", "TWO GRIDS, ONE BRAIN." },
            { "UP IS DOWN, DOWN IS SIDEWAYS.", "FOLLOW THE ARROW." },
            { "EVERY HAZARD AT ONCE.", "THE CORE IS WAITING." },
        };

        bn::string<32> dynamic_lines[3];
    }

    story_scene::story_scene(story_kind kind) :
        _kind(kind),
        _camera(bn::camera_ptr::create(0, 0)),
        _fx(_camera)
    {
        const app& a = the_app();
        _fx.set_options(a.save.settings.shake, a.save.settings.flashes);
        _start_beat(0);
    }

    story_scene::~story_scene()
    {
        bn::bg_palettes::set_fade_intensity(0);
    }

    int story_scene::_beat_count() const
    {
        switch(_kind)
        {
        case story_kind::intro: return int(sizeof(intro_beats) / sizeof(intro_beats[0]));
        case story_kind::ending: return int(sizeof(ending_beats) / sizeof(ending_beats[0]));
        case story_kind::world_intro:
        default: return 1;
        }
    }

    int story_scene::_beat_length(int beat) const
    {
        switch(_kind)
        {
        case story_kind::intro: return intro_beats[beat].frames;
        case story_kind::ending: return ending_beats[beat].frames;
        case story_kind::world_intro:
        default: return 280;
        }
    }

    scene_id story_scene::_next_scene() const
    {
        if(_kind == story_kind::ending)
        {
            the_app().flow.credits_from_ending = true;
        }
        switch(_kind)
        {
        case story_kind::intro: return scene_id::title;
        case story_kind::ending: return scene_id::credits;
        case story_kind::world_intro:
        default: return scene_id::stage_card;
        }
    }

    void story_scene::_set_backdrop(backdrop_id id, bn::fixed dim)
    {
        if(! _backdrop || _backdrop_id != id)
        {
            _backdrop.reset();
            _backdrop.emplace(id, 3);
            _backdrop->set_camera(_camera);
            _backdrop_id = id;
        }
        _backdrop->set_dim(dim);
    }

    void story_scene::_start_beat(int beat)
    {
        _beat = beat;
        _beat_timer = 0;
        _typed = 0;
        _big.hide();
        _sub.hide();
        for(label& l : _lines)
        {
            l.hide();
        }
        app& a = the_app();
        if(_kind == story_kind::intro)
        {
            switch(beat)
            {
            case 0:
                _set_backdrop(backdrop_id::world1, bn::fixed(0.15));
                audio::play_music(audio::music_id::classic);
                break;
            case 1:
                _backdrop->excite(190, 2);
                audio::sfx(audio::sfx_id::spark);
                break;
            case 2:
                _fx.flash(bn::color(31, 6, 2), 45, 14);
                _fx.shake(shake_profile::large);
                audio::sfx(audio::sfx_id::nuke);
                break;
            case 3:
                _raining = true;
                break;
            case 4:
                _raining = false;
                for(auto& r : _rain)
                {
                    r.reset();
                }
                _set_backdrop(backdrop_id::world6, bn::fixed(0.35));
                _boss.emplace(6, _camera, 120, 62);
                audio::play_music(audio::music_id::final_boss);
                audio::sfx(audio::sfx_id::boss_charge);
                break;
            case 5:
                _alarm = true;
                break;
            case 6:
                _alarm = false;
                bn::bg_palettes::set_fade_intensity(0);
                _boss.reset();
                _set_backdrop(backdrop_id::title, bn::fixed(0));
                _fx.flash(bn::color(31, 31, 31), 70, 20);
                audio::play_music(audio::music_id::title);
                break;
            default:
                break;
            }
        }
        else if(_kind == story_kind::ending)
        {
            switch(beat)
            {
            case 0:
            {
                _set_backdrop(backdrop_id::world6, bn::fixed(0.3));
                _boss.emplace(6, _camera, 120, 62);
                event e;
                e.type = event_type::boss_defeated;
                _boss->on_event(e, _fx);
                audio::play_music(audio::music_id::final_boss);
                break;
            }
            case 1:
                _boss.reset();
                _set_backdrop(backdrop_id::world1, bn::fixed(0.1));
                _fx.flash(bn::color(31, 31, 31), 80, 24);
                audio::play_music(audio::music_id::ending);
                break;
            case 2:
                _raining = true;
                break;
            case 3:
            {
                _raining = false;
                const save_data& sv = a.save;
                int cleared = 0;
                int s_ranks = 0;
                for(const stage_record& r : sv.stages)
                {
                    cleared += r.cleared ? 1 : 0;
                    s_ranks += r.best_rank >= 4 ? 1 : 0;
                }
                dynamic_lines[0] = "STAGES CLEARED ";
                dynamic_lines[0].append(bn::to_string<4>(cleared));
                dynamic_lines[0].append("/30");
                dynamic_lines[1] = "S RANKS OR BETTER ";
                dynamic_lines[1].append(bn::to_string<4>(s_ranks));
                const uint32_t mins = sv.stats.play_seconds / 60;
                dynamic_lines[2] = "PLAY TIME ";
                dynamic_lines[2].append(bn::to_string<8>(mins / 60));
                dynamic_lines[2].append("H ");
                dynamic_lines[2].append(bn::to_string<4>(mins % 60));
                dynamic_lines[2].append("M");
                break;
            }
            default:
                break;
            }
        }
        else
        {
            const int w = a.flow.world >= 1 && a.flow.world <= 6 ? a.flow.world : 1;
            _set_backdrop(backdrop_id(int(backdrop_id::world1) + w - 1), bn::fixed(0.3));
            audio::play_music(audio::music_id(int(audio::music_id::world1) + w - 1));
            bn::string<16> t = "WORLD ";
            t.append(bn::to_string<4>(w));
            _big.show_big(120, 30, t, big_color::fire, align::center);
            _sub.show(120, 54, world_name(w), text_color::yellow, align::center);
        }
        // Big titles appear at once; the story lines type out.
        beat = beat < _beat_count() ? beat : _beat_count() - 1;
        if(_kind == story_kind::intro && intro_beats[beat].big)
        {
            // The Core of Mayhem fills the centre during the emergency: title above it, text below.
            _big.show_big(120, beat == 5 ? 6 : 46, intro_beats[beat].big, beat == 5 ? big_color::blood : big_color::fire,
                          align::center);
        }
        if(_kind == story_kind::ending && ending_beats[beat].big)
        {
            _big.show_big(120, 40, ending_beats[beat].big, big_color::gold, align::center);
        }
        _hint.show_tiny(236, 153, "START: SKIP", text_color::gray, align::right);
    }

    void story_scene::_type_lines()
    {
        const char* lines[3] = { nullptr, nullptr, nullptr };
        int y0 = 112;
        if(_kind == story_kind::intro)
        {
            for(int i = 0; i < 3; ++i)
            {
                lines[i] = intro_beats[_beat].lines[i];
            }
            y0 = _beat == 5 ? 126 : (intro_beats[_beat].big ? 76 : 112);
        }
        else if(_kind == story_kind::ending)
        {
            for(int i = 0; i < 3; ++i)
            {
                lines[i] = _beat == 3 ? dynamic_lines[i].c_str() : ending_beats[_beat].lines[i];
            }
            y0 = ending_beats[_beat].big ? 72 : 108;
        }
        else
        {
            const int w = the_app().flow.world >= 1 && the_app().flow.world <= 6 ? the_app().flow.world : 1;
            static bn::string<32> boss_line;
            boss_line = "BOSS: ";
            boss_line.append(boss(stage(w, 5).boss).short_name);
            lines[0] = world_flavor[w][0];
            lines[1] = world_flavor[w][1];
            lines[2] = boss_line.c_str();
            y0 = 92;
        }
        int budget = _typed;
        for(int i = 0; i < 3; ++i)
        {
            if(! lines[i] || ! lines[i][0])
            {
                _lines[i].hide();
                continue;
            }
            const bn::string_view full(lines[i]);
            const int n = budget < full.size() ? budget : full.size();
            budget -= n;
            const text_color c = _kind == story_kind::world_intro && i == 2 ? text_color::red : text_color::white;
            if(n > 0)
            {
                _lines[i].show(120, y0 + i * 13, full.substr(0, n), c, align::center);
            }
            else
            {
                _lines[i].hide();
            }
        }
    }

    bn::optional<scene_id> story_scene::update()
    {
        ++_frame;
        ++_beat_timer;
        _input.update();
        if(_backdrop)
        {
            _backdrop->update();
        }
        _fx.update();
        if(_boss)
        {
            _boss->update(_kind == story_kind::intro && _beat == 5, _frame, _fx);
        }

        // Typewriter: one character every two frames.
        if(_frame % 2 == 0)
        {
            ++_typed;
        }
        _type_lines();

        // Beat-specific animation.
        if(_kind == story_kind::intro)
        {
            if(_beat == 1 && _beat_timer % 24 == 0)
            {
                _fx.burst(particle_kind::electric, 30 + (_frame * 37) % 180, 30 + (_frame * 17) % 60, 4, 3);
                _fx.shake(shake_profile::small);
            }
            if(_beat == 2 && _beat_timer % 18 == 0)
            {
                _fx.explosion(30 + (_frame * 53) % 180, 30 + (_frame * 29) % 80);
                audio::sfx(audio::sfx_id::bomb);
            }
            if(_beat == 4 && _beat_timer == 120)
            {
                _fx.flash(bn::color(31, 0, 0), 30, 18);
            }
        }
        if(_alarm)
        {
            const int k = (_beat_timer / 2) % 30;
            bn::bg_palettes::set_fade(bn::color(31, 0, 0), bn::fixed(k < 15 ? k : 30 - k) / 40);
            if(_beat_timer % 30 == 1)
            {
                audio::sfx(audio::sfx_id::danger);
            }
        }
        if(_raining)
        {
            for(int i = 0; i < rain_count; ++i)
            {
                if(! _rain[i])
                {
                    const int frame = _kind == story_kind::ending ? int(particle_kind::star) : 24 + i % 4;
                    _rain[i] = bn::sprite_items::spr_particles.create_sprite(0, 0, frame);
                    _rain_y[i] = -10 - i * 17;
                }
                _rain_y[i] += 1 + i % 3;
                if(_rain_y[i] > 170)
                {
                    _rain_y[i] = -10;
                }
                _rain[i]->set_position(14 + (i * 23) % 212 - 120, _rain_y[i] - 80);
            }
        }

        if(_input.start())
        {
            audio::sfx(audio::sfx_id::menu_ok);
            return _next_scene();
        }
        const bool typed_all = _typed > 80;
        if(_input.ok())
        {
            if(! typed_all)
            {
                _typed = 200;       // finish typing first
            }
            else
            {
                _beat_timer = _beat_length(_beat);
            }
        }
        if(the_app().flow.autoplay && _beat_timer > 60)
        {
            _beat_timer = _beat_length(_beat);
        }
        if(_beat_timer >= _beat_length(_beat))
        {
            if(_beat + 1 >= _beat_count())
            {
                return _next_scene();
            }
            _start_beat(_beat + 1);
        }
        return bn::nullopt;
    }
}
