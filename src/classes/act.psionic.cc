//
// File: act.psionic.c                     -- Part of TempusMUD
//
// Copyright 1998 by John Watson, all rights reserved.
//

//
// File: act.psionic.c -- psionic stuff
//
// Copyright 1998 by John Watson, all rights reserved
//

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "char_class.h"
#include "vehicle.h"
#include "materials.h"
#include "fight.h"

ACCMD(do_offensive_skill);

ACMD(do_psidrain) {

    struct char_data *vict = NULL;
    int dist, drain, prob;
    int find_distance(struct room_data *tmp, struct room_data *location);

    ACMD_set_return_flags( 0 );

    skip_spaces(&argument);
  
    if (CHECK_SKILL(ch, SKILL_PSIDRAIN) < 30 || !IS_PSIONIC(ch)) {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }

    if (!*argument && !(vict = FIGHTING(ch))) {
        send_to_char("Psidrain who?\r\n", ch);
        return;
    }

    if (vict == ch) {
        send_to_char("Ha ha... Funny!\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_NOPSIONICS) && GET_LEVEL(ch) < LVL_GOD) {
        send_to_char("Psychic powers are useless here!\r\n", ch);
        return;
    }

    if (*argument) {
        if (!(vict = get_char_room_vis(ch, argument)) &&
            !(vict = get_char_vis(ch, argument))) {
            sprintf(buf, "You cannot locate %s '%s'.\r\n", AN(argument), argument);
            send_to_char(buf, ch);
            return;
        }
    }
    if (!vict)
        return;

    if (FIGHTING(ch) && vict->in_room != ch->in_room) {
        send_to_char("You cannot focus outside the room during battle!\r\n", ch);
        return;
    }
    
    if (ch->in_room != vict->in_room && 
        ch->in_room->zone != vict->in_room->zone) {
        act("$N is not in your zone.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }

    if (ROOM_FLAGGED(vict->in_room,ROOM_PEACEFUL)&&GET_LEVEL(ch) < LVL_GOD) {
        act("Your psychic powers cannot hurt $M there!",FALSE,ch,0,vict,TO_CHAR);
        return;
    }
    if (ROOM_FLAGGED(vict->in_room,ROOM_NOPSIONICS)&&GET_LEVEL(ch) < LVL_GOD) {
        act("Psychic powers are useless where $E is!",FALSE,ch,0,vict,TO_CHAR);
        return;
    }

    if (!peaceful_room_ok(ch, vict, true))
        return;

    if (GET_MOVE(ch) < 20) {
        send_to_char("You are too physically exhausted.\r\n", ch);
        return;
    }
  
    if ((dist = find_distance(ch->in_room, vict->in_room)) > 
        ((GET_LEVEL(ch) / 6))) {
        act("$N is out of your psychic range.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }

    if (NULL_PSI(vict)) {
        act("It is pointless to attempt this on $M.", FALSE,ch,0,vict, TO_CHAR);
        return;
    }

    if (AFF3_FLAGGED(vict, AFF3_PSISHIELD) && 
        !mag_savingthrow(vict, GET_LEVEL(ch), SAVING_PSI)) {
        act("Your attack is deflected by $N's psishield!",
            FALSE, ch, 0, vict, TO_CHAR);
        act("$n's psychic attack is deflected by your psishield!",
            FALSE, ch, 0, vict, TO_VICT);
        act("$n staggers under an unseen force.", 
            TRUE, ch, 0, vict, TO_NOTVICT);
        return;
    }

    if (GET_MANA(vict) <= 0) {
        act("$E is completely drained of psychic energy.",
            TRUE, ch, 0, vict, TO_CHAR);
        return;
    }

    drain = dice(GET_LEVEL(ch), GET_INT(ch) + GET_REMORT_GEN(ch)) + 
        CHECK_SKILL(ch, SKILL_PSIDRAIN);
    if (dist > 0)
        drain /= dist + 1;

    drain >>= 2;

    drain = MAX(0, MIN(GET_MANA(vict), drain));

    prob = CHECK_SKILL(ch, SKILL_PSIDRAIN) + GET_INT(ch) +
        (AFF3_FLAGGED(vict, AFF3_PSISHIELD) ? -20 : 0);

    if (FIGHTING(vict))
        prob += 15;

    if (dist > 0)
        prob -= dist * 3;

    act("$n strains against an unseen force.",
        FALSE, ch, 0, vict, TO_ROOM);

    //
    // failure
    //

    if (number(0, 121) > prob) {
        send_to_char("You are unable to create the drainage link!\r\n", ch);
        WAIT_STATE(ch, 2 RL_SEC);
        
        if (IS_NPC(vict) && !FIGHTING(vict)) {
            
            if (ch->in_room == vict->in_room) {
                int retval = hit(vict, ch, TYPE_UNDEFINED);
                retval = SWAP_DAM_RETVAL( retval );

                ACMD_set_return_flags( retval );

                if ( IS_SET( retval, DAM_ATTACKER_KILLED ) )
                    return;
            }
            
            else {
                remember(vict, ch);
                if (MOB2_FLAGGED(vict, MOB2_HUNT))
                    HUNTING(vict) = ch;
            }
        }
    } 
    
    //
    // success
    //
    else {
        
        act("A torrent of psychic energy is ripped out of $N's mind!",
            FALSE, ch, 0, vict, TO_CHAR);
        if (ch->in_room != vict->in_room &&
            GET_LEVEL(vict) + number(0, CHECK_SKILL(vict, SKILL_PSIDRAIN)) >
            GET_LEVEL(ch))
            act("Your psychic energy is ripped from you from afar!",
                FALSE, ch, 0, vict, TO_VICT);
        else
            act("Your psychic energy is ripped from you by $n!",
                FALSE, ch, 0, vict, TO_VICT);
        GET_MANA(vict) -= drain;
        GET_MANA(ch)   = MIN(GET_MAX_MANA(ch), GET_MANA(ch) + drain);
        GET_MOVE(ch)   -= 20;
        WAIT_STATE(vict, 1 RL_SEC);
        WAIT_STATE(ch, 5 RL_SEC);
        gain_skill_prof(ch, SKILL_PSIDRAIN);

        if (IS_NPC(vict) && !FIGHTING(vict)) {
            if (ch->in_room == vict->in_room) {
                
                int retval = hit(vict, ch, TYPE_UNDEFINED);
                retval = SWAP_DAM_RETVAL( retval );
                
                ACMD_set_return_flags( retval );

                if ( IS_SET( retval, DAM_ATTACKER_KILLED ) )
                    return;

            }
            else {
                remember(vict, ch);
                if (MOB2_FLAGGED(vict, MOB2_HUNT))
                    HUNTING(vict) = ch;
            }
        }
    }
}

int mob_fight_psionic( struct char_data *ch, struct char_data * precious_vict ) {

    CHAR *vict = 0;

    if ( !FIGHTING( ch ) )
        return 0;

    // pick an enemy
    if ( ! ( vict = choose_opponent(ch, precious_vict ) ) )
        return 0;

    // dermal hardening
    if ( GET_LEVEL( ch ) >= 12 && 
         !affected_by_spell( ch, SPELL_DERMAL_HARDENING ) &&
         GET_MANA( ch ) > mag_manacost( ch, SPELL_DERMAL_HARDENING ) )
        cast_spell( ch, ch, NULL, SPELL_DERMAL_HARDENING );
    // wound closure
    else if ( GET_LEVEL( ch ) >= 10 &&
              GET_HIT( ch ) < ( GET_MAX_HIT( ch ) >> 2 ) &&
              GET_MANA( ch ) > mag_manacost( ch, SPELL_WOUND_CLOSURE ) )
        cast_spell( ch, ch, NULL, SPELL_WOUND_CLOSURE );
    // psychic resistance
    else if ( IS_PSIONIC( vict ) && GET_LEVEL( vict ) >= 5 &&
              GET_LEVEL( ch ) >= 20 &&
              !affected_by_spell( ch, SPELL_PSYCHIC_RESISTANCE ) &&
              GET_MANA( ch ) > mag_manacost( ch, SPELL_PSYCHIC_RESISTANCE ) )
        cast_spell( ch, ch, NULL, SPELL_PSYCHIC_RESISTANCE );
    // adrenaline
    else if ( GET_LEVEL( ch ) >= 27 &&
              !AFF_FLAGGED( ch, AFF_ADRENALINE ) &&
              GET_MANA( ch ) > mag_manacost( ch, SPELL_ADRENALINE ) )
        cast_spell( ch, ch, NULL, SPELL_ADRENALINE );
    // drain the enemy!
    else if ( GET_LEVEL( ch ) >= 24 &&
              ( IS_MAGE( vict ) || IS_PSIONIC( vict ) || IS_CLERIC( vict ) |
                IS_KNIGHT( vict ) || IS_PHYSIC( vict ) ) &&
              !NULL_PSI( vict ) &&
              GET_MANA( vict ) > 50 && !number( 0, 2 ) &&
              GET_MOVE( ch ) > 30 ) {
        if ( ! CAN_SEE( ch, vict ) )
            // just attack the default opponent
            do_psidrain( ch, "", 0, 0 );
        else 
            do_psidrain( ch, GET_NAME( vict ), 0, 0 );
    }
    // fear
    else if ( GET_LEVEL( ch ) >= 29 && 
              GET_MANA( ch ) > mag_manacost( ch, SPELL_FEAR ) &&
              !affected_by_spell( vict, SPELL_FEAR) && !number(0, 2) )
        cast_spell( ch, vict, NULL, SPELL_FEAR );
    // clumsiness
    else if ( GET_LEVEL( ch ) >= 28 &&
              !affected_by_spell( vict, SPELL_CLUMSINESS ) &&
              GET_MANA( ch ) > mag_manacost( ch, SPELL_CLUMSINESS ) &&
              !number( 0, 2 ) )
        cast_spell( ch, vict, NULL, SPELL_CLUMSINESS );
    // weakness
    else if ( GET_LEVEL( ch ) >= 16 &&
              !affected_by_spell( vict, SPELL_WEAKNESS ) &&
              GET_MANA( ch ) > mag_manacost( ch, SPELL_WEAKNESS ) &&
              !number( 0, 2 ) )
        cast_spell( ch, vict, NULL, SPELL_WEAKNESS );
    // psychic crush
    else if ( GET_LEVEL( ch ) >= 35 &&
              !AFF3_FLAGGED( vict, SPELL_PSYCHIC_CRUSH ) &&
              GET_MANA( ch ) > mag_manacost( ch, SPELL_PSYCHIC_CRUSH ) )
        cast_spell( ch, vict, NULL, SPELL_PSYCHIC_CRUSH );
    // ego whip
    else if ( GET_LEVEL( ch ) >= 22 &&
              GET_MANA( ch ) > mag_manacost( ch, SPELL_EGO_WHIP ) &&
              ( !number( 0, 2 ) || ( !CAN_SEE( ch, vict ) &&
                                     vict != FIGHTING( ch ) ) ) )
        cast_spell( ch, vict, NULL, SPELL_EGO_WHIP );
    // psiblast
    else if ( GET_LEVEL( ch ) >= 5 &&
              GET_MANA( ch ) > mag_manacost( ch, SKILL_PSIBLAST ) ) {
        if ( !CAN_SEE( ch, vict ) )
            // just attack the default opponent
            do_offensive_skill( ch, "", 0, SKILL_PSIBLAST );
        else
            do_offensive_skill( ch, GET_NAME( vict ), 0, SKILL_PSIBLAST );
    }
    else
        return 0;
    
    return 1;
}
    
