#ifndef __HAZ__
#define __HAZ__

#include "tonc.h"
#include "collision.hpp"
#include "terminal.hpp"


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
        bool rendered, erased;
};

//inherited classes
class YellowLaser : public Hazard {
    public:
        YellowLaser(int x1, int y1, int x2, int y2, u16* assetMap);
        void update(int scrollX, Rectangle playerbounds);
        void render();
        void erase();
    private:
        u16* map;
        s16 tileWidth, tileHeight;
        u16 topTile, leftTile;
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

class OrangeLaser : public Hazard {
    public:
        //probably doesnt need ha at all.
        OrangeLaser(int x, int y, int diameter, const unsigned int* assetMap);
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
        RedLaser(int x, int y, u16 tileInd);
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
        Missile(int x, int y, const unsigned int* assetMap);
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