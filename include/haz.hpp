#ifndef __HAZ__
#define __HAZ__

#include "tonc.h"
#include "collision.hpp"
#include "terminal.hpp"


/// @brief hazardAsset
/// idk, allows me to hardcode some values into digestible bits
///
/// @note the units for width, height, x, and y are non-specific, they could be FIXED, pixels, tiles, anything. be careful using them properly
/// @param map const unsigned short*
/// @param width u8
/// @param height u8
/// @param x u16
/// @param y u16
struct hazardAsset{
    const unsigned short* map;
    u8 width, height;
    u16 x, y;
};

extern hazardAsset diag;

//virtual class
class Hazard {
    public:
        virtual ~Hazard();
        bool checkCollision(const Rectangle playerBounds);
        virtual void update(int scrollx, Rectangle playerbounds) = 0;

    protected:
        virtual void render() = 0;
        virtual void erase() = 0;

        CollisionShape* hitbox;
        hazardAsset asset;
        bool rendered, erased;
};

//inherited classes
class YellowLaser : public Hazard {
    public:
        YellowLaser(hazardAsset ha);
        void update(int scrollX, Rectangle playerbounds);
        void render();
        void erase();
};

//manager
class HazardManager{
    public:
        HazardManager();
        void update(int scrollx, Rectangle playerbounds);
        void createTest();

    private:
        int hazardsCt;
        Hazard* hazards[8];
};

class OrangeLaser : public Hazard{
    public:
        //probably doesnt need ha at all.
        OrangeLaser(hazardAsset ha);
        void update(int scrollX, Rectangle playerbounds);
        void render();
        void erase();

    private:
        OBJ_ATTR* obj;
        OBJ_AFFINE* aff;
        int fixedX;
        u16 y;
        u16 alpha;
};

class RedLaser : public Hazard{
    public:
        RedLaser(hazardAsset ha);
        void update(int scrollX, Rectangle playerbounds);
        void render();
        void erase();

    private:
        int fixedX;
        u16 tileInd;
        u16 y; // 0 - 6 for laser index
};

class Missile : public Hazard{
    public:
        //probably doesnt need ha at all.
        Missile(hazardAsset ha);
        void update(int scrollX, Rectangle playerbounds);
        void render();
        void erase();

    private:
        OBJ_ATTR* obj;
        int fixedX;
        u16 y;
        int velocity;
};

#endif