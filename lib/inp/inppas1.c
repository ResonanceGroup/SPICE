/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "inpdefs.h"
#include "util.h"
#include "ifsim.h"
#include "strext.h"
#include "suffix.h"


/*
	First pass of teh SPICE2 compatible input parse.
	This routine currently only considers ".MODEL" cards, passing them to INPdoModel to
	save for pass 2
*/
///<param name = "*ckt"> The circuit structure of circuit being parsed </param>
///<param name = "*deck"> The input deck to parse </param>
///<param name = "*tab> The symbol table for this circuit </param>
void INPpas1(GENERIC *ckt, card *deck, INPtables *tab)
{
	card *current;
	char *INPdomodel();
	char *temp, *thisline;

	for (current = deck; current != NULL; current = current->nextcard)
	{
		/* SPICE-2 keys off of the first character of the line */
		thisline = current->line;

		while (*thisline && ((*thisline == ' ') || (*thisline == '\t')))
			thisline++;

		if (*thisline == '.')
		{
			if (strncmp(thisline, ".model", 6) == 0)
			{
				temp = INPdomodel(ckt, current, tab);
				current->error = INPerrCat(current->error, temp);
			}
		}

		/* for now, we do nothing with the other cards - just
		 * keep them in the list for pass 2
		 */
	}
}
