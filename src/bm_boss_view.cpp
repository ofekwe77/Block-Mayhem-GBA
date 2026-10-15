#include "bm_boss_view.h"

#include "bm_audio.h"

#include "bn_color.h"
#include "bn_math.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_string.h"

#include "bn_sprite_items_boss1_arm.h"
#include "bn_sprite_items_boss1_body.h"
#include "bn_sprite_items_boss1_eye.h"
#include "bn_sprite_items_boss1_mouth.h"
#include "bn_sprite_items_boss2_body.h"
#include "bn_sprite_items_boss2_breath.h"
#include "bn_sprite_items_boss2_crystal.h"
#include "bn_sprite_items_boss2_head.h"
#include "bn_sprite_items_boss3_base.h"
#include "bn_sprite_items_boss3_fire.h"
#include "bn_sprite_items_boss3_head.h"
#include "bn_sprite_items_boss3_neck.h"
#include "bn_sprite_items_boss4_body.h"
#include "bn_sprite_items_boss4_bolt.h"
#include "bn_sprite_items_boss4_face.h"
#include "bn_sprite_items_boss4_orb.h"
#include "bn_sprite_items_boss5_body.h"
#include "bn_sprite_items_boss5_cube.h"
#include "bn_sprite_items_boss5_head.h"
#include "bn_sprite_items_boss5_ring.h"
#include "bn_sprite_items_boss6_eye.h"
#include "bn_sprite_items_boss6_heart.h"
#include "bn_sprite_items_boss6_iris.h"
#include "bn_sprite_items_boss6_ring.h"
#include "bn_sprite_items_boss6_tentacle.h"
#include "bn_sprite_items_boss6_wing.h"

namespace bm
{
    namespace
    {
        namespace si = bn::sprite_items;
        using r = part_role;

        // ------------------------------------------------------------ rigs (part, role, offset, flip, instance, z)
        const part_def rig1[] = {
            { &si::boss1_arm, r::arm, 34, -4, true, 1, 3, false },
            { &si::boss1_body, r::body, 0, 0, false, 0, 2, false },
            { &si::boss1_eye, r::eye, -7, -14, false, 0, 1, false },
            { &si::boss1_eye, r::eye, 7, -14, true, 1, 1, false },
            { &si::boss1_mouth, r::mouth, 0, -1, false, 0, 1, false },
            { &si::boss1_arm, r::arm, -34, -4, false, 0, 0, false },
        };

        const part_def rig2[] = {
            { &si::boss2_crystal, r::crystal, -30, -18, false, 0, 3, false },
            { &si::boss2_crystal, r::crystal, 30, -18, true, 1, 3, false },
            { &si::boss2_body, r::body, 0, 8, false, 0, 2, false },
            { &si::boss2_head, r::head, 0, -30, false, 0, 1, false },
            { &si::boss2_breath, r::breath, -30, -24, false, 0, 0, false },
        };

        const part_def rig3[] = {
            { &si::boss3_base, r::body, 0, 44, false, 0, 4, false },
            { &si::boss3_neck, r::neck, 0, 0, false, 0, 3, false },
            { &si::boss3_neck, r::neck, 0, 0, false, 1, 3, false },
            { &si::boss3_neck, r::neck, 0, 0, false, 2, 3, false },
            { &si::boss3_neck, r::neck, 0, 0, false, 3, 3, false },
            { &si::boss3_neck, r::neck, 0, 0, false, 4, 3, false },
            { &si::boss3_neck, r::neck, 0, 0, false, 5, 3, false },
            { &si::boss3_head, r::head, -22, -26, false, 0, 1, false },
            { &si::boss3_head, r::head, 6, -46, false, 1, 1, false },
            { &si::boss3_head, r::head, 26, -10, false, 2, 1, false },
            { &si::boss3_fire, r::fire, 0, 0, false, 0, 0, false },
        };

        const part_def rig4[] = {
            { &si::boss4_body, r::body, 0, 0, false, 0, 2, false },
            { &si::boss4_face, r::face, 0, 6, false, 0, 1, false },
            { &si::boss4_orb, r::orb, 0, 0, false, 0, 0, false },
            { &si::boss4_orb, r::orb, 0, 0, false, 1, 0, false },
            { &si::boss4_orb, r::orb, 0, 0, false, 2, 0, false },
            { &si::boss4_orb, r::orb, 0, 0, false, 3, 0, false },
            { &si::boss4_bolt, r::bolt, -24, 40, false, 0, 0, false },
            { &si::boss4_bolt, r::bolt, 24, 40, true, 1, 0, false },
        };

        const part_def rig5[] = {
            { &si::boss5_ring, r::ring, 0, -10, false, 0, 4, false },
            { &si::boss5_body, r::body, 0, 16, false, 0, 2, false },
            { &si::boss5_head, r::head, 0, -24, false, 0, 1, false },
            { &si::boss5_cube, r::cube, 0, 0, false, 0, 0, false },
            { &si::boss5_cube, r::cube, 0, 0, false, 1, 0, false },
            { &si::boss5_cube, r::cube, 0, 0, false, 2, 0, false },
            { &si::boss5_cube, r::cube, 0, 0, false, 3, 0, false },
        };

        const part_def rig6[] = {
            { &si::boss6_ring, r::ring, 0, 0, false, 0, 4, true },
            { &si::boss6_wing, r::wing, -44, -8, false, 0, 4, true },
            { &si::boss6_wing, r::wing, 44, -8, true, 1, 4, true },
            { &si::boss6_tentacle, r::tentacle, -18, 38, false, 0, 3, false },
            { &si::boss6_tentacle, r::tentacle, 18, 38, true, 1, 3, false },
            { &si::boss6_heart, r::heart, 0, 30, false, 0, 2, false },
            { &si::boss6_eye, r::eye, 0, 0, false, 0, 1, false },
            { &si::boss6_iris, r::iris, 0, 0, false, 0, 0, false },
        };

        struct rig_ref
        {
            const part_def* parts;
            int count;
        };

        rig_ref rig_for(int id)
        {
            switch(id)
            {
            case 2: return { rig2, int(sizeof(rig2) / sizeof(rig2[0])) };
            case 3: return { rig3, int(sizeof(rig3) / sizeof(rig3[0])) };
            case 4: return { rig4, int(sizeof(rig4) / sizeof(rig4[0])) };
            case 5: return { rig5, int(sizeof(rig5) / sizeof(rig5[0])) };
            case 6: return { rig6, int(sizeof(rig6) / sizeof(rig6[0])) };
            default: return { rig1, int(sizeof(rig1) / sizeof(rig1[0])) };
            }
        }

        bn::fixed sine(int degrees)
        {
            degrees %= 360;
            if(degrees < 0)
            {
                degrees += 360;
            }
            return bn::degrees_lut_sin(degrees);
        }

        bn::fixed cosine(int degrees)
        {
            return sine(degrees + 90);
        }

        // Hydra head positions for the neck curves.
        constexpr int hydra_heads[3][2] = { { -22, -26 }, { 6, -46 }, { 26, -10 } };
    }

    const char* attack_name(int kind)
    {
        switch(attack_kind(kind))
        {
        case attack_kind::garbage: return "GARBAGE";
        case attack_kind::punch: return "MEGA PUNCH";
        case attack_kind::armor_inject: return "ARMOR PLATES";
        case attack_kind::ice_inject: return "ICE COAT";
        case attack_kind::frozen_wall: return "FROZEN WALL";
        case attack_kind::chill: return "DEEP CHILL";
        case attack_kind::crystal_shards: return "CRYSTAL SHARDS";
        case attack_kind::fire_breath: return "FIRE BREATH";
        case attack_kind::molten_rain: return "MOLTEN RAIN";
        case attack_kind::heat_drain: return "HEAT DRAIN";
        case attack_kind::surge: return "POWER SURGE";
        case attack_kind::cross_garbage: return "CROSS STRIKE";
        case attack_kind::forced_switch: return "BOARD SWAP";
        case attack_kind::gravity_left: return "GRAVITY ←";
        case attack_kind::gravity_right: return "GRAVITY →";
        case attack_kind::gravity_up: return "GRAVITY ↑";
        case attack_kind::gravity_flip: return "GRAVITY FLIP";
        case attack_kind::core_combo: return "MAYHEM COMBO";
        default: return "ATTACK";
        }
    }

    const char* hazard_name(int kind)
    {
        switch(hazard_kind(kind))
        {
        case hazard_kind::freeze_cells: return "FREEZE";
        case hazard_kind::lava_pressure: return "MAGMA ROW";
        case hazard_kind::gravity_shift: return "GRAVITY SHIFT";
        case hazard_kind::settle_cycle: return "SETTLE SHIFT";
        case hazard_kind::garbage_drip: return "GARBAGE";
        case hazard_kind::armor_plates: return "ARMOR";
        case hazard_kind::surge: return "SURGE";
        case hazard_kind::molten_rain: return "MOLTEN RAIN";
        case hazard_kind::mixed: return "HAZARD";
        default: return "HAZARD";
        }
    }

    boss_view::boss_view(int boss_id, const bn::camera_ptr& camera, int center_x, int center_y) :
        _id(boss_id),
        _cx(center_x),
        _cy(center_y),
        _tx(center_x),
        _ty(center_y)
    {
        const rig_ref rig = rig_for(boss_id);
        _count = rig.count < max_parts ? rig.count : max_parts;
        for(int i = 0; i < _count; ++i)
        {
            part& p = _parts[i];
            p.def = rig.parts[i];
            p.frames = int8_t(p.def.item->tiles_item().graphics_count());
            p.sprite = p.def.item->create_sprite(center_x - 120 + p.def.x, center_y - 80 + p.def.y, 0);
            p.sprite->set_camera(camera);
            p.sprite->set_horizontal_flip(p.def.hflip);
            p.sprite->set_bg_priority(p.def.behind_board ? 2 : 1);
            p.sprite->set_z_order(p.def.z);
            if(! _palette)
            {
                _palette = p.sprite->palette();
            }
            else
            {
                p.sprite->set_palette(*_palette);
            }
        }
        _set_state(boss_anim::intro, 90);
    }

    int boss_view::_rand(int n)
    {
        _seed = _seed * 1103515245u + 12345u;
        return n > 0 ? int((_seed >> 16) % uint32_t(n)) : 0;
    }

    void boss_view::_set_state(boss_anim state, int timer)
    {
        _state = state;
        _timer = timer;
    }

    void boss_view::move_to(int center_x, int center_y, int frames)
    {
        _tx = center_x;
        _ty = center_y;
        _move_frames = frames > 0 ? frames : 1;
    }

    void boss_view::on_event(const event& e, fx& effects)
    {
        if(_state == boss_anim::defeat || _state == boss_anim::finished)
        {
            return;
        }
        const int cx = center_x();
        const int cy = center_y();
        switch(e.type)
        {
        case event_type::boss_telegraph:
            _attack_kind = e.a;
            _attacking_head = _phase >= 2 ? 2 : (_phase + _rand(2)) % 3;
            if(_attacking_head < _phase)
            {
                _attacking_head = _phase < 3 ? _phase : 2;
            }
            if(_state != boss_anim::phase_change)
            {
                _set_state(boss_anim::telegraph, 0);
            }
            audio::sfx(audio::sfx_id::boss_charge);
            effects.burst(particle_kind::spark, cx, cy - 10, 4, 2);
            break;

        case event_type::boss_attack:
            _attack_kind = e.a;
            _set_state(boss_anim::attack, 28);
            audio::sfx(audio::sfx_id::boss_attack);
            effects.shake(shake_profile::boss);
            break;

        case event_type::boss_damage:
        {
            _flash = 8;
            if(_state == boss_anim::idle || _state == boss_anim::recover)
            {
                _set_state(boss_anim::hit, 14);
            }
            audio::sfx(audio::sfx_id::boss_hit);
            bn::string<12> t = "-";
            t.append(bn::to_string<8>(e.a));
            effects.popup(cx + _rand(20) - 10, cy - 30, t, text_color::red, 34);
            effects.burst(particle_kind::spark, cx + _rand(24) - 12, cy + _rand(24) - 12, 5, 3);
            if(e.a >= 20)
            {
                effects.explosion(cx + _rand(20) - 10, cy + _rand(20) - 10);
                effects.hitstop(3);
            }
            break;
        }

        case event_type::boss_phase:
            _phase = e.a;
            _set_state(boss_anim::phase_change, cfg::boss_phase_transition_frames);
            effects.shake(shake_profile::large);
            effects.flash(bn::color(31, 6, 4), 35, 12);
            effects.explosion(cx, cy);
            audio::sfx(audio::sfx_id::boss_defeat);
            break;

        case event_type::boss_defeated:
            _set_state(boss_anim::defeat, 200);
            effects.flash(bn::color(31, 31, 31), 45, 10);
            audio::sfx(audio::sfx_id::boss_defeat);
            break;

        default:
            break;
        }
    }

    void boss_view::update(const session& s, int frame, fx& effects)
    {
        if(_move_frames > 0)
        {
            _cx += (_tx - _cx) / _move_frames;
            _cy += (_ty - _cy) / _move_frames;
            --_move_frames;
        }

        if(_timer > 0)
        {
            --_timer;
            if(_timer == 0)
            {
                switch(_state)
                {
                case boss_anim::intro:
                case boss_anim::hit:
                case boss_anim::recover:
                case boss_anim::phase_change:
                    _set_state(s.boss_action().source ? boss_anim::telegraph : boss_anim::idle, 0);
                    break;
                case boss_anim::attack:
                    _set_state(boss_anim::recover, 16);
                    break;
                case boss_anim::defeat:
                    _set_state(boss_anim::finished, 0);
                    effects.explosion(center_x(), center_y());
                    effects.shake(shake_profile::final_blast);
                    effects.flash(bn::color(31, 31, 31), 60, 14);
                    break;
                default:
                    break;
                }
            }
        }
        if(_state == boss_anim::telegraph && ! s.boss_action().source)
        {
            _set_state(boss_anim::idle, 0);     // attack cancelled (shield / phase change)
        }

        // Defeat sequence: escalating localized explosions, then a collapse.
        if(_state == boss_anim::defeat)
        {
            if((_timer % 12) == 0)
            {
                const part& p = _parts[_rand(_count)];
                effects.explosion(center_x() + p.def.x + _rand(16) - 8, center_y() + p.def.y + _rand(16) - 8);
                effects.burst(particle_kind::spark, center_x() + _rand(40) - 20, center_y() + _rand(40) - 20, 4, 3);
                effects.shake(_timer > 100 ? shake_profile::medium : shake_profile::large);
                audio::sfx(audio::sfx_id::bomb);
            }
            if(_timer == 40)
            {
                effects.flash(bn::color(31, 31, 31), 55, 12);
                effects.shake(shake_profile::final_blast);
                audio::sfx(audio::sfx_id::nuke);
            }
        }

        // Palette: hit flash, enraged tint in late phases.
        if(_palette)
        {
            if(_flash > 0)
            {
                --_flash;
                _palette->set_fade(bn::color(31, 31, 31), bn::fixed(_flash) / 12);
            }
            else if(_state == boss_anim::defeat)
            {
                _palette->set_fade(bn::color(31, 8, 0), bn::fixed((frame >> 2) & 3) / 8);
            }
            else if(_phase >= 2 && _state != boss_anim::finished)
            {
                _palette->set_fade(bn::color(31, 0, 0), bn::fixed(((frame >> 4) & 1) ? 12 : 6) / 100);
            }
            else
            {
                _palette->set_fade_intensity(0);
            }
        }

        _apply(frame);
    }

    void boss_view::_apply(int frame)
    {
        const bool finished = _state == boss_anim::finished;
        const bool enraged = _phase >= 2;
        int shake_x = 0;
        int shake_y = 0;
        if(_state == boss_anim::phase_change || _state == boss_anim::defeat)
        {
            const int a = _state == boss_anim::defeat ? 1 + (200 - _timer) / 50 : 2;
            shake_x = _rand(a * 2 + 1) - a;
            shake_y = _rand(a * 2 + 1) - a;
        }
        else if(_state == boss_anim::telegraph)
        {
            shake_x = (frame & 2) ? 1 : -1;
        }
        int intro_dx = 0;
        if(_state == boss_anim::intro)
        {
            intro_dx = _timer * _timer / 60;
        }
        const int bob = (sine(frame * (enraged ? 6 : 4)) * (_id == 5 || _id == 6 ? 4 : 2)).round_integer();
        int lunge = 0;
        if(_state == boss_anim::attack)
        {
            lunge = _timer > 16 ? -(28 - _timer) : -(_timer * 12 / 16);
        }
        else if(_state == boss_anim::hit)
        {
            lunge = (_timer & 4) ? 3 : 1;
        }
        int fall = 0;
        if(_state == boss_anim::defeat && _timer < 100)
        {
            fall = (100 - _timer) * (100 - _timer) / 160;
        }

        const int base_x = center_x() + shake_x + intro_dx;
        const int base_y = center_y() + shake_y + bob;
        const bool blink = (frame % 170) < 6;

        for(int i = 0; i < _count; ++i)
        {
            part& p = _parts[i];
            if(! p.sprite)
            {
                continue;
            }
            if(finished)
            {
                p.sprite->set_visible(false);
                continue;
            }
            int x = base_x + p.def.x;
            int y = base_y + p.def.y + fall * (1 + (i % 3)) / 2;
            int f = 0;
            bool visible = true;
            const int idx = p.def.index;

            switch(p.def.role)
            {
            case part_role::body:
                x += lunge / 2;
                break;

            case part_role::eye:
                if(_id == 6)
                {
                    f = _phase >= 4 ? 3 : 0;
                    if(_state == boss_anim::telegraph || _state == boss_anim::hit)
                    {
                        f = 1;
                    }
                    if(_state == boss_anim::defeat || blink)
                    {
                        f = 2;
                    }
                }
                else
                {
                    f = enraged ? 3 : 0;
                    if(_state == boss_anim::telegraph || _state == boss_anim::attack)
                    {
                        f = 3;
                    }
                    if(_state == boss_anim::hit || _state == boss_anim::defeat || (blink && f == 0))
                    {
                        f = 2;
                    }
                    if(_state == boss_anim::idle && _phase == 1 && f == 0 && (frame & 64))
                    {
                        f = 1;
                    }
                }
                x += lunge / 2;
                break;

            case part_role::mouth:
                f = _state == boss_anim::attack || _state == boss_anim::defeat || _state == boss_anim::phase_change ? 2 :
                    (_state == boss_anim::telegraph ? 1 : 0);
                x += lunge / 2;
                break;

            case part_role::arm:
                if(idx == 0)
                {
                    f = _state == boss_anim::telegraph ? 1 : (_state == boss_anim::attack ? 2 : 0);
                    x += lunge;
                    y += (sine(frame * 3) * 2).round_integer();
                }
                else
                {
                    f = enraged && _state == boss_anim::telegraph ? 1 : 0;
                    y += (sine(frame * 3 + 180) * 2).round_integer();
                }
                break;

            case part_role::head:
                if(_id == 3)
                {
                    const bool stone = idx < (_phase < 3 ? _phase : 2);
                    f = stone ? 3 : 0;
                    if(! stone && (_state == boss_anim::telegraph || _state == boss_anim::attack) && idx == _attacking_head)
                    {
                        f = 1;
                    }
                    if(! stone && _state == boss_anim::hit)
                    {
                        f = 2;
                    }
                    x += (sine(frame * 3 + idx * 120) * 3).round_integer();
                    y += (cosine(frame * 2 + idx * 90) * 2).round_integer() + (stone ? 8 : 0);
                    if(idx == _attacking_head && _state == boss_anim::attack)
                    {
                        x += lunge;
                    }
                }
                else if(_id == 2)
                {
                    f = _phase >= 1 ? 2 : 0;
                    if(_state == boss_anim::telegraph || _state == boss_anim::attack)
                    {
                        f = 1;
                    }
                    if(_state == boss_anim::hit || _state == boss_anim::phase_change)
                    {
                        f = 3;
                    }
                    x += lunge / 2;
                }
                else
                {
                    f = enraged ? 2 : ((_state == boss_anim::telegraph || _state == boss_anim::attack) ? 1 : 0);
                    y += (sine(frame * 5) * 1).round_integer();
                }
                break;

            case part_role::crystal:
                f = _phase > 2 ? 2 : _phase;
                break;

            case part_role::breath:
                visible = _state == boss_anim::attack;
                f = (28 - _timer) / 6;
                f = f > 2 ? 2 : (f < 0 ? 0 : f);
                x += lunge / 2;
                break;

            case part_role::neck:
            {
                const int head = idx / 2;
                const int seg = idx % 2;
                const int hx = hydra_heads[head][0] + (sine(frame * 3 + head * 120) * 3).round_integer();
                const int hy = hydra_heads[head][1] + (cosine(frame * 2 + head * 90) * 2).round_integer();
                const int t = seg == 0 ? 35 : 68;
                x = base_x + (hx * t) / 100;
                y = base_y + 40 + ((hy - 40) * t) / 100 + (sine(frame * 4 + idx * 50) * 2).round_integer();
                break;
            }

            case part_role::fire:
            {
                visible = _state == boss_anim::attack;
                const int head = _attacking_head;
                x = base_x + hydra_heads[head][0] - 28 + lunge;
                y = base_y + hydra_heads[head][1] + 6;
                f = (28 - _timer) / 6;
                f = f > 2 ? 2 : (f < 0 ? 0 : f);
                break;
            }

            case part_role::face:
                f = enraged ? 2 : 0;
                if(_state == boss_anim::telegraph)
                {
                    f = 3;
                }
                else if(_state == boss_anim::attack || _state == boss_anim::recover)
                {
                    f = 1;
                }
                else if(_state == boss_anim::hit)
                {
                    f = 2;
                }
                break;

            case part_role::orb:
            {
                const int angle = frame * (enraged ? 4 : 2) + idx * 90;
                x = base_x + (cosine(angle) * 40).round_integer();
                y = base_y + (sine(angle) * 14).round_integer() - 4;
                f = ((frame >> 3) + idx) & 1;
                p.sprite->set_z_order(sine(angle) > 0 ? 0 : 3);
                break;
            }

            case part_role::bolt:
                visible = _state == boss_anim::attack && ((idx == 0) == ((_attack_kind & 1) == 0) || enraged);
                f = (frame >> 2) & 1;
                break;

            case part_role::ring:
            {
                const int speed = _id == 6 ? (2 + _phase) : (1 + _phase);
                p.sprite->set_rotation_angle((frame * speed) % 360);
                break;
            }

            case part_role::cube:
            {
                const int angle = frame * 3 + idx * 90;
                x = base_x + (cosine(angle) * 34).round_integer();
                y = base_y - 22 + (sine(angle) * 10).round_integer();
                f = ((frame >> 2) + idx) & 3;
                p.sprite->set_z_order(sine(angle) > 0 ? 0 : 3);
                break;
            }

            case part_role::wing:
                f = (frame / (enraged ? 6 : 10)) % 3;
                y += (sine(frame * 4 + idx * 180) * 3).round_integer();
                break;

            case part_role::iris:
            {
                visible = ! (blink || _state == boss_anim::defeat);
                const int look = 0;
                x += look;
                f = _state == boss_anim::telegraph ? 1 : 0;
                x += (sine(frame * 2) * 4).round_integer();
                y += (cosine(frame * 3) * 2).round_integer();
                break;
            }

            case part_role::tentacle:
                f = ((frame >> 3) + idx) & 3;
                break;

            case part_role::heart:
                y += (sine(frame * 8) * 1).round_integer();
                break;

            default:
                break;
            }

            if(f >= p.frames)
            {
                f = p.frames - 1;
            }
            p.sprite->set_visible(visible);
            p.sprite->set_position(x - 120, y - 80);
            if(f != p.shown)
            {
                p.shown = int8_t(f);
                p.sprite->set_tiles(p.def.item->tiles_item(), f);
            }
        }
    }
}
