#ifdef HAS_CONFIG_H
#include "config.h"
#endif

#include "structs.h"
#include "comm.h"
#include "creature_list.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "help.h"
#include "paths.h"
#include "login.h"
#include "house.h"
#include "clan.h"
#include "security.h"
#include "fight.h"
#include "player_table.h"
#include "prog.h"
#include "quest.h"

extern CreatureList defendingList;
extern CreatureList mountedList;
extern CreatureList huntingList;

void extract_norents(struct obj_data *obj);
void char_arrest_pardoned(Creature *ch);
void remove_fighting_affects(struct Creature *ch);
extern struct descriptor_data *descriptor_list;
struct player_special_data dummy_mob;	/* dummy spec area for mobs         */

Creature::Creature(bool pc)
    : thing(CREATURE)
{
    initialize();

	if (pc) {
		player_specials = new player_special_data;
	} else {
		player_specials = &dummy_mob;
		SET_BIT(MOB_FLAGS(this), MOB_ISNPC);
	}

    this->fighting = new CombatDataList();
    this->fighting->clear();
    this->language_data = new char_language_data();

	clear();
}

Creature::~Creature(void)
{
	clear();

	if (player_specials != &dummy_mob) {
		delete player_specials;
		free(player.title);
	}

    delete this->fighting;
    delete this->language_data;
}

Creature::Creature(const Creature &c)
    : thing(c)
{
    // Far as I'm concerned there is NEVER a good reason to copy a player
    // this way
    if (!IS_NPC(&c)) {
		slog("Creature::Creature(const Creature &c) called on a player!");
        raise(SIGSEGV);
    }
    initialize();

    this->in_room = c.in_room;
	this->player_specials = &dummy_mob;
    //todo: duplicate affects

    this->account = c.account;

    //todo: duplicate equipment?

    this->fighting = new CombatDataList(*(c.fighting));
    this->fighting->clear();

    this->player = c.player;
    this->real_abils = c.real_abils;
    this->aff_abils = c.aff_abils;
    this->points = c.points;
    this->language_data = new char_language_data(*c.language_data);
    this->mob_specials =  c.mob_specials;
    this->char_specials = c.char_specials;
}

void
Creature::checkPosition(void)
{
	if (GET_HIT(this) > 0) {
		if (getPosition() < POS_STUNNED)
			setPosition(POS_RESTING);
		return;
	}

	if (GET_HIT(this) > -3)
		setPosition(POS_STUNNED);
	else if (GET_HIT(this) > -6)
		setPosition(POS_INCAP);
	else
		setPosition(POS_MORTALLYW);
}

/**
 * Returns true if this character is in the Testers access group.
**/
bool Creature::isTester(){
	if( IS_NPC(this) )
		return false;
	return Security::isMember( this, "Testers", false );
}

// Returns this creature's account id.
long Creature::getAccountID() const {
    if( account == NULL )
        return 0;
    return account->get_idnum();
}

/**
 * Modifies the given experience to be appropriate for this character's
 *  level/gen and class.
 * if victim != NULL, assume that this char is fighting victim to gain
 *  experience.
 *
**/
int Creature::getPenalizedExperience( int experience, Creature *victim)
{

	// Mobs are easily trained
	if( IS_NPC(this) )
		return experience;
	// Immortals are not
	if( getLevel() >= LVL_AMBASSADOR ) {
		return 0;
	}

	if ( victim != NULL ) {
		if( victim->getLevel() >= LVL_AMBASSADOR )
			return 0;

		// good clerics & knights penalized for killing good
		if( IS_GOOD(victim) && IS_GOOD(this) &&
			(IS_CLERIC(this) || IS_KNIGHT(this)) ) {
			experience /= 2;
		}
	}

	// Slow remorting down a little without slowing leveling completely.
	// This penalty works out to:
	// gen lvl <=15 16->39  40>
	//  1     23.3%	 33.3%  43.3%
	//  2     40.0%  50.0%  60.0%
	//  3     50.0%  60.0%  70.0%
	//  4     56.6%  66.6%  76.6%
	//  5     61.4%  71.4%  81.4%
	//  6     65.0%  75.0%  85.0%
	//  7     67.7%  77.7%  87.7%
	//  8     70.0%  80.0%  90.0%
	//  9     71.8%  81.8%  91.8%
	// 10     73.3%  83.3%  93.3%
	if( IS_REMORT(this) ) {
		float gen = GET_REMORT_GEN(this);
		float multiplier = (gen / ( gen + 2 ));

		if( getLevel() <= 15 )
			multiplier -= 0.10;
		else if( getLevel() >= 40 )
			multiplier += 0.10;

		experience -= (int)(experience * multiplier);
	}

	return experience;
}

//Positive or negative percent modifier based on buyer vs seller charisma.
int
Creature::getCostModifier(Creature* seller) {
    int cost_modifier = (GET_CHA(seller)-GET_CHA(this))*2;
    return cost_modifier;
}

int
Creature::modifyCarriedWeight(int mod_weight)
{
	return (setCarriedWeight(getCarriedWeight() + mod_weight));
}

int
Creature::modifyWornWeight(int mod_weight)
{
	return (setWornWeight(getWornWeight() + mod_weight));
}

short
char_player_data::modifyWeight(short mod_weight)
{
	return setWeight(getWeight() + mod_weight);
}

int
Creature::getSpeed(void)
{
	// if(IS_NPC(this))
	if (char_specials.saved.act & MOB_ISNPC)
		return 0;
	return (int)player_specials->saved.speed;
}

void
Creature::setSpeed(int speed)
{
	// if(IS_NPC(this))
	if (char_specials.saved.act & MOB_ISNPC)
		return;
	speed = MAX(speed, 0);
	speed = MIN(speed, 100);
	player_specials->saved.speed = (char)(speed);
}

bool
Creature::isNewbie()
{
	if (char_specials.saved.act & MOB_ISNPC)
		return false;
	if ((char_specials.saved.remort_generation) > 0)
		return false;
	if (player.level > 24)
		return false;

	return true;
}

// Utility function to determine if a char should be affected by sanctuary
// on a hit by hit level... --N
bool
Creature::affBySanc(Creature *attacker)
{

	Creature *ch = this;

	if (AFF_FLAGGED(ch, AFF_SANCTUARY)) {
		if (attacker && IS_EVIL(ch) &&
			affected_by_spell(attacker, SPELL_RIGHTEOUS_PENETRATION))
			return false;
		else if (attacker && IS_GOOD(ch) &&
			affected_by_spell(attacker, SPELL_MALEFIC_VIOLATION))
			return false;
		else
			return true;
	}

	return false;
}

// Pass in the attacker for conditional reduction such as PROT_GOOD and
// PROT_EVIL.  Or leave it blank for the characters base reduction --N
float
Creature::getDamReduction(Creature *attacker)
{
	struct Creature *ch = this;
	struct affected_type *af = NULL;
	float dam_reduction = 0;

	if (GET_CLASS(ch) == CLASS_CLERIC && IS_GOOD(ch)) {
		// good clerics get an alignment-based protection, up to 30% in the
		// full moon, up to 10% otherwise
		if (get_lunar_phase(lunar_day) == MOON_FULL)
			dam_reduction += GET_ALIGNMENT(ch) / 30;
		else
			dam_reduction += GET_ALIGNMENT(ch) / 100;
	}
	//*************************** Sanctuary ****************************
	//******************************************************************
	if (ch->affBySanc(attacker)) {
		if (IS_VAMPIRE(ch))
			dam_reduction += 0;
		else if (IS_CLERIC(ch) || IS_KNIGHT(ch) && !IS_NEUTRAL(ch))
			dam_reduction += 25;
		else if (GET_CLASS(ch) == CLASS_CYBORG || GET_CLASS(ch) == CLASS_PHYSIC)
			dam_reduction += 8;
		else
			dam_reduction += 15;
	}
	//***************************** Oblivity ****************************
	//*******************************************************************
	// damage reduction ranges up to about 35%
	if (AFF2_FLAGGED(ch, AFF2_OBLIVITY) && IS_NEUTRAL(ch)) {
		dam_reduction += (((GET_LEVEL(ch) +
					ch->getLevelBonus(ZEN_OBLIVITY)) * 10) +
			(1000 - abs(GET_ALIGNMENT(ch))) +
			(CHECK_SKILL(ch, ZEN_OBLIVITY) * 10)) / 100;
	}
	//**************************** No Pain *****************************
	//******************************************************************
	if (AFF_FLAGGED(ch, AFF_NOPAIN)) {
		dam_reduction += 25;
	}
	//**************************** Berserk *****************************
	//******************************************************************
	if (AFF2_FLAGGED(ch, AFF2_BERSERK)) {
		if (IS_BARB(ch))
			dam_reduction += (ch->getLevelBonus(SKILL_BERSERK)) / 6;
		else
			dam_reduction += 7;
	}
	//************************** Damage Control ************************
	//******************************************************************
	if (AFF3_FLAGGED(ch, AFF3_DAMAGE_CONTROL)) {
		dam_reduction += (ch->getLevelBonus(SKILL_DAMAGE_CONTROL)) / 5;
	}
	//**************************** ALCOHOLICS!!! ***********************
	//******************************************************************
	if (GET_COND(ch, DRUNK) > 5)
		dam_reduction += GET_COND(ch, DRUNK);

	//********************** Shield of Righteousness *******************
	//******************************************************************
	if ((af = affected_by_spell(ch, SPELL_SHIELD_OF_RIGHTEOUSNESS))) {

		// Find the caster apply for the shield of righteousness spell
        while (af) {
			if (af->type == SPELL_SHIELD_OF_RIGHTEOUSNESS
					&& af->location == APPLY_CASTER)
				break;
			af = af->next;
		}

        // We found the shield of righteousness caster
        if (af && af->modifier == GET_IDNUM(ch)) {
            dam_reduction +=
                (ch->getLevelBonus(SPELL_SHIELD_OF_RIGHTEOUSNESS) / 20)
                + (GET_ALIGNMENT(ch) / 100);
        } else if (af && ch->in_room) {

            CreatureList::iterator it = ch->in_room->people.begin();
            for (; it != ch->in_room->people.end(); ++it) {
                if (IS_NPC((*it))
                    && af->modifier == (short int)-MOB_IDNUM((*it))) {
                    dam_reduction +=
                        ((*it)->getLevelBonus(SPELL_SHIELD_OF_RIGHTEOUSNESS) /
                        20)
                        + (GET_ALIGNMENT(ch) / 100);
                    break;
                } else if (!IS_NPC((*it)) && af->modifier == GET_IDNUM((*it))) {
                    dam_reduction +=
                        ((*it)->getLevelBonus(SPELL_SHIELD_OF_RIGHTEOUSNESS) /
                        20)
                        + (GET_ALIGNMENT(ch) / 100);
                    break;
                }
            }
		}
	}
	//************************** Aria of Asylum ************************
	//******************************************************************
    // This should be very similar to Shield of Righteousness as it's also
    // a group thing
	if ((af = affected_by_spell(ch, SONG_ARIA_OF_ASYLUM))) {

		// Find the caster apply
        while (af) {
			if (af->type == SONG_ARIA_OF_ASYLUM
					&& af->location == APPLY_CASTER)
				break;
			af = af->next;
		}

        // We found the aria of asylum singer
        if (af && af->modifier == GET_IDNUM(ch)) {
            dam_reduction += 5 + (((1000 - abs(GET_ALIGNMENT(ch))) / 100) +
                                  (ch->getLevelBonus(SONG_ARIA_OF_ASYLUM) / 10));
        } else if (af && ch->in_room) {

            CreatureList::iterator it = ch->in_room->people.begin();
            for (; it != ch->in_room->people.end(); ++it) {
                if (IS_NPC((*it))
                    && af->modifier == (short int)-MOB_IDNUM((*it))) {
                    dam_reduction += 5 + (((1000 - abs(GET_ALIGNMENT((*it)))) / 100) +
                                         ((*it)->getLevelBonus(SONG_ARIA_OF_ASYLUM) / 10));
                    break;
                } else if (!IS_NPC((*it)) && af->modifier == GET_IDNUM((*it))) {
                    dam_reduction += 5 + (((1000 - abs(GET_ALIGNMENT((*it)))) / 100) +
                                         ((*it)->getLevelBonus(SONG_ARIA_OF_ASYLUM) / 10));
                    break;
                }
            }
		}
	}
	//*********************** Lattice Hardening *************************
	//*******************************************************************
	if (affected_by_spell(ch, SPELL_LATTICE_HARDENING))
		dam_reduction += (ch->getLevelBonus(SPELL_LATTICE_HARDENING)) / 6;

	//************** Stoneskin Barkskin Dermal Hardening ****************
	//*******************************************************************
	struct affected_type *taf;
	if ((taf = affected_by_spell(ch, SPELL_STONESKIN)))
		dam_reduction += (taf->level) / 4;
	else if ((taf = affected_by_spell(ch, SPELL_BARKSKIN)))
		dam_reduction += (taf->level) / 6;
	else if ((taf = affected_by_spell(ch, SPELL_DERMAL_HARDENING)))
		dam_reduction += (taf->level) / 6;

	//************************** Petrification **************************
	//*******************************************************************
	if (AFF2_FLAGGED(ch, AFF2_PETRIFIED))
		dam_reduction += 75;

	if (attacker) {
		///****************** Various forms of protection ***************
		if (IS_EVIL(attacker) && AFF_FLAGGED(ch, AFF_PROTECT_EVIL))
			dam_reduction += 8;
		if (IS_GOOD(attacker) && AFF_FLAGGED(ch, AFF_PROTECT_GOOD))
			dam_reduction += 8;
		if (IS_UNDEAD(attacker) && AFF2_FLAGGED(ch, AFF2_PROTECT_UNDEAD))
			dam_reduction += 8;
		if (IS_DEMON(attacker) && AFF2_FLAGGED(ch, AFF2_PROT_DEMONS))
			dam_reduction += 8;
		if (IS_DEVIL(attacker) && AFF2_FLAGGED(ch, AFF2_PROT_DEVILS))
			dam_reduction += 8;
	}

    dam_reduction += abs(MIN(0, GET_AC(ch) + 300) / 5);
	dam_reduction = MIN(dam_reduction, 75);

	return (dam_reduction / 100);
}

//
// Compute level bonus factor.
// Currently, a gen 4 level 49 secondary should equate to level 49 mort primary.
//
//   params: primary - Add in remort gen as a primary?
//   return: a number from 1-100 based on level and primary/secondary)

int
Creature::getLevelBonus(bool primary)
{
	int bonus = MIN(50, player.level + 1);
	short gen = char_specials.saved.remort_generation;

	if( gen == 0 && IS_NPC(this) ) {
		if ((player.remort_char_class % NUM_CLASSES) == 0) {
			gen = 0;
		} else {
			gen = (aff_abils.intel + aff_abils.str + aff_abils.wis) / 3;
			gen = MAX(0, gen - 18);
		}
	}

	if (gen == 0) {
		return bonus;
	} else {
		if (primary) {			// Primary. Give full remort bonus per gen.
			return bonus + (MIN(gen, 10)) * 5;
		} else {				// Secondary. Give less level bonus and less remort bonus.
			return (bonus * 3 / 4) + (MIN(gen, 10) * 3);
		}
	}
}

//
// Compute level bonus factor.
// Should be used for a particular skill in general.
// Returns 50 for max mort, 100 for max remort.
// params: skill - the skill # to check bonus for.
// return: a number from 1-100 based on level/gen/can learn skill.

int
Creature::getLevelBonus(int skill)
{
	// Immorts get full bonus.
	if( player.level >= 50 )
		return 100;

	// Irregular skill #s get 1
	if( skill > TOP_SPELL_DEFINE || skill < 0 )
		return 1;

	if( IS_NPC(this) && GET_CLASS(this) >= NUM_CLASSES ) {
		// Check to make sure they have the skill
		int skill_lvl = CHECK_SKILL(this, skill);
		if (!skill_lvl)
			return 1;
		// Average the basic level bonus and the skill level
		return MIN(100, (getLevelBonus(true) + skill_lvl) / 2);
	} else {
		int pclass = GET_CLASS(this);
		int sclass = GET_REMORT_CLASS(this);
		int spell_lvl = SPELL_LEVEL(skill, pclass);
		int spell_gen = SPELL_GEN( skill, pclass );

		if( pclass < 0 || pclass >= NUM_CLASSES )
			pclass = CLASS_WARRIOR;

		if( sclass >= NUM_CLASSES )
			sclass = CLASS_WARRIOR;

		if( spell_lvl <= player.level && spell_gen <= GET_REMORT_GEN(this) ) {
			return getLevelBonus(true);
		}
        else if( sclass >= 0 && (SPELL_LEVEL(skill, sclass) <= player.level)) {
			return getLevelBonus(false);
		}
        else {
			return player.level/2;
		}
	}
}

/**
 *  attempts to set character's position.
 *
 *  @return success or failure
 *  @param new_position the enumerated int position to be set to.
 *  @param mode: 1 == from update_pos;
 *  			 2 == from perform violence;
 *  			 NOTE: Previously used for debugging. No longer used.
**/
bool
Creature::setPosition(int new_pos, int mode)
{
	if (new_pos == char_specials.getPosition())
		return false;
	if (new_pos < BOTTOM_POS || new_pos > TOP_POS)
		return false;
	// Petrified
	if (AFF2_FLAGGED(this, AFF2_PETRIFIED)) {
		// Stoners can stop fighting
		if (char_specials.getPosition() == POS_FIGHTING && new_pos == POS_STANDING ) {
			char_specials.setPosition(new_pos);
			return true;
		} else if (new_pos > char_specials.getPosition()) {
			return false;
		}
	}
	if (new_pos == POS_STANDING && this->isFighting()) {
		char_specials.setPosition(POS_FIGHTING);
	} else {
		char_specials.setPosition(new_pos);
	}
	return true;
}

/**
 * Extract a ch completely from the world, and destroy his stuff
 * @param con_state the connection state to change the descriptor to, if one exists
**/
void
Creature::extract(cxn_state con_state)
{
	ACMD(do_return);
	void die_follower(struct Creature *ch);
    void verify_tempus_integrity(Creature *);

	struct obj_data* obj;
	struct descriptor_data* t_desc;
	int idx;
	CreatureList::iterator cit;

	if (!IS_NPC(this) && !desc) {
		for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
			if (t_desc->original == this)
				do_return(t_desc->creature, tmp_strdup(""), 0, SCMD_FORCED, 0);
	}

    if (desc && desc->original) {
        do_return(desc->creature, tmp_strdup(""), 0, SCMD_FORCED, 0);
    }

	if (in_room == NULL) {
		errlog("NOWHERE extracting char. (handler.c, extract_char)");
		slog("...extract char = %s", GET_NAME(this));
		raise(SIGSEGV);
	}

	if (followers || master)
		die_follower(this);

	// remove fighters, defenders, hunters and mounters
    for (cit = defendingList.begin(); cit != defendingList.end(); ++cit) {
	  if (this == (*cit)->isDefending())
		(*cit)->stopDefending();
    }

    for (cit = mountedList.begin(); cit != mountedList.end(); ++cit) {
		if (this == (*cit)->isMounted()) {
            (*cit)->dismount();
			if ((*cit)->getPosition() == POS_MOUNTED) {
				if ((*cit)->in_room->sector_type == SECT_FLYING)
					(*cit)->setPosition(POS_FLYING);
				else
					(*cit)->setPosition(POS_STANDING);
			}
		}
        mountedList.remove(*cit);
    }

	for (cit = huntingList.begin(); cit != huntingList.end(); ++cit) {
		if (this == (*cit)->isHunting())
			(*cit)->stopHunting();
   	}

	destroy_attached_progs(this);
	char_arrest_pardoned(this);

	if (this->isMounted()) {
		REMOVE_BIT(AFF2_FLAGS(this->isMounted()), AFF2_MOUNTED);
        this->dismount();
	}

	// Make sure they aren't editing a help topic.
	if (GET_OLC_HELP(this)) {
		GET_OLC_HELP(this)->editor = NULL;
		GET_OLC_HELP(this) = NULL;
	}
	// Forget snooping, if applicable
	if (desc) {
		if (desc->snooping) {
            vector<descriptor_data *>::iterator vi = desc->snooping->snoop_by.begin();
            for (; vi != desc->snooping->snoop_by.end(); ++vi) {
                if ((*vi) == desc) {
                    desc->snooping->snoop_by.erase(vi);
                    break;
                }
            }
			desc->snooping = NULL;
		}
		if (desc->snoop_by.size()) {
            for (unsigned x = 0; x < desc->snoop_by.size(); x++) {
			    SEND_TO_Q("Your victim is no longer among us.\r\n",
				          desc->snoop_by[x]);
			    desc->snoop_by[x]->snooping = NULL;
            }
			desc->snoop_by.clear();
		}
	}

	// destroy all that equipment
	for (idx = 0; idx < NUM_WEARS; idx++) {
		if (GET_EQ(this, idx))
			extract_obj(unequip_char(this, idx, EQUIP_WORN, true));
		if (GET_IMPLANT(this, idx))
			extract_obj(unequip_char(this, idx, EQUIP_IMPLANT, true));
		if (GET_TATTOO(this, idx))
			extract_obj(unequip_char(this, idx, EQUIP_TATTOO, true));
	}

	// transfer inventory to room, if any
	while (carrying) {
		obj = carrying;
		obj_from_char(obj);
		extract_obj(obj);
	}

	if (desc && desc->original)
		do_return(this, tmp_strdup(""), 0, SCMD_NOEXTRACT, 0);

    removeAllCombat();

	char_from_room(this,false);

	// pull the char from the various lists
	defendingList.remove(this);
	huntingList.remove(this);
	mountedList.remove(this);
	characterList.remove(this);
    if (IS_NPC(this))
        characterMap.erase(-MOB_IDNUM(this));
    else
        characterMap.erase(GET_IDNUM(this));

	// remove any paths
	path_remove_object(this);

	if (IS_NPC(this)) {
		if (GET_MOB_VNUM(this) > -1)	// if mobile
			mob_specials.shared->number--;
		clearMemory();			// Only NPC's can have memory
		delete this;
		return;
	}

	if (desc) {					// PC's have descriptors. Take care of them
		set_desc_state(con_state, desc);
	} else {					// if a player gets purged from within the game
		delete this;
	}
}

// erase ch's memory
void
Creature::clearMemory()
{
	memory_rec *curr, *next;

	curr = MEMORY(this);

	while (curr) {
		next = curr->next;
		free(curr);
		curr = next;
	}

	MEMORY(this) = NULL;
}

// Retrieves the characters appropriate loadroom.
room_data *Creature::getLoadroom() {
    room_data *load_room = NULL;

	if (PLR_FLAGGED(this, PLR_FROZEN)) {
		load_room = r_frozen_start_room;
	} else if (GET_LOADROOM(this)) {
		if ((load_room = real_room(GET_LOADROOM(this))) &&
			(!House_can_enter(this, load_room->number) ||
			!clan_house_can_enter(this, load_room)))
		{
			load_room = NULL;
		}
	} else if (GET_HOMEROOM(this)) {
		if ((load_room = real_room(GET_HOMEROOM(this))) &&
			(!House_can_enter(this, load_room->number) ||
			!clan_house_can_enter(this, load_room)))
		{
			load_room = NULL;
		}

	}

	if( load_room != NULL )
		return load_room;

	if ( GET_LEVEL(this) >= LVL_AMBASSADOR ) {
		load_room = r_immort_start_room;
	} else {
		if( GET_HOME(this) == HOME_NEWBIE_SCHOOL ) {
			if (GET_LEVEL(this) > 5) {
				population_record[HOME_NEWBIE_SCHOOL]--;
				GET_HOME(this) = HOME_MODRIAN;
				population_record[HOME_MODRIAN]--;
				load_room = r_mortal_start_room;
			} else {
				load_room = r_newbie_school_start_room;
			}
		} else if (GET_HOME(this) == HOME_ELECTRO) {
			load_room = r_electro_start_room;
		} else if (GET_HOME(this) == HOME_NEWBIE_TOWER) {
			if (GET_LEVEL(this) > 5) {
				population_record[HOME_NEWBIE_TOWER]--;
				GET_HOME(this) = HOME_MODRIAN;
				population_record[HOME_MODRIAN]--;
				load_room = r_mortal_start_room;
			} else
				load_room = r_tower_modrian_start_room;
		} else if (GET_HOME(this) == HOME_NEW_THALOS) {
			load_room = r_new_thalos_start_room;
		} else if (GET_HOME(this) == HOME_KROMGUARD) {
			load_room = r_kromguard_start_room;
		} else if (GET_HOME(this) == HOME_ELVEN_VILLAGE){
			load_room = r_elven_start_room;
		} else if (GET_HOME(this) == HOME_ISTAN){
			load_room = r_istan_start_room;
		} else if (GET_HOME(this) == HOME_ARENA){
			load_room = r_arena_start_room;
		} else if (GET_HOME(this) == HOME_MONK){
			load_room = r_monk_start_room;
		} else if (GET_HOME(this) == HOME_SKULLPORT_NEWBIE) {
			load_room = r_skullport_newbie_start_room;
		} else if (GET_HOME(this) == HOME_SOLACE_COVE){
			load_room = r_solace_start_room;
		} else if (GET_HOME(this) == HOME_MAVERNAL){
			load_room = r_mavernal_start_room;
		} else if (GET_HOME(this) == HOME_DWARVEN_CAVERNS){
			load_room = r_dwarven_caverns_start_room;
		} else if (GET_HOME(this) == HOME_HUMAN_SQUARE){
			load_room = r_human_square_start_room;
		} else if (GET_HOME(this) == HOME_SKULLPORT){
			load_room = r_skullport_start_room;
		} else if (GET_HOME(this) == HOME_DROW_ISLE){
			load_room = r_drow_isle_start_room;
		} else if (GET_HOME(this) == HOME_ASTRAL_MANSE) {
			load_room = r_astral_manse_start_room;
		// zul dane
		} else if (GET_HOME(this) == HOME_ZUL_DANE) {
			// newbie start room for zul dane
			if (GET_LEVEL(this) > 5)
				load_room = r_zul_dane_newbie_start_room;
			else
				load_room = r_zul_dane_start_room;
		} else {
			load_room = r_mortal_start_room;
		}
	}
    return load_room;
}

// Called by constructors to initialize
void
Creature::initialize(void)
{
    in_room = NULL;
    carrying = NULL;
    master = NULL;
    followers = NULL;
    affected = NULL;
    desc = NULL;
    account = NULL;
    prog_state = NULL;
    memset(&aff_abils, 0, sizeof(aff_abils));
    memset(&real_abils, 0, sizeof(real_abils));
    memset(&points, 0, sizeof(points));
    memset(&mob_specials, 0, sizeof(mob_specials));
    memset(&char_specials, 0, sizeof(char_specials));
    memset(&player, 0, sizeof(player));
    memset(equipment, 0, sizeof(equipment));
    memset(implants, 0, sizeof(implants));
    memset(tattoos, 0, sizeof(tattoos));
}

// Free all structures and return to a virginal state
void
Creature::clear(void)
{
	struct Creature *tmp_mob;
	struct alias_data *a;
	bool is_pc;

	void free_alias(struct alias_data *a);

	//
	// first make sure the char is no longer in the world
	//
	if (this->in_room != NULL || this->carrying != NULL ||
		(this->getCombatList() && this->isFighting() != 0) ||
        this->followers != NULL || this->master != NULL) {
		errlog("attempted clear of creature who is still connected to the world.");
		raise(SIGSEGV);
	}

    //
    // also check equipment and implants
    //
    for (int pos = 0;pos < NUM_WEARS;pos++) {
        if (this->equipment[pos]
            || this->implants[pos]
            || this->tattoos[pos]) {
            errlog("attempted clear of creature who is still connected to the world.");
            raise(SIGSEGV);
        }
    }

	//
	// next remove and free all alieases
	//

	while ((a = GET_ALIASES(this)) != NULL) {
		GET_ALIASES(this) = (GET_ALIASES(this))->next;
		free_alias(a);
	}

	//
	// now remove all affects
	//

	while (this->affected)
		affect_remove(this, this->affected);

	//
	// free mob strings:
	// free strings only if the string is not pointing at proto
	//
	if (mob_specials.shared && GET_MOB_VNUM(this) > -1) {

		tmp_mob = real_mobile_proto(GET_MOB_VNUM(this));

		if (this->player.name != tmp_mob->player.name)
			free(this->player.name);
		if (this->player.title != tmp_mob->player.title)
			free(this->player.title);
		if (this->player.short_descr != tmp_mob->player.short_descr)
			free(this->player.short_descr);
		if (this->player.long_descr != tmp_mob->player.long_descr)
			free(this->player.long_descr);
		if (this->player.description != tmp_mob->player.description)
			free(this->player.description);
        free(this->mob_specials.func_data);
        prog_state_free(this->prog_state);
	} else {
		//
		// otherwise this is a player, so free all
		//

        free(this->player.name);
        free(this->player.title);
        free(this->player.short_descr);
        free(this->player.long_descr);
        free(this->player.description);
        free(this->mob_specials.func_data);
        prog_state_free(this->prog_state);
	}

	// remove player_specials
	is_pc = !IS_NPC(this);

	if (this->player_specials != NULL && this->player_specials != &dummy_mob) {
        free(this->player_specials->poofin);
        free(this->player_specials->poofout);
        free(this->player_specials->afk_reason);
        this->player_specials->afk_notifies.clear();
		delete this->player_specials;

		if (IS_NPC(this)) {
			errlog("Mob had player_specials allocated!");
			raise(SIGSEGV);
		}
	}

    //
    // next remove all the combat this creature might be involved in
    //
    removeAllCombat();
    delete this->fighting;
    delete this->language_data;

    initialize();

	// And we reset all the values to their initial settings
    this->fighting = new CombatDataList();
	this->setPosition(POS_STANDING);
	GET_CLASS(this) = -1;
	GET_REMORT_CLASS(this) = -1;

    // language data
    this->language_data = new char_language_data();

	GET_AC(this) = 100;			/* Basic Armor */
	if (this->points.max_mana < 100)
		this->points.max_mana = 100;

	if (is_pc) {
		player_specials = new player_special_data;
		set_title(this, "");
	} else {
		player_specials = &dummy_mob;
		SET_BIT(MOB_FLAGS(this), MOB_ISNPC);
		GET_TITLE(this) = NULL;
	}

}

void
Creature::restore(void)
{
	int i;

	GET_HIT(this) = GET_MAX_HIT(this);
	GET_MANA(this) = GET_MAX_MANA(this);
	GET_MOVE(this) = GET_MAX_MOVE(this);

	if (GET_COND(this, FULL) >= 0)
		GET_COND(this, FULL) = 24;
	if (GET_COND(this, THIRST) >= 0)
		GET_COND(this, THIRST) = 24;

	if ((GET_LEVEL(this) >= LVL_GRGOD)
			&& (GET_LEVEL(this) >= LVL_AMBASSADOR)) {
		for (i = 1; i <= MAX_SKILLS; i++)
			SET_SKILL(this, i, 100);
        for (i = 0; i < MAX_TONGUES; i++)
            SET_TONGUE(this, i, 100);
		if (GET_LEVEL(this) >= LVL_IMMORT) {
			real_abils.intel = 25;
			real_abils.wis = 25;
			real_abils.dex = 25;
			real_abils.str = 25;
			real_abils.con = 25;
			real_abils.cha = 25;
		}
		aff_abils = real_abils;
	}
	update_pos(this);
}

bool
Creature::rent(void)
{
    removeAllCombat();
	player_specials->rentcode = RENT_RENTED;
	player_specials->rent_per_day =
		(GET_LEVEL(this) < LVL_IMMORT) ? calc_daily_rent(this, 1, NULL, NULL):0;
	player_specials->desc_mode = CXN_UNKNOWN;
	player_specials->rent_currency = in_room->zone->time_frame;
	GET_LOADROOM(this) = in_room->number;
	player.time.logon = time(0);
	saveObjects();
	saveToXML();
	if (GET_LEVEL(this) < 50)
		mlog(Security::ADMINBASIC, MAX(LVL_AMBASSADOR, GET_INVIS_LVL(this)),
			NRM, true,
			"%s has rented (%d/day, %lld %s)", GET_NAME(this),
			player_specials->rent_per_day, CASH_MONEY(this) + BANK_MONEY(this),
			(player_specials->rent_currency == TIME_ELECTRO) ? "gold":"creds");
	extract(CXN_MENU);

	return true;
}

bool
Creature::cryo(void)
{
	player_specials->rentcode = RENT_CRYO;
	player_specials->rent_per_day = 0;
	player_specials->desc_mode = CXN_UNKNOWN;
	player_specials->rent_currency = in_room->zone->time_frame;
	GET_LOADROOM(this) = in_room->number;
	player.time.logon = time(0);
	saveObjects();
	saveToXML();

	mlog(Security::ADMINBASIC, MAX(LVL_AMBASSADOR, GET_INVIS_LVL(this)),
		NRM, true,
		"%s has cryo-rented", GET_NAME(this));
	extract(CXN_MENU);
	return true;
}

bool
Creature::quit(void)
{
	obj_data *obj, *next_obj, *next_contained_obj;
	int pos;

	if (IS_NPC(this))
		return false;

	for (pos = 0;pos < NUM_WEARS;pos++) {
		// Drop all non-cursed equipment worn
		if (GET_EQ(this, pos)) {
			obj = GET_EQ(this, pos);
			if (IS_OBJ_STAT(obj, ITEM_NODROP) ||
					IS_OBJ_STAT2(obj, ITEM2_NOREMOVE)) {
				for (obj = obj->contains;obj;obj = next_contained_obj) {
					next_contained_obj = obj->next_content;
					obj_from_obj(obj);
					obj_to_room(obj, in_room);
				}
			} else {
				obj = unequip_char(this, pos, EQUIP_WORN);
				obj_to_room(obj, in_room);
			}
		}

		// Drop all implanted items, breaking them
		if (GET_IMPLANT(this, pos)) {
			obj = unequip_char(this, pos, EQUIP_IMPLANT);
			GET_OBJ_DAM(obj) = GET_OBJ_MAX_DAM(obj) >> 3 - 1;
			SET_BIT(GET_OBJ_EXTRA2(obj), ITEM2_BROKEN);
			obj_to_room(obj, in_room);
			act("$p drops to the ground!", false, 0, obj, 0, TO_ROOM);
		}
	}

	// Drop all uncursed inventory items
	for (obj = carrying;obj;obj = next_obj) {
		next_obj = obj->next_content;
		if (IS_OBJ_STAT(obj, ITEM_NODROP) ||
				IS_OBJ_STAT2(obj, ITEM2_NOREMOVE)) {
			for (obj = obj->contains;obj;obj = next_contained_obj) {
				next_contained_obj = obj->next_content;
				obj_from_obj(obj);
				obj_to_room(obj, in_room);
			}
		} else {
			obj_from_char(obj);
			obj_to_room(obj, in_room);
		}
	}

	player_specials->rentcode = RENT_QUIT;
	player_specials->rent_per_day = calc_daily_rent(this, 3, NULL, NULL);
	player_specials->desc_mode = CXN_UNKNOWN;
	player_specials->rent_currency = in_room->zone->time_frame;
	GET_LOADROOM(this) = 0;
	player.time.logon = time(0);
	saveObjects();
	saveToXML();
	extract(CXN_MENU);

	return true;
}

bool
Creature::idle(void)
{
	if (IS_NPC(this))
		return false;
	player_specials->rentcode = RENT_FORCED;
	player_specials->rent_per_day = calc_daily_rent(this, 3, NULL, NULL);
	player_specials->desc_mode = CXN_UNKNOWN;
	player_specials->rent_currency = in_room->zone->time_frame;
	GET_LOADROOM(this) = 0;
	player.time.logon = time(0);
	saveObjects();
	saveToXML();

	mlog(Security::ADMINBASIC, LVL_GOD, CMP, true,
		"%s force-rented and extracted (idle).",
		GET_NAME(this));

	extract(CXN_MENU);
	return true;
}

bool
Creature::die(void)
{
	obj_data *obj, *next_obj;
	int pos;

    removeAllCombat();

	// If their stuff hasn't been moved out, they dt'd, so we need to dump
	// their stuff to the room
	for (pos = 0;pos < NUM_WEARS;pos++) {
		if (GET_EQ(this, pos)) {
			obj = unequip_char(this, pos, EQUIP_WORN);
			obj_to_room(obj, in_room);
		}
		if (GET_IMPLANT(this, pos)) {
			obj = unequip_char(this, pos, EQUIP_IMPLANT);
			obj_to_room(obj, in_room);
		}
        if (GET_TATTOO(this, pos)) {
            obj = unequip_char(this, pos, EQUIP_TATTOO);
            extract_obj(obj);
        }
	}
	for (obj = carrying;obj;obj = next_obj) {
		next_obj = obj->next_content;
		obj_from_char(obj);
		obj_to_room(obj, in_room);
	}

	if (!IS_NPC(this)) {
		player_specials->rentcode = RENT_QUIT;
		player_specials->rent_per_day = 0;
		player_specials->desc_mode = CXN_AFTERLIFE;
		player_specials->rent_currency = 0;
		GET_LOADROOM(this) = in_room->zone->respawn_pt;
		player.time.logon = time(0);
		saveObjects();
		saveToXML();
	}
	extract(CXN_AFTERLIFE);

	return true;
}

bool
Creature::npk_die(void)
{
    removeAllCombat();

	if (!IS_NPC(this)) {
		player_specials->rentcode = RENT_QUIT;
		player_specials->rent_per_day = 0;
		player_specials->desc_mode = CXN_AFTERLIFE;
		player_specials->rent_currency = 0;
		GET_LOADROOM(this) = in_room->zone->respawn_pt;
		player.time.logon = time(0);
		saveObjects();
		saveToXML();
	}
	extract(CXN_AFTERLIFE);

	return true;
}

bool
Creature::arena_die(void)
{
    // Remove any combat this character might have been involved in
    // And make sure all defending creatures stop defending
    removeAllCombat();

	// Rent them out
	if (!IS_NPC(this)) {
		player_specials->rentcode = RENT_RENTED;
		player_specials->rent_per_day =
			(GET_LEVEL(this) < LVL_IMMORT) ? calc_daily_rent(this, 1, NULL, NULL):0;
		player_specials->desc_mode = CXN_UNKNOWN;
		player_specials->rent_currency = in_room->zone->time_frame;
		GET_LOADROOM(this) = in_room->zone->respawn_pt;
		player.time.logon = time(0);
		saveObjects();
		saveToXML();
		if (GET_LEVEL(this) < 50)
			mudlog(MAX(LVL_AMBASSADOR, GET_INVIS_LVL(this)), NRM, true,
				"%s has died in arena (%d/day, %lld %s)", GET_NAME(this),
				player_specials->rent_per_day, CASH_MONEY(this) + BANK_MONEY(this),
				(player_specials->rent_currency == TIME_ELECTRO) ? "gold":"creds");
	}

	// But extract them to afterlife
	extract(CXN_AFTERLIFE);
	return true;
}

bool
Creature::purge(bool destroy_obj)
{
	obj_data *obj, *next_obj;

	if (!destroy_obj) {
		int pos;

		for (pos = 0;pos < NUM_WEARS;pos++) {
			if (GET_EQ(this, pos)) {
				obj = unequip_char(this, pos, EQUIP_WORN);
				obj_to_room(obj, in_room);
			}
			if (GET_IMPLANT(this, pos)) {
				obj = unequip_char(this, pos, EQUIP_IMPLANT);
				obj_to_room(obj, in_room);
			}
		}

		for (obj = carrying;obj;obj = next_obj) {
			next_obj = obj->next_content;
			obj_from_char(obj);
			obj_to_room(obj, in_room);
		}
	}

	if (!IS_NPC(this)) {
		player_specials->rentcode = RENT_QUIT;
		player_specials->rent_per_day = 0;
		player_specials->desc_mode = CXN_UNKNOWN;
		player_specials->rent_currency = 0;
		GET_LOADROOM(this) = 0;
		player.time.logon = time(0);
		saveObjects();
		saveToXML();
	}

	extract(CXN_DISCONNECT);
	return true;
}

bool
Creature::remort(void)
{
	if (IS_NPC(this))
		return false;
	player_specials->rentcode = RENT_QUIT;
	player_specials->rent_per_day = 0;
	player_specials->desc_mode = CXN_UNKNOWN;
	player_specials->rent_currency = 0;
	GET_LOADROOM(this) = 0;
	player.time.logon = time(0);
	saveObjects();
	saveToXML();
	extract(CXN_REMORT_AFTERLIFE);
	return true;
}

bool
Creature::trusts(long idnum)
{
	if (IS_NPC(this))
		return false;

	return account->isTrusted(idnum);
}

bool
Creature::distrusts(long idnum)
{
	return !trusts(idnum);
}

bool
Creature::trusts(Creature *ch)
{
	if (IS_NPC(this))
		return false;

	if (AFF_FLAGGED(this, AFF_CHARM) && master == ch)
		return true;

	return trusts(GET_IDNUM(ch));
}

bool
Creature::distrusts(Creature *ch)
{
	return !trusts(ch);
}

int
Creature::get_reputation(void)
{
	Account *acct;

	if (IS_NPC(this))
		return 0;

	acct = account;
	if (!acct)
		acct = Account::retrieve(playerIndex.getAccountID(GET_IDNUM(this)));
	if (acct && GET_LEVEL(this) < LVL_AMBASSADOR)
		return MAX(0, MIN(1000, (player_specials->saved.reputation * 95 / 100)
			+ (acct->get_reputation() * 5 / 100)));
	return player_specials->saved.reputation;
}

void
Creature::gain_reputation(int amt)
{
	 Account *acct;

	if (IS_NPC(this))
		return;

	acct = account;
	if (!acct)
		acct = Account::retrieve(playerIndex.getAccountID(GET_IDNUM(this)));
	if (acct && GET_LEVEL(this) < LVL_AMBASSADOR)
		acct->gain_reputation(amt);

    player_specials->saved.reputation += amt;
    if (player_specials->saved.reputation < 0)
        player_specials->saved.reputation = 0;
}

void
Creature::set_reputation(int amt)
{
	player_specials->saved.reputation = MIN(1000, MAX(0, amt));
}

void
Creature::addCombat(Creature *ch, bool initiated)
{
    Creature *defender;
    bool previously_fighting;

    if (!ch)
        return;

	if (this == ch || this->in_room != ch->in_room)
        return;

    if (!isOkToAttack(ch))
        return;

	previously_fighting = (getCombatList()->size() != 0);

	CreatureList::iterator cit;
    cit = ch->in_room->people.begin();
    for (; cit != ch->in_room->people.end(); ++cit) {
        defender = NULL;
        if ((*cit) != ch &&
            (*cit) != this &&
            (*cit)->isDefending() == ch &&
            !(*cit)->isFighting() &&
            (*cit)->getPosition() > POS_RESTING) {

            defender = *cit;
			if (findCombat(defender))
				return;

			send_to_char(defender, "You defend %s from %s's vicious attack!\r\n",
				PERS(ch, defender), PERS(this, defender));
			send_to_char(ch, "%s defends you from %s's vicious attack!\r\n",
				PERS(defender, ch), PERS(this, ch));
			act("$n comes to $N's defense!", false, defender, 0,
				ch, TO_NOTVICT);

			if (defender->isDefending() == this)
				defender->stopDefending();

			// If we're already in combat with the victim, move him
			// to the front of the list
			CombatDataList::iterator li = getCombatList()->begin();
			for (; li != getCombatList()->end(); ++li) {
				if (li->getOpponent() == ch) {
					bool ini = li->getInitiated();
					getCombatList()->remove(li);
					getCombatList()->add_front(CharCombat(defender, ini));
					return;
				}
			}

			getCombatList()->add_back(CharCombat(defender, initiated));

			update_pos(this);
			trigger_prog_fight(this, defender);

			//By not breaking here and not adding defenders to combatList we get
            //the desired effect of a) having new defenders kick in after the first dies
            //because they are on the attackers combat list, and b) still likely have
            //defenders left to defend from other potential attackers because they
            //won't actually begin combat until the attacker hits them.  This is the
            //theory at least.
        }
    }

    if (ch->isDefending() == this)
        ch->stopDefending();

    // If we're already in combat with the victim, move him
    // to the front of the list
    CombatDataList::iterator li = getCombatList()->begin();
    for (; li != getCombatList()->end(); ++li) {
        if (li->getOpponent() == ch) {
            bool ini = li->getInitiated();
            getCombatList()->remove(li);
            getCombatList()->add_front(CharCombat(ch, ini));
            return;
        }
    }

    getCombatList()->add_back(CharCombat(ch, initiated));

    update_pos(this);
    trigger_prog_fight(this, ch);

    if (!previously_fighting && isFighting() > 0) {
        CreatureList::iterator li;
        bool found = false;
        for (li = combatList.begin(); li != combatList.end(); ++li) {
            if (*li == this) {
                found = true;
                errlog("attempted to add a creature to combatList who was already in it.");
            }
        }
        if (!found) {
            combatList.add(this);
        }
    }

}

void
Creature::removeCombat(Creature *ch)
{
    if (!ch)
        return;

    if (getCombatList()->empty())
        return;

    CombatDataList::iterator li = getCombatList()->begin();
    for (; li != getCombatList()->end(); ++li) {
        if (li->getOpponent() == ch) {
            getCombatList()->remove(li);
            break;
        }
    }

    if (!isFighting()) {
        remove_fighting_affects(this);
        combatList.remove(this);
    }
}

void
Creature::removeAllCombat()
{
    if (!getCombatList()) {
        slog("getCombatList() returned NULL in removeAllCombat()!");
        raise(SIGSEGV);
    }

    if (!getCombatList()->empty()) {
        getCombatList()->clear();
        remove_fighting_affects(this);
    }

    CreatureList::iterator cit = combatList.begin();
    for (;cit != combatList.end(); ++cit)
        (*cit)->removeCombat(this);

    combatList.remove(this);
}

Creature *
Creature::findCombat(Creature *ch)
{
    if (!ch || !getCombatList())
        return NULL;

    CombatDataList::iterator li = getCombatList()->begin();
    for (; li != getCombatList()->end(); li++) {
        if (li->getOpponent() == ch)
            return (li->getOpponent());
    }

    return NULL;
}

// This function checks to see if (this) initiated combat with ch
bool
Creature::initiatedCombat(Creature *ch)
{

    if (ch == NULL || !getCombatList())
        return false;

    CombatDataList::iterator li = getCombatList()->begin();
    for (; li != getCombatList()->end(); ++li) {
        if (li->getOpponent() == ch)
            return (li->getInitiated());
    }

    return false;
}

bool
Creature::isFighting()
{
    return !getCombatList()->empty();
}

int
Creature::numCombatants()
{
    if (!this || !getCombatList())
        return 0;

    return getCombatList()->size();
}

Creature *
Creature::findRandomCombat()
{

    if (!isFighting())
        return NULL;

    // Most of the time fighting will be one on one so let's save
    // the iterator creation and the call to random_fractional_10
    if (numCombatants() == 1)
        return getCombatList()->begin()->getOpponent();

    CombatDataList::iterator li = getCombatList()->begin();
    for (; li != getCombatList()->end(); ++li) {
       if (!random_fractional_10())
           return (li->getOpponent());
    }

    return getCombatList()->begin()->getOpponent();
}

bool
Creature::isOkToAttack(Creature *vict, bool mssg)
{
    extern int get_hunted_id(int hunter_id);

    if (!vict) {
        errlog("ERROR:  NULL victim passed to isOkToAttack()");
        return false;
    }

    // Immortals over level LVL_GOD can always attack
    // anyone they want
    if (this->getLevel() >= LVL_GOD) {
        return true;
    }

    // If they're already fighting, let them have at it!
    if (this->findCombat(vict) || vict->findCombat(this)) {
        return true;
    }

    // Charmed players can't attack their master
    if (AFF_FLAGGED(this, AFF_CHARM) && (this->master == vict)) {
        if (mssg)
            act("$N is just such a good friend, you simply can't hurt $M.",
                false, this, NULL, vict, TO_CHAR);
        return false;
    }

    // If we have a bounty situation, we ignore NVZs and !PKs
    if (IS_PC(this) && IS_PC(vict) &&
        get_hunted_id(this->getIdNum()) == vict->getIdNum()) {
        return true;
    }

    // Now if we're in an arena room anbody can attack anybody
    if (is_arena_combat(this, vict))
        return true;

    // If anyone is in an NVZ, no attacks are allowed
    if (ROOM_FLAGGED(this->in_room, ROOM_PEACEFUL)) {
        if (mssg) {
            send_to_char(this, "The universal forces of order "
                         "prevent violence here!\r\n");
            if (!number(0, 1))
                act("$n seems to be violently disturbed.", false,
                    this, NULL, NULL, TO_ROOM);
            else
                act("$n becomes violently agitated for a moment.",
                    false, this, NULL, NULL, TO_ROOM);
        }
        return false;
    }

    if (ROOM_FLAGGED(vict->in_room, ROOM_PEACEFUL)) {
        if (mssg) {
            send_to_char(this, "The universal forces of order "
                         "prevent violence there!\r\n");
            if (!number(0, 1))
                act("$n seems to be violently disturbed.", false,
                    this, NULL, NULL, TO_ROOM);
            else
                act("$n becomes violently agitated for a moment.",
                    false, this, NULL, NULL, TO_ROOM);
        }
        return false;
    }

    // Disallow attacking members of your own group
    if (IS_PC(this)
        && AFF_FLAGGED(this, AFF_GROUP)
        && AFF_FLAGGED(vict, AFF_GROUP)
        && this->master
        && vict->master
        && (this->master == vict
            || vict->master == this
            || this->master == vict->master)) {
        send_to_char(this, "You can't attack a member of your group!\r\n");
        return false;
    }

    // If either Creature is a mob and we're not in an NVZ
    // It's always ok
    if (IS_NPC(vict) || IS_NPC(this))
        return true;

    // At this point, we have to be dealing with PVP
    // Start checking killer prefs and zone restrictions
    if (!PRF2_FLAGGED(this, PRF2_PKILLER)) {
        if (mssg) {
            send_to_char(this, "A small dark shape flies in from the future "
                         "and sticks to your tongue.\r\n");
        }
        return false;
    }

    // If a newbie is trying to attack someone, don't let it happen
    if (this->isNewbie()) {
        if (mssg) {
            send_to_char(this, "You are currently under new player "
                         "protection, which expires at level 41\r\n");
            send_to_char(this, "You cannot attack other players "
                         "while under this protection.\r\n");
        }
        return false;
    }

    // If someone is trying to attack a newbie, also don't let it
    // happen
    if (vict->isNewbie()) {
        if (mssg) {
            act("$N is currently under new character protection.",
                false, this, NULL, vict, TO_CHAR);
            act("You are protected by the gods against $n's attack!",
                false, this, NULL, vict, TO_VICT);
            slog("%s protected against %s (Creature::isOkToAttack()) at %d",
                 GET_NAME(vict), GET_NAME(this), vict->in_room->number);
        }

        return false;
    }

    // If they aren't in the same quest it's not ok to attack them
    if (GET_QUEST(this) && GET_QUEST(this) != GET_QUEST(vict)) {
        if (mssg)
            send_to_char(this,
                    "%s is not in your quest and may not be attacked!\r\n",
                    PERS(vict, this));
        qlog(this,
             tmp_sprintf("%s has attacked non-questing PC %s",
                         GET_NAME(this), GET_NAME(vict)),
             QLOG_BRIEF, MAX(GET_INVIS_LVL(this), LVL_AMBASSADOR), true);

        return false;
    }

    if (GET_QUEST(vict) && GET_QUEST(vict) != GET_QUEST(this)) {
        if (mssg)
            send_to_char(this,
                         "%s is on a godly quest and may not be attacked!\r\n",
                         PERS(vict, this));

        qlog(this,
             tmp_sprintf("%s has attacked questing PC %s",
                         GET_NAME(this), GET_NAME(vict)),
             QLOG_BRIEF, MAX(GET_INVIS_LVL(this), LVL_AMBASSADOR), true);

        return false;
    }

    // We're not in an NVZ, or an arena, and nobody is a newbie, so
    // check to see if we're in a !PK zone
    if (this->in_room->zone->getPKStyle() == ZONE_NO_PK ||
        vict->in_room->zone->getPKStyle() == ZONE_NO_PK) {
        if (mssg) {
            send_to_char(this, "You seem to be unable to bring "
                             "your weapon to bear on %s.\r\n",
                         GET_NAME(vict));
            act("$n shakes with rage as $e tries to bring $s "
                "weapon to bear.", false, this, NULL, NULL, TO_ROOM);

        }
        return false;
    }

   return true;
}

void
Creature::ignite(Creature *ch)
{
    affected_type af;

    memset(&af, 0x0, sizeof(affected_type));
    af.type = SPELL_ABLAZE;
    af.duration = -1;
    af.bitvector = AFF2_ABLAZE;
    af.aff_index = 2;

    if (ch)
        af.owner = ch->getIdNum();

    affect_to_char(this, &af);
}

void
Creature::extinguish()
{
    affect_from_char(this, SPELL_ABLAZE);
}

void
Creature::stopDefending()
{
    if (!this->isDefending())
        return;

    act("You stop defending $N.",
        true, this, 0, this->isDefending(), TO_CHAR);
    if (this->in_room == this->isDefending()->in_room) {
        act("$n stops defending you against attacks.",
            false, this, 0, this->isDefending(), TO_VICT);
        act("$n stops defending $N.",
            false, this, 0, this->isDefending(), TO_NOTVICT);
    }

    char_specials.defending = NULL;

    defendingList.remove(this);
}

void
Creature::startDefending(Creature *vict)
{
    if (this->isDefending())
        this->stopDefending();

    char_specials.defending = vict;

    act("You start defending $N against attacks.",
        true, this, 0, vict, TO_CHAR);
    act("$n starts defending you against attacks.",
        false, this, 0, vict, TO_VICT);
    act("$n starts defending $N against attacks.",
        false, this, 0, vict, TO_NOTVICT);

    defendingList.add(this);
}

void
Creature::dismount()
{
    if (!this->isMounted())
        return;

    char_specials.mounted = NULL;

    mountedList.remove(this);
}

void
Creature::mount(Creature *vict)
{
    if (this->isMounted())
        return;

    char_specials.mounted = vict;

    mountedList.add(this);
}

void
Creature::startHunting(Creature *vict)
{
	if (!char_specials.hunting)
		huntingList.add(this);

    char_specials.hunting = vict;
}

void
Creature::stopHunting()
{
    char_specials.hunting = NULL;

    huntingList.remove(this);
}

bool
Creature::checkReputations(Creature *vict)
{
    bool ch_msg = false, vict_msg = false;

    if (!this)
        return false;

    if (is_arena_combat(this, vict))
        return false;

    if (GET_LEVEL(this) > LVL_AMBASSADOR)
        return false;

    if (IS_NPC(vict))
        return false;

    if (IS_NPC(this) && this->master && !IS_NPC(this->master)) {
        if (GET_REPUTATION(this->master) <= 0)
            ch_msg = true;
        else if (GET_REPUTATION(vict) <= 0)
            vict_msg = true;
    }
    else if (IS_NPC(this))
        return false;

    if (GET_REPUTATION(this) <= 0)
        ch_msg = true;
    else if (GET_REPUTATION(vict) <= 0)
        vict_msg = true;

    if (ch_msg) {
        send_to_char(this, "Your reputation is 0.  If you want to be "
                           "a player killer, type PK on yes.\r\n");
        send_to_char(vict, "%s has just tried to attack you but was "
                           "prevented by %s reputation being 0.\r\n",
                     GET_NAME(this), HSHR(this));
        return true;
    }

    if (vict_msg) {
        send_to_char(this, "%s's reputation is 0 and %s is immune to player "
                           "versus player violence.\r\n", GET_NAME(vict), HSSH(vict));
        send_to_char(vict, "%s has just tried to attack you but was "
                           "prevented by your reputation being 0.\r\n",
                     GET_NAME(this));
        return true;
    }

    return false;
}

//not inlined because we need access to spells.h
bool
affected_type::clearAtDeath(void) {
    return (type != SPELL_ITEM_REPULSION_FIELD &&
    type != SPELL_ITEM_ATTRACTION_FIELD);
}
#undef __Creature_cc__

