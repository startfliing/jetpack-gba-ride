#include "tonc.h"
#include "maxmod.h"

#include "terminal.hpp"

#include "basicV2.h"
#include "player.h"
#include "soundbank.h"
#include "soundbank_bin.h"

#define MAX_SY 108
#define MIN_SY 4

#define MAX_Y 0xD00
#define MIN_Y 0

struct playerCharacter{
    //true position
    int x, y;

    //screen position
    int sx, sy;

    //velocity and acceleration
    FIXED vy, ay;
};

playerCharacter playerChar = {
    0, MIN_Y,
    16, MAX_SY,
    0, 0
};

FIXED current_speed = 0;

void updateChar(){
    // acceleration
    playerChar.ay = key_is_down(KEY_A) ? 3 : -3;

    // velocity, if @ top or bot, velocity = 0
    playerChar.vy = clamp(playerChar.vy + playerChar.ay, -255, 256);
    if(playerChar.y == MIN_Y || playerChar.y == MAX_Y){
        playerChar.vy = 0;
    }

    // update true position and screen position
    playerChar.y = clamp(playerChar.y + playerChar.vy, MIN_Y, MAX_Y+1);
    playerChar.sy = clamp(MAX_SY - (playerChar.y>>5), MIN_SY, MAX_SY);

    obj_set_pos(&obj_mem[0], playerChar.sx, playerChar.sy);
}

int main(){
    //enable Border BG
    u8 cbb = 0;
    u8 sbb = 16;
    REG_BG0CNT = BG_BUILD(cbb, sbb, 1, 0, 1, 0, 0);

    // background
    memcpy16(pal_bg_mem, basicV2Pal, basicV2PalLen/2);
    LZ77UnCompVram(basicV2Tiles, tile_mem[cbb]);
    memcpy16(&se_mem[sbb], basicV2Map, basicV2MapLen/2);

    oam_init(obj_mem, 128);

    memcpy16(tile_mem_obj, playerTiles, playerTilesLen/2);
    LZ77UnCompVram(playerPal, pal_obj_mem);
    obj_set_attr(&obj_mem[0],
        ATTR0_BUILD(playerChar.sy, 0, 0, 0, 0, 0 ,0),
        ATTR1_BUILDR(playerChar.sx, 2, 0, 0),
        ATTR2_BUILD(1, 0, 0)
    );

    //enable Text BG
    REG_BG1CNT = Terminal::setCNT(1, cbb+3, sbb+4);
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_OBJ | DCNT_OBJ_1D | DCNT_MODE0;

    // Initialize Interrupts
    irq_init(nullptr);

	// Maxmod requires the vblank interrupt to reset sound DMA. 
	irq_set( II_VBLANK, mmVBlank, 0);
	irq_enable(II_VBLANK);

    // Setup is done. Lets put it into action!
    Terminal::log("Hello World!");

    FIXED scrollx = 0;
    while(1){

        if(key_hit(KEY_START)){
            Terminal::reset();
        }

        updateChar();
        scrollx += 2;
        REG_BG0HOFS = scrollx;

        //update random nunmber
        qran();

        //poll what keys are down
        key_poll();

        //helps with visual tearing
        vid_vsync();
    }
}