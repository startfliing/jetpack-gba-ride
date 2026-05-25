#include "tonc.h"
#include "maxmod.h"

#include "terminal.hpp"

#include "zone1.h"
#include "tileset.h"

#include "playerCharacter.hpp"

#include "soundbank.h"
#include "soundbank_bin.h"


PlayerCharacter* playerChar = new PlayerCharacter();


// HAZARDS
/*  Hazards are the lasers and missiles that could end your run

*/
int hazardTimer = 0;


int main(){
    //enable Border BG
    u8 cbb = 0;
    u8 sbb = 16;
    REG_BG0CNT = BG_BUILD(cbb, sbb, 0, 0, 1, 0, 0);

    // background
    LZ77UnCompVram(tilesetPal, pal_bg_mem);
    LZ77UnCompVram(tilesetTiles, tile_mem[cbb]);
    memcpy16(&se_mem[sbb], zone1Map, zone1MapLen/2);

    oam_init(obj_mem, 128);

    playerChar->init();

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

        scrollx += 32;
        playerChar->update(scrollx);
        REG_BG0HOFS = scrollx>>4;

        //update random nunmber
        qran();

        //poll what keys are down
        key_poll();

        //helps with visual tearing
        vid_vsync();
    }
}