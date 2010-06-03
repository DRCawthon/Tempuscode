#ifdef HAS_CONFIG_H
#include "config.h"
#endif

#include "mobile_map.h"
#include "utils.h"

MobileMap_MobileMap() : map<int, struct creature *>()
{
}

bool MobileMap_add(struct creature *ch)
{
    int vnum = 0;

    if (!ch || !ch->mob_specials.shared) {
        mudlog(LVL_GRGOD, NRM, true, "WARNING:  Attempt to add NULL mobile to MobileMap");
        return false;
    }

    if ((vnum = ch->mob_specials.shared->vnum) == 0) {
        mudlog(LVL_GRGOD, NRM, true, "WARNING:  Attempt to add mobile with vnum 0 to MobileMap");
        return false;
    }

    if (count(vnum) > 0) {
        mudlog(LVL_GRGOD, NRM, true, "WARNING:  MobileMap_add() tried to insert "
               "existing vnum [%d].", vnum);
        return false;
    }

    (*this)[vnum] = ch;

    return true;
}

bool MobileMap_remove(struct creature *ch)
{
    int vnum = 0;

    if (!ch || !ch->mob_specials.shared)
        return false;

    if ((vnum = ch->mob_specials.shared->vnum) == 0)
        return false;

    if (count(vnum) <= 0)
        return false;

    return erase(vnum);

}

bool MobileMap_remove(int vnum)
{
    return erase(vnum);
}

struct creature *MobileMap_find(int vnum)
{
    if (count(vnum) <= 0)
        return NULL;

    return (*this)[vnum];

}

MobileMap mobilePrototypes;
