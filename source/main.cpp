#include "tonc.h"
#include "maxmod.h"

#include "terminal.hpp"

#include "zone1.h"
#include "tileset.h"

#include "player.h"
#include "bullets.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#define MAX_SY 104
#define MIN_SY 4

#define MAX_BULLET_SY 70
#define MIN_BULLET_SY 26

#define MAX_Y 0xD00
#define MIN_Y 0

#define MIN_SCALE 0
#define MAX_SCALE 2<<8

struct playerCharacter{
    //true position
    int x, y;

    //screen position
    int sx, sy;

    //velocity and acceleration
    FIXED vy, ay;

    // anim tick for updating per frame, anim_speed for ticks to update sprite, curr_frame 
    u8 curr_anim_tick, anim_speed, curr_frame;

    // anim tick for updating per frame, anim_speed for ticks to update sprite, curr_frame 
    u8 buller_anim_tick, bullet_speed, bullet_frame;
};

playerCharacter playerChar = {
    0, MIN_Y,
    16, MAX_SY,
    0, 0,
    0, 6, 0,
    0, 5, 0
};

FIXED current_speed = 0;

void updateChar(){
    // acceleration
    playerChar.ay = key_is_down(KEY_A) ? 4 : -4;

    // velocity, if @ top or bot, velocity = 0
    playerChar.vy = clamp(playerChar.vy + playerChar.ay, -255, 256);

    // update true position and screen position
    playerChar.y = clamp(playerChar.y + playerChar.vy, MIN_Y, MAX_Y+1);
    playerChar.sy = clamp(MAX_SY - (playerChar.y>>5), MIN_SY, MAX_SY+1);

    if(playerChar.y == MIN_Y || playerChar.y == MAX_Y) playerChar.vy = 0;

    u8 temp_frame = playerChar.curr_frame;
    u8 temp_bullet_frame = playerChar.bullet_frame;
    playerChar.curr_anim_tick++;
    
    //flying
    if(playerChar.y > 0){
        playerChar.curr_frame = 4;

        //unhide bullets. 
        obj_unhide(&obj_mem[1], ATTR0_AFF_DBL);

        //player sy <4,104> and bullet sy <26,70>
        int vertical_offset = MIN_BULLET_SY + ((playerChar.sy - MIN_SY)*(MAX_BULLET_SY-MIN_BULLET_SY))/(MAX_SY-MIN_SY);

        //min scale = 0, max scale = 2<<8 (Fixed Notation = 0x000002.00)
        u16 scale_factor = clamp(playerChar.y * (MAX_SCALE)/MAX_Y, MIN_SCALE, MAX_SCALE);

        obj_set_pos(&obj_mem[1], playerChar.sx-53, vertical_offset);
        obj_aff_scale_inv(&obj_aff_mem[1], scale_factor, scale_factor);


        playerChar.buller_anim_tick++;
        if(playerChar.buller_anim_tick == playerChar.bullet_speed){
            playerChar.buller_anim_tick = 0;
            playerChar.bullet_frame = clamp(playerChar.bullet_frame + 1, 0, 12);
            if(playerChar.bullet_frame == 7){
                playerChar.bullet_frame = 3;
            }

            if(key_is_up(KEY_A) && playerChar.bullet_frame >= 3 && playerChar.bullet_frame <= 7) playerChar.bullet_frame = 8;

            if(key_is_down(KEY_A) && playerChar.bullet_frame >=8 ) playerChar.bullet_frame = 0;
        }

        //running
    } else if(playerChar.curr_anim_tick > playerChar.anim_speed){
        obj_hide(&obj_mem[1]);
        playerChar.curr_anim_tick = 0;
        playerChar.curr_frame = wrap(playerChar.curr_frame + 1, 0, 4);
        playerChar.buller_anim_tick = 0;
        playerChar.bullet_frame = 0;
    }

    if(temp_frame != playerChar.curr_frame){
        obj_mem[0].attr2 = (obj_mem[0].attr2 & ~ATTR2_ID_MASK) | ATTR2_ID((playerChar.curr_frame * 16) + 1);
    }

    if(temp_bullet_frame != playerChar.bullet_frame){
        obj_mem[1].attr2 = (obj_mem[1].attr2 & ~ATTR2_ID_MASK) | ATTR2_ID((playerChar.bullet_frame * 64) + 129);
    }

    obj_set_pos(&obj_mem[0], playerChar.sx, playerChar.sy);
}

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

    //player
    memcpy16(tile_mem_obj, playerTiles, playerTilesLen/2);
    LZ77UnCompVram(playerPal, pal_obj_mem);
    obj_set_attr(&obj_mem[0],
        ATTR0_BUILD(playerChar.sy, 0, 0, 0, 0, 0 ,0),
        ATTR1_BUILDR(playerChar.sx, 2, 0, 0),
        ATTR2_BUILD(1, 0, 0)
    );

    //bullets
    memcpy16(&tile_mem_obj[0][128], bulletsTiles, bulletsTilesLen/2);
    obj_set_attr(&obj_mem[1],
        ATTR0_SQUARE | ATTR0_HIDE | ATTR0_Y(playerChar.sy),
        ATTR1_BUILDA(playerChar.sx-53, 3, 1),
        ATTR2_BUILD(129, 0, 0)
    );
    obj_aff_identity(&obj_aff_mem[1]);

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
        scrollx += 32;
        REG_BG0HOFS = scrollx>>4;

        //update random nunmber
        qran();

        //poll what keys are down
        key_poll();

        //helps with visual tearing
        vid_vsync();
    }
}