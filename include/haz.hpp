#ifndef __HAZ__
#define __HAZ__

#include "tonc.h"
#include "collision.hpp"
#include "terminal.hpp"

struct hazardAsset{
    const unsigned short* map;
    u8 width, height;
    u16 x, y;
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
            hitbox = new Pill((ha.x * 8) + 8, (ha.y * 8) + 8, ((ha.x + ha.width) * 8) - 8, ((ha.y + ha.height) * 8) - 8, 4);
            rendered = false;
            erased = false;
        };

        void update(int scrollX){
            //>> 4 for FIXED notation >> 3 for pixel-to-tile ratio
            bool shouldRender = rendered ? false : (scrollX>>7)+32 > asset.x;
            if(shouldRender) render();

            bool shouldErase = erased ? false : (scrollX>>7) > asset.x + (asset.width + 1);
            if(shouldErase) erase();
        }

        void render(){
            rendered = true;
            // tile between 0 - 63 for drawing to BG
            u8 bgx = asset.x & 0x3F;
            for(int i = 0; i < asset.height; i++){
                //use x to decide which sbb
                int rowStartInd = 32*(asset.y + i) + /*start x*/(asset.x & 0x1F);
                
                if(bgx < (31 - asset.width)){
                    memcpy16(&se_mem[18][rowStartInd], &asset.map[asset.width * i], asset.width);
                } else if(bgx < 31){
                    memcpy16(
                        &se_mem[18][rowStartInd], 
                        &asset.map[asset.width * i], 
                        32 - bgx
                    );

                    memcpy16(&se_mem[19][rowStartInd + (32 - bgx)], 
                        &asset.map[(asset.width * i) + (31 - bgx)], 
                        asset.width - (31 - bgx)
                    );
                } else if(bgx < (63 - asset.width)){ // fully in sbb 19
                    memcpy16(&se_mem[19][rowStartInd], &asset.map[asset.width * i], asset.width);
                } else {
                    memcpy16(
                        &se_mem[19][rowStartInd], 
                        &asset.map[asset.width * i], 
                        64 - bgx
                    );

                    memcpy16(&se_mem[18][rowStartInd + (64 - bgx)], 
                        &asset.map[(asset.width * i) + (63 - bgx)], 
                        asset.width - (63 - bgx)
                    );
                }
            }
        };

        void erase(){
            Terminal::log("erasing!");
            erased = true;
            // tile between 0 - 63 for drawing to BG
            u8 bgx = asset.x & 0x3F;
            for(int i = 0; i < asset.height; i++){
                //use x to decide which sbb
                int rowStartInd = 32*(asset.y + i) + /*start x*/(asset.x & 0x1F);
                
                if(bgx < (31 - asset.width)){
                    memset16(&se_mem[18][rowStartInd], 0, asset.width);
                } else if(bgx < 31){
                    memset16(
                        &se_mem[18][rowStartInd], 
                        0, 
                        31 - bgx
                    );

                    memset16(&se_mem[19][rowStartInd + (32 - bgx)], 
                        0, 
                        asset.width - (31 - bgx)
                    );

                } else if(bgx < (63 - asset.width)){ // fully in sbb 19
                    memset16(&se_mem[19][rowStartInd], 0, asset.width);
                } else {
                    memset16(
                        &se_mem[19][rowStartInd], 
                        0, 
                        64 - bgx
                    );

                    memset16(&se_mem[18][rowStartInd + (64 - bgx)], 
                        0, 
                        asset.width - (63 - bgx)
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