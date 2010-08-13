#ifdef HAS_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <string>
#include <dirent.h>
#include <list>
using namespace std;

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "security.h"
#include "account.h"
#include "utils.h"
#include "house.h"
#include "screen.h"
#include "tokenizer.h"
#include "tmpstr.h"
#include "accstr.h"
#include "player_table.h"
#include "clan.h"
#include "object_map.h"

// usage message
#define HCONTROL_FIND_FORMAT \
"Usage: hcontrol find <'owner' | 'guest' | 'landlord'> <name|id>\r\n"
#define HCONTROL_DESTROY_FORMAT \
"Usage: hcontrol destroy <house#>\r\n"
#define HCONTROL_ADD_FORMAT \
"Usage: hcontrol add <house#> <'room'|'guest'> <room# | name >\r\n"
#define HCONTROL_DELETE_FORMAT \
"Usage: hcontrol delete <house#> <'room'|'guest'> <room# | name >\r\n"
#define HCONTROL_SET_FORMAT \
"Usage: hcontrol set <house#> <rate|owner|type|landlord> <'value'|public|private|rental>\r\n"
#define HCONTROL_SHOW_FORMAT \
"Usage: hcontrol show [house#]\r\n"
#define HCONTROL_BUILD_FORMAT \
"Usage: hcontrol build <player name|account#> <first room#> <last room#>\r\n"

#define HCONTROL_FORMAT \
(HCONTROL_BUILD_FORMAT \
  HCONTROL_DESTROY_FORMAT \
  HCONTROL_ADD_FORMAT \
  HCONTROL_DELETE_FORMAT \
  HCONTROL_SET_FORMAT \
  HCONTROL_SHOW_FORMAT \
  "Usage: hcontrol save/recount\r\n" \
  "Usage: hcontrol where\r\n" \
  "Usage: hcontrol reload [house#] (Use with caution!)\r\n")

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern int no_plrtext;
void extract_norents(struct obj_data *obj);

HouseControl Housing;

/**
 * mode:  true, recursively sums object costs.
 *       false, recursively sums object rent prices
 * obj: the object the add to the current sum
 * top_o: the first object totalled.
**/
int
recurs_obj_cost(struct obj_data *obj, bool mode, struct obj_data *top_o)
{
	if (obj == NULL)
		return 0;				// end of list

	if (obj->in_obj && obj->in_obj != top_o) {
		if (!mode) { /** rent mode **/
			return ((IS_OBJ_STAT(obj, ITEM_NORENT) ? 0 : GET_OBJ_RENT(obj)) +
				recurs_obj_cost(obj->contains, mode, top_o) +
				recurs_obj_cost(obj->next_content, mode, top_o));
		} else {	 /** cost mode **/
			return (GET_OBJ_COST(obj) + recurs_obj_cost(obj->contains, mode,
					top_o)
				+ recurs_obj_cost(obj->next_content, mode, top_o));
		}
	} else if (!mode) {			// rent mode
		return ((IS_OBJ_STAT(obj, ITEM_NORENT) ? 0 : GET_OBJ_RENT(obj)) +
			recurs_obj_cost(obj->contains, mode, top_o));
	} else {
		return (GET_OBJ_COST(obj) + recurs_obj_cost(obj->contains, mode,
				top_o));
	}
}

int
recurs_obj_contents(struct obj_data *obj, struct obj_data *top_o)
{
	if (!obj)
		return 0;

	if (obj->in_obj && obj->in_obj != top_o)
		return (1 + recurs_obj_contents(obj->next_content, top_o) +
			recurs_obj_contents(obj->contains, top_o));

	return (1 + recurs_obj_contents(obj->contains, top_o));
}

const char*
House_getTypeName() {
	switch(getType()) {
		case PRIVATE:
			return "Private";
		case PUBLIC:
			return "Public";
		case RENTAL:
			return "Rental";
		case CLAN:
			return "Clan";
		default:
			return "Invalid";
	}
}

const char*
House_getTypeShortName()
{
	switch(getType()) {
		case PRIVATE:
			return "PRIV";
		case PUBLIC:
			return "PUB";
		case RENTAL:
			return "RENT";
		case CLAN:
			return "CLAN";
		default:
			return "ERR";
	}
}

House_Type
House_getTypeFromName(const char* name) {
	if (name == NULL)
		return INVALID;
	if (strcmp(name, "Private") == 0)
		return PRIVATE;
	if (strcmp(name, "Public") == 0)
		return PUBLIC;
	if (strcmp(name, "Rental") == 0)
		return RENTAL;
	if (strcmp(name, "Clan") == 0)
		return CLAN;
	return INVALID;
}

House_House(int idnum, int owner, room_num first)
	: id(idnum), created(time(0)),
	  type(PRIVATE), ownerID(owner),
	  landlord(0), rentalRate(0), rentOverflow(0), guests(), rooms(),
	  repoNotes()
{
	addRoom(first);
}

House_House()
	: id(0), created(time(0)),
	  type(PRIVATE), ownerID(0),
	  landlord(0), rentalRate(0), rentOverflow(0), guests(), rooms(),
	  repoNotes()
{
}

House_House(const House &h)
	: guests(), rooms(), repoNotes()
{
	*this = h;
}

House&
House_operator=(const House &h)
{
	id = h.id;

	created = h.created;
	type = h.type;
	ownerID = h.ownerID;
	landlord = h.landlord;
	rentalRate = h.rentalRate;
	rentOverflow = h.rentOverflow;

	guests = h.guests;
	rooms = h.rooms;
	repoNotes = h.repoNotes;

	return *this;
}

bool
House_addGuest(long guest)
{
	if (isGuest(guest)) {
		return false;
	} else {
		guests.push_back(guest);
		return true;
	}
}

bool
House_removeGuest(long guest)
{
	GuestList_iterator it = find(guests.begin(), guests.end(), guest);
	if (it == guests.end())
		return false;
	guests.erase(it);
	return true;
}

bool
House_addRoom(room_num room)
{
	if (hasRoom(room)) {
		return false;
	} else {
		rooms.push_back(room);
		return true;
	}
}

bool
House_removeRoom(room_num room)
{
	RoomList_iterator it = find(rooms.begin(), rooms.end(), room);
	if (it == rooms.end())
		return false;
	rooms.erase(it);
	return true;
}

bool
House_isGuest(struct creature *c) const
{
	return isGuest(GET_IDNUM(c));
}

bool
House_isGuest(long idnum) const
{
	switch(getType()) {
		case PRIVATE:
		case PUBLIC:
		case RENTAL:
			return find(guests.begin(), guests.end(), idnum) != guests.end();
		case CLAN: {
			// if there is no clan then check guests
			clan_data *clan = real_clan(getOwnerID());
			if (clan == NULL)
				return find(guests.begin(), guests.end(), idnum) != guests.end();
			// if they're not a member, check the guests
			clanmember_data *member = real_clanmember(idnum, clan);
			if (member == NULL)
				return find(guests.begin(), guests.end(), idnum) != guests.end();
			return true;
		}
		default:
			return false;
	}
}

bool House_hasRoom(struct room_data *room) const
{
	return hasRoom(room->number);
}

bool House_hasRoom(room_num room) const
{
	return find(rooms.begin(), rooms.end(), room) != rooms.end();
}

bool
House_isOwner(struct creature *ch)const
{
	switch(getType()) {
		case PRIVATE:
		case PUBLIC:
		case RENTAL:
			return ownerID == playerIndex.getstruct accountID(GET_IDNUM(ch));
		case CLAN: {
			clan_data *clan = real_clan(getOwnerID());
			if (clan == NULL)
				return false;
			clanmember_data *member = real_clanmember(GET_IDNUM(ch), clan);
			if (member == NULL)
				return false;
			return PLR_FLAGGED(ch, PLR_CLAN_LEADER);
		}
		default:
			return false;
	}
}

unsigned int
HouseControl_getHouseCount() const
{
	return vector<House*>_size();
}

House*
HouseControl_getHouse(int index)
{
	return (*this)[index];
}

bool
HouseControl_createHouse(int owner, room_num firstRoom, room_num lastRoom)
{
	int id = topId +1;
	struct room_data *room = real_room(firstRoom);

	if (room == NULL)
		return false;

	House *house = new House(id, owner, firstRoom);

	SET_BIT(ROOM_FLAGS((room)), ROOM_HOUSE);
	for (int i = firstRoom + 1; i <= lastRoom; i++) {
		struct room_data *room = real_room(i);
		if (room != NULL) {
			house->addRoom(room->number);
			SET_BIT(ROOM_FLAGS((room)), ROOM_HOUSE);
		}
	}
	++topId;
	push_back(house);
	house->save();
	return true;
}

House*
HouseControl_findHouseByRoom(room_num room)
{
	for (unsigned int i = 0; i < getHouseCount(); i++) {
		if (getHouse(i)->hasRoom(room))
			return getHouse(i);
	}
	return NULL;
}

House*
HouseControl_findHouseById(int id)
{
	for (unsigned int i = 0; i < getHouseCount(); i++) {
		if (getHouse(i)->getID() == id)
			return getHouse(i);
	}
	return NULL;
}

House*
HouseControl_findHouseByOwner(int id, bool isstruct account)
{
	for (unsigned int i = 0; i < getHouseCount(); i++) {
		House *house = getHouse(i);
		if (house->getType() == House_PUBLIC)
			continue;
		if (isstruct account && house->getType() == House_CLAN)
			continue;
		if (!isstruct account && house->getType() != House_CLAN)
			continue;
		if (house->getOwnerID() == id)
			return getHouse(i);
	}
	return NULL;
}

/* note: arg passed must be house vnum, so there. */
bool
HouseControl_canEnter(struct creature *ch, room_num room_vnum)
{
	House *house = findHouseByRoom(room_vnum);
	if (house == NULL)
		return true;

	if (   Security_isMember(ch, "House")
		|| Security_isMember(ch, "AdminBasic")
		|| Security_isMember(ch, "WizardFull")) {
		return true;
	}

	if (IS_NPC(ch)) {
		// so charmies can walk around in the master's house
		if (AFF_FLAGGED(ch, AFF_CHARM) && ch->master)
			return canEnter(ch->master, room_vnum);
		return false;
	}

	switch(house->getType()) {
		case House_PUBLIC:
			return true;
		case House_RENTAL:
		case House_PRIVATE:
			if (ch->getstruct accountID() == house->getOwnerID())
				return true;
			return house->isGuest(ch);
		case House_CLAN: {
			clan_data *clan = real_clan(house->getOwnerID());
			if (clan == NULL)
				return true;
			clanmember_data *member = real_clanmember(GET_IDNUM(ch), clan);
			if (member != NULL)
				return true;
			return false;
		}
		case House_INVALID:
			return false;
	}

	return false;
}

bool
HouseControl_canEdit(struct creature *c, struct room_data *room)
{
	return canEdit(c, findHouseByRoom(room->number));
}

bool
HouseControl_canEdit(struct creature *c, House *house)
{
	if (house == NULL)
		return false;
	return Security_isMember(c, "House");
}

bool
HouseControl_destroyHouse(House *house)
{
	iterator it = std_find(begin(), end(), house);
	if (it == end()) {
		errlog("House %d not in HouseControl list.", house->getID());
		return false;
	}

	for (unsigned int i = 0; i < house->getRoomCount(); i++) {
		struct room_data *room = real_room(house->getRoom(i));
		if (room == NULL) {
			errlog("House had invalid room number in destroy: %d", house->getRoom(i));
		} else {
			REMOVE_BIT(ROOM_FLAGS(room), ROOM_HOUSE | ROOM_HOUSE_CRASH);
		}
	}
	unlink(get_house_file_path(house->getID()));
	erase(it);
	delete house;

	return true;
}

void
House_notifyReposession(struct creature *ch)
{
	extern int MAIL_OBJ_VNUM;
	struct obj_data *note;

	if (getRepoNoteCount() == 0)
		return;

	note = read_object(MAIL_OBJ_VNUM);
	if (!note) {
		errlog("Failed to read object MAIL_OBJ_VNUM (#%d)", MAIL_OBJ_VNUM);
		return;
	}

	// Build the repossession note
	acc_string_clear();
	acc_strcat("The following items were sold at auction to cover your back rent:\r\n\r\n", NULL);
	for (unsigned int i = 0; i < getRepoNoteCount(); ++i)
		acc_strcat(getRepoNote(i).c_str(), NULL);
	acc_strcat("\r\n\r\nSincerely,\r\n    The Management\r\n", NULL);

	note->action_desc = strdup(acc_get_string());
	note->plrtext_len = strlen(note->action_desc) + 1;
	obj_to_char(note, ch);
	send_to_char(ch, "The TempusMUD Landlord gives you a letter detailing your bill.\r\n");
	clearRepoNotes();
	save();
}

void
count_objects(struct obj_data *obj)
{
	if (!obj)
		return;

	count_objects(obj->contains);
	count_objects(obj->next_content);

	// don't count NORENT items as being in house
	if (obj->shared->proto && !IS_OBJ_STAT(obj, ITEM_NORENT))
		obj->shared->house_count++;
}
void
HouseControl_countObjects()
{
	struct obj_data *obj = NULL;
    ObjectMap_iterator oi = objectPrototypes.begin();
    for (; oi != objectPrototypes.end(); ++oi) {
        obj = oi->second;
		obj->shared->house_count = 0;
    }

	for (unsigned int i = 0; i < getHouseCount(); i++) {
		House *house = getHouse(i);
		for (unsigned int j = 0; j < house->getRoomCount(); j++) {
			struct room_data *room = real_room(house->getRoom(j));
			if (room != NULL ) {
				count_objects(room->contents);
			}
		}
	}
}

int
House_calcObjectCount() const
{
    int count = 0;
    for (unsigned int j = 0; j < getRoomCount(); j++) {
        struct room_data *room = real_room(getRoom(j));
        if (room != NULL) {
            count += calcObjectCount(room);
        }
    }
    return count;
}

int
House_calcObjectCount(struct room_data* room) const
{
    int count = 0;
    for (struct obj_data* obj = room->contents; obj; obj = obj->next_content) {
        count += recurs_obj_contents(obj, NULL);
    }
    return count;
}

bool
House_save()
{
	char* path = get_house_file_path(getID());
	FILE* ouf = fopen(path, "w");
	if (!ouf) {
		fprintf(stderr, "Unable to open XML house file for save.[%s] (%s)\n",
						path, strerror(errno));
		return false;
	}
	fprintf(ouf, "<housefile>\n");
	fprintf(ouf, "<house id=\"%d\" type=\"%s\" owner=\"%d\" created=\"%ld\"",
				  getID(), getTypeName(), getOwnerID(), getCreated());
	fprintf(ouf, " landlord=\"%ld\" rate=\"%d\" overflow=\"%ld\" >\n",
			      getLandlord(), getRentalRate(), rentOverflow);
	for (unsigned int i = 0; i < getRoomCount(); i++) {
		struct room_data *room = real_room(getRoom(i));
		if (room == NULL)
			continue;
		fprintf(ouf, "    <room number=\"%d\">\n", getRoom(i));
		for (struct obj_data *obj = room->contents; obj != NULL; obj = obj->next_content) {
			obj->saveToXML(ouf);
		}
		fprintf(ouf, "    </room>\n");
		REMOVE_BIT(ROOM_FLAGS(room), ROOM_HOUSE_CRASH);
	}
	for (unsigned int i = 0; i < getGuestCount(); i++) {
		fprintf(ouf, "    <guest id=\"%ld\"></guest>\n", getGuest(i));
	}
	for (unsigned int i = 0; i < getRepoNoteCount(); i++) {
		fprintf(ouf, "    <repossession note=\"%s\"></repossession>\n",
                 xmlEncodeSpecialTmp(getRepoNote(i).c_str()));
	}
	fprintf(ouf, "</house>");
	fprintf(ouf, "</housefile>\n");
	fclose(ouf);

	return true;
}

bool
House_load(const char* filename)
{
	int axs = access(filename, W_OK);

	if (axs != 0) {
		if (errno != ENOENT) {
			errlog("Unable to open xml house file '%s': %s",
				 filename, strerror(errno));
			return false;
		} else {
			return false; // normal no eq file
		}
	}
    xmlDocPtr doc = xmlParseFile(filename);
    if (!doc) {
        errlog("XML parse error while loading %s", filename);
        return false;
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (!root) {
        xmlFreeDoc(doc);
        errlog("XML file %s is empty", filename);
        return false;
    }

	xmlNodePtr houseNode;
	for (houseNode = root->xmlChildrenNode; houseNode; houseNode = houseNode->next) {
		if (xmlMatches(houseNode->name, "house"))
			break;
	}
	if (houseNode == NULL) {
        xmlFreeDoc(doc);
        errlog("XML house file %s has no house node.", filename);
        return false;
	}

	//read house node stuff
	id = xmlGetIntProp(houseNode, "id", -1);
	if (id == -1 || Housing.findHouseById(id) != NULL) {
		errlog("Duplicate house id %d loaded from file %s.", getID(), filename);
		return false;
	}

	char *typeName = xmlGetProp(houseNode, "type");
	setType(getTypeFromName(typeName));
	if (typeName != NULL)
		free(typeName);

	ownerID = xmlGetIntProp(houseNode, "owner", -1);
	created = xmlGetLongProp(houseNode, "created", 0);
	landlord = xmlGetLongProp(houseNode, "landlord", -1);
	rentalRate = xmlGetIntProp(houseNode, "rate", 0);
	rentOverflow = xmlGetLongProp(houseNode, "rentOverflow", 0);

	for (xmlNodePtr node = houseNode->xmlChildrenNode; node; node = node->next)
	{
        if (xmlMatches(node->name, "room")) {
			loadRoom(node);
		} else if (xmlMatches(node->name, "guest")) {
			int id = xmlGetIntProp(node, "id", -1);
				addGuest(id);
		} else if (xmlMatches(node->name, "repossession")) {
			char* note = xmlGetProp(node, "note");
			repoNotes.push_back(note);
			free(note);
		}
	}

	xmlFreeDoc(doc);
	return true;
}
bool
House_loadRoom(xmlNodePtr roomNode)
{
	room_num number = xmlGetIntProp(roomNode, "number", -1);
	struct room_data *room = real_room(number);
	if (room == NULL) {
        errlog("House %d has invalid room: %d", getID(), number);
		return false;
	}

	SET_BIT(ROOM_FLAGS(room), ROOM_HOUSE);
	addRoom(number);
	for (xmlNodePtr node = roomNode->xmlChildrenNode; node; node = node->next)
	{
        if (xmlMatches(node->name, "object")) {
			struct obj_data *obj = create_obj();
			if (! obj->loadFromXML(NULL, NULL, room, node)) {
				extract_obj(obj);
			}
		}
	}
	extract_norents(room->contents);
	return true;
}

void
HouseControl_save()
{
    if (getHouseCount() == 0)
        return;

	slog("HOUSE: Saving %d houses.", getHouseCount());
	for (unsigned int i = 0; i < getHouseCount(); i++) {
		House *house = getHouse(i);
		if (! house->save())
			errlog("Failed to save house %d.",house->getID());
	}
}

void
HouseControl_load()
{
	DIR* dir;
	dirent *file;
	char *dirname;

	for (int i = 0; i <= 9; i++) {
		// If we don't have
		dirname = tmp_sprintf("players/housing/%d", i);
		dir = opendir(dirname);
		if (!dir) {
			mkdir(dirname, 0644);
			dir = opendir(dirname);
			if (!dir) {
				errlog("Couldn't open or create directory %s", dirname);
				safe_exit(-1);
			}
		}
		while ((file = readdir(dir)) != NULL) {
			// Check for correct filename (*.dat)
			if (!rindex(file->d_name, '.'))
				continue;
			if (strcmp(rindex(file->d_name, '.'), ".dat"))
				continue;

			char *filename = tmp_sprintf("%s/%s", dirname, file->d_name);
			House *house = new House();
			if (house->load(filename)) {
				push_back(house);
				slog("HOUSE: Loaded house %d", house->getID());
			} else {
				errlog("Failed to load house file: %s ", filename);
				delete house;
			}

		}
		closedir(dir);
	}
	std_sort(begin(), end(), HouseComparator());
	topId = 1;
	if (getHouseCount() > 0) {
		topId = getHouse(getHouseCount() -1)->getID();
	}

    // Now we preload the accounts that are attached to the house
    extern int production_mode;

    if (production_mode) {
        acc_string_clear();
        acc_sprintf("idnum in (");
        bool first = true;
        for (unsigned int i = 0;i < getHouseCount();i++) {
            House *house = getHouse(i);
            if (house->getOwnerID() &&
                (house->getType() == House_PRIVATE ||
                 house->getType() == House_RENTAL)) {
                if (first)
                    first = false;
                else
                    acc_strcat(",", NULL);
                acc_sprintf("%d", house->getOwnerID());
            }
        }
        acc_strcat(")", NULL);

        slog("Preloading accounts with houses...");
        struct account_preload(acc_get_string());
    }
}

void
HouseControl_collectRent()
{
    extern int production_mode;

    if (production_mode) {
        lastCollection = time(0);

        for (unsigned int i = 0; i < getHouseCount(); i++) {
            House *house = getHouse(i);
            if (house->getType() == House_PUBLIC)
                continue;
            if (house->getType() == House_PRIVATE
                || house->getType() == House_RENTAL)
            {
                // If the player is online, do not charge rent.
                struct account *account = struct account_retrieve(house->getOwnerID());
                if (account == NULL  || account->is_logged_in())
                    continue;
            }

            // Cost per minute
            int cost = (int) ((house->calcRentCost() / 24.0) / 60);
            house->collectRent(cost);
            house->save();
        }
    }
}

int
House_calcRentCost() const
{
	int sum = 0;
	for (unsigned int i = 0; i < getRoomCount(); i++)
    {
		struct room_data *room = real_room(getRoom(i));
        bool pc_in_room;

        struct creatureList_iterator it = room->people.begin();

        pc_in_room = false;
        for (; it != room->people.end(); ++it)
            if (IS_PC(*it))
                pc_in_room = true;

        if (!pc_in_room)
            sum += calcRentCost(room);
	}

	if (getType() == RENTAL)
		sum += getRentalRate() * getRoomCount();

	return sum;
}

int
House_calcRentCost(struct room_data *room) const
{
	if (room == NULL)
		return 0;
	int room_count = calcObjectCount(room);
	int room_sum = 0;

	for (struct obj_data* obj = room->contents; obj; obj = obj->next_content) {
		room_sum += recurs_obj_cost(obj, false, NULL);
	}
	if (room_count > House_MAX_ITEMS) {
		room_sum *= (room_count/House_MAX_ITEMS) + 1;
	}
	return room_sum;
}

void
House_display(struct creature *ch)
{

	const char* landlord = "NONE";
	char created_buf[30];

	if (playerIndex.exists(getLandlord()))
		landlord = playerIndex.getName(getLandlord());

	send_to_desc(ch->desc, "&yHouse[&n%4d&y]  Type:&n %s  &yLandlord:&n %s\r\n",
				  getID(), getTypeName(), landlord);

	if (getType() == PRIVATE || getType() == RENTAL || getType() == PUBLIC) {
		struct account *account = struct account_retrieve(ownerID);
		if (account != NULL) {
			const char* email = "";
			if (account->get_email_addr() && *account->get_email_addr()) {
				email = account->get_email_addr();
			}
			send_to_desc(ch->desc, "&yOwner:&n %s [%d] &c%s&n\r\n",
					account->get_name(), account->get_idnum(), email);
		}
	} else if (getType() == CLAN) {
		clan_data *clan = real_clan(ownerID);
		if (clan == NULL) {
			send_to_desc(ch->desc, "&yOwned by Clan:&n NONE\r\n");
		} else {
			send_to_desc(ch->desc, "&yOwned by Clan:&c %s&n [%d]\r\n",
					      clan->name, clan->number);
		}
	} else {
		send_to_desc(ch->desc, "&yOwner:&n NONE\r\n");
	}

	strftime(created_buf, 29, "%a %b %d, %Y %H:%M:%S", localtime(&created));
	send_to_desc(ch->desc, "&yCreated:&n %s\r\n", created_buf);
	listGuests(ch);
	listRooms(ch);
	if (repoNotes.size() > 0) {
		send_to_desc(ch->desc, "&cReposession Notifications:&n \r\n");
		for (unsigned int i = 0; i < repoNotes.size(); ++i) {
			send_to_desc(ch->desc, "    %s\r\n", repoNotes[i].c_str());
		}
	}
}

char*
print_room_contents(struct creature *ch, struct room_data *real_house_room, bool showContents)
{
	int count;
	char* buf = NULL;
	const char* buf2 = "";

	struct obj_data *obj, *cont;
	if (PRF_FLAGGED(ch, PRF_HOLYLIGHT)) {
		buf2 = tmp_sprintf(" %s[%s%5d%s]%s", CCGRN(ch, C_NRM),
							CCNRM(ch, C_NRM), real_house_room->number,
							CCGRN(ch, C_NRM), CCNRM(ch, C_NRM));
	}

	buf = tmp_sprintf("Room%s: %s%s%s\r\n", buf2, CCCYN(ch, C_NRM),
						real_house_room->name, CCNRM(ch, C_NRM));
	if (! showContents)
		return buf;
	for (obj = real_house_room->contents; obj; obj = obj->next_content) {
		count = recurs_obj_contents(obj, NULL) - 1;
		buf2 = "\r\n";
		if (count > 0) {
			buf2 = tmp_sprintf("     (contains %d)\r\n", count);
		}
		buf = tmp_sprintf("%s   %s%-35s%s  %10d Au%s", buf, CCGRN(ch, C_NRM),
						obj->name, CCNRM(ch, C_NRM),
						recurs_obj_cost(obj, false, NULL), buf2);
		if (obj->contains) {
			for (cont = obj->contains; cont; cont = cont->next_content) {
				count = recurs_obj_contents(cont, obj) - 1;
				buf2 = "\r\n";
				if (count > 0) {
					buf2 = tmp_sprintf("     (contains %d)\r\n", count);
				}
				buf = tmp_sprintf("%s     %s%-33s%s  %10d Au%s", buf, CCGRN(ch, C_NRM),
									cont->name, CCNRM(ch, C_NRM),
									recurs_obj_cost(cont, false, obj), buf2);
			}
		}
	}
	return buf;
}
void
House_listRooms(struct creature *ch, bool showContents)
{
	const char *buf = "";
	for (unsigned int i = 0; i < getRoomCount(); ++i) {
		struct room_data* room = real_room(getRoom(i));
		if (room != NULL) {
			char *line = print_room_contents(ch, room,showContents);
			buf = tmp_strcat(buf,line);
		} else {
			errlog("Room [%5d] of House [%5d] does not exist.", getRoom(i),
				getID());
		}
	}
	if (strlen(buf) > 0)
		page_string(ch->desc, buf);
}

void
House_listGuests(struct creature *ch)
{
	if (getGuestCount() == 0) {
		send_to_char(ch, "No guests defined.\r\n");
		return;
	}

	const char *buf = "     Guests: ";
	char namebuf[80];
	for (unsigned int i = 0; i < getGuestCount(); ++i) {
		sprintf(namebuf, "%s ", playerIndex.getName(getGuest(i)));
		buf = tmp_strcat(buf,namebuf);
	}
	buf = tmp_strcat(buf, "\r\n");
	page_string(ch->desc, buf);
}

int
House_reconcileCollection(int cost)
{
	switch(getType()) {
		case PUBLIC:
			return 0;
		case RENTAL:
		case PRIVATE:
			return reconcilePrivateCollection(cost);
		case CLAN:
			return reconcileClanCollection(cost);
		default:
			return 0;
	}
}

int
House_reconcileClanCollection(int cost)
{
	clan_data *clan = real_clan(getOwnerID());
	if (clan == NULL)
		return 0;
	// First we get as much gold as we can
	if (cost < clan->bank_account) {
		clan->bank_account -= cost;
		cost = 0;
	} else {
		cost -= clan->bank_account;
		clan->bank_account = 0;
	}
	sql_exec("update clans set bank=%lld where idnum=%d",
		clan->bank_account, clan->number);
	return cost;
}

int
House_reconcilePrivateCollection(int cost)
{
	struct account *account = struct account_retrieve(ownerID);
	if (account == NULL) {
		return false;
	}

	// First we get as much gold as we can
	if (cost < account->get_past_bank()) {
		account->withdraw_past_bank(cost);
		cost = 0;
	} else {
		cost -= account->get_past_bank();
		account->set_past_bank(0);
	}

	// If they didn't have enough, try credits
	if (cost > 0) {
		if (cost < account->get_future_bank()) {
			account->withdraw_future_bank(cost);
			cost = 0;
		} else {
			cost -= account->get_future_bank();
			account->set_future_bank(0);
		}
	}
	return cost;
}

// collects the house's rent, selling off items to meet the
// bill, if necessary.
bool
House_collectRent(int cost)
{

	if (cost < rentOverflow) {
		slog("HOUSE: [%d] Previous repossessions covering %d rent.", getID(),cost);
		rentOverflow -= cost;
		cost = 0;
		return false;
	} else {
		cost -= rentOverflow;
	}

	cost = reconcileCollection(cost);

	// If they still don't have enough, start selling stuff
	if (cost > 0) {
		struct obj_data *doomed_obj, *tmp_obj;

		while (cost > 0) {
			doomed_obj = findCostliestObj();
			if (!doomed_obj)
				break;
			time_t ct = time(0);
			char* tm_str = asctime(localtime(&ct));
			*(tm_str + strlen(tm_str) - 1) = '\0';

			char *s = tmp_sprintf("%s : %s sold for %d.\r\n",
				tm_str, tmp_capitalize(doomed_obj->name),
				GET_OBJ_COST(doomed_obj));
			repoNotes.push_back(s);

			slog("HOUSE: [%d] Repossessing [%d]%s for %d to cover rent.",
				getID(), GET_OBJ_VNUM(doomed_obj),
				tmp_capitalize(doomed_obj->name),
				GET_OBJ_COST(doomed_obj));

			// Credit player with value of object
			cost -= GET_OBJ_COST(doomed_obj);

			struct obj_data *destObj = doomed_obj->in_obj;
			struct room_data *destRoom = doomed_obj->in_room;
			// Remove objects within doomed object, if any
			while (doomed_obj->contains) {
				tmp_obj = doomed_obj->contains;
				obj_from_obj(tmp_obj);
				if (destObj != NULL) {
					obj_to_obj(tmp_obj, destObj);
				} else {
					obj_to_room(tmp_obj, destRoom);
				}
			}

			// Remove doomed object
			if (destRoom != NULL) {
				act("$p vanishes in a puff of smoke!",
					false, 0, doomed_obj, 0, TO_ROOM);
			}
			extract_obj(doomed_obj);
		}

		if (cost < 0) {
			// If there's any money left over, store it for the next tick
			rentOverflow = -cost;
		}
		return true;
	}
	return false;
}

struct obj_data *
House_findCostliestObj(void)
{
 	struct obj_data *result = NULL;

	for (unsigned int i = 0; i < getRoomCount(); ++i) {
		struct room_data *room = real_room(getRoom(i));
		if (room != NULL) {
			struct obj_data *o = findCostliestObj(room);
			if (o == NULL)
				continue;
			if (result == NULL || GET_OBJ_COST(o) > GET_OBJ_COST(result)) {
				result = o;
			}
		}
	}
	return result;
}

struct obj_data*
House_findCostliestObj(struct room_data* room)
{
	struct obj_data *result = NULL;
	struct obj_data *cur_obj = room->contents;
	while (cur_obj) {
		if (cur_obj && (!result || GET_OBJ_COST(result) < GET_OBJ_COST(cur_obj)))
			result = cur_obj;

		if (cur_obj->contains)
			cur_obj = cur_obj->contains;	// descend into obj
		else if (!cur_obj->next_content && cur_obj->in_obj)
			cur_obj = cur_obj->in_obj->next_content; // ascend out of obj
		else
			cur_obj = cur_obj->next_content; // go to next obj
	}

	return result;
}

void
hcontrol_build_house(struct creature *ch, char *arg)
{
	char *str;
	room_num virt_top_room, virt_atrium;
	struct room_data *real_atrium;
	int owner;

	// FIRST arg: player's name
	str = tmp_getword(&arg);
	if (!*str) {
		send_to_char(ch, HCONTROL_FORMAT);
		return;
	}
	if (is_number(str)) {
		int id = atoi(str);
		if (id == 0) {
			send_to_char(ch, "Warning, creating house with no owner.\r\n");
		} else if (!struct account_exists(id)) {
			send_to_char(ch, "Invalid account id '%d'.\r\n", id);
			return;
		}
		owner = id;
	} else {
		if (!playerIndex.exists(str)) {
			send_to_char(ch, "Unknown player '%s'.\r\n", str);
			return;
		}
		owner = playerIndex.getstruct accountID(str);
	}

	// SECOND arg: the first room of the house
	str = tmp_getword(&arg);
	if (!*str) {
		send_to_char(ch, "You have to give a beginning room for the range.\r\n");
		send_to_char(ch, HCONTROL_FORMAT);
		return;
	} else if (!is_number(str)) {
        send_to_char(ch, "The beginning room must be a number: %s\r\n", str);
		return;
	}
	virt_atrium = atoi(str);
	House *house = Housing.findHouseByRoom(virt_atrium);
	if (house != NULL) {
		send_to_char(ch, "Room [%d] already exists as room of house [%d]\r\n",
					virt_atrium, house->getID());
		return;
	}
	if ((real_atrium = real_room(virt_atrium)) == NULL) {
		send_to_char(ch, "No such room exists.\r\n");
		return;
	}
	// THIRD arg: Top room of the house.
	// Inbetween rooms will be added
	str = tmp_getword(&arg);
	if (!*str) {
		send_to_char(ch, "You have to give an ending room for the range.\r\n");
		send_to_char(ch, HCONTROL_FORMAT);
		return;
	} else if (!is_number(str)) {
        send_to_char(ch, "The ending room must be a number: %s\r\n", str);
		return;
	}
	virt_top_room = atoi(str);
	if (virt_top_room < virt_atrium) {
		send_to_char(ch, "Top room number is less than Atrium room number.\r\n");
		return;
	} else if (virt_top_room - virt_atrium > 20) {
        send_to_char(ch, "Don't be greedy! Who needs more than 20 rooms?\r\n");
		return;
	}
	House *h = Housing.findHouseByOwner(owner);
	if (h != NULL) {
		send_to_char(ch, "struct account %d already owns house %d.\r\n", owner, h->getID());
	} else if (Housing.createHouse(owner, virt_atrium, virt_top_room)) {
		send_to_char(ch, "House built.  Mazel tov!\r\n");
		House *house = Housing.getHouse(Housing.getHouseCount() - 1);
		house->setLandlord(GET_IDNUM(ch));
		Housing.save();
		slog("HOUSE: %s created house %d for account %d with first room %d.",
			GET_NAME(ch), house->getID(), owner, house->getRoom(0));
	} else {
		send_to_char(ch, "House build failed.\r\n");
	}
}

void
hcontrol_destroy_house(struct creature *ch, char *arg)
{
	if (!*arg) {
		send_to_char(ch, HCONTROL_FORMAT);
		return;
	}
	House *house = Housing.findHouseById(atoi(arg));
	if (house == NULL) {
		send_to_char(ch, "Someone already destroyed house %d.\r\n", atoi(arg));
		return;
	}

	if (! Housing.canEdit(ch, house)) {
		send_to_char(ch, "You cannot edit that house.\r\n");
		return;
	}

	if (Housing.destroyHouse(house)) {
		send_to_char(ch, "House %d destroyed.\r\n", atoi(arg));
		slog("HOUSE: %s destroyed house %d.", GET_NAME(ch), atoi(arg));
	} else {
		send_to_char(ch, "House %d failed to die.\r\n", atoi(arg));
	}
}

void
set_house_clan_owner(struct creature *ch, House *house, char *arg)
{
	int clanID = 0;
	if (is_number(arg)) { // to clan id
		clan_data *clan = real_clan(atoi(arg));
		if (clan != NULL) {
			clanID = atoi(arg);
		} else {
			send_to_char(ch, "Clan %d doesn't exist.\r\n", atoi(arg));
			return;
		}
	} else {
		send_to_char(ch, "When setting a clan owner, the clan ID must be used.\r\n");
		return;
	}
	// An account may only own one house
	House *h = Housing.findHouseByOwner(clanID, false);
	if (h != NULL) {
		send_to_char(ch, "Clan %d already owns house %d.\r\n",
					 clanID, h->getID());
		return;
	}

	house->setOwnerID(clanID);
	send_to_char(ch, "Owner set to clan %d.\r\n", house->getOwnerID());
	slog("HOUSE: Owner of house %d set to clan %d by %s.",
			house->getID(), house->getOwnerID(), GET_NAME(ch));
}

void
set_house_account_owner(struct creature *ch, House *house, char *arg)
{
	int accountID = 0;
	if (isdigit(*arg)) { // to an account id
		if (struct account_exists(atoi(arg))) {
			accountID = atoi(arg);
		} else {
			send_to_char(ch, "struct account %d doesn't exist.\r\n", atoi(arg));
			return;
		}
	} else { // to a player name
		if (playerIndex.exists(arg)) {
			accountID = playerIndex.getstruct accountID(arg);
		} else {
			send_to_char(ch, "There is no such player, '%s'\r\n", arg);
			return;
		}
	}
	// An account may only own one house
	House *h = Housing.findHouseByOwner(accountID);
	if (h != NULL) {
		send_to_char(ch, "struct account %d already owns house %d.\r\n",
					 accountID, h->getID());
		return;
	}

	house->setOwnerID(accountID);
	send_to_char(ch, "Owner set to account %d.\r\n", house->getOwnerID());
	slog("HOUSE: Owner of house %d set to account %d by %s.",
			house->getID(), house->getOwnerID(), GET_NAME(ch));

}

void
hcontrol_set_house(struct creature *ch, char *arg)
{
	char *arg1 = tmp_getword(&arg);
	char *arg2 = tmp_getword(&arg);

	if (!*arg1 || !*arg2) {
		send_to_char(ch, HCONTROL_FORMAT);
		return;
	}

	if (!isdigit(*arg1)) {
		send_to_char(ch, "The house id must be a number.\r\n");
		return;
	}

	House *house = Housing.findHouseById(atoi(arg1));
	if (house == NULL) {
		send_to_char(ch, "House id %d not found.\r\n", atoi(arg1));
		return;
	}
	if (!Housing.canEdit(ch, house)) {
		send_to_char(ch, "You cannot edit that house.\r\n");
		return;
	}

	if (is_abbrev(arg2, "rate")) {
		if (!*arg) {
			send_to_char(ch, "Set rental rate to what?\r\n");
		} else {
			arg1 = tmp_getword(&arg);
			house->setRentalRate(atoi(arg1));
			send_to_char(ch, "House %d rental rate set to %d/day.\r\n",
				house->getID(), house->getRentalRate());
			slog("HOUSE: Rental rate of house %d set to %d by %s.",
					house->getID(), house->getRentalRate(), GET_NAME(ch));
		}
	} else if (is_abbrev(arg2, "type")) {
		if (!*arg) {
			send_to_char(ch, "Set mode to public, private, or rental?\r\n");
			return;
		} else if (is_abbrev(arg, "public")) {
			if (house->getType() != House_PUBLIC) {
				house->setType(House_PUBLIC);
				house->setOwnerID(0);
			}
		} else if (is_abbrev(arg, "private")) {
			if (house->getType() != House_PRIVATE) {
				if (house->getType() != House_RENTAL)
					house->setOwnerID(0);
				house->setType(House_PRIVATE);
			}
		} else if (is_abbrev(arg, "rental")) {
			if (house->getType() != House_RENTAL) {
				if (house->getType() != House_PRIVATE)
					house->setOwnerID(0);
				house->setType(House_RENTAL);
			}
		} else if (is_abbrev(arg, "clan")) {
			if (house->getType() != House_CLAN) {
				house->setType(House_CLAN);
				house->setOwnerID(0);
			}
		} else {
			send_to_char(ch,
				"You must specify public, private, clan, or rental!!\r\n");
			return;
		}
		send_to_char(ch, "House %d type set to %s.\r\n",
				house->getID(), house->getTypeName());
		slog("HOUSE: Type of house %d set to %s by %s.",
				house->getID(), house->getTypeName(), GET_NAME(ch));

	} else if (is_abbrev(arg2, "landlord")) {
		if (!*arg) {
			send_to_char(ch, "Set who as the landlord?\r\n");
			return;
		}
		char *landlord = tmp_getword(&arg);
		if (!playerIndex.exists(landlord)) {
			send_to_char(ch, "There is no such player, '%s'\r\n", arg);
			return;
		}

		house->setLandlord(playerIndex.getID(landlord));

		send_to_char(ch, "Landlord of house %d set to %s.\r\n",
				house->getID(), playerIndex.getName(house->getLandlord()));
		slog("HOUSE: Landlord of house %d set to %s by %s.",
				house->getID(), playerIndex.getName(house->getLandlord()), GET_NAME(ch));
		return;
	} else if (is_abbrev(arg2, "owner")) {
		if (!*arg) {
			send_to_char(ch, "Set owner of what house?\r\n");
			return;
		}
		char *owner = tmp_getword(&arg);
		switch(house->getType()) {
			case House_PRIVATE:
			case House_PUBLIC:
			case House_RENTAL:
				set_house_account_owner(ch, house, owner);
				break;
			case House_CLAN:
				set_house_clan_owner(ch, house, owner);
				break;
			case House_INVALID:
				send_to_char(ch, "Invalid house type. Nothing set.\r\n");
				break;
		}

		return;
	}
	Housing.save();
}

void
hcontrol_where_house(struct creature *ch)
{
	House *h = Housing.findHouseByRoom(ch->in_room->number);

	if (h == NULL) {
		send_to_char(ch, "You are not in a house.\r\n");
		return;
	}

	send_to_char(ch,
		"You are in house id: %s[%s%6d%s]%s\n"
		"              Owner: %d\n",
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM),
		h->getID(),
		CCGRN(ch, C_NRM), CCNRM(ch, C_NRM), h->getOwnerID());
}

void
hcontrol_reload_house(struct creature *ch, char *arg)
{
    const char *arg1 = tmp_getword(&arg);
    House *h = NULL;

    if (arg1 == NULL || *arg1 == '\0') {
        if (!(h = Housing.findHouseByRoom(ch->in_room->number))) {
            send_to_char(ch, "You are not in a house.\r\n");
            return;
        }
    } else {
        if (!atoi(arg1)) {
		    send_to_char(ch, HCONTROL_FORMAT);
            return;
        }
	    if (!(h = Housing.findHouseById(atoi(arg1)))) {
            send_to_char(ch, "House id: [%d] does not exist\r\n", atoi(arg1));
            return;
        }
    }
    if (Housing.reload(h)) {
        send_to_char(ch, "Reload complete. It might even have worked.\r\n");
    } else {
        send_to_char(ch, "Reload failed. Figure it out yourself oh great coder.\r\n");
    }
}

bool
HouseControl_reload(House *house)
{
    if (house == NULL)
        return false;

 	char *path = tmp_strcat(get_house_file_path(house->getID()), ".reload", NULL);
	int axs = access(path, R_OK);
	if (axs != 0) {
		if (errno != ENOENT) {
			errlog("Unable to open xml house file '%s' for reload: %s",
                   path, strerror(errno));
			return false;
		} else {
			errlog("Unable to open xml house file '%s' for reload.", path);
			return false;
		}
	}

    for (unsigned int i = 1; i < house->getRoomCount(); i++) {
        int room_num = house->getRoom(i);
        struct room_data* room = real_room(room_num);
        for (struct obj_data* obj = room->contents; obj;) {
            struct obj_data* next_o = obj->next_content;
            extract_obj(obj);
            obj = next_o;
        }
    }

    Housing.destroyHouse(house);
    house = new House();
    if (house->load(path)) {
        push_back(house);
        std_sort(begin(), end(), HouseComparator());
        topId = 1;
        if (getHouseCount() > 0) {
            topId = getHouse(getHouseCount() -1)->getID();
        }
        slog("HOUSE: Reloaded house %d", house->getID());
        return true;
    } else {
        errlog("Failed to reload house file: %s ", path);
        return false;
    }
}
/* Misc. administrative functions */

void
hcontrol_add_to_house(struct creature *ch, char *arg)
{
	// first arg is the atrium vnum
	char *str = tmp_getword(&arg);
	if (!*str || !*arg) {
		send_to_char(ch, HCONTROL_ADD_FORMAT);
		return;
	}

	House *house = Housing.findHouseById(atoi(str));

	if (house == NULL) {
		send_to_char(ch, "No such house exists with id %d.\r\n", atoi(str));
		return;
	}

	if (! Housing.canEdit(ch, house)) {
		send_to_char(ch, "You cannot edit that house.\r\n");
		return;
	}
	// turn arg into the final argument and arg1 into the room/guest/owner arg
	str = tmp_getword(&arg);

	if (!*str || !*arg) {
		send_to_char(ch, HCONTROL_ADD_FORMAT);
		return;
	}

	if (is_abbrev(str, "room")) {
		int roomID = atoi(arg);
		struct room_data *room = real_room(roomID);
		if (room == NULL) {
			send_to_char(ch, "No such room exists.\r\n");
			return;
		}
		House* h = Housing.findHouseByRoom(roomID);
		if (h != NULL) {
			send_to_char(ch, "Room [%d] already exists as room of house [%d]\r\n",
						roomID, h->getID());
			return;
		}
		house->addRoom(roomID);
		SET_BIT(ROOM_FLAGS(room), ROOM_HOUSE);
		send_to_char(ch, "Room %d added to house %d.\r\n",
					roomID, house->getID());
		slog("HOUSE: Room %d added to house %d by %s.",
				roomID, house->getID(), GET_NAME(ch));
		house->save();
	} else {
		send_to_char(ch, HCONTROL_ADD_FORMAT);
	}
}

void
hcontrol_delete_from_house(struct creature *ch, char *arg)
{

	// first arg is the atrium vnum
	char* str = tmp_getword(&arg);
	if (!*str || !*arg) {
		send_to_char(ch, HCONTROL_DELETE_FORMAT);
		return;
	}

	House *house = Housing.findHouseById(atoi(str));

	if (house == NULL) {
		send_to_char(ch, "No such house exists with id %d.\r\n", atoi(str));
		return;
	}

	if (! Housing.canEdit(ch, house)) {
		send_to_char(ch, "You cannot edit that house.\r\n");
		return;
	}

	// turn arg into the final argument and arg1 into the room/guest/owner arg
	str = tmp_getword(&arg);

	if (!*str || !*arg) {
		send_to_char(ch, HCONTROL_DELETE_FORMAT);
		return;
	}
	// delete room
	if (is_abbrev(str, "room")) {

		int roomID = atoi(arg);
		struct room_data *room = real_room(roomID);
		if (room == NULL) {
			send_to_char(ch, "No such room exists.\r\n");
			return;
		}
		House* h = Housing.findHouseByRoom(roomID);
		if (h == NULL) {
			send_to_char(ch, "Room [%d] isn't a room of any house!\r\n", roomID);
			return;
		} else if (house != h) {
			send_to_char(ch, "Room [%d] belongs to house [%d]!\r\n",roomID, h->getID());
			return;
		} else if (house->getRoomCount() == 1) {
			send_to_char(ch, "Room %d is the last room in house [%d]. Destroy it instead.\r\n",
					roomID, h->getID());
			return;
		}

		house->removeRoom(roomID);
		REMOVE_BIT(ROOM_FLAGS(room), ROOM_HOUSE | ROOM_HOUSE_CRASH);
		send_to_char(ch, "Room %d removed from house %d.\r\n",
					roomID, house->getID());
		slog("HOUSE: Room %d removed from house %d by %s.",
				roomID, house->getID(), GET_NAME(ch));
		house->save();
	} else {
		send_to_char(ch, HCONTROL_FORMAT);
	}
}

list<House*>_iterator
remove_house( list<House*> &houses,
               list<House*>_iterator h)
{
    if (h == houses.begin()) {
        houses.erase(h);
        return houses.begin();
    } else {
        list<House*>_iterator it = h;
        --it;
        houses.erase(h);
        return ++it;
    }
}

void
match_houses(list<House*> &houses, int mode, const char *arg)
{
    list<House*>_iterator cur = houses.begin();
    while(cur != houses.end()) {
        switch(mode) {
            case HC_OWNER:
			{
				long id = 0;
				if (isdigit(*arg)) {
					id = atoi(arg);
				} else {
					id = playerIndex.getstruct accountID(arg);
				}
                if ((*cur)->getOwnerID() != id) {
                    cur = remove_house(houses, cur);
                } else {
                    cur++;
                }
                break;
			}
            case HC_LANDLORD:
			{
				long id = playerIndex.getID(arg);
                if ((*cur)->getLandlord() != id) {
                    cur = remove_house(houses, cur);
                } else {
                    cur++;
                }
                break;
			}
            case HC_GUEST:
			{
				long id = playerIndex.getID(arg);
                if (! (*cur)->isGuest(id)) {
                    cur = remove_house(houses, cur);
                } else {
                    cur++;
                }
                break;
			}
            default:
                continue;
        }
    }
}

void
hcontrol_find_houses(struct creature *ch, char *arg)
{
    char token[256];

    if (arg == NULL || *arg == '\0') {
        send_to_char(ch,HCONTROL_FIND_FORMAT);
        return;
    }

    Tokenizer tokens(arg);
    list<House*> houses;

    for (unsigned int i = 0; i < Housing.getHouseCount(); i++) {
		houses.push_back(Housing.getHouse(i));
    }

    while(tokens.hasNext()) {
        tokens.next(token);
        if (strcmp(token,"owner") == 0 && tokens.hasNext()) {
            tokens.next(token);
            match_houses(houses, HC_OWNER, token);
        } else if (strcmp(token,"landlord") == 0 && tokens.hasNext()) {
            tokens.next(token);
            match_houses(houses, HC_LANDLORD, token);
        } else if (strcmp(token,"guest") == 0 && tokens.hasNext()) {
            tokens.next(token);
            match_houses(houses, HC_GUEST, token);
        } else {
            send_to_char(ch,HCONTROL_FIND_FORMAT);
            return;
        }
    }
    if (houses.size() <= 0) {
        send_to_char(ch,"No houses found.\r\n");
        return;
    }
    Housing.displayHouses(houses,ch);
}

/* The hcontrol command itself, used by imms to create/destroy houses */
ACMD(do_hcontrol)
{
	char *action_str;

	if (!Security_isMember(ch, "House")) {
		send_to_char(ch, "You aren't able to edit houses!\r\n");
		return;
	}

	action_str = tmp_getword(&argument);

	if (is_abbrev(action_str, "save")) {
		Housing.save();
		send_to_char(ch, "Saved.\r\n");
		slog("HOUSE: Saved by %s.", GET_NAME(ch));
	} else if (is_abbrev(action_str, "recount")) {
		if (Security_isMember(ch, "Coder")) {
			Housing.countObjects();
			slog("HOUSE: Re-Counted by %s.", GET_NAME(ch));
			send_to_char(ch, "Objs recounted.\r\n");
		} else {
			send_to_char(ch, "You probably shouldn't be doing that.\r\n");
		}
	} else if (is_abbrev(action_str, "build")) {
		hcontrol_build_house(ch, argument);
	} else if (is_abbrev(action_str, "destroy")) {
		hcontrol_destroy_house(ch, argument);
	} else if (is_abbrev(action_str, "add")) {
		hcontrol_add_to_house(ch, argument);
	} else if (is_abbrev(action_str, "delete")) {
		hcontrol_delete_from_house(ch, argument);
	} else if (is_abbrev(action_str, "set")) {
		hcontrol_set_house(ch, argument);
    } else if (is_abbrev(action_str, "find")) {
        hcontrol_find_houses(ch, argument);
    } else if (is_abbrev(action_str, "where")) {
		hcontrol_where_house(ch);
	} else if (is_abbrev(action_str, "show")) {
		if (!*argument) {
			list<House*> houses;
			for (unsigned int i = 0; i < Housing.getHouseCount(); ++i) {
				houses.push_back(Housing.getHouse(i));
			}
			Housing.displayHouses(houses, ch);
		} else {
			char* str = tmp_getword(&argument);
			if (isdigit(*str)) {
				House *house = Housing.findHouseById(atoi(str));
				if (house == NULL) {
					send_to_char(ch, "No such house %d.\r\n", atoi(str));
					return;
				}
				house->display(ch);
			} else {
				send_to_char(ch, HCONTROL_SHOW_FORMAT);
			}
		}
	}
    else if (is_abbrev(action_str, "reload")) {

        if (!Security_isMember(ch, "Coder")) {
            send_to_char(ch, "What are you thinking? You don't even _LOOK_ like a coder.\r\n");
            return;
        }

        hcontrol_reload_house(ch, argument);
    } else {
		send_to_char(ch,HCONTROL_FORMAT);
    }
}

/* The house command, used by mortal house owners to assign guests */
ACMD(do_house)
{
	int found = false;
	char *action_str;
	House *house = NULL;
	action_str = tmp_getword(&argument);

	if (!IS_SET(ROOM_FLAGS(ch->in_room), ROOM_HOUSE)) {
		send_to_char(ch, "You must be in your house to set guests.\r\n");
		return;
	}
	house = Housing.findHouseByRoom(ch->in_room->number);

	if (house == NULL) {
		send_to_char(ch, "Um.. this house seems to be screwed up.\r\n");
		return;
	}

	if (!house->isOwner(ch) && !Security_isMember(ch, "House")) {
		send_to_char(ch, "Only the owner can set guests.\r\n");
		return;
	}

	// No arg, list the guests
	if (!*action_str) {
		send_to_char(ch, "Guests of your house:\r\n");
		if (house->getGuestCount() == 0) {
			send_to_char(ch, "  None.\r\n");
		} else {
			unsigned int j;
			for (j = 0; j < house->getGuestCount(); j++) {
				send_to_char(ch, "%-19s", playerIndex.getName(house->getGuest(j)));
				if (!((j + 1) % 4))
					send_to_char(ch, "\r\n");
			}
			if (j % 4) {
				send_to_char(ch, "\r\n");
			}
		}
		return;
	}

	if (!playerIndex.exists(action_str)) {
		send_to_char(ch, "No such player.\r\n");
		return;
	}
	int playerID = playerIndex.getID(action_str);

	if (house->isGuest(playerID)) {
		house->removeGuest(playerID);
		send_to_char(ch, "Guest deleted.\r\n");
		return;
	}

    int accountID = playerIndex.getstruct accountID(playerID);
    if (house->getOwnerID() == accountID) {
        send_to_char(ch, "They already own the house.\r\n");
		return;
    }

	if (house->getGuestCount() == House_MAX_GUESTS) {
		send_to_char(ch,
			"Sorry, you have the maximum number of guests already.\r\n");
		return;
	}

	if (house->addGuest(playerID)) {
		send_to_char(ch, "Guest added.\r\n");
	}

	// delete any bogus ids
	for (unsigned int i = 0; i < house->getGuestCount(); i++) {
		long guest = house->getGuest(i);
		if (! playerIndex.exists(guest)) {
			house->removeGuest(guest);
			found++;
		}
	}

	if (found > 0) {
		send_to_char(ch,
			"%d bogus guest%s deleted.\r\n", found, found == 1 ? "" : "s");
	}
	Housing.save();
}

void
HouseControl_displayHouses(list<House*> houses, struct creature *ch)
{
    string output;
    send_to_char(ch,"ID   Size Owner  Landlord   Type Rooms\r\n");
    send_to_char(ch,"---- ---- ------ ---------- ---- -----"
                    "-----------------------------------------\r\n");
    list<House*>_iterator cur = houses.begin();

    for (; cur != houses.end(); ++cur)
	{
		House *house = *cur;
		const char *roomlist = "";
		if ( house->getRoomCount() > 0) {
			roomlist = tmp_sprintf("%d", house->getRoom(0));
			for (unsigned int i = 1; i < house->getRoomCount(); i++) {
				roomlist = tmp_sprintf("%s, %d", roomlist, house->getRoom(i));
			}
		}
		if (strlen(roomlist) > 40)
            roomlist = tmp_strcat(tmp_substr(roomlist, 0, 35), "...");

		const char* landlord = "none";
		if (playerIndex.exists(house->getLandlord()))
			landlord = playerIndex.getName(house->getLandlord());
        send_to_char(ch, "%4d %4d %6d %-10s %4s %-45s\r\n",
                      house->getID(),
					  house->getRoomCount(),
                      house->getOwnerID(),
					  landlord,
					  house->getTypeShortName(),
					  roomlist);
    }
}

