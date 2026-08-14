#ifndef PLAYER_XP_XP_CONFIG_H
#define PLAYER_XP_XP_CONFIG_H
#define XP_CURVE_KNEE_LO        16   // below this, cheap-ish flat-ish ramp
#define XP_CURVE_KNEE_HI        31   // above this, steeper ramp
#define XP_CAP_LEVEL            900  // hard ceiling so the int math stays sane
#define XP_ORB_MAX              512   // pool size. plenty for a few mobs popping
#define XP_ORB_PICKUP_RANGE     1.20f // feet-radius at which an orb is absorbed
#define XP_ORB_MAGNET_RANGE     5.00f // orbs start drifting toward you here
#define XP_ORB_MAGNET_ACCEL     34.0f // pull strength once magnetized
#define XP_ORB_MAX_SPEED        12.0f // clamp so they dont slingshot past you
#define XP_ORB_GRAVITY          -16.0f
#define XP_ORB_DRAG             1.8f  // air drag per second (linear-ish)
#define XP_ORB_BOUNCE           0.30f // floor restitution, they're squishy
#define XP_ORB_LIFETIME         300.0f// seconds before an orb gives up
