/*
 * BLOCK MAYHEM - secret visual events during play (PART 35.1). Presentation only: none of
 * them changes the rules, the timing or the input of the game.
 *
 *  - ROBOT: a tiny maintenance robot pops up in the corner and cheers after a huge chain or combo.
 *  - MALFUNCTION: a burst of explosions overloads the machine (flicker, sparks, smoke, then a reboot).
 *  - STORM: a rare lightning storm strikes the factory during a major chain reaction.
 *
 * The dizzy face of a defeated boss, the S+ fireworks and the title-screen chili rain live in
 * their scenes; all six share the secret bits below (save_data::secrets_found).
 */
#ifndef BM_SECRETS_H
#define BM_SECRETS_H

#include "bn_camera_ptr.h"
#include "bn_fixed.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"

namespace bm
{
    class backdrop;
    class fx;

    enum secret_bit : uint8_t
    {
        secret_robot = 1,
        secret_storm = 2,
        secret_malfunction = 4,
        secret_dizzy_boss = 8,
        secret_fireworks = 16,
        secret_chili = 32
    };

    class secret_events
    {
    public:
        static constexpr int robot_chain = 6;       // chain waves that call the robot
        static constexpr int robot_combo = 6;       // or consecutive clearing pieces
        static constexpr int storm_chain = 8;       // chain waves that start the storm
        static constexpr int blast_count = 6;       // explosions ...
        static constexpr int blast_window = 240;    // ... within this many frames overload the machine

        explicit secret_events(const bn::camera_ptr& camera);

        void reset();
        void on_chain(int waves);
        void on_combo(int combo);
        void on_explosion(int frame);

        // Advances the running events. Returns the secret bits of events that started this frame.
        int update(fx& effects, backdrop& scenery, int frame);

        [[nodiscard]] bool storm_active() const { return _storm_timer > 0; }
        [[nodiscard]] bool malfunction_active() const { return _malfunction_timer > 0; }
        [[nodiscard]] bool robot_active() const { return _robot_timer > 0; }

    private:
        bn::camera_ptr _camera;

        bn::optional<bn::sprite_ptr> _robot;
        int _robot_timer = 0;
        int _robot_cooldown = 0;
        bool _robot_pending = false;

        bn::optional<bn::sprite_ptr> _bolt[2];
        int _storm_timer = 0;
        int _storm_cooldown = 0;
        bool _storm_pending = false;
        int _bolt_x = 0;
        int _bolt_life = 0;

        int _blasts[blast_count] = {};
        int _blast_head = 0;
        int _blast_total = 0;
        int _malfunction_timer = 0;
        int _malfunction_cooldown = 0;
        bool _malfunction_pending = false;

        bn::fixed _dim;                 // backdrop dimming requested by the storm / malfunction
        bn::fixed _malfunction_dim;
        uint32_t _noise = 0x2545F491u;

        [[nodiscard]] int _rand(int limit);
        void _update_robot();
        void _update_storm(fx& effects, backdrop& scenery);
        void _update_malfunction(fx& effects, backdrop& scenery);
        void _strike(fx& effects, backdrop& scenery);
    };
}

#endif
