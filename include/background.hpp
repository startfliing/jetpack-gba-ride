#ifndef __BACKGROUND__
#define __BACKGROUND__

#include "tonc.h"

#include "tileset.h"

struct backgroundEntry{
    u8 duration; // if there is a specific duration for a background, else 0
    const SCREENBLOCK* map;
};

class BackgroundManager{
    public:
        BackgroundManager(u16 sb, u16 cb);

        void init();
        void update(int scrollX);

    private:
        void selectNewBE();

        int current_screenblock;
        u16 sbb, cbb;

        u16 totalBECt;
        backgroundEntry be;
        u16 current_background_duration; //how long we've been on this background
};

#endif