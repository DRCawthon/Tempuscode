//
// File: newbie_healer.spec                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

SPECIAL(newbie_healer)
{
  ACMD(do_drop);
  struct char_data *i, *next_i = NULL;
  struct obj_data *p;

  if (cmd)
    return 0;

  for (i= ch->in_room->people; i; i = next_i) {
    next_i = i->next_in_room;
    if (i == ch)
      continue;
    if (IS_NPC(i)) {
      act("$n banishes $N!", FALSE, ch, 0, i, TO_ROOM);
      extract_char(i, 0);
      continue;
    }
    if (!IS_NPC(i) && GET_LEVEL(i) < 5 && !number(0, GET_LEVEL(i))) {
      if (GET_HIT(i) < GET_MAX_HIT(i))
        cast_spell(ch, i, 0, SPELL_CURE_CRITIC);
      else if (IS_AFFECTED(i, AFF_POISON))
        cast_spell(ch, i, 0, SPELL_REMOVE_POISON);
      else if (!affected_by_spell(i, SPELL_BLESS))
        cast_spell(ch, i, 0, SPELL_BLESS);
      else if (!affected_by_spell(i, SPELL_ARMOR))
        cast_spell(ch, i, 0, SPELL_ARMOR);
      else if (!affected_by_spell(i, SPELL_DETECT_MAGIC))
        cast_spell(ch, i, 0, SPELL_DETECT_MAGIC);
      else
        return 0;
      return 1;
    }
  }
  for (p = ch->carrying; p; p= p->next_content) {
    act("$p.", FALSE, ch, p, 0, TO_CHAR);
    if (GET_OBJ_TYPE(p) == ITEM_WORN)
      cast_spell(ch, 0, p, SPELL_MAGICAL_VESTMENT);
    else
      send_to_char("No WEAR.\r\n", ch);
    do_drop(ch, fname(p->name), 0, 0);
    return 1;
  }
  return 0;
}
  


