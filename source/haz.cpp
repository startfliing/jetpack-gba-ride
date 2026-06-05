#include "haz.hpp"

#include "diagonal_laser.h"
#include "missile.h"
#include "orange_64_laser.h"
#include "redLaserTiles.h"


// Hazard implementations
Hazard::~Hazard() {
    delete hitbox;
}

u8 Hazard::checkCollision(const Rectangle playerBounds) {
    return hitbox->collidesWithRectangle(playerBounds) ? hazardIndex : 0;
}

// YellowLaser implementations
/**
 * @brief Construct a new Yellow Laser:: Yellow Laser object
 * 
 * Plots both points of the laser included in the asset map.
 * 
 * @param x1 
 * @param y1 
 * @param x2 
 * @param y2 
 * @param assetMap 
 */
YellowLaser::YellowLaser(int x1, int y1, int x2, int y2, u16* assetMap) {
    hitbox = new Pill(x1, y1, x2, y2, 4);
    rendered = false;
    erased = false;
    map = assetMap;

    //hazard index
    hazardIndex = 1;

    //honestly don't know how this logic exactly works,
    //but when I moved away from HazardAsset, this was the
    //solution and there were some rendering issues.

    tileWidth = (x2>>3) - (x1>>3);
    leftTile = x1>>3;
    //if width is negative, x2 is left of x1
    if(tileWidth < 0){
        leftTile = x2>>3;
        tileWidth *= -1;
    }
    tileWidth++;

    tileHeight = (y2>>3) - (y1>>3);
    topTile = y1>>3;
    //if width is negative, x2 is left of x1
    if(tileHeight < 0){
        topTile = y2>>3;
        tileHeight *= -1;
    }
    topTile--;
    tileHeight += 2;
    Terminal::log("TL : <%%,%%> WH <%%,%%>",topTile, leftTile, tileWidth, tileHeight);
}

void YellowLaser::update(int scrollX, Rectangle playerbounds) {
    //>> 4 for FIXED notation >> 3 for pixel-to-tile ratio
    bool shouldRender = rendered ? false : (scrollX>>7)+32 > leftTile;
    if(shouldRender) render();

    bool shouldErase = erased ? false : (scrollX>>7) > leftTile + (tileWidth + 1);
    if(shouldErase) erase();
}

void YellowLaser::render() {
    Terminal::log("rendering!");
    rendered = true;
    erased = false;
    // tile between 0 - 63 for drawing to BG
    u8 bgx = leftTile & 0x3F;
    for(int i = 0; i < tileHeight; i++) {
        //use x to decide which sbb
        int rowStartInd = 32*(topTile + i) + /*start x*/(leftTile & 0x1F);
        
        if(bgx < (31 - tileWidth)) {
            memcpy16(&se_mem[18][rowStartInd], &map[tileWidth * i], tileWidth);
        } else if(bgx < 31) {
            memcpy16(
                &se_mem[18][rowStartInd], 
                &map[tileWidth * i], 
                32 - bgx
            );

            memcpy16(&se_mem[19][rowStartInd + (32 - bgx)], 
                &map[(tileWidth * i) + (31 - bgx)], 
                tileWidth - (31 - bgx)
            );
        } else if(bgx < (63 - tileWidth)) { // fully in sbb 19
            memcpy16(&se_mem[19][rowStartInd], &map[tileWidth * i], tileWidth);
        } else {
            memcpy16(
                &se_mem[19][rowStartInd], 
                &map[tileWidth * i], 
                64 - bgx
            );

            memcpy16(&se_mem[18][rowStartInd + (64 - bgx)], 
                &map[(tileWidth * i) + (63 - bgx)], 
                tileWidth - (63 - bgx)
            );
        }
    }
}

void YellowLaser::erase() {
    Terminal::log("erasing!");
    erased = true;
    rendered = true;
    // tile between 0 - 63 for drawing to BG
    u8 bgx = leftTile & 0x3F;
    for(int i = 0; i < tileHeight; i++) {
        //use x to decide which sbb
        int rowStartInd = 32*(topTile + i) + /*start x*/(leftTile & 0x1F);
        
        if(bgx < (31 - tileWidth)) {
            memset16(&se_mem[18][rowStartInd], 0, tileWidth);
        } else if(bgx < 31) {
            memset16(
                &se_mem[18][rowStartInd], 
                0, 
                31 - bgx
            );

            memset16(&se_mem[19][rowStartInd + (32 - bgx)], 
                0, 
                tileWidth - (31 - bgx)
            );

        } else if(bgx < (63 - tileWidth)) { // fully in sbb 19
            memset16(&se_mem[19][rowStartInd], 0, tileWidth);
        } else {
            memset16(
                &se_mem[19][rowStartInd], 
                0, 
                64 - bgx
            );

            memset16(&se_mem[18][rowStartInd + (64 - bgx)], 
                0, 
                tileWidth - (63 - bgx)
            );
        }
    }
}

// HazardManager implementations
HazardManager::HazardManager() {
}

void HazardManager::update(int scrollx, PlayerCharacter* player) {
    
    for(int i = 0; i < hazardsCt; i++) {
        hazards[i]->update(scrollx, *player->getHitBox());
    }

    for(int i = 0; i < hazardsCt; i++) {
        u8 hazardIndex = hazards[i]->checkCollision(*player->getHitBox());
        if(hazardIndex){ //if hazardIndex isn't 0, player died
            //death initiate
            player->dies(hazardIndex);
        };
    }
}

void HazardManager::createTest() {
    //hazards[hazardsCt++] = new YellowLaser(464, 24, 520, 72, (u16*)diagonal_laserMap);
    hazards[hazardsCt++] = new Missile(60, 110, missileTiles);
    //hazards[hazardsCt++] = new RedLaser(512, 0, 65);
    //hazards[hazardsCt++] = new OrangeLaser(256,60,64,orange_64_laserTiles);
}

#define MISSILE_START_IND 3
#define MISSILE_MAX_CT 8

// Missile implementations
Missile::Missile(int x, int y, const unsigned int* assetMap) {
    hitbox = new Rectangle(x*8, y*8, 16, 8);
    rendered = false;
    erased = false;
    hazardIndex = 4;

    velocity = -16;
    fixedY = y << 4;
    fixedX = x << 7;

    LZ77UnCompVram((u32*)assetMap, &tile_mem_obj[0][0xC0]);

    obj = obj_set_attr(&obj_mem[MISSILE_START_IND],
        ATTR0_BUILD(y, 0, 0, 1, 0, 0, 0),
        ATTR1_BUILDR(0, 1, 0, 0),
        ATTR2_BUILD(0xC1, 0, 0)
    );
}

void Missile::update(int scrollx, Rectangle playerbounds){
    fixedX += velocity;
    // during warning phase, update y
    int newX = (fixedX - scrollx) >> 4;
    int maxDY = 4;
    if(fixedY>>4 != playerbounds.getTop()){
        s16 dy = clamp(playerbounds.getTop() - (fixedY>>4), -1*maxDY, maxDY + 1);
        fixedY += dy;
    }

    hitbox->setPos(fixedX>>4, fixedY>>4);
    obj_set_pos(obj, newX, (fixedY>>4)-2);

    if(fixedX < scrollx + (32 << 7) && !rendered){
        render();
    }

    if(fixedX < scrollx - (16 << 7)  && !erased){
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
OrangeLaser::OrangeLaser(int x, int y, int diameter, const unsigned int* assetMap) {
    //based on orientation, update pill location
    int tempY = (y + (diameter/2));
    hitbox = new Pill(x+4, tempY, x + diameter - 4, tempY, 4);
    rendered = false;
    erased = false;
    hazardIndex = 2;

    alpha = 1; //TODO calibrate  this
    this->y = y;
    fixedX = x << 4;

    int affInd = 3;

    LZ77UnCompVram((u32*)assetMap, &tile_mem_obj[0][0xC5]);

    obj = obj_set_attr(&obj_mem[O_LASER_START_IND],
        ATTR0_BUILD(y, 0, 0, 2, 0, 0, 0),
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
        render();
    }

    if(fixedX < scrollx - (16 << 7)  && !erased){
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

// RedLaser implementations
// tile start bg_mem[cbb+1][64]

u16 getNewSE(int column, u16 tileInd){
    u16 returnSE = 0;
    switch(column){
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
            returnSE |= tileInd+2;
            break;

        //tileInd+1
        case 28:
            returnSE |= SE_HFLIP;
        case 1:
            returnSE |= tileInd+1;
            break;

        //tileInd
        case 29: //add horizontal flip
            returnSE |= SE_HFLIP;
        case 0: //set tileInd
            returnSE |= tileInd;
            break;
        default:
            returnSE = 0;
            break;
    }
    return returnSE;
}

TILE tileset[20];

struct animationframe {
    u16 tile0, tile1, tile2;
};

//15 tick increments
animationframe redlaserAnimation[29] = {
    {0,0,0},
    {1,0,0},
    {2,0,0},
    {3,6,0},
    {4,7,0},
    {4,7,0},
    {4,8,0},
    {4,9,0},
    {4,8,0},
    {4,9,0},
    {4,8,17},
    {4,9,18},
    {4,8,17},
    {4,9,18},
    {4,10,19}, // i = 14
    {4,10,19},
    {4,10,19},
    {4,10,19},
    {4,10,19},
    {4,10,19},
    {4,10,19},
    {4,10,19}, // i = 21
    {4,9,18},
    {4,8,17},
    {4,7,0},
    {3,6,0},
    {2,0,0},
    {1,0,0},
    {0,0,0}
};

void loadLaserAnimation(animationframe af, u16 tileInd){
    memcpy16(&tile_mem[1][tileInd], &tileset[af.tile0], sizeof(TILE)/2);
    memcpy16(&tile_mem[1][tileInd+1], &tileset[af.tile1], sizeof(TILE)/2);
    memcpy16(&tile_mem[1][tileInd+2], &tileset[af.tile2], sizeof(TILE)/2);
}

void buildLaser(int yIndex, u16 tileInd){

    loadLaserAnimation(redlaserAnimation[0], tileInd);

    SE* bg = se_mem[17];
    int startInd = ((yIndex*2)+3)*32;
    for(int j = 0; j < 30; j++){//column
        u16 newSE = getNewSE(j,tileInd);
        //top tile
        bg[startInd+j] = newSE;
        //bottom tile
        bg[startInd+j+32] = newSE | SE_VFLIP;
    }
}



void updateLaser(int frame, u16 tileInd){
    loadLaserAnimation(redlaserAnimation[frame], tileInd);
}

/**
 * @brief Construct a new Red Laser:: Red Laser object
 * 
 * @param x 
 * @param yInd 0-6 for each of the potential red lasers
 * @param ind 
 */
RedLaser::RedLaser(int x, int yIndex, u16 ind) {
    tileInd = ind;
    // calculate hitbox of where player will be when laser is activated
    hitbox = new Pill(
        x+(24*14)+53 /*just right of player*/, 
        (yIndex * 16) + 32, 
        x+(24*21)+21 /*just left*/, 
        (yIndex * 16) + 32, 
        4
    );
    rendered = false;
    erased = false;
    hazardIndex = 3;
    fixedX = x<<4;
    yInd = yIndex;

    //every red laser will use the same tiles so this is fine hard-coded
    LZ77UnCompVram(redLaserTilesTiles, tileset);
    buildLaser(yIndex, ind);
}

void RedLaser::update(int scrollX, Rectangle playerbounds) {
    //frame number
    int dx = ((scrollX - fixedX)>>4)/24;
    if(dx > 0 && dx < 29){
        updateLaser(dx, tileInd);
    }

}

void RedLaser::render() {
    rendered = true;
    erased = false;

}

void RedLaser::erase() {
    erased = true;
    rendered = false;
}