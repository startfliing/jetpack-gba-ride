#include "background.hpp"

#include "zone1.h"
#include "zone2.h"
#include "zone3.h"


backgroundEntry zone1 = {
    1,
    (SCREENBLOCK*)zone1Map
};

backgroundEntry zone2 = {
    1,
    (SCREENBLOCK*)zone2Map
};

backgroundEntry zone3 = {
    3,
    (SCREENBLOCK*)zone3Map
};

backgroundEntry entries[] = {
    zone1,
    zone2,
    zone3
};
