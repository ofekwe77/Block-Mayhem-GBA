#include "bm_fx.h"

#include "bn_bg_palettes.h"
#include "bn_sprite_palettes.h"

#include "bn_sprite_items_spr_beam.h"
#include "bn_sprite_items_spr_explosion.h"
#include "bn_sprite_items_spr_particles.h"

namespace bm
{
    fx::fx(const bn::camera_ptr& camera) :
        _camera(camera)
    {
        for(popup_slot& p : _popups)
        {
            p.text.set_camera(camera);
        }
    }

    void fx::set_options(int shake_setting, int flash_setting)
    {
        _shake_setting = shake_setting;
        _flash_setting = flash_setting;
    }

    int fx::_rand(int n)
    {
        _seed = _seed * 1664525u + 1013904223u;
        return n > 0 ? int((_seed >> 16) % uint32_t(n)) : 0;
    }

    void fx::particle(particle_kind kind, int x, int y, bn::fixed vx, bn::fixed vy, int life, bn::fixed gravity)
    {
        particle_slot& p = _particles[_next_particle];
        _next_particle = (_next_particle + 1) % max_particles;
        p.x = x;
        p.y = y;
        p.vx = vx;
        p.vy = vy;
        p.gravity = gravity;
        p.life = int16_t(life);
        p.max_life = int16_t(life);
        p.first = uint8_t(kind);
        switch(kind)
        {
        case particle_kind::spark:
        case particle_kind::smoke:
        case particle_kind::ember:
        case particle_kind::debris:
            p.frames = 4;
            break;
        case particle_kind::snow:
        case particle_kind::crystal:
        case particle_kind::star:
        case particle_kind::electric:
        case particle_kind::dust:
            p.frames = 2;
            break;
        default:
            p.frames = 1;
            break;
        }
        if(! p.sprite)
        {
            p.sprite = bn::sprite_items::spr_particles.create_sprite(x - 120 + 4, y - 80 + 4, p.first);
            p.sprite->set_camera(_camera);
            p.sprite->set_bg_priority(0);
        }
        else
        {
            p.sprite->set_tiles(bn::sprite_items::spr_particles.tiles_item(), p.first);
            p.sprite->set_position(x - 120 + 4, y - 80 + 4);
            p.sprite->set_visible(true);
        }
    }

    void fx::burst(particle_kind kind, int x, int y, int count, int speed)
    {
        for(int i = 0; i < count; ++i)
        {
            const bn::fixed vx = bn::fixed(_rand(speed * 32) - speed * 16) / 16;
            const bn::fixed vy = bn::fixed(_rand(speed * 32) - speed * 20) / 16;
            particle(kind, x + _rand(5) - 2, y + _rand(5) - 2, vx, vy, 18 + _rand(14), bn::fixed(1) / 8);
        }
    }

    void fx::explosion(int x, int y)
    {
        for(explosion_slot& e : _explosions)
        {
            if(e.timer <= 0)
            {
                e.timer = 24;
                if(! e.sprite)
                {
                    e.sprite = bn::sprite_items::spr_explosion.create_sprite(x - 120, y - 80, 0);
                    e.sprite->set_camera(_camera);
                    e.sprite->set_bg_priority(0);
                }
                else
                {
                    e.sprite->set_position(x - 120, y - 80);
                    e.sprite->set_tiles(bn::sprite_items::spr_explosion.tiles_item(), 0);
                    e.sprite->set_visible(true);
                }
                return;
            }
        }
    }

    void fx::beam(int x, int y_top, int y_bottom, int frames)
    {
        int y = y_top;
        for(beam_slot& b : _beam)
        {
            if(y >= y_bottom)
            {
                if(b.sprite)
                {
                    b.sprite->set_visible(false);
                }
                b.timer = 0;
                continue;
            }
            b.timer = int16_t(frames);
            if(! b.sprite)
            {
                b.sprite = bn::sprite_items::spr_beam.create_sprite(x - 120, y - 80 + 16, 0);
                b.sprite->set_camera(_camera);
                b.sprite->set_bg_priority(0);
            }
            else
            {
                b.sprite->set_position(x - 120, y - 80 + 16);
                b.sprite->set_visible(true);
            }
            y += 32;
        }
    }

    void fx::popup(int x, int y, const bn::string_view& text, text_color color, int life)
    {
        const int half = text::small_width(text) / 2 + 2;
        x = x < half ? half : (x > 240 - half ? 240 - half : x);
        int best = 0;
        for(int i = 1; i < max_popups; ++i)
        {
            if(_popups[i].life < _popups[best].life)
            {
                best = i;
            }
        }
        popup_slot& p = _popups[best];
        p.life = int16_t(life);
        p.x = int16_t(x);
        p.y = int16_t(y);
        p.big = false;
        p.text.show(x, y, text, color, align::center);
        p.text.set_visible(true);
    }

    void fx::callout(int x, int y, const bn::string_view& text, big_color color, int life)
    {
        // Keep call-outs fully on screen.
        const int half = text::big_width(text) / 2 + 10;    // generous margin: glyph outlines extend past widths
        x = half >= 120 ? 120 : (x < half ? half : (x > 240 - half ? 240 - half : x));
        _callout.life = int16_t(life);
        _callout.x = int16_t(x);
        _callout.y = int16_t(y);
        _callout.big = true;
        _callout.text.show_big(x, y, text, color, align::center);
        _callout.text.set_visible(true);
    }

    void fx::shake(shake_profile profile)
    {
        if(! _shake_setting)
        {
            return;
        }
        static constexpr int amp[5] = { 1, 2, 4, 5, 7 };
        static constexpr int len[5] = { 6, 10, 14, 20, 40 };
        int a = amp[int(profile)];
        if(_shake_setting == 1)
        {
            a = (a + 1) / 2;
        }
        if(a >= _shake_amp || _shake_timer <= 0)
        {
            _shake_amp = a;
            _shake_timer = len[int(profile)];
        }
    }

    void fx::hitstop(int frames)
    {
        if(frames > _hitstop)
        {
            _hitstop = frames > 8 ? 8 : frames;
        }
    }

    void fx::flash(bn::color color, int percent, int frames)
    {
        if(! _flash_setting)
        {
            percent = percent > 25 ? 25 : percent;     // reduced flashing: never a bright full-screen flash
            frames = frames > 6 ? 6 : frames;
        }
        _flash_color = color;
        _flash_percent = percent;
        _flash_timer = frames;
        _flash_length = frames;
    }

    void fx::clear()
    {
        for(particle_slot& p : _particles)
        {
            p.life = 0;
            p.sprite.reset();
        }
        for(explosion_slot& e : _explosions)
        {
            e.timer = 0;
            e.sprite.reset();
        }
        for(popup_slot& p : _popups)
        {
            p.life = 0;
            p.text.hide();
        }
        _callout.life = 0;
        _callout.text.hide();
        for(beam_slot& b : _beam)
        {
            b.timer = 0;
            b.sprite.reset();
        }
        _camera.set_position(0, 0);
        bn::bg_palettes::set_fade_intensity(0);
        bn::sprite_palettes::set_fade_intensity(0);
    }

    void fx::update()
    {
        if(_hitstop > 0)
        {
            --_hitstop;
        }

        for(particle_slot& p : _particles)
        {
            if(p.life <= 0)
            {
                continue;
            }
            --p.life;
            if(p.life <= 0)
            {
                p.sprite->set_visible(false);
                continue;
            }
            p.vy += p.gravity;
            p.x += p.vx;
            p.y += p.vy;
            const int age = p.max_life - p.life;
            const int frame = p.frames > 1 ? (age * p.frames / (p.max_life ? p.max_life : 1)) : 0;
            p.sprite->set_position(p.x - 120 + 4, p.y - 80 + 4);
            p.sprite->set_tiles(bn::sprite_items::spr_particles.tiles_item(), p.first + (frame < p.frames ? frame : p.frames - 1));
            if(p.life < 6)
            {
                p.sprite->set_visible(p.life & 1);
            }
        }

        for(explosion_slot& e : _explosions)
        {
            if(e.timer <= 0)
            {
                continue;
            }
            --e.timer;
            const int frame = (24 - e.timer) / 4;
            if(e.timer <= 0 || frame >= 6)
            {
                e.sprite->set_visible(false);
                e.timer = 0;
                continue;
            }
            e.sprite->set_tiles(bn::sprite_items::spr_explosion.tiles_item(), frame);
        }

        for(beam_slot& b : _beam)
        {
            if(b.timer <= 0)
            {
                continue;
            }
            --b.timer;
            if(b.timer <= 0)
            {
                b.sprite->set_visible(false);
                continue;
            }
            b.sprite->set_tiles(bn::sprite_items::spr_beam.tiles_item(), (b.timer >> 1) & 1);
        }

        for(popup_slot& p : _popups)
        {
            if(p.life <= 0)
            {
                continue;
            }
            --p.life;
            if(p.life <= 0)
            {
                p.text.hide();
                continue;
            }
            if((p.life & 1) == 0)
            {
                --p.y;
                p.text.move(p.x, p.y);
            }
            if(p.life < 10)
            {
                p.text.set_visible(p.life & 2);
            }
        }
        if(_callout.life > 0)
        {
            --_callout.life;
            if(_callout.life <= 0)
            {
                _callout.text.hide();
            }
            else if(_callout.life < 12)
            {
                _callout.text.set_visible(_callout.life & 2);
            }
        }

        if(_shake_timer > 0)
        {
            --_shake_timer;
            const int a = _shake_amp * (_shake_timer + 4) / (_shake_timer + 8);
            _camera.set_position(_rand(a * 2 + 1) - a, _rand(a * 2 + 1) - a);
            if(! _shake_timer)
            {
                _camera.set_position(0, 0);     // always returns to the exact rest position
            }
        }

        if(_flash_timer > 0)
        {
            --_flash_timer;
            const bn::fixed intensity = bn::fixed(_flash_percent * _flash_timer) / (100 * (_flash_length ? _flash_length : 1));
            bn::bg_palettes::set_fade(_flash_color, intensity);
            bn::sprite_palettes::set_fade(_flash_color, intensity);
            if(! _flash_timer)
            {
                bn::bg_palettes::set_fade_intensity(0);
                bn::sprite_palettes::set_fade_intensity(0);
            }
        }
    }
}
