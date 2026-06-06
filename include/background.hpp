#ifndef __BACKGROUND__
#define __BACKGROUND__

#include "tonc.h"

#include "tileset.h"

struct backgroundEntry{
    u8 duration; // if there is a specific duration for a background, else 0
    const SCREENBLOCK* map;
};

extern backgroundEntry entries[];

class BackgroundManager{
    public:
        BackgroundManager(u16 sb, u16 cb){
            current_screenblock = -1;
            sbb = sb;
            cbb = cb;
            be = entries[2];
        };

        void init(){
            LZ77UnCompVram(tilesetPal, pal_bg_mem);
            LZ77UnCompVram(tilesetTiles, tile_mem[cbb]);
            //memcpy16(&se_mem[sbb], zone1Map, zone1MapLen/2);
        };

        void update(int scrollX){ //>>4 to get pixels >>3 more to get tiles >>5 to get whole screen Blocks
            int playerSB = (scrollX - (16<<4))>>12;
            
            if(playerSB != current_screenblock){ //transition time
                
                memcpy16(se_mem[sbb + 1 - (playerSB & 1)], be.map[current_background_duration], sizeof(SCREENBLOCK)/2);
                current_background_duration++;
                if(current_background_duration == be.duration){
                    be = entries[playerSB & 1];
                    current_background_duration = 0;
                }
                current_screenblock = playerSB;
            }

        };

    private:
        int current_screenblock;
        u16 sbb, cbb;

        backgroundEntry be;
        u16 current_background_duration; //how long we've been on this background
};

#endif