#include "bm_secrets.h"

#include "bm_audio.h"
#include "bm_backdrop.h"
#include "bm_fx.h"

#include "bn_color.h"
#include "bn_sprite_items_spr_bolt.h"
#include "bn_sprite_items_spr_robot.h"

namespace bm
{
    namespace
    {
        constexpr int robot_frames = 146;
        constexpr int storm_frames = 96;
        constexpr int malfunction_frames = 100;
        constexpr int robot_rest_frames = 60 * 40;      // cooldowns keep the surprises rare
        constexpr int storm_rest_frames = 60 * 30;
        constexpr int malfunction_rest_frames = 60 * 25;
        constexpr int strike_at[4] = { 0, 22, 46, 70 };
        constexpr int robot_x = 212;
        constexpr int robot_y = 146;
    }

    secret_events::secret_events(const bn::camera_ptr& camera) :
        _camera(camera)
    {
    }

    int secret_events::_rand(int limit)
    {
        _noise = _noise * 1664525u + 1013904223u;
        return int((_noise >> 16) % uint32_t(limit));
    }

    void secret_events::reset()
    {
        _robot.reset();
        _robot_timer = 0;
        _robot_cooldown = 0;
        _robot_pending = false;
        _bolt[0].reset();
        _bolt[1].reset();
        _storm_timer = 0;
        _storm_cooldown = 0;
        _storm_pending = false;
        _bolt_life = 0;
        _blast_head = 0;
        _blast_total = 0;
        _malfunction_timer = 0;
        _malfunction_cooldown = 0;
        _malfunction_pending = false;
        _malfunction_dim = 0;
    }

    void secret_events::on_chain(int waves)
    {
        if(waves >= storm_chain && ! _storm_timer && ! _storm_cooldown)
        {
            _storm_pending = true;
        }
        if(waves >= robot_chain && ! _robot_timer && ! _robot_cooldown)
        {
            _robot_pending = true;
        }
    }

    void secret_events::on_combo(int combo)
    {
        // The combo event counts from 0 (the first follow-up clear shows "COMBO x2").
        if(combo + 1 >= robot_combo && ! _robot_timer && ! _robot_cooldown)
        {
            _robot_pending = true;
        }
    }

    void secret_events::on_explosion(int frame)
    {
        _blasts[_blast_head] = frame;
        _blast_head = (_blast_head + 1) % blast_count;
        ++_blast_total;
        // _blasts[_blast_head] is now the oldest of the last blast_count explosions.
        if(_blast_total >= blast_count && frame - _blasts[_blast_head] <= blast_window && ! _malfunction_timer &&
           ! _malfunction_cooldown)
        {
            _malfunction_pending = true;
        }
    }

    int secret_events::update(fx& effects, backdrop& scenery, int frame)
    {
        (void) frame;
        int started = 0;
        _robot_cooldown -= _robot_cooldown > 0 ? 1 : 0;
        _storm_cooldown -= _storm_cooldown > 0 ? 1 : 0;
        _malfunction_cooldown -= _malfunction_cooldown > 0 ? 1 : 0;

        if(_robot_pending)
        {
            _robot_pending = false;
            _robot_timer = robot_frames;
            _robot = bn::sprite_items::spr_robot.create_sprite(252 - 120, robot_y - 80, 1);
            _robot->set_camera(_camera);
            _robot->set_bg_priority(0);
            audio::sfx(audio::sfx_id::unlock);
            started |= secret_robot;
        }
        if(_storm_pending)
        {
            _storm_pending = false;
            _storm_timer = storm_frames;
            started |= secret_storm;
        }
        if(_malfunction_pending)
        {
            _malfunction_pending = false;
            _malfunction_timer = malfunction_frames;
            effects.shake(shake_profile::medium);
            audio::sfx(audio::sfx_id::danger);
            started |= secret_malfunction;
        }

        _update_robot();
        _update_storm(effects, scenery);
        _update_malfunction(effects, scenery);

        // Both the storm and the malfunction darken the scenery: the stronger request wins.
        bn::fixed dim = 0;
        if(_storm_timer > 0)
        {
            dim = bn::fixed(0.35);
        }
        if(_malfunction_timer > 0 && _malfunction_timer > 20)
        {
            const int t = malfunction_frames - _malfunction_timer;
            if(t % 3 == 0)
            {
                _malfunction_dim = _rand(3) == 0 ? bn::fixed(0.6) : (_rand(2) ? bn::fixed(0.25) : bn::fixed(0));
            }
            dim = _malfunction_dim > dim ? _malfunction_dim : dim;
        }
        if(dim != _dim)
        {
            _dim = dim;
            scenery.set_dim(dim);
        }
        return started;
    }

    void secret_events::_update_robot()
    {
        if(_robot_timer <= 0 || ! _robot)
        {
            return;
        }
        const int t = robot_frames - _robot_timer;
        int x = robot_x;
        int y = robot_y;
        int f = 0;
        if(t < 30)
        {
            x = 252 - t * (252 - robot_x) / 30;     // walks in along the conveyor
            f = 1 + ((t / 6) & 1);
        }
        else if(t < 90)
        {
            f = ((t - 30) / 8) & 1 ? 4 : 3;         // waves
        }
        else if(t < 106)
        {
            f = ((t - 90) / 4) & 1 ? 5 : 0;         // cheers
        }
        else
        {
            x = robot_x + (t - 106);                // walks off to the right
            f = 1 + ((t / 6) & 1);
        }
        _robot->set_position(x - 120, y - 80);
        _robot->set_tiles(bn::sprite_items::spr_robot.tiles_item(), f);
        if(--_robot_timer == 0)
        {
            _robot.reset();
            _robot_cooldown = robot_rest_frames;
        }
    }

    void secret_events::_strike(fx& effects, backdrop& scenery)
    {
        _bolt_x = 24 + _rand(192);
        _bolt_life = 9;
        const bool flip = _rand(2);
        for(int i = 0; i < 2; ++i)
        {
            if(! _bolt[i])
            {
                _bolt[i] = bn::sprite_items::spr_bolt.create_sprite(0, 0, i);
                _bolt[i]->set_camera(_camera);
                _bolt[i]->set_bg_priority(0);
            }
            _bolt[i]->set_tiles(bn::sprite_items::spr_bolt.tiles_item(), (i + _rand(2)) & 1);
            _bolt[i]->set_horizontal_flip(flip);
            _bolt[i]->set_position(_bolt_x - 120, 32 + i * 64 - 80);
            _bolt[i]->set_visible(true);
        }
        effects.flash(bn::color(28, 28, 31), 40, 6);
        effects.burst(particle_kind::electric, _bolt_x, 126, 5, 2);
        scenery.excite(30, 3);
        audio::sfx(_storm_timer == storm_frames ? audio::sfx_id::nuke : audio::sfx_id::spark);
    }

    void secret_events::_update_storm(fx& effects, backdrop& scenery)
    {
        if(_storm_timer <= 0)
        {
            return;
        }
        const int t = storm_frames - _storm_timer;
        for(int s : strike_at)
        {
            if(t == s)
            {
                _strike(effects, scenery);
            }
        }
        if(_bolt_life > 0)
        {
            --_bolt_life;
            const bool visible = _bolt_life > 0 && (_bolt_life % 3) != 1;      // a short flicker
            for(bn::optional<bn::sprite_ptr>& b : _bolt)
            {
                if(b)
                {
                    b->set_visible(visible);
                }
            }
        }
        if(--_storm_timer == 0)
        {
            _bolt[0].reset();
            _bolt[1].reset();
            _storm_cooldown = storm_rest_frames;
        }
    }

    void secret_events::_update_malfunction(fx& effects, backdrop& scenery)
    {
        (void) scenery;
        if(_malfunction_timer <= 0)
        {
            return;
        }
        const int t = malfunction_frames - _malfunction_timer;
        if(_malfunction_timer > 20)
        {
            if(t % 8 == 0)
            {
                // Sparks leap from the machine's edges.
                const int x = _rand(2) ? 6 : 234;
                effects.burst(particle_kind::electric, x, 20 + _rand(120), 3, 2);
            }
            if(t % 16 == 4)
            {
                audio::sfx(audio::sfx_id::spark);
            }
            if(t % 10 == 5)
            {
                effects.particle(particle_kind::smoke, 20 + _rand(200), 150, 0, -bn::fixed(3) / 4, 40);
            }
        }
        else if(_malfunction_timer == 20)
        {
            // Reboot: one clean flash and the power-on chime.
            effects.flash(bn::color(31, 31, 31), 25, 6);
            audio::sfx(audio::sfx_id::menu_ok);
        }
        if(--_malfunction_timer == 0)
        {
            _malfunction_cooldown = malfunction_rest_frames;
        }
    }
}
