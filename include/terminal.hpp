#ifndef __TERMINAL__
#define __TERMINAL__

#include "tonc.h"

#include "font.hpp"

class Terminal{
    public:
        static u16 setCNT(u8 bg, u16 cbb, u16 sbb);

        static void reset();
        static void eraseLine();

        static void log(const char* word){
            if(need_new_line){
                advanceOneLine();
                need_new_line = false;
            }
            while(*word != '\0'){
                //draw next char
                drawVal(*word);
                word++;
            }
            need_new_line = true;
        };


        template <typename T, typename... Args>
        static void log(const char* word, T val, Args... args){
            if(need_new_line){
                advanceOneLine();
                need_new_line = false;
            }
            while(*word != '\0'){

                //'%%' will be used to display a char or int
                if(*word == '%' && *word == '%'){
                    //draw value
                    drawVal(val);
                    //move past '%%'
                    word += 2;
                    //continue logging w/o val
                    log(word, args...);
                    return;
                }else{
                    //draw next char
                    drawVal(*word);
                    word++; 
                }
            }
            need_new_line = true;
        };


    private:
        static void loadTile(char c, int vram_tile_ind);
        static void drawVal(int val);
        static void drawVal(char c);
        static void drawVal(const char* string);
        static void clearTopRow();
        static void updateScreen();
        static void advanceOneLine();
        
        static u16 curr_line_num;
        static u16 curr_tile_num;
        static u8 curr_pixel_num;
        static bool need_new_line;

        //For DCNT_BG$(0-3)
        static u8 bg_ind;

        static u16 text_sbb;
        static u16 text_cbb;
        static font text_font;
};

#endif