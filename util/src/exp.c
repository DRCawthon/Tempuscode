/* ************************************************************************
*  file:  showplay.c                                  Part of CircleMud   *
*  Usage: list a diku playerfile                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
*  Edited for TempusMUD 6-21-95                                           *
************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include "oldstructs.h"


void	show(char *filename)
{
   FILE * fl;
   struct char_file_u player;
   int	num = 0;

   if (!(fl = fopen(filename, "r"))) {
      perror("error opening playerfile");
      exit(1);
   }

   for (; ; ) {
      fread(&player, sizeof(struct char_file_u ), 1, fl);
      if (feof(fl)) {
	 fclose(fl);
	 exit(1);
      }

      if (player.level > 40 && player.level < 50) 
        printf("{%2d, \"%s\", %d}, \r\n",
          player.level, player.name, player.points.exp);
   }
}


int main(int argc, char **argv)
{
   if (argc != 2)
      printf("Usage: %s playerfile-name\n", argv[0]);
   else
      show(argv[1]);

   return 0;
}

