#ifndef _LOGIN_H_
#define _LOGIN_H_

//
// File: login.h
//
// Copyright 1998 John Watson, all rights reserved
//

void show_menu(struct descriptor_data *d);
void show_pc_race_menu(struct descriptor_data *d);
void show_race_restrict(struct descriptor_data *d, int timeframe);
void show_char_class_menu(struct descriptor_data *d, bool remort);
void show_pc_race_help(struct descriptor_data *d, char *arg);
void show_pc_class_help(struct descriptor_data *d, char *arg);

int parse_pc_race(struct descriptor_data *d, char *arg);

#define MODE_RENT_MENU 0
#define MODE_SHOW_MENU 1
#endif
