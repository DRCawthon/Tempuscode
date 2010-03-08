//
// File: cyberfiend.spec                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

SPECIAL(cyberfiend)
{

	if (spec_mode != SPECIAL_CMD)
		return 0;
	struct Creature *fiend = (struct Creature *)me;

	if (!cmd || !AWAKE(fiend))
		return 0;

	if (CMD_IS("enter") || CMD_IS("hop")) {

		act("$n levels you with a terrible blow to the head!!",
			false, fiend, 0, ch, TO_VICT);
		act("$n levels $N with a terrible blow to the head!!",
			false, fiend, 0, ch, TO_NOTVICT);

		ch->setPosition(POS_SITTING);
		WAIT_STATE(ch, 4 RL_SEC);
		return 1;
	}
	return 0;
}
