//
// File: life_bldg_guard.spec                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

SPECIAL(life_bldg_guard)
{
	struct Creature *guard = (struct Creature *)me;
	if (spec_mode != SPECIAL_CMD && spec_mode != SPECIAL_TICK)
		return 0;
	if (cmd != EAST + 1 || GET_LEVEL(ch) > 25)
		return false;

	if (!can_see_creature(guard, ch) || !AWAKE(guard))
		return 0;

	act("$N snickers at $n and pushes $m back.", true, ch, 0, guard, TO_ROOM);
	act("$N snickers at you and pushes you back.", true, ch, 0, guard,
		TO_CHAR);
	return true;
}
