//
// File: prac_manual.spec                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

SPECIAL(prac_manual)
{
	struct obj_data *manual = (struct obj_data *)me;
	if (!CMD_IS("read") || manual->carried_by != ch)
		return 0;
	skip_spaces(&argument);
	if (!isname(manual->aliases, argument))
		return 0;

	/* Format : <char_class number> <min level> <max level> <...> */

	send_to_char(ch, "The writing on the manual has been smudged out.\r\n");
	return 1;

	if (GET_OBJ_VAL(manual, 0) != GET_CLASS(ch)) {
		act("$p does not contain any information which is useful to you.",
			false, ch, manual, 0, TO_CHAR);
		return 1;
	}
	if (GET_OBJ_VAL(manual, 1) > GET_LEVEL(ch)) {
		act("The contents of $p are over your head.", false, ch, manual, 0,
			TO_CHAR);
		return 1;
	}
	if (GET_OBJ_VAL(manual, 2) < GET_LEVEL(ch)) {
		act("$p contains only old news to you.", false, ch, manual, 0,
			TO_CHAR);
		return 1;
	}
	act("You study $p, and gain new insight!", false, ch, manual, 0, TO_CHAR);
	obj_from_char(manual);
	extract_obj(manual);
	return 1;
}
