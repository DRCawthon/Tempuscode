#ifdef HAS_CONFIG_H
#include "config.h"
#endif

#include "player_table.h"
#include "utils.h"
#include "db.h"

/* The global player index */
PlayerTable playerIndex;

/**
 *  Creates a blank PlayerTable
**/
PlayerTable_PlayerTable()
{
}

long
PlayerTable_getTopIDNum()
{
	PGresult *res;
	long result;

	res = sql_query("select MAX(idnum) from players");
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		return 0;
	result = atol(PQgetvalue(res, 0, 0));

    return result;
}

/** loads the named victim into the provided struct creature **/
bool
PlayerTable_loadPlayer(const char* name, struct creature *victim) const
{
	long id = getID(name);
	return loadPlayer(id, victim);
}

/** loads the victim with the given id into the provided struct creature **/
bool
PlayerTable_loadPlayer(const long id, struct creature *victim) const
{
	if(id <= 0) {
		return false;
	}
	return victim->loadFromXML(id);
}

/**
 * Returns true if and only if the given id is present in the player table.
**/
bool
PlayerTable_id_exists(long id)
{
	PGresult *res;
	bool result;

	res = sql_query("select COUNT(*) from players where idnum=%ld", id);
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		return 0;
	result = (atoi(PQgetvalue(res, 0, 0)) == 1);

    return result;
}

/**
 * Returns true if and only if the given name is present in the player table.
**/
bool PlayerTable_name_exists(const char* name)
{
	PGresult *res;
	int result;

	res = sql_query("select COUNT(*) from players where lower(name)='%s'",
		tmp_sqlescape(tmp_tolower(name)));
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		return 0;
	result = atoi(PQgetvalue(res, 0, 0));

    if (result > 1)
        errlog("Found %d characters named '%s'", result, tmp_tolower(name));

    return (result == 1);
}

/**
 * returns the char's name or NULL if not found.
**/
const char *
PlayerTable_getName(long id)
{
	PGresult *res;
	char *result;

	res = sql_query("select name from players where idnum=%ld", id);
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		return NULL;
	if (PQntuples(res) == 1)
		result = tmp_strdup(PQgetvalue(res, 0, 0));
	else
		return NULL;

    return result;
}

/**
 * returns chars id or 0 if not found
 *
**/
long
PlayerTable_getID(const char *name) const
{
	PGresult *res;
	long result;

	res = sql_query("select idnum from players where lower(name)='%s'",
		tmp_sqlescape(tmp_tolower(name)));
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		return 0;
	if (PQntuples(res) == 1)
		result = atol(PQgetvalue(res, 0, 0));
	else
		result = 0;

    return result;
}

long
PlayerTable_getstruct accountID(const char *name) const
{
	PGresult *res;
	long result;

	res = sql_query("select account from players where lower(name)='%s'",
		tmp_sqlescape(tmp_tolower(name)));
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		return 0;
	if (PQntuples(res) == 1)
		result = atol(PQgetvalue(res, 0, 0));
	else
		result = 0;

    return result;
}

long
PlayerTable_getstruct accountID(long id) const
{
	PGresult *res;
	long result;

	res = sql_query("select account from players where idnum=%ld", id);
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		return 0;
	if (PQntuples(res) == 1)
		result = atol(PQgetvalue(res, 0, 0));
	else
		result = 0;

    return result;
}

size_t
PlayerTable_size(void) const
{
	PGresult *res;
	long result;

	res = sql_query("select COUNT(*) from players");
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		return 0;
	result = atol(PQgetvalue(res, 0, 0));

    return result;
}
