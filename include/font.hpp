#ifndef __FONT__
#define __FONT__

#include "tonc.h"

class font{
    protected:
        const u8* tile_widths;
        TILE* tiles;

    public:
        font(const void* t, const u8* widths);

        u8 getWidth(char c);
        TILE getTile(char c);
        TILE getTile(int i);
};

#endif