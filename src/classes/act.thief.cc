//
// File: act.thief.c                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

//
// act.thief.c
//

#define __act_thief_c__

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
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
#include "constants.h"
#include "events.h"

SPECIAL(shop_keeper);
int check_mob_reaction(struct Creature *ch, struct Creature *vict);
void send_to_queue(MobileEvent * e);

ACMD(do_steal)
{
	struct Creature *vict = NULL;
	struct obj_data *obj;
	char vict_name[MAX_INPUT_LENGTH];
	char obj_name[MAX_INPUT_LENGTH];
	int percent, gold, eq_pos, pcsteal = 0;
	bool ohoh = false;

	argument = one_argument(argument, obj_name);
	one_argument(argument, vict_name);

	if (!(vict = get_char_room_vis(ch, vict_name))) {
		send_to_char(ch, "Steal what from who?\r\n");
		return;
	} else if (vict == ch) {
		send_to_char(ch, "Come on now, that's rather stupid!\r\n");
		return;
	}
	if (GET_LEVEL(ch) < LVL_IMMORT && IS_NPC(vict)
		&& MOB2_FLAGGED(vict, MOB2_SELLER)) {
		send_to_char(ch, "That's probably a bad idea.\r\n");
		return;
	}

	if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_PEACEFUL) &&
		!PLR_FLAGGED(vict, PLR_THIEF) && GET_LEVEL(ch) < LVL_AMBASSADOR) {
		send_to_char(ch, 
			"The universal forces of order prevent those acts here.\r\n");
		act("$n looks kinda sketchy for a moment.", FALSE, ch, 0, vict,
			TO_ROOM);
		return;
	}
	if (vict->isNewbie() && GET_LEVEL(ch) < LVL_IMMORT) {
		send_to_char(ch, "You cannot steal from newbies!\r\n");
		return;
	}
	if (!IS_MOB(vict) && !vict->desc && GET_LEVEL(ch) < LVL_ELEMENT) {
		send_to_char(ch, "You cannot steal from linkless players!!!\r\n");
		mudlog(GET_LEVEL(ch), CMP, true,
			"%s attempted to steal from linkless %s.", GET_NAME(ch),
			GET_NAME(vict));
		return;
	}
	if (!IS_MOB(vict) && ch->isNewbie()) {
		send_to_char(ch, "You can't steal from players. You're a newbie!\r\n");
		return;
	}
	if ((GET_LEVEL(vict) + 5) < GET_LEVEL(ch) && !IS_MOB(vict) &&
		!PLR_FLAGGED(vict, PLR_THIEF) && !PLR_FLAGGED(vict, PLR_KILLER) &&
		!PLR_FLAGGED(vict, PLR_TOUGHGUY) &&
		!ZONE_FLAGGED(ch->in_room->zone, ZONE_NOLAW) &&
		!PLR_FLAGGED(ch, PLR_THIEF) && GET_LEVEL(ch) < LVL_AMBASSADOR) {
		send_to_char(ch, "Okay... You will now be a THIEF!\r\n");
		SET_BIT(PLR_FLAGS(ch), PLR_THIEF);
		mudlog(MAX(GET_INVIS_LEV(ch), GET_INVIS_LEV(vict)), NRM, true,
			"PC THIEF bit set on %s for robbing %s.", GET_NAME(ch),
			GET_NAME(vict));
	}


	if (!IS_NPC(ch) && !IS_NPC(vict) &&
		!PLR_FLAGGED(vict, PLR_KILLER | PLR_THIEF) &&
		(!PLR_FLAGGED(ch, PLR_TOUGHGUY) ||
			!PLR_FLAGGED(ch, PLR_REMORT_TOUGHGUY)))
		check_toughguy(ch, vict, 1);

	/* 101% is a complete failure */
	percent = number(1, 101) - dex_app_skill[GET_DEX(ch)].p_pocket;

	if (CHECK_SKILL(ch, SKILL_STEAL) < 50)
		percent >>= 1;

	if (vict->getPosition() < POS_SLEEPING)
		percent = -15;			/* ALWAYS SUCCESS */

	if (AFF3_FLAGGED(vict, AFF3_ATTRACTION_FIELD))
		percent = 121;

	/* NO NO With Imp's and Shopkeepers! */
	if ((GET_LEVEL(vict) >= LVL_AMBASSADOR) || pcsteal ||
		GET_MOB_SPEC(vict) == shop_keeper)
		percent = 121;			/* Failure */

	if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold")) {

		if (!(obj = get_obj_in_list_vis(ch, obj_name, vict->carrying))) {

			for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
				if (GET_EQ(vict, eq_pos) &&
					(isname(obj_name, GET_EQ(vict, eq_pos)->name)) &&
					CAN_SEE_OBJ(ch, GET_EQ(vict, eq_pos))) {
					obj = GET_EQ(vict, eq_pos);
					break;
				}
			if (!obj) {
				act("$E hasn't got that item.", FALSE, ch, 0, vict, TO_CHAR);
				return;
			} else {			/* It is equipment */
				percent += obj->getWeight();	/* Make heavy harder */

				if (vict->getPosition() > POS_SLEEPING) {
					send_to_char(ch, "Steal the equipment now?  Impossible!\r\n");
					return;
				} else {
					percent += 20 * (eq_pos == WEAR_WIELD);
					if (IS_OBJ_STAT(obj, ITEM_NODROP))
						percent += 20;
					if (IS_OBJ_STAT2(obj, ITEM2_CURSED_PERM))
						percent += 20;
					if (IS_OBJ_STAT2(obj, ITEM2_NOREMOVE))
						percent += 30;
					if (GET_LEVEL(ch) > LVL_TIMEGOD &&
						GET_LEVEL(vict) < GET_LEVEL(ch))
						percent = 0;
					if (percent < CHECK_SKILL(ch, SKILL_STEAL)) {
						act("You unequip $p and steal it.", FALSE, ch, obj, 0,
							TO_CHAR);
						act("$n steals $p from $N.", FALSE, ch, obj, vict,
							TO_NOTVICT);
						obj_to_char(unequip_char(vict, eq_pos, MODE_EQ), ch);
						GET_EXP(ch) += MIN(1000, GET_OBJ_COST(obj));
						gain_skill_prof(ch, SKILL_STEAL);
						if (GET_LEVEL(ch) >= LVL_AMBASSADOR || !IS_NPC(vict)) {
							slog("%s stole %s from %s.",
								GET_NAME(ch), obj->short_description,
								GET_NAME(vict));
						}
					} else {
						if (vict->getPosition() == POS_SLEEPING) {
							act("You wake $N up trying to steal it!",
								FALSE, ch, 0, vict, TO_CHAR);
							send_to_char(vict, 
								"You are awakened as someone tries to steal your equipment!\r\n");
							vict->setPosition(POS_RESTING);
							ohoh = true;
						} else if (vict->getPosition() == POS_SITTING &&
							IS_AFFECTED_2(vict, AFF2_MEDITATE)) {

							act("You disturb $M in your clumsy attempt.",
								FALSE, ch, 0, vict, TO_CHAR);
							act("You are disturbed as $n attempts to pilfer your inventory.", FALSE, ch, 0, vict, TO_VICT);
							REMOVE_BIT(AFF2_FLAGS(vict), AFF2_MEDITATE);
						} else
							send_to_char(ch, "You fail to get it.\r\n");
					}
					WAIT_STATE(ch, PULSE_VIOLENCE);
				}
			}
			/*
			   if (!IS_NPC(ch) && !IS_NPC(vict) && !PLR_FLAGGED(vict, PLR_TOUGHGUY)
			   && !PLR_FLAGGED(vict,PLR_THIEF) && !PLR_FLAGGED(ch,PLR_TOUGHGUY)){
			   SET_BIT(PLR_FLAGS(ch), PLR_TOUGHGUY);
			   sprintf(buf, "PC Toughguy bit set on %s for robbing %s at %s.",
			   GET_NAME(ch), GET_NAME(vict), vict->in_room->name);
			   mudlog(buf, BRF, LVL_AMBASSADOR, TRUE);
			   }
			 */
		} else {				/* obj found in inventory */

			percent += obj->getWeight();	/* Make heavy harder */
			if (IS_OBJ_STAT(obj, ITEM_NODROP))
				percent += 30;
			if (IS_OBJ_STAT2(obj, ITEM2_CURSED_PERM))
				percent += 40;

			if (AWAKE(vict) && (percent > CHECK_SKILL(ch, SKILL_STEAL)) &&
				(GET_LEVEL(ch) < LVL_AMBASSADOR ||
					GET_LEVEL(ch) < GET_LEVEL(vict))) {
				ohoh = true;
				act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
				act("You catch $n trying to steal something from you!",
					FALSE, ch, 0, vict, TO_VICT);
				act("$N catches $n trying to steal something from $M.",
					TRUE, ch, 0, vict, TO_NOTVICT);
			} else {			/* Steal the item */
				if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
					if ((IS_CARRYING_W(ch) + obj->getWeight()) <
						CAN_CARRY_W(ch)) {
						obj_from_char(obj);
						obj_to_char(obj, ch);
						send_to_char(ch, "Got it!\r\n");
						GET_EXP(ch) += MIN(100, GET_OBJ_COST(obj));
						WAIT_STATE(ch, PULSE_VIOLENCE);
						gain_skill_prof(ch, SKILL_STEAL);
						if (GET_LEVEL(ch) >= LVL_AMBASSADOR || !IS_NPC(vict)) {
							slog("%s stole %s from %s.",
								GET_NAME(ch), obj->short_description,
								GET_NAME(vict));
						}
					} else
						send_to_char(ch, "You cannot carry that much weight.\r\n");
				} else
					send_to_char(ch, "You cannot carry that much.\r\n");
			}
		}

	} else {					/* Steal some coins */
		if (AWAKE(vict) && (percent > CHECK_SKILL(ch, SKILL_STEAL)) &&
			GET_LEVEL(ch) < LVL_IMPL) {
			ohoh = true;
			act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
			act("You discover that $n has $s hands in your wallet.", FALSE, ch,
				0, vict, TO_VICT);
			act("$n tries to steal gold from $N.", TRUE, ch, 0, vict,
				TO_NOTVICT);
		} else {
			/* Steal some gold coins */
			gold = (int)((GET_GOLD(vict) * number(1, 10)) / 100);
			gold = MIN(1782, gold);
			if (gold > 0) {
				GET_GOLD(ch) += gold;
				GET_GOLD(vict) -= gold;
				send_to_char(ch, "Bingo!  You got %d gold coins.\r\n", gold);
				if (GET_LEVEL(ch) >= LVL_AMBASSADOR) {
					slog("%s stole %d coins from %s.", GET_NAME(ch),
						gold, GET_NAME(vict));
				}
			} else {
				send_to_char(ch, "You couldn't get any gold...\r\n");
			}
		}
	}

	// Drop remort vis upon steal failure
	if (ohoh && !IS_NPC(vict) && !IS_NPC(ch) &&
		GET_REMORT_GEN(ch) > GET_REMORT_GEN(vict) &&
		GET_REMORT_INVIS(ch) > GET_LEVEL(vict)) {
		GET_REMORT_INVIS(ch) = GET_LEVEL(vict);
		send_to_char(ch, "You feel a bit more visible.\n");
	}

	if (ohoh && IS_NPC(vict) && AWAKE(vict) && check_mob_reaction(ch, vict))
		hit(vict, ch, TYPE_UNDEFINED);
	if (ohoh && IS_NPC(vict) && AWAKE(vict)
		&& IS_SET(MOB_FLAGS(vict), MOB_ISCRIPT)) {
		MobileEvent *e = new MobileEvent(ch, vict, 0, 0, 0, 0, "EVT_STEAL");
		send_to_queue(e);
	}
}

ACMD(do_backstab)
{
	struct Creature *vict;
	int percent, prob;
	struct obj_data *weap = NULL;

	ACMD_set_return_flags(0);

	one_argument(argument, buf);

	if (!(vict = get_char_room_vis(ch, buf))) {
		send_to_char(ch, "Backstab who?\r\n");
		WAIT_STATE(ch, 4);
		return;
	}
	if (vict == ch) {
		send_to_char(ch, "How can you sneak up on yourself?\r\n");
		return;
	}
	if (!peaceful_room_ok(ch, vict, true))
		return;

	if (!(((weap = GET_EQ(ch, WEAR_WIELD)) && STAB_WEAPON(weap)) ||
			((weap = GET_EQ(ch, WEAR_WIELD_2)) && STAB_WEAPON(weap)) ||
			((weap = GET_EQ(ch, WEAR_HANDS)) && STAB_WEAPON(weap)))) {
		send_to_char(ch, "You need to be using a stabbing weapon.\r\n");
		return;
	}
	if (FIGHTING(vict)) {
		send_to_char(ch, "Backstab a fighting person? -- they're too alert!\r\n");
		return;
	}

	percent = number(1, 101) + GET_INT(vict);

	prob = CHECK_SKILL(ch, SKILL_BACKSTAB) + (!CAN_SEE(vict, ch) ? (32) : 0) +
		(IS_AFFECTED(ch, AFF_SNEAK) ? number(10, 25) : (-5)) +
		dex_app_skill[GET_DEX(ch)].sneak;

	cur_weap = weap;
	if (AWAKE(vict) && (percent > prob)) {
		WAIT_STATE(ch, 2 RL_SEC);
		int retval = damage(ch, vict, 0, SKILL_BACKSTAB, WEAR_BACK);
		ACMD_set_return_flags(retval);
	}

	else {
		WAIT_STATE(vict, 1 RL_SEC);
		WAIT_STATE(ch, 4 RL_SEC);
		int retval = hit(ch, vict, SKILL_BACKSTAB);
		ACMD_set_return_flags(retval);
	}
}

ACMD(do_circle)
{
	struct Creature *vict;
	int percent, prob;
	struct obj_data *weap = NULL;

	ACMD_set_return_flags(0);

	one_argument(argument, buf);

	if (!(vict = get_char_room_vis(ch, buf)) && !(vict = FIGHTING(ch))) {
		send_to_char(ch, "Circle around who?\r\n");
		WAIT_STATE(ch, 4);
		return;
	}
	if (vict == ch) {
		send_to_char(ch, "How can you sneak up on yourself?\r\n");
		return;
	}
	if (!peaceful_room_ok(ch, vict, true))
		return;

	if (!(((weap = GET_EQ(ch, WEAR_WIELD)) && STAB_WEAPON(weap)) ||
			((weap = GET_EQ(ch, WEAR_WIELD_2)) && STAB_WEAPON(weap)) ||
			((weap = GET_EQ(ch, WEAR_HANDS)) && STAB_WEAPON(weap)))) {
		send_to_char(ch, "You need to be using a stabbing weapon.\r\n");
		return;
	}
	if (FIGHTING(vict) && FIGHTING(vict) == ch) {
		send_to_char(ch, 
			"You can't circle someone who is actively fighting you!\r\n");
		return;
	}

	percent = number(1, 101) + GET_INT(vict);	/* 101% is a complete failure */
	prob = CHECK_SKILL(ch, SKILL_CIRCLE) +
		number(0, 20) * (IS_AFFECTED(ch, AFF_SNEAK));
	if (FIGHTING(ch))
		prob -= number(20, 30);
	prob += 20 * CAN_SEE(vict, ch);

	cur_weap = weap;
	if (percent > prob) {
		WAIT_STATE(ch, 2 RL_SEC);
		int retval = damage(ch, vict, 0, SKILL_CIRCLE, WEAR_BACK);
		ACMD_set_return_flags(retval);
	}

	else {
		gain_skill_prof(ch, SKILL_CIRCLE);
		WAIT_STATE(ch, 5 RL_SEC);
		int retval = hit(ch, vict, SKILL_CIRCLE);

		if (retval)
			return;

		//
		// possibly make vict start attacking ch
		//

		if ((number(1, 40) + GET_LEVEL(vict)) > CHECK_SKILL(ch, SKILL_CIRCLE)) {
			stop_fighting(vict);
			set_fighting(vict, ch, FALSE);
		}
	}
}


ACMD(do_sneak)
{
	struct affected_type af;

	if (IS_AFFECTED(ch, AFF_SNEAK)) {
		send_to_char(ch, "Okay, you will now attempt to walk normally.\r\n");
		affect_from_char(ch, SKILL_SNEAK);
		return;
	}

	send_to_char(ch, "Okay, you'll try to move silently until further notice.\r\n");

	if (CHECK_SKILL(ch, SKILL_SNEAK) < number(20, 70))
		return;

	af.type = SKILL_SNEAK;
	af.is_instant = 0;
	af.duration = GET_LEVEL(ch);
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_SNEAK;
	af.aff_index = 0;
	af.level = GET_LEVEL(ch) + GET_REMORT_GEN(ch);

	affect_to_char(ch, &af);

}

ACMD(do_hide)
{
	byte percent;

	send_to_char(ch, "You attempt to hide yourself.\r\n");

	if (IS_AFFECTED(ch, AFF_HIDE))
		REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);

	percent = number(1, 101);	/* 101% is a complete failure */
	if (IS_WEARING_W(ch) > (CAN_CARRY_W(ch) * 0.75)
		&& GET_LEVEL(ch) < LVL_AMBASSADOR) {
		send_to_char(ch, 
			"...but it sure will be hard with all that heavy equipment.\r\n");
		percent += 30;
	}
	if (IS_AFFECTED(ch, AFF_SANCTUARY) || IS_AFFECTED(ch, AFF_GLOWLIGHT) ||
		IS_AFFECTED_2(ch, AFF2_DIVINE_ILLUMINATION) ||
		affected_by_spell(ch, SPELL_QUAD_DAMAGE) ||
		IS_AFFECTED_2(ch, AFF2_FIRE_SHIELD))
		percent += 60;
	if (IS_DARK(ch->in_room))
		percent -= 30;
	else
		percent += 20;

	if (percent > CHECK_SKILL(ch,
			SKILL_HIDE) + dex_app_skill[GET_DEX(ch)].hide)
		return;

	SET_BIT(AFF_FLAGS(ch), AFF_HIDE);
	gain_skill_prof(ch, SKILL_HIDE);
}

ACMD(do_disguise)
{
	struct Creature *vict = NULL;
	struct affected_type af;

	if (CHECK_SKILL(ch, SKILL_DISGUISE) < 20) {
		send_to_char(ch, "You do not know how.\r\n");
		return;
	}
	if (subcmd) {				// undisguise
		affect_from_char(ch, SKILL_DISGUISE);
		send_to_char(ch, "You are no longer disguised.\r\n");
		return;
	}

	skip_spaces(&argument);
	if (!*argument) {
		send_to_char(ch, "Disguise yourself as what mob?\r\n");
		return;
	}
	if (!(vict = get_char_room_vis(ch, argument))) {
		send_to_char(ch, "No-one by the name of '%s' here.\r\n", argument);
		return;
	}
	if (!IS_NPC(vict)) {
		send_to_char(ch, "You can't disguise yourself as other players.\r\n");
		return;
	}
	if (!HUMANOID_TYPE(vict)) {
		send_to_char(ch, "You can only disguise yourself as other humanoids.\r\n");
		return;
	}
	if (GET_HEIGHT(vict) > (GET_HEIGHT(ch) * 1.25) ||
		GET_HEIGHT(vict) < (GET_HEIGHT(ch) * 0.75) ||
		GET_WEIGHT(vict) > (GET_WEIGHT(ch) * 1.25) ||
		GET_WEIGHT(vict) < (GET_WEIGHT(ch) * 0.75)) {
		act("Your body size is not similar enough to $N's.",
			FALSE, ch, 0, vict, TO_CHAR);
		return;
	}
	if (GET_LEVEL(vict) > GET_LEVEL(ch) + GET_REMORT_GEN(ch)) {
		act("You are too puny to pass as $N.", FALSE, ch, 0, vict, TO_CHAR);
		return;
	}
	if (GET_MOVE(ch) < GET_LEVEL(vict)) {
		send_to_char(ch, "You don't have enough movement.\r\n");
		return;
	}
	affect_from_char(ch, SKILL_DISGUISE);

	GET_MOVE(ch) -= GET_LEVEL(vict);
	WAIT_STATE(ch, 5 RL_SEC);
	if (number(0, 120) > CHECK_SKILL(ch, SKILL_DISGUISE)) {
		send_to_char(ch, "You fail.\r\n");
		return;
	}
	af.type = SKILL_DISGUISE;
	af.is_instant = 0;
	af.duration = GET_LEVEL(ch) + GET_REMORT_GEN(ch) + GET_INT(ch);
	af.modifier = GET_MOB_VNUM(vict);
	af.location = APPLY_DISGUISE;
	af.level = GET_LEVEL(ch) + GET_REMORT_GEN(ch);
	af.bitvector = 0;
	af.aff_index = 0;

	act("$n disguises $mself as $N.", TRUE, ch, 0, vict, TO_ROOM);
	affect_to_char(ch, &af);
	act("You are now disguised as $N.", FALSE, ch, 0, vict, TO_CHAR);
	gain_skill_prof(ch, SKILL_DISGUISE);
}

#undef __act_thief_c__
