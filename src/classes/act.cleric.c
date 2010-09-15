//
// File: act.cleric.c                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

#ifdef HAS_CONFIG_H
#include "config.h"
#endif

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "vehicle.h"
#include "materials.h"
#include "flow_room.h"
#include "house.h"
#include "char_class.h"
#include "fight.h"

ASPELL(spell_dispel_evil)
{
    int dam = 0;
    int retval = 0;

    if (IS_EVIL(ch) && GET_LEVEL(ch) < LVL_IMMORT) {
        send_to_char(ch,
            "Your soul is not righteous enough to cast this spell.\n");
        return;
    }

    if (victim == ch) {
        send_to_char(ch,
            "That is not the way to the path of righteousness.\n");
        return;
    }

    if (victim) {
        if (!IS_NPC(ch) && !PRF2_FLAGGED(ch, PRF2_PKILLER)) {
            if (!IS_NPC(victim) && IS_EVIL(victim)) {
                act("You cannot do this because you have chosen not "
                    "to be a pkiller.\r\nYou can toggle this with the "
                    "command 'pkiller'.", false, ch, 0, victim, TO_CHAR);
                return;
            }
        }
        if (IS_EVIL(victim)) {
            dam = dice(10, 15) + skill_bonus(ch, SPELL_DISPEL_EVIL);
            retval = damage(ch, victim, dam, SPELL_DISPEL_EVIL, WEAR_RANDOM);
        }

        GET_ALIGNMENT(victim) += MAX(10,
            skill_bonus(ch, SPELL_DISPEL_EVIL) >> 2);

        if (!IS_SET(retval, DAM_ATTACKER_KILLED))
            WAIT_STATE(ch, 2 RL_SEC);
        if (!IS_SET(retval, DAM_VICT_KILLED))
            WAIT_STATE(victim, 1 RL_SEC);

        return;
    }

    if (obj) {
        if (!IS_OBJ_STAT(obj, ITEM_DAMNED)) {
            act("This item does not need to be cleansed.", false, ch, 0,
                NULL, TO_CHAR);
            return;
        }

        if (dice(30, (skill_bonus(ch, SPELL_DISPEL_EVIL) / 5)) < 50) {
            act("$p crumbles to dust as you attempt to cleanse it.", false,
                ch, obj, NULL, TO_CHAR);
            act("$p crumbles to dust as $n tries to cleanse it.", false,
                ch, obj, NULL, TO_NOTVICT);
            destroy_object(ch, obj, SPELL_DISPEL_EVIL);

            return;
        }

        REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_DAMNED);
        act("$p has been cleansed of evil!", false, ch, obj, NULL, TO_CHAR);

        return;
    }

    return;
}

ASPELL(spell_dispel_good)
{
    int dam = 0;
    int retval = 0;

    if (IS_GOOD(ch) && GET_LEVEL(ch) < LVL_IMMORT) {
        send_to_char(ch,
            "Your soul is not stained enough to cast this spell.\n");
        return;
    }

    if (victim == ch) {
        send_to_char(ch,
            "That is not the way to the path of unrighteousness.\n");
        return;
    }

    if (victim) {
        if (!IS_NPC(ch) && !PRF2_FLAGGED(ch, PRF2_PKILLER)) {
            if (!IS_NPC(victim) && IS_GOOD(victim)) {
                act("You cannot do this because you have chosen not "
                    "to be a pkiller.\r\nYou can toggle this with the "
                    "command 'pkiller'.", false, ch, 0, victim, TO_CHAR);
                return;
            }
        }

        if (IS_GOOD(victim)) {
            dam = dice(15, 20) + skill_bonus(ch, SPELL_DISPEL_GOOD);
            retval = damage(ch, victim, dam, SPELL_DISPEL_GOOD, WEAR_RANDOM);
        }

        GET_ALIGNMENT(victim) -= MAX(5,
            skill_bonus(ch, SPELL_DISPEL_GOOD) / 5);

        if (!IS_SET(retval, DAM_ATTACKER_KILLED))
            WAIT_STATE(ch, 2 RL_SEC);
        if (!IS_SET(retval, DAM_VICT_KILLED))
            WAIT_STATE(victim, 1 RL_SEC);

        return;
    }

    if (obj) {
        if (!IS_OBJ_STAT(obj, ITEM_BLESS)) {
            act("This item does not need to be defiled.", false, ch, 0,
                NULL, TO_CHAR);
            return;
        }

        if (dice(30, (skill_bonus(ch, SPELL_DISPEL_GOOD) / 5)) < 50) {
            act("$p crumbles to dust as you attempt to defile it.", false,
                ch, obj, NULL, TO_CHAR);
            act("$p crumbles to dust as $n tries to defile it.", false,
                ch, obj, NULL, TO_NOTVICT);
            destroy_object(ch, obj, SPELL_DISPEL_GOOD);

            return;
        }

        REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_BLESS);
        act("$p has been defiled!", false, ch, obj, NULL, TO_CHAR);

        return;
    }

    return;
}

#undef __act_cleric_c__
