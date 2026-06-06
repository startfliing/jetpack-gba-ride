#include "tonc.h"
#include "maxmod.h"

#include "terminal.hpp"
#include "haz.hpp"

#include "zone1.h"
#include "tileset.h"
#include "hazards.h"

#include "playerCharacter.hpp"
#include "background.hpp"

#include "soundbank.h"
#include "soundbank_bin.h"


PlayerCharacter* playerChar = new PlayerCharacter();
BackgroundManager* bm = new BackgroundManager(16, 0);

int main(){
    //enable Border BG
    u8 cbb = 0;
    u8 sbb = 16;
    REG_BG0CNT = BG_BUILD(cbb, sbb, 1, 0, 1, 0, 0);

    // background
    bm->init();

    //yellow lasers
    REG_BG2CNT = BG_BUILD(cbb+1, sbb+2, 1, 0, 0, 0, 0);
    LZ77UnCompVram(hazardsTiles, tile_mem[cbb+1]);

    //red lasers
    REG_BG3CNT = BG_BUILD(cbb+1, sbb+4, 0, 0, 0, 0, 0);
    
    oam_init(obj_mem, 128);

    playerChar->init();

    //enable Text BG
    REG_BG1CNT = Terminal::setCNT(1, cbb+3, sbb+4);
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_OBJ | DCNT_OBJ_1D | DCNT_MODE0;

    // Initialize Interrupts
    irq_init(nullptr);

	// Maxmod requires the vblank interrupt to reset sound DMA. 
	irq_set( II_VBLANK, mmVBlank, 0);
	irq_enable(II_VBLANK);

    //TODO: remove this
    HazardManager* hm = new HazardManager();
    hm->createDiag();

    int speed = 256;

    FIXED scrollx = (-32)<<4;
    while(1){

        if(key_hit(KEY_START)){
            Terminal::reset();
        }

        scrollx += speed>>3;
        playerChar->update(scrollx, speed);
        bm->update(scrollx);
        hm->update(scrollx, playerChar);
        if( scrollx>>4 > 16 ){ //if during startup
            REG_BG0HOFS = (scrollx>>4)-16;
            REG_BG2HOFS = (scrollx>>4)-16;
        }

        if(playerChar->isDead()){
            //we can animate here
            speed = clamp(speed - 1, 0, INT16_MAX);
        }

        if(speed == 0 && key_hit(KEY_START)){
            return 0;
        }
        //update random nunmber
        qran();

        //poll what keys are down
        key_poll();

        //helps with visual tearing
        vid_vsync();
    }
}