#include "haz.hpp"

#include "diagonal_laser.h"
#include "missile.h"
#include "orange_64_laser.h"


// units = tiles
hazardAsset diag = {
    diagonal_laserMap,
    8, 8,
    58, 2
};

//pixels
hazardAsset miss = {
    (u16*)missileTiles,
    16, 8,
    62, 110
};

//pixel units
hazardAsset orangeLaser = {
    (u16*)orange_64_laserTiles,
    64, 64,
    256, 60
};


// Hazard implementations
Hazard::~Hazard() {
    delete hitbox;
}

bool Hazard::checkCollision(const Rectangle playerBounds) {
    return hitbox->collidesWithRectangle(playerBounds);
}

// YellowLaser implementations
YellowLaser::YellowLaser(hazardAsset ha) {
    asset = ha;
    hitbox = new Pill((ha.x * 8) + 8, (ha.y * 8) + 8, ((ha.x + ha.width) * 8) - 8, ((ha.y + ha.height) * 8) - 8, 4);
    rendered = false;
    erased = false;
}

void YellowLaser::update(int scrollX, Rectangle playerbounds) {
    //>> 4 for FIXED notation >> 3 for pixel-to-tile ratio
    bool shouldRender = rendered ? false : (scrollX>>7)+32 > asset.x;
    if(shouldRender) render();

    bool shouldErase = erased ? false : (scrollX>>7) > asset.x + (asset.width + 1);
    if(shouldErase) erase();
}

void YellowLaser::render() {
    rendered = true;
    erased = false;
    // tile between 0 - 63 for drawing to BG
    u8 bgx = asset.x & 0x3F;
    for(int i = 0; i < asset.height; i++) {
        //use x to decide which sbb
        int rowStartInd = 32*(asset.y + i) + /*start x*/(asset.x & 0x1F);
        
        if(bgx < (31 - asset.width)) {
            memcpy16(&se_mem[18][rowStartInd], &asset.map[asset.width * i], asset.width);
        } else if(bgx < 31) {
            memcpy16(
                &se_mem[18][rowStartInd], 
                &asset.map[asset.width * i], 
                32 - bgx
            );

            memcpy16(&se_mem[19][rowStartInd + (32 - bgx)], 
                &asset.map[(asset.width * i) + (31 - bgx)], 
                asset.width - (31 - bgx)
            );
        } else if(bgx < (63 - asset.width)) { // fully in sbb 19
            memcpy16(&se_mem[19][rowStartInd], &asset.map[asset.width * i], asset.width);
        } else {
            memcpy16(
                &se_mem[19][rowStartInd], 
                &asset.map[asset.width * i], 
                64 - bgx
            );

            memcpy16(&se_mem[18][rowStartInd + (64 - bgx)], 
                &asset.map[(asset.width * i) + (63 - bgx)], 
                asset.width - (63 - bgx)
            );
        }
    }
}

void YellowLaser::erase() {
    Terminal::log("erasing!");
    erased = true;
    rendered = false;
    // tile between 0 - 63 for drawing to BG
    u8 bgx = asset.x & 0x3F;
    for(int i = 0; i < asset.height; i++) {
        //use x to decide which sbb
        int rowStartInd = 32*(asset.y + i) + /*start x*/(asset.x & 0x1F);
        
        if(bgx < (31 - asset.width)) {
            memset16(&se_mem[18][rowStartInd], 0, asset.width);
        } else if(bgx < 31) {
            memset16(
                &se_mem[18][rowStartInd], 
                0, 
                31 - bgx
            );

            memset16(&se_mem[19][rowStartInd + (32 - bgx)], 
                0, 
                asset.width - (31 - bgx)
            );

        } else if(bgx < (63 - asset.width)) { // fully in sbb 19
            memset16(&se_mem[19][rowStartInd], 0, asset.width);
        } else {
            memset16(
                &se_mem[19][rowStartInd], 
                0, 
                64 - bgx
            );

            memset16(&se_mem[18][rowStartInd + (64 - bgx)], 
                0, 
                asset.width - (63 - bgx)
            );
        }
    }
}

// HazardManager implementations
HazardManager::HazardManager() {
}

void HazardManager::update(int scrollx, Rectangle playerbounds) {
    for(int i = 0; i < hazardsCt; i++) {
        hazards[i]->update(scrollx, playerbounds);
    }

    for(int i = 0; i < hazardsCt; i++) {
        bool hit = hazards[i]->checkCollision(playerbounds);
        if(hit) Terminal::log("hit something!");
    }
}

void HazardManager::createTest() {
    //hazards[hazardsCt++] = new YellowLaser(diag);
    //hazards[hazardsCt++] = new Missile(miss);
    hazards[hazardsCt++] = new OrangeLaser(orangeLaser);
}

#define MISSILE_START_IND 3
#define MISSILE_MAX_CT 8

// Missile implementations
Missile::Missile(hazardAsset ha) {
    asset = ha;
    hitbox = new Rectangle(ha.x * 8, ha.y * 8, ha.width, ha.height);
    rendered = false;
    erased = false;

    velocity = -16;
    y = ha.y;
    fixedX = ha.x << 7;

    LZ77UnCompVram((u32*)ha.map, &tile_mem_obj[0][0xC0]);

    obj = obj_set_attr(&obj_mem[MISSILE_START_IND],
        ATTR0_BUILD(ha.y, 0, 0, 1, 0, 0, 0),
        ATTR1_BUILDR(0, 1, 0, 0),
        ATTR2_BUILD(0xC1, 0, 0)
    );
}

void Missile::update(int scrollx, Rectangle playerbounds){
    fixedX += velocity;
    // during warning phase, update y
    int newX = (fixedX - scrollx) >> 4;
    if(y != playerbounds.getTop()){
        s16 dy = clamp(playerbounds.getTop() - y, -1, 2);
        //y += dy;
    }

    hitbox->setPos(fixedX>>4, y);
    obj_set_pos(obj, newX, y-2);

    if(fixedX < scrollx + (32 << 7) && !rendered){
        //Terminal::log("rendered");
        render();
    }

    if(fixedX - scrollx > -100 && fixedX - scrollx < 100  ){
        Terminal::log("<x, y> : <%%, %%>", (fixedX - scrollx)>>4, playerbounds.getTop() - y);
    }

    if(fixedX < scrollx - (16 << 7)  && !erased){
        //Terminal::log("erased");
        erase();
    }
}

void Missile::render() {
    rendered = true;
    erased = false;
    // tile between 0 - 63 for drawing to BG
    obj_unhide(obj, 0);
}

void Missile::erase() {
    erased = true;
    rendered = false;
    // tile between 0 - 63 for drawing to BG
    obj_hide(obj);
}

#define O_LASER_START_IND 12

// Missile implementations
OrangeLaser::OrangeLaser(hazardAsset ha) {
    asset = ha;
    //based on orientation, update pill location
    int tempY = (ha.y + (ha.height/2));
    hitbox = new Pill(ha.x+4, tempY, ha.x + ha.width - 4, tempY, 4);
    rendered = false;
    erased = false;

    alpha = 1; //TODO calibrate  this
    y = ha.y;
    fixedX = ha.x << 4;

    int affInd = 3;

    LZ77UnCompVram((u32*)ha.map, &tile_mem_obj[0][0xC5]);

    obj = obj_set_attr(&obj_mem[O_LASER_START_IND],
        ATTR0_BUILD(ha.y, 0, 0, 2, 0, 0, 0),
        ATTR1_BUILDA(0, 3, affInd),
        ATTR2_BUILD(0xC6, 0, 0)
    );

    aff = &obj_aff_mem[affInd];
    obj_aff_identity(aff);
}

void OrangeLaser::update(int scrollx, Rectangle playerbounds){
    alpha += 30;
    int newX = (fixedX - scrollx) >> 4;
    hitbox->rotate(alpha);
    obj_set_pos(obj, newX, y);
    obj_aff_rotate(aff, alpha);

    if(fixedX < scrollx + (32 << 7) && !rendered){
        //Terminal::log("<%%,%%>", lu_cos(0x4000), lu_sin(0x4000));
        render();
    }

    if(fixedX < scrollx - (16 << 7)  && !erased){
        //Terminal::log("erased");
        erase();
    }
}

void OrangeLaser::render() {
    rendered = true;
    erased = false;
    // tile between 0 - 63 for drawing to BG
    obj_unhide(obj, ATTR0_AFF);
}

void OrangeLaser::erase() {
    erased = true;
    rendered = false;
    // tile between 0 - 63 for drawing to BG
    obj_hide(obj);
}