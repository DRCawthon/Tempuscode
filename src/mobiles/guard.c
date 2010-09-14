#ifdef HAS_CONFIG_H
#include "config.h"
#endif

#include "actions.h"
#include "db.h"
#include "comm.h"
#include "fight.h"
#include "handler.h"
#include "interpreter.h"
#include "tmpstr.h"
#include "screen.h"
#include "utils.h"

void call_for_help(struct creature *, struct creature *);

SPECIAL(guard)
{
	struct creature *self = (struct creature *)me;
	int cmd_idx, lineno, dir = -1;
	struct reaction *reaction = make_reaction();
	const char *to_vict = "You are blocked by $n.";
	const char *to_room = "$N is blocked by $n.";
	char *str, *line, *param_key, *dir_str, *room_str;
	bool attack = false, fallible = false, callsforhelp = false;
	const char *err = NULL;
	long room_num = -1;

	// we only handle ticks if we're fighting, and commands only if they're
	// movement commands
	if (!GET_MOB_PARAM(self)
			|| (spec_mode != SPECIAL_TICK && spec_mode != SPECIAL_CMD)
			|| (spec_mode == SPECIAL_TICK && !self->fighting)
			|| (spec_mode == SPECIAL_CMD && !IS_MOVE(cmd)))
		return 0;

	str = GET_MOB_PARAM(self);
	for (line = tmp_getline(&str), lineno = 1; line;
			line = tmp_getline(&str), lineno++) {

		if (add_reaction(reaction, line))
			continue;

		param_key = tmp_getword(&line);
		if (!strcmp(param_key, "guard")) {
			dir_str = tmp_getword(&line);
			room_str = tmp_getword(&line);
			room_num = 0;
			if (*room_str) {
				// Ignore directions that don't pertain to the room
				room_num = atoi(room_str);
				if (!room_num || !real_room(room_num)) {
					err = "a bad room number";
					break;
				}
			}
			cmd_idx = find_command(dir_str);
			if (cmd_idx == -1 || !IS_MOVE(cmd_idx)) {
				err = "a bad direction";
				break;
			}
			if (room_num != ch->in_room->number)
				continue;
			if (cmd_idx == cmd)
				dir = cmd_idx;
		} else if (!strcmp(param_key, "tovict")) {
			to_vict = line;
		} else if (!strcmp(param_key, "toroom")) {
			to_room = line;
		} else if (!strcmp(param_key, "attack")) {
			attack = (is_abbrev(line, "yes") || is_abbrev(line, "on") ||
				is_abbrev(line, "1") || is_abbrev(line, "true"));
		} else if (!strcmp(param_key, "fallible")) {
			fallible = (is_abbrev(line, "yes") || is_abbrev(line, "on") ||
				is_abbrev(line, "1") || is_abbrev(line, "true"));
		} else if (!strcmp(param_key, "callsforhelp")) {
			callsforhelp = (is_abbrev(line, "yes") || is_abbrev(line, "on") ||
				is_abbrev(line, "1") || is_abbrev(line, "true"));
		} else {
			err = "an invalid directive";
			break;
		}
	}

	if (spec_mode == SPECIAL_TICK) {
		if (callsforhelp && !number(0, 10) && self->fighting) {
			call_for_help(self, random_opponent(self));
            free_reaction(reaction);
			return true;
		}
        free_reaction(reaction);
		return false;
	}

	if (dir == -1) {
        free_reaction(reaction);
		return false;
    }

	if (err) {
		// Specparam error
		if (IS_PC(ch)) {
			if (IS_IMMORT(ch))
				perform_tell(self, ch, tmp_sprintf(
					"I have %s in line %d of my specparam", err, lineno));
			else {
				mudlog(LVL_IMMORT, NRM, true,
					"ERR: Mobile %d has %s in line %d of specparam",
					GET_MOB_VNUM(self), err, lineno);
                perform_say_to(self, ch, "Sorry.  I'm broken, but a god has already been notified.");
			}
		}
	} else if (ch == self || IS_IMMORT(ch) || ALLOW == react(reaction, ch)) {
        free_reaction(reaction);
		return false;
    }

    // We don't need the reaction anymore, so we discard it here
    free_reaction(reaction);

	// If we're a fallible guard, check to see if they can get past us
	if (fallible && check_sneak(ch, self, true, true) == SNEAK_OK)
		return false;

	// Guards must be at least standing to be able to block people
	if (GET_POSITION(self) <= POS_SITTING)
		return false;

	// Petrified guards can't do much
	if (AFF2_FLAGGED(ch, AFF2_PETRIFIED))
		return false;

	// Set to deny if undecided
	act(to_vict, false, self, 0, ch, TO_VICT);
	act(to_room, false, self, 0, ch, TO_NOTVICT);
	if (!err
			&& attack
			&& !self->fighting
			&& IS_PC(ch)
			&& !PRF_FLAGGED(ch, PRF_NOHASSLE)) {
        add_combat(self, ch, false);
        add_combat(ch, self, false);
    }

	WAIT_STATE(ch, 1 RL_SEC);
	return true;
}
