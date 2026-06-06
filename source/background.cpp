#include "background.hpp"

#include "zone1.h"
#include "zone2.h"
#include "zone3.h"
#include "startZone.h"

backgroundEntry entries[] = {
    { 2, (SCREENBLOCK*)startZoneMap },
    { 1, (SCREENBLOCK*)zone1Map },
    { 1, (SCREENBLOCK*)zone2Map },
    { 3, (SCREENBLOCK*)zone3Map }
};

BackgroundManager::BackgroundManager(u16 sb, u16 cb){
    current_screenblock = -1;
    sbb = sb;
    cbb = cb;
    be = entries[0];
    current_background_duration = 1;
    totalBECt = sizeof(entries) / sizeof(backgroundEntry);
};

void BackgroundManager::init(){
    LZ77UnCompVram(tilesetPal, pal_bg_mem);
    LZ77UnCompVram(tilesetTiles, tile_mem[cbb]);
    memcpy16(&se_mem[sbb], be.map[0], sizeof(SCREENBLOCK)/2);
};

void BackgroundManager::selectNewBE(){
    // totalBECt, except 0
    int random = (qran() % (totalBECt-1))+1;
    be = entries[random];
    current_background_duration = 0;
}

void BackgroundManager::update(int scrollX){ 
    //find the sb 16 pixels behind the player a.k.a the one the player is in
    int playerSB = (scrollX - (16<<4))>>12; //>>4 to get pixels >>3 more to get tiles >>5 to get whole screen Blocks
    
    if(playerSB != current_screenblock){ //transition time

        if(current_background_duration == be.duration){
            selectNewBE();
        }
        // sbb + 1 - (playerSB & 1)
        // if you are in 0, update 1, if in 1 update 0
        memcpy16(se_mem[sbb + 1 - (playerSB & 1)], be.map[current_background_duration], sizeof(SCREENBLOCK)/2);
        //Time to Select a new background
        current_background_duration++;
        current_screenblock = playerSB;
    }

}
