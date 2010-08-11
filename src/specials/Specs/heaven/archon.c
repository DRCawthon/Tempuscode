//
// File: archon.spec                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

SPECIAL(archon)
{

	struct room_data *room = real_room(43252);

	if (spec_mode != SPECIAL_CMD && spec_mode != SPECIAL_TICK)
		return 0;
	if (cmd)
		return 0;
	if (!ch->fighting && ch->in_room->zone->plane != PLANE_HEAVEN) {
		struct creatureList_iterator it = ch->in_room->people.begin();
		for (; it != ch->in_room->people.end(); ++it)
			if ((*it) != ch && IS_ARCHON((*it)) && (*it)->isFighting()) {
				do_rescue(ch, fname((*it)->player.name), 0, 0, 0);
				return 1;
			}

		act("$n disappears in a flash of light.", false, ch, 0, 0, TO_ROOM);
		if (room) {
			char_from_room(ch, false);
			char_to_room(ch, room, false);
			act("$n appears at the center of the room.", false, ch, 0, 0,
				TO_ROOM);
		} else {
			ch->purge(true);
		}
		return 1;
	}
	return 0;
}
