#ifndef __PLAYER_CHARACTER__
#define __PLAYER_CHARACTER__

#include "tonc.h"
#include "collision.hpp"

#include "player.h"
#include "bullets.h"
#include "playerDeaths.h"

#define MAX_SY 104
#define MIN_SY 4

#define MAX_BULLET_SY 70
#define MIN_BULLET_SY 26

#define MAX_Y 0xD00
#define MIN_Y 0

#define MIN_SCALE 0
#define MAX_SCALE 2<<8

class PlayerCharacter{
    public:
        PlayerCharacter(){
            x = 0;
            y = MIN_Y;
            sx = 16;
            sy = MAX_SY;
            ay = 0;
            vy = 0;
            status = 2;
            curr_anim_tick = 0;  
            anim_speed = 6;  
            curr_frame = 0;  
            
            buller_anim_tick = 0;    
            bullet_speed = 5;    
            bullet_frame = 0;    
            
            hitbox = new Rectangle(sx+9, sy + 14, 16, 18);
        };

        void init(){
            //player
            memcpy16(tile_mem_obj, playerTiles, playerTilesLen/10);
            LZ77UnCompVram(playerPal, pal_obj_mem);
            obj_set_attr(&obj_mem[0],
                ATTR0_BUILD(sy, 0, 0, 1, 0, 0 ,0),
                ATTR1_BUILDA(sx, 2, 0),
                ATTR2_BUILD(1, 0, 0)
            );
            obj_aff_identity(&obj_aff_mem[0]);

            //bullets
            memcpy16(&tile_mem_obj[0][128], bulletsTiles, bulletsTilesLen/24);
            obj_set_attr(&obj_mem[1],
                ATTR0_SQUARE | ATTR0_HIDE | ATTR0_Y(sy),
                ATTR1_BUILDA(sx-53, 3, 1),
                ATTR2_BUILD(129, 0, 0)
            );
            obj_aff_identity(&obj_aff_mem[1]);
        };

        void update(int scrollX, int speed){
            switch(status){
                case 0: //dead
                    updateDead(scrollX, speed);
                    break;
                case 1: //alive
                    updateAlive(scrollX);
                    break;
                case 2: //entering the level
                    updateEntering(scrollX);
                    break;
                default:
                    break;
            }
        };

        void updateAlive(int scrollX){
            // acceleration
            ay = key_is_down(KEY_A) ? 4 : -4;

            // velocity, if @ top or bot, velocity = 0
            vy = clamp(vy + ay, -255, 256);

            // update true position and screen position
            y = clamp(y + vy, MIN_Y, MAX_Y+1);
            sy = clamp(MAX_SY - (y>>5), MIN_SY, MAX_SY+1);

            if(y == MIN_Y || y == MAX_Y) vy = 0;

            u8 temp_frame = curr_frame;
            u8 temp_bullet_frame = bullet_frame;
            curr_anim_tick++;
            
            //flying
            if(y > 0){
                curr_frame = 4;

                //unhide bullets. 
                obj_unhide(&obj_mem[1], ATTR0_AFF_DBL);

                //player sy <4,104> and bullet sy <26,70>
                int vertical_offset = MIN_BULLET_SY + ((sy - MIN_SY)*(MAX_BULLET_SY-MIN_BULLET_SY))/(MAX_SY-MIN_SY);

                //min scale = 0, max scale = 2<<8 (Fixed Notation = 0x000002.00)
                u16 scale_factor = clamp(y * (MAX_SCALE)/MAX_Y, MIN_SCALE, MAX_SCALE);

                obj_set_pos(&obj_mem[1], sx-53, vertical_offset);
                obj_aff_scale_inv(&obj_aff_mem[1], scale_factor, scale_factor);


                buller_anim_tick++;
                if(buller_anim_tick == bullet_speed){
                    buller_anim_tick = 0;
                    bullet_frame = clamp(bullet_frame + 1, 0, 11);
                    if(bullet_frame == 7) bullet_frame = 3;

                    if(key_is_up(KEY_A) && bullet_frame >= 3 && bullet_frame <= 7) bullet_frame = 8;

                    if(key_is_down(KEY_A) && bullet_frame >=8 ) bullet_frame = 0;
                }

                //running
            } else if(curr_anim_tick > anim_speed){
                obj_hide(&obj_mem[1]);
                curr_anim_tick = 0;
                curr_frame = wrap(curr_frame + 1, 0, 4);
                buller_anim_tick = 0;
                bullet_frame = 0;
            }

            if(temp_frame != curr_frame){
                memcpy16(tile_mem_obj, &playerTiles[curr_frame * 128], sizeof(TILE)*8);
            }

            if(temp_bullet_frame != bullet_frame){
                memcpy16(&tile_mem_obj[0][128], &bulletsTiles[bullet_frame * 8 * 64], sizeof(TILE)*32);
            }

            x = (scrollX>>4);
            obj_set_pos(&obj_mem[0], sx, sy);
            hitbox->setPosition(x+9, sy+14);
        }

        void updateDead(int scrollX, int speed){
            // acceleration
            ay = -4;

            // velocity, if @ top or bot, velocity = 0
            vy = clamp(vy + ay, -255, 256);

            y = y+vy;
            //check if we hit the ground, if so, flip velocity for bounce
            if(y < 0){
                vy = (vy * -1) * 0.60;
                y *= -1;
                if(vy > 0x30 /*arbitrary height*/){
                    //update character theta
                    theta = qran();
                }else{
                    //set faceplant sprite
                    //set theta to facedown
                    theta = 0xDFFF;
                }
            }
            // update true position and screen position
            y = clamp(y, MIN_Y, MAX_Y+1);

            if(y > MIN_Y+15){
                theta -= clamp(0xA0, 0, speed<<3);
            }


            sy = clamp(MAX_SY - (y>>5), MIN_SY, MAX_SY+1);

            if(y == MIN_Y || y == MAX_Y) vy = 0;

            u8 temp_frame = curr_frame;
            u8 temp_bullet_frame = bullet_frame;

            buller_anim_tick++;
            if(buller_anim_tick == bullet_speed){
                buller_anim_tick = 0;
                bullet_frame = clamp(bullet_frame + 1, 8, 11);
            }

            curr_anim_tick++;
            if(killedBy != 4 && speed > 0){
                
                if(curr_anim_tick < 64){
                    curr_frame = ((curr_anim_tick>>3) & 1) + 1;
                }else{
                    curr_frame = 3;
                }

            }else{
                if(curr_anim_tick >= 24){
                    curr_frame = 3;
                }
            }

            

            if(temp_frame != curr_frame){
                memcpy16(tile_mem_obj, &playerDeathsTiles[curr_frame * 128], sizeof(TILE)*8);
            }

            if(temp_bullet_frame != bullet_frame){
                memcpy16(&tile_mem_obj[0][128], &bulletsTiles[bullet_frame * 8 * 64], sizeof(TILE)*32);
            }

            x = (scrollX>>4);
            obj_set_pos(&obj_mem[0], sx, sy+5);
            obj_aff_rotate(&obj_aff_mem[0], theta);
            hitbox->setPosition(x+9, sy+7);
        }

        void updateEntering(int scrollX){
            x = scrollX>>4;

            u8 temp_frame = curr_frame;
            curr_anim_tick++;

            if(curr_anim_tick > anim_speed){
                curr_anim_tick = 0;
                curr_frame = wrap(curr_frame + 1, 0, 4);
            }

            if(temp_frame != curr_frame){
                memcpy16(tile_mem_obj, &playerTiles[curr_frame * 128], sizeof(TILE)*8);
            }

            obj_set_pos(&obj_mem[0], x, sy);
            hitbox->setPosition(x+9, sy+14);
            if(x == 16/* */) status = 1; //transition to updateAlive
        }

        Rectangle* getHitBox(){
            return hitbox;
        }

        void dies(u8 hazardIndex){
            status = 0;
            curr_frame = 0;
            curr_anim_tick = 0;
            killedBy = hazardIndex;
            memcpy16(tile_mem_obj, &playerDeathsTiles[curr_frame * 128], sizeof(TILE)*8);
        };

        bool isDead(){
            return status == 0;
        }

        //status 0 = dead, 1 = alive
        int status;

        //8-bit index for what killed barry
        // 0 alive, 1 - yellow, 2 - orange, 3 - red, 4 - missile
        u8 killedBy;

        //true position
        int x, y;

        //screen position
        int sx, sy;

        //velocity and acceleration
        FIXED vy, ay;

        // theta for spinning barry while he dies
        u16 theta;

        // anim tick for updating per frame, anim_speed for ticks to update sprite, curr_frame 
        u8 curr_anim_tick, anim_speed, curr_frame;

        // anim tick for updating per frame, anim_speed for ticks to update sprite, curr_frame 
        u8 buller_anim_tick, bullet_speed, bullet_frame;
    private:
        Rectangle* hitbox;
};


#endif