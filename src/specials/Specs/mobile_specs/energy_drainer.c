//
// File: energy_drainer.spec                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

SPECIAL(energy_drainer)
{
	struct creature *vict;
	int loss;

	if (cmd)
		return false;

	if (spec_mode != SPECIAL_TICK)
		return false;

	if (GET_POSITION(ch) != POS_FIGHTING || !ch->fighting)
		return false;

    vict = random_opponent(ch);
	if (vict && (vict->in_room == ch->in_room) &&
		!number(0, 6)) {
		if (mag_savingthrow(vict, GET_LEVEL(ch), SAVING_PARA))
			damage(ch, vict, 0, SPELL_ENERGY_DRAIN, -1);
		else {
			damage(ch, vict, number(8, 30), SPELL_ENERGY_DRAIN, -1);
			loss = GET_EXP(vict) >> 5;
			GET_EXP(vict) = MAX(0, GET_EXP(vict) - loss);
			return true;
		}
	}
	return false;
}
