#ifndef __HAZ__
#define __HAZ__

#include "tonc.h"
#include "collision.hpp"
#include "terminal.hpp"

struct hazardAsset{
    const unsigned short* map;
    u8 tileWidth, tileHeight;
    u16 tileX, tileY;
};

extern hazardAsset diag;

//virtual class
class Hazard {
    public:
        virtual ~Hazard(){
            delete hitbox;
        };

        bool checkCollision(const Rectangle playerBounds){
            return hitbox->collidesWithRectangle(playerBounds);
        };

        virtual void update(int scrollx) = 0;

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
        YellowLaser(hazardAsset ha){
            asset = ha;
            hitbox = new Pill((ha.tileX * 8) + 8, (ha.tileY * 8) + 8, ((ha.tileX + ha.tileWidth) * 8) - 8, ((ha.tileY + ha.tileHeight) * 8) - 8, 4);
            rendered = false;
        };

        void update(int scrollX){
            //>> 4 for FIXED notation >> 3 for pixel-to-tile ratio
            bool shouldRender = rendered ? false : (scrollX>>7)+32 > asset.tileX;
            if(shouldRender) render();

            bool shouldErase = erased ? false : (scrollX>>7) > asset.tileX + (asset.tileWidth + 1);
            if(shouldErase) erase();
        }

        void render(){
            rendered = true;
            // tile between 0 - 63 for drawing to BG
            u8 bgTileX = asset.tileX & 0x3F;
            for(int i = 0; i < asset.tileHeight; i++){
                //use tileX to decide which sbb
                int rowStartInd = 32*(asset.tileY + i) + /*start x*/(asset.tileX & 0x1F);
                
                if(bgTileX < (31 - asset.tileWidth)){
                    memcpy16(&se_mem[18][rowStartInd], &asset.map[asset.tileWidth * i], asset.tileWidth);
                } else if(bgTileX < 31){
                    memcpy16(
                        &se_mem[18][rowStartInd], 
                        &asset.map[asset.tileWidth * i], 
                        32 - bgTileX
                    );

                    memcpy16(&se_mem[19][rowStartInd + (32 - bgTileX)], 
                        &asset.map[(asset.tileWidth * i) + (31 - bgTileX)], 
                        asset.tileWidth - (31 - bgTileX)
                    );
                } else if(bgTileX < (63 - asset.tileWidth)){ // fully in sbb 19
                    memcpy16(&se_mem[19][rowStartInd], &asset.map[asset.tileWidth * i], asset.tileWidth);
                } else {
                    memcpy16(
                        &se_mem[19][rowStartInd], 
                        &asset.map[asset.tileWidth * i], 
                        64 - bgTileX
                    );

                    memcpy16(&se_mem[18][rowStartInd + (64 - bgTileX)], 
                        &asset.map[(asset.tileWidth * i) + (63 - bgTileX)], 
                        asset.tileWidth - (63 - bgTileX)
                    );
                }
            }
        };

        void erase(){
            Terminal::log("erasing!");
            erased = true;
            // tile between 0 - 63 for drawing to BG
            u8 bgTileX = asset.tileX & 0x3F;
            for(int i = 0; i < asset.tileHeight; i++){
                //use tileX to decide which sbb
                int rowStartInd = 32*(asset.tileY + i) + /*start x*/(asset.tileX & 0x1F);
                
                if(bgTileX < (31 - asset.tileWidth)){
                    memset16(&se_mem[18][rowStartInd], 0, asset.tileWidth);
                } else if(bgTileX < 31){
                    memset16(
                        &se_mem[18][rowStartInd], 
                        0, 
                        31 - bgTileX
                    );

                    memset16(&se_mem[19][rowStartInd + (32 - bgTileX)], 
                        0, 
                        asset.tileWidth - (31 - bgTileX)
                    );

                } else if(bgTileX < (63 - asset.tileWidth)){ // fully in sbb 19
                    memset16(&se_mem[19][rowStartInd], 0, asset.tileWidth);
                } else {
                    memset16(
                        &se_mem[19][rowStartInd], 
                        0, 
                        64 - bgTileX
                    );

                    memset16(&se_mem[18][rowStartInd + (64 - bgTileX)], 
                        0, 
                        asset.tileWidth - (63 - bgTileX)
                    );
                }
            }
        };
        
};

//manager
class HazardManager{
    public:
        HazardManager(){};

        void update(int scrollx, Rectangle playerbounds){
            for(int i = 0; i < hazardsCt; i++){
                hazards[i]->update(scrollx);
            }

            for(int i = 0; i < hazardsCt; i++){
                bool hit = hazards[i]->checkCollision(playerbounds);
                if(hit) Terminal::log("hit something!");
            }
        };

        void createDiag(){
            hazards[hazardsCt++] = new YellowLaser(diag);
        }

    private:
        int hazardsCt;
        Hazard* hazards[8];

};

class OrangeLaser{
    public:

    private:
        Pill hitbox;
};

class RedLaser{
    public:

    private:
};

class Missile{
    public:

    private:
};

#endif