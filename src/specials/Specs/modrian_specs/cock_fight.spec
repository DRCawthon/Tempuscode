//
// File: cock_fight.spec                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

SPECIAL(cock_fight)
{
	if (spec_mode != SPECIAL_TICK)
		return 0;

	if (!ch->isFighting()) {
		switch (number(0, 40)) {
		case 0:
			act("$n scratches the ground with $s claw.", true, ch, 0, 0,
				TO_ROOM);
			send_to_char(ch, "You scratch.\r\n");
			return 1;
		case 1:
			act("$n squawks loudly!", false, ch, 0, 0, TO_ROOM);
			send_to_char(ch, "You squawk!\r\n");
			return 1;
		case 2:
			act("$n struts around proudly.", true, ch, 0, 0, TO_ROOM);
			send_to_char(ch, "You strut.\r\n");
			return 1;
		default:
			return 0;
		}
	}
    Creature *target = ch->findRandomCombat();
	switch (number(0, 16)) {
	case 0:
		act("$n leaps into the air, feathers flying!", true, ch, 0, 0,
			TO_ROOM);
		send_to_char(ch, "You leap.\r\n");
		return 1;
	case 1:
		send_to_room("Feathers fly everywhere!\r\n", ch->in_room);
		return 1;
	case 2:
		act("$N screams as $E attacks you!", false, target, 0, ch,
			TO_CHAR);
		act("$N screams as $E attacks $n!", false, target, 0, ch,
			TO_ROOM);
		return 1;
	default:
		return 0;
	}
	return 0;
}
