#ifndef __HAZ__
#define __HAZ__

#include "tonc.h"
#include "collision.hpp"
#include "terminal.hpp"
#include "playerCharacter.hpp"
#include "ii.hpp"

enum HAZARD_STATUS{PERSIST, DELETE};

//virtual class
class Hazard {
    public:
        virtual ~Hazard();
        u8 checkCollision(const Rectangle playerBounds);
        virtual HAZARD_STATUS update(int scrollx, Rectangle playerbounds) = 0;

    protected:
        virtual void render() = 0;
        virtual void erase() = 0;

        CollisionShape* hitbox;
        u8 hazardIndex;
        bool rendered, erased;
};

//manager
class HazardManager{
    public:
        HazardManager();
        void update(int scrollx, PlayerCharacter* player);
        void createTest();

    private:
        int hazardsCt;
        II<8>* ii;
        Hazard* hazards[8];
};

struct yellowLaserAsset{
    u16* map;
    u16 width, height;
    u16 xOffset, yOffset;
};

//inherited classes
class YellowLaser : public Hazard {
    public:
        YellowLaser(int x1, int y1, yellowLaserAsset yla);
        HAZARD_STATUS update(int scrollX, Rectangle playerbounds);
        void render();
        void erase();
    private:
        u16* map;
        u16 tileWidth, tileHeight;
        u16 topTile, leftTile;
};

class OrangeLaser : public Hazard {
    public:
        //probably doesnt need ha at all.
        OrangeLaser(int x, int y, int diameter, const unsigned int* assetMap);
        HAZARD_STATUS update(int scrollX, Rectangle playerbounds);
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
        HAZARD_STATUS update(int scrollX, Rectangle playerbounds);
        void render();
        void erase();

    private:
        int fixedX;
        u16 tileInd;
        u16 yInd; // 0 - 6 for laser index
};

class Missile : public Hazard{
    public:
        //probably doesnt need ha at all.
        Missile(int x, int y, const unsigned int* assetMap);
        HAZARD_STATUS update(int scrollX, Rectangle playerbounds);
        void render();
        void erase();

    private:
        OBJ_ATTR* obj;
        int fixedX;
        int fixedY;
        int velocity;
};

#endif