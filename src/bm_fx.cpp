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
        // Pre-create the pooled sprites (hidden) so a chain reaction only moves and reveals them.
        for(particle_slot& p : _particles)
        {
            p.sprite = bn::sprite_items::spr_particles.create_sprite(0, 0, 0);
            p.sprite->set_camera(_camera);
            p.sprite->set_bg_priority(0);
            p.sprite->set_visible(false);
            p.shown = 0;
        }
        for(explosion_slot& e : _explosions)
        {
            e.sprite = bn::sprite_items::spr_explosion.create_sprite(0, 0, 0);
            e.sprite->set_camera(_camera);
            e.sprite->set_bg_priority(0);
            e.sprite->set_visible(false);
            e.shown = 0;
        }
        // Glyphs of the chain call-outs and score pop-ups exist before the first chain (creating them
        // mid-chain was the most expensive moment of a big reaction).
        glyph_label::warm("ABCDEFGHIKLMNOPRSTUVWXYZ!", true);
        glyph_label::warm("+0123456789COMBx", false);
        _callout.text.prepare(15, true);
        for(popup_slot& p : _popups)
        {
            p.text.prepare(7, false);
        }
    }

    fx::~fx()
    {
        glyph_label::release_cache();
    }

    void fx::set_options(int shake_setting, int flash_setting)
    {
        _shake_setting = shake_setting;
        _flash_setting = flash_setting;
    }

    int fx::_rand(int n)
    {
        _seed = _seed * 1664525u + 1013904223u;
        // Multiply-shift instead of a modulo: no division on the GBA's divider-less CPU.
        return n > 0 ? int(((_seed >> 16) * uint32_t(n)) >> 16) : 0;
    }

    const bn::sprite_tiles_ptr& fx::_particle_frame(int index)
    {
        index = index < 0 ? 0 : (index > 31 ? 31 : index);
        if(! _particle_tiles[index])
        {
            _particle_tiles[index] = bn::sprite_items::spr_particles.tiles_item().create_tiles(index);
        }
        return *_particle_tiles[index];
    }

    const bn::sprite_tiles_ptr& fx::_explosion_frame(int index)
    {
        index = index < 0 ? 0 : (index > 5 ? 5 : index);
        if(! _explosion_tiles[index])
        {
            _explosion_tiles[index] = bn::sprite_items::spr_explosion.tiles_item().create_tiles(index);
        }
        return *_explosion_tiles[index];
    }

    void fx::particle(particle_kind kind, int x, int y, bn::fixed vx, bn::fixed vy, int life, bn::fixed gravity)
    {
        if(_particle_budget <= 0)
        {
            return;     // this frame already spawned its share; the effect still reads as dense
        }
        --_particle_budget;
        if(_particle_style > 0 && kind >= particle_kind::shard_white && kind <= particle_kind::shard_violet)
        {
            static constexpr particle_kind styles[5] = {
                particle_kind::shard_white, particle_kind::star, particle_kind::snow, particle_kind::ember,
                particle_kind::heart
            };
            kind = styles[_particle_style < 5 ? _particle_style : 0];
        }
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
            p.shown = int8_t(p.first);
        }
        else
        {
            if(p.shown != p.first)
            {
                p.sprite->set_tiles(_particle_frame(p.first));
                p.shown = int8_t(p.first);
            }
            p.sprite->set_position(x - 120 + 4, y - 80 + 4);
            p.sprite->set_visible(true);
        }
    }

    void fx::burst(particle_kind kind, int x, int y, int count, int speed)
    {
        for(int i = 0; i < count && _particle_budget > 0; ++i)
        {
            const bn::fixed vx = bn::fixed(_rand(speed * 32) - speed * 16) / 16;
            const bn::fixed vy = bn::fixed(_rand(speed * 32) - speed * 20) / 16;
            particle(kind, x + _rand(5) - 2, y + _rand(5) - 2, vx, vy, 18 + _rand(14), bn::fixed(1) / 8);
        }
    }

    void fx::explosion(int x, int y)
    {
        if(_explosion_budget <= 0)
        {
            return;
        }
        --_explosion_budget;
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
                    if(e.shown != 0)
                    {
                        e.sprite->set_tiles(_explosion_frame(0));
                    }
                    e.sprite->set_visible(true);
                }
                e.shown = 0;
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
        if(_popup_cooldown > 0)
        {
            // Coalesce: only the latest pop-up of a burst is shown once the cooldown ends.
            _deferred_popup.text = text.size() > 24 ? text.substr(0, 24) : text;
            _deferred_popup.x = int16_t(x);
            _deferred_popup.y = int16_t(y);
            _deferred_popup.life = int16_t(life);
            _deferred_popup.color = uint8_t(color);
            _deferred_popup.pending = true;
            return;
        }
        _show_popup(x, y, text, color, life);
    }

    void fx::_show_popup(int x, int y, const bn::string_view& text, text_color color, int life)
    {
        _popup_cooldown = popup_cooldown_frames;
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
        if(_callout_cooldown > 0)
        {
            // A chain announces every wave: show the newest tier once the previous call-out had a moment.
            _deferred_callout.text = text.size() > 24 ? text.substr(0, 24) : text;
            _deferred_callout.x = int16_t(x);
            _deferred_callout.y = int16_t(y);
            _deferred_callout.life = int16_t(life);
            _deferred_callout.color = uint8_t(color);
            _deferred_callout.pending = true;
            return;
        }
        _show_callout(x, y, text, color, life);
    }

    void fx::_show_callout(int x, int y, const bn::string_view& text, big_color color, int life)
    {
        _callout_cooldown = callout_cooldown_frames;
        // Keep call-outs fully on screen.
        const int half = text::big_width(text) / 2 + 10;    // generous margin: glyph outlines extend past widths
        x = half >= 120 ? 120 : (x < half ? half : (x > 240 - half ? 240 - half : x));
        _callout.life = int16_t(life);
        _callout.x = int16_t(x);
        _callout.y = int16_t(y);
        _callout.big = true;
        _callout.text.show_big(x, y, text, color, align::center);
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

    void fx::_callout_hide()
    {
        _callout.text.hide();
    }

    void fx::_callout_visible(bool visible)
    {
        _callout.text.set_visible(visible);
    }

    void fx::set_base_tint(bn::color color, bn::fixed intensity)
    {
        _base_color = color;
        _base_intensity = intensity;
    }

    void fx::clear()
    {
        for(particle_slot& p : _particles)
        {
            p.life = 0;
            if(p.sprite)
            {
                p.sprite->set_visible(false);   // pooled sprites stay allocated
            }
        }
        for(explosion_slot& e : _explosions)
        {
            e.timer = 0;
            if(e.sprite)
            {
                e.sprite->set_visible(false);
            }
        }
        for(popup_slot& p : _popups)
        {
            p.life = 0;
            p.text.hide();
        }
        _callout.life = 0;
        _callout_hide();
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
        _particle_budget = particles_per_frame;
        _explosion_budget = explosions_per_frame;
        if(_popup_cooldown > 0 && --_popup_cooldown == 0 && _deferred_popup.pending)
        {
            _deferred_popup.pending = false;
            _show_popup(_deferred_popup.x, _deferred_popup.y, _deferred_popup.text, text_color(_deferred_popup.color),
                        _deferred_popup.life);
        }
        if(_callout_cooldown > 0 && --_callout_cooldown == 0 && _deferred_callout.pending)
        {
            _deferred_callout.pending = false;
            _show_callout(_deferred_callout.x, _deferred_callout.y, _deferred_callout.text,
                          big_color(_deferred_callout.color), _deferred_callout.life);
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
            const int wanted = p.first + (frame < p.frames ? frame : p.frames - 1);
            if(wanted != p.shown)
            {
                p.sprite->set_tiles(_particle_frame(wanted));
                p.shown = int8_t(wanted);
            }
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
            if(frame != e.shown)
            {
                e.sprite->set_tiles(_explosion_frame(frame));
                e.shown = int8_t(frame);
            }
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
                _callout_hide();
            }
            else if(_callout.life < 12)
            {
                _callout_visible(_callout.life & 2);
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

        // Screen flashes tint the background palettes only, in sixteenth steps: the engine re-processes every
        // palette whenever the fade changes, so sprites stay untouched and unchanged levels are not re-sent.
        if(_flash_timer > 0)
        {
            --_flash_timer;
            const int level = _flash_percent * _flash_timer * 16 / (100 * (_flash_length ? _flash_length : 1));
            if(level > 0)
            {
                if(level + 1000 != _applied_fade)
                {
                    _applied_fade = level + 1000;
                    bn::bg_palettes::set_fade(_flash_color, bn::fixed(level) / 16);
                }
            }
            else
            {
                _flash_timer = 0;
            }
        }
        if(_flash_timer <= 0)
        {
            const int base = (_base_intensity * 16).round_integer();
            if(base != _applied_fade)
            {
                _applied_fade = base;
                bn::bg_palettes::set_fade(_base_color, bn::fixed(base) / 16);
            }
        }
    }
}
