//
// File: fire_breather.spec                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

SPECIAL(fire_breather)
{

	if (cmd)
		return false;
	if (spec_mode != SPECIAL_TICK)
		return false;
	if (ch->getPosition() != POS_FIGHTING || !ch->isFighting())
		return false;

    Creature *vict = ch->findRandomCombat();
	if (vict && (vict->in_room == ch->in_room) &&
		!number(0, 4)) {
		if (mag_savingthrow(vict, GET_LEVEL(ch), SAVING_BREATH))
			damage(ch, vict, 0, SPELL_FIRE_BREATH, -1);
		else
			damage(ch, vict, GET_LEVEL(ch) + number(8, 30),
				SPELL_FIRE_BREATH, -1);
		return true;
	}
	return false;
}
