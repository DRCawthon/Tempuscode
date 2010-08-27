#ifdef HAS_CONFIG_H
#include "config.h"
#endif

#include "room_data.h"

bool
struct room_data_isOpenAir(void)
{

	//
	// sector types must not only be added here, but also in
	// act.movement.cc can_travel_sector()
	//

	if (sector_type == SECT_FLYING ||
		sector_type == SECT_ELEMENTAL_AIR ||
		sector_type == SECT_ELEMENTAL_RADIANCE ||
		sector_type == SECT_ELEMENTAL_LIGHTNING ||
		sector_type == SECT_ELEMENTAL_VACUUM)
		return true;

	return false;
}

struct room_data_struct room_data(room_num n, struct zone_data *z)
    : thing(ROOM), people(true)
{

	affects = NULL;
	contents = NULL;
	description = NULL;
	ex_description = NULL;
    prog = NULL;
    progobj = NULL;
	prog_state = NULL;
	find_first_step_index = 0;
	flow_dir = 0;
	flow_speed = 0;
	flow_type = 0;
	func = NULL;
	func_param = NULL;
	light = 0;
	max_occupancy = 256;
	name = NULL;
	next = NULL;
	number = n;
	//people = NULL;
	room_flags = 0;
	search = NULL;
	sector_type = 0;
	sounds = NULL;
	trail = NULL;
	zone = z;

	for (int i = 0; i < NUM_OF_DIRS; i++)
		dir_option[i] = NULL;
}

int
struct room_data_countExits(void)
{
	int idx, result = 0;

	for (idx = 0;idx < NUM_OF_DIRS; idx++)
		if (dir_option[idx] &&
				dir_option[idx]->to_room &&
				dir_option[idx]->to_room != this)
			result++;

	return result;
}

#undef __struct room_data_cc__