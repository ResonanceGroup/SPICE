/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "inpdefs.h"
#include "inpmacs.h"
#include "fteext.h"
#include "suffix.h"


/*
	Parses a transmission line.
	MOdels are not permitted in the SPICE2 input format for this element, so a default
	model is maintained and all transmission lines are instances of this model.
*/
///<param name = "*ckt"> The current circuit </param>
///<param name = "*tab"> The symbol table </param>
///<param name = "*current"> The curren input line </param>
void INP2T(GENERIC *ckt, INPtables *tab, card *current)
{

	/* Tname <node> <node> <node> <node> [TD=<val>]
	 *       [F=<val> [NL=<val>]][IC=<val>,<val>,<val>,<val>] */

	int type;   /* the type the model says it is */
	char *line; /* the part of the current line left to parse */
	char *name; /* the resistor's name */
	char *nname1;   /* the first node's name */
	char *nname2;   /* the second node's name */
	char *nname3;   /* the third node's name */
	char *nname4;   /* the fourth node's name */
	GENERIC *node1; /* the first node's node pointer */
	GENERIC *node2; /* the second node's node pointer */
	GENERIC *node3; /* the third node's node pointer */
	GENERIC *node4; /* the fourth node's node pointer */
	int error;      /* error code temporary */
	GENERIC *fast;  /* pointer to the actual instance */
	int waslead;    /* flag to indicate that funny unlabeled number was found */
	double leadval; /* actual value of unlabeled number */
	IFuid uid;      /* uid for default model */


	type = INPtypelook("Tranline");

	if (type < 0)
	{
		LITERR("Device type Tranline not supported by this binary\n")
			return;
	}

	line = current->line;
	INPgetTok(&line, &name, 1);
	INPinsert(&name, tab);
	INPgetTok(&line, &nname1, 1);
	INPtermInsert(ckt, &nname1, tab, &node1);
	INPgetTok(&line, &nname2, 1);
	INPtermInsert(ckt, &nname2, tab, &node2);
	INPgetTok(&line, &nname3, 1);
	INPtermInsert(ckt, &nname3, tab, &node3);
	INPgetTok(&line, &nname4, 1);
	INPtermInsert(ckt, &nname4, tab, &node4);

	if (!tab->defTmod)
	{
		/* create deafult T model */
		IFnewUid(ckt, &uid, (IFuid)NULL, "T", UID_MODEL, (GENERIC**)NULL);
		IFC(newModel, (ckt, type, &(tab->defTmod), uid))
	}

	IFC(newInstance, (ckt, tab->defTmod, &fast, name))
	IFC(bindNode, (ckt, fast, 1, node1))
	IFC(bindNode, (ckt, fast, 2, node2))
	IFC(bindNode, (ckt, fast, 3, node3))
	IFC(bindNode, (ckt, fast, 4, node4))
	PARSECALL((&line, ckt, type, fast, &leadval, &waslead, tab))
}
