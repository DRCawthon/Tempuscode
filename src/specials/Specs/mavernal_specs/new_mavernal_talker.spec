//
// File: new_mavernal_talker.spec                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

SPECIAL(new_mavernal_talker)
{
  char tmpstr[80];
  struct char_data *i = NULL;

  if (cmd || FIGHTING(ch) || !AWAKE(ch))
    return 0;
  
  switch (number(0, 40)) {
  case 0:
    do_say(ch, "New Mavernal is so cool!  This carnival rocks!", 0, 0);
    return (1);
  case 1:
    for (i = ch->in_room->people; i; i = i->next_in_room)
      if (!IS_NPC(i) && i != ch && CAN_SEE(ch, i)) {
	act("$n pats you on the back.", FALSE, ch, 0, i, TO_VICT);
	act("$n pats $N on the back.", FALSE, ch, 0, i, TO_NOTVICT);
        sprintf(tmpstr, "%s, you're pretty cool.", GET_NAME(i));
        do_say(ch, tmpstr, 0, 0);
        break;
      }
    act("$n smiles happily.", FALSE, ch, 0, 0, TO_ROOM);
    return (1);
  case 2:
    if(GET_POS(ch) == POS_STANDING) {
      do_say(ch, "I really need a break from all this standing.", 0, 0);
      act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_RESTING;
    } else {
      do_say(ch, "Well, I'm tired of resting, I think I'll stand.", 0, 0);
      act("$n stops resting and clambers to $s feet.", FALSE, ch, 0, 0,
          TO_ROOM);
      GET_POS(ch) = POS_STANDING;
    }
    act("$n smiles happily.", FALSE, ch, 0, 0, TO_ROOM);
    return (1);
  case 3:
    do_say(ch, "Ahhhhh nothing like a carnival, I love it here!", 0, 0);
    act("$n grins like a maniac.", FALSE, ch, 0, 0, TO_ROOM);
    return (1);
  case 4:
    do_say(ch, "Did you see the house of horrors yet?  It rules!", 0, 0);
    act("$n cackles like an insane fool!", FALSE, ch, 0, 0, TO_ROOM);
    return (1);
  case 5:
    do_say(ch, "I know what you are thinking!  Stop looking at me like that!", 0, 0);
    return (1);
  case 6:
    do_say(ch, "Have you been to the Tunnel of Love?", 0 , 0);
    do_say(ch, "It's soooo romantic!", 0, 0);
    act("$n kneels down on one leg and sings a sweet love song.", 
	FALSE, ch, 0, 0, TO_ROOM);
    return (1);
  default:
    return (0);
  }
  return (0);
}

