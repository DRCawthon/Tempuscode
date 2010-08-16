//
// File: progeditor.cc                        -- part of TempusMUD
//

#ifdef HAS_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <fcntl.h>
// Tempus Includes
#include "screen.h"
#include "desc_data.h"
#include "comm.h"
#include "db.h"
#include "utils.h"
#include "login.h"
#include "interpreter.h"
#include "boards.h"
#include "mail.h"
#include "editor.h"
#include "tmpstr.h"
#include "accstr.h"
#include "help.h"
#include "comm.h"
#include "player_table.h"

void
start_editing_prog(descriptor_data *d, void *owner, prog_evt_type owner_type)
{
	if (d->text_editor) {
		errlog("Text editor object not null in start_editing_mail");
		REMOVE_BIT(PLR_FLAGS(d->creature),
			PLR_WRITING | PLR_OLC | PLR_MAILING);
		return;
	}

    SET_BIT(PLR_FLAGS(d->creature), PLR_WRITING | PLR_OLC);

	d->text_editor = new CProgEditor(d, owner, owner_type);
}

CProgEditor_CProgEditor(descriptor_data *desc,
                         void *o,
                         prog_evt_type t)
    : CEditor(desc, MAX_STRING_LENGTH)
{
    owner = o;
    owner_type = t;
    ImportText(prog_get_text(owner, owner_type));

    SendStartupMessage();
    DisplayBuffer();
}

bool
CProgEditor_PerformCommand(char cmd, char *args)
{
    switch (cmd) {
    case 'u':
        UndoChanges();
        break;
    default:
        return CEditor_PerformCommand(cmd, args);
    }

    return true;
}

void
CProgEditor_Finalize(const char *text)
{
    char *new_prog = (*text) ? strdup(text):NULL;

    switch (owner_type) {
    case PROG_TYPE_MOBILE:
        if (GET_MOB_PROG((struct creature *)owner))
            free(GET_MOB_PROG((struct creature *)owner));
        ((struct creature *)owner)->mob_specials.shared->prog = new_prog;
        break;
    case PROG_TYPE_ROOM:
        if (GET_ROOM_PROG((struct room_data *)owner))
            free(((struct room_data *)owner)->prog);
        ((struct room_data *)owner)->prog = new_prog;
        break;
    default:
        errlog("Can't happen");
    }

    prog_compile(desc->creature, owner, owner_type);

    if (IS_PLAYING(desc))
        act("$n nods with satisfaction as $e saves $s work.", true, desc->creature, 0, 0, TO_NOTVICT);
}
