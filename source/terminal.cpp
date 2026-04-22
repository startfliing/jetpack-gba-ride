#include "terminal.hpp"

#include "text.h"
#include "textWidths.hpp"

#define IS_TEXT_SCROLL_ENABLED false

#define TERMINAL_TEXT_WIDTH 28
#define TERMINAL_TEXT_HEIGHT 18

#define TERMINAL_TOP_ROW 1
#define TERMINAL_LEFT_COL 1

//static class stuff
u16 Terminal::curr_line_num = 0;
u16 Terminal::curr_tile_num = 0;
u8 Terminal::curr_pixel_num = 0;
u16 Terminal::text_sbb = 0;
u16 Terminal::text_cbb = 0;
u8 Terminal::bg_ind = 0;
font Terminal::text_font(textTiles, textWidths);
bool Terminal::need_new_line = true;

//set control flags for bg
u16 Terminal::setCNT(u8 bg, u16 cbb, u16 sbb){
    bg_ind = clamp(bg, 0, 4);
    text_sbb = sbb;
    text_cbb = cbb;

    reset();
    
    //These could be added to the function parameters if you call for it
    return BG_BUILD(cbb,sbb,0,0,0,0,1);
}

// reset line, clear the whole screen, and reset offset
void Terminal::reset(){
    curr_line_num = 0;
    memset16(&se_mem[text_sbb], 0, sizeof(SCREENBLOCK)/2);
    TILE blank = text_font.getTile(0);
    for(int i = 0; i < TERMINAL_TEXT_WIDTH*TERMINAL_TEXT_HEIGHT; i++){
        tile_mem[text_cbb][i+1] = blank;
    }
}

//turn int (0,9) to char
char intToChar(int x){
    return (char)(x + '0');
}

u32 tileRowShiftR(u32 val, u8 rhs){
    u32 temp = val<<(rhs * 4);
    return temp;
}

u32 tileRowShiftL(u32 val, u8 lhs){
    u32 temp = val>>(lhs * 4);
    return temp;
}

void Terminal::loadTile(char c, int vram_tile_ind){
    //get info from font
    TILE charTile;
    u8 charWidth;
    if(c == '\t'){
        charTile = text_font.getTile(0);
        charWidth = 16 - curr_pixel_num;
    }else{
        charTile = text_font.getTile(c);
        charWidth = text_font.getWidth(c);
    }

    //get the offset from the left side of the tile
    u8 offset = curr_pixel_num;
    curr_pixel_num += charWidth;

    //iterate through rows of the tile
    for(int row = 0; row < 8; row++){
        //load the row w/ offset
        tile_mem[text_cbb][vram_tile_ind].data[row] |= tileRowShiftR(charTile.data[row], offset);

        //if we need to use next tile
        if(curr_pixel_num >= 8){
            tile_mem[text_cbb][vram_tile_ind+1].data[row] |= tileRowShiftL(charTile.data[row], 8-offset);
        }
    }
    //if we started using the next tile
    if(curr_pixel_num >= 8){
        curr_pixel_num -= 8;
        curr_tile_num++;
    }

    //adds the scroll effect
    if(IS_TEXT_SCROLL_ENABLED) vid_vsync();
}

//cool use of recursion and function overloading
void Terminal::drawVal(int val){
    //draw a negative sign but only once
    if(val < 0){
        drawVal('-');
        val = val * -1;
    }

    //if value is greater than 9
    if(val > 9){
        //recurse
        drawVal(val/10);
    }

    //base case, sorta
    drawVal(intToChar(val%10));
}

//every single char will be drawn with this function, so put in new line logic and screen logic
void Terminal::drawVal(char c){
    
    if(c == '\n'){
        advanceOneLine();
        return;
    }

    //new line
    if(curr_tile_num >= (TERMINAL_TEXT_WIDTH - 3) && c == ' '){      //if current tile is a space and close to the right
        advanceOneLine();
        return;
    }

    //calculate where in VRAM the current tile is
    int vram_tile_ind = (curr_line_num * TERMINAL_TEXT_WIDTH) + curr_tile_num + 1;
    
    //load char in that Tile in VRAM
    loadTile(c, vram_tile_ind);
}

void Terminal::drawVal(const char* string){
    //draw string by drawing each character
    while(*string != '\0'){
        drawVal(*string++);
    }
}

//clear a row right before writing on it
void Terminal::clearTopRow(){
    TILE blank = text_font.getTile(0);
    for(int j = 0; j < TERMINAL_TEXT_WIDTH; j++){
        tile_mem[text_cbb][(curr_line_num*TERMINAL_TEXT_WIDTH)+j+1] = blank;
    }
}

//redraw screen so that curr_line_num is on the bottom of the screen
void Terminal::updateScreen(){
    //get next line number
    u16 temp_line = (curr_line_num+1)%TERMINAL_TEXT_HEIGHT;
    for(int i = 0; i < TERMINAL_TEXT_HEIGHT; i++){
        for(int j = 0; j < TERMINAL_TEXT_WIDTH; j++){
            //draw onto screen by iterating through VRAM and screen data at the same time
            se_mem[text_sbb][(TERMINAL_TOP_ROW*32)+TERMINAL_LEFT_COL+(i*32)+j] = (temp_line*TERMINAL_TEXT_WIDTH)+j+1;
        }
        temp_line = (temp_line+1)%TERMINAL_TEXT_HEIGHT;
    }
}

void Terminal::advanceOneLine(){
    //update line num
    curr_line_num = (curr_line_num+1) % TERMINAL_TEXT_HEIGHT;

    //clear_topRow
    clearTopRow();

    //redraw rows on screen
    updateScreen();

    //reset tile and pixel num
    curr_tile_num = 0;
    curr_pixel_num = 0;
}

void Terminal::eraseLine(){
    clearTopRow();

    //update line num
    curr_line_num = wrap(curr_line_num - 1, 0, TERMINAL_TEXT_HEIGHT);
    

    //redraw rows on screen
    updateScreen();

    //reset tile and pixel num
    curr_tile_num = 0;
    curr_pixel_num = 0;
}

