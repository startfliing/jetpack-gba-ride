#include "font.hpp"

font::font(const void* t, const u8* widths){
    tile_widths = widths;
    tiles = (TILE*)t;
}

u8 font::getWidth(char c){
    size_t ind = ((int)c) - 31;
    return tile_widths[ind];
}

TILE font::getTile(char c){
    size_t ind = ((int)c) - 31;
    return tiles[ind];
}

TILE font::getTile(int i){
    return tiles[i];
}