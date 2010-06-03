//
// File: basher.spec                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

SPECIAL(basher)
{
	struct creature *vict = NULL;
	ACMD(do_bash);
	if (spec_mode != SPECIAL_TICK)
		return false;
	if (ch->getPosition() != POS_FIGHTING || !ch->isFighting())
		return 0;

	if (number(0, 81) > GET_LEVEL(ch))
		return 0;

    CombatDataList_iterator li = ch->getCombatList()->begin();
    for (; li != ch->getCombatList()->end(); ++li) {
        if (IS_MAGE(li->getOpponent()) && number(0, 1)) {
            vict = li->getOpponent();
            break;
        }
    }
	if (vict == NULL)
		vict = ch->findRandomCombat();

	do_bash(ch, tmp_strdup(""), 0, 0, 0);
	return 1;
}
