#include "tonc.h"
#include "maxmod.h"

#include "terminal.hpp"

#include "basicV2.h"
#include "soundbank.h"
#include "soundbank_bin.h"

FIXED current_speed = 0;


int main(){
    //enable Border BG
    u8 cbb = 0;
    u8 sbb = 16;
    REG_BG0CNT = BG_BUILD(cbb, sbb, 1, 0, 1, 0, 0);

    // background
    memcpy16(pal_bg_mem, basicV2Pal, basicV2PalLen/2);
    LZ77UnCompVram(basicV2Tiles, tile_mem[cbb]);
    memcpy16(&se_mem[sbb], basicV2Map, basicV2MapLen/2);


    //enable Text BG
    REG_BG1CNT = Terminal::setCNT(1, cbb+3, sbb+4);
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_MODE0;

    // Initialize Interrupts
    irq_init(nullptr);

	// Maxmod requires the vblank interrupt to reset sound DMA. 
	irq_set( II_VBLANK, mmVBlank, 0);
	irq_enable(II_VBLANK);

	// initialise maxmod with soundbank and 8 channels
    mmInitDefault( (mm_addr)soundbank_bin, 16 );
    
    //Setup is done. Lets put it into action!
    Terminal::log("Hello World!");
    mmStart( MOD_FLATOUTLIES, MM_PLAY_ONCE );

    FIXED scrollx = 0;
    while(1){

        if(key_hit(KEY_START)){
            Terminal::reset();
        }

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