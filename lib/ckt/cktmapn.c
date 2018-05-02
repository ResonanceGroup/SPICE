/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /* 
	(ckt,node)
     *  map the given node to the compact node numbering set of the
     * specified circuit
     */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "sperror.h"
#include "cktdefs.h"
#include "suffix.h"


/*
	Is a version of CKTmkVolt that first looks through the entire list of existing nodes to
	find out if the node alreay exists before creating it.
	This function is much less efficienct than CKTmkVolt due to the need to search through the 
	entire list of circuit nodes, and is no longer used.
*/
///<param name = "*ckt"> Circuit to operate on </param>
///<param name = "**node"> Node to be returned </param>
///<param name = "name"> Name of node </param>
int CKTmapNode(GENERIC *ckt, GENERIC **node, IFuid name)
{
	register CKTnode *here;
	int error;
	IFuid uid;
	CKTnode *mynode;

	for (here = ((CKTcircuit *)ckt)->CKTnodes; here; here = here->next)
	{
		if (here->name == name)
		{
			if (node)
				*node = (char *)here;

			return(E_EXISTS);
		}
	}
	/* not found, so must be a new one */
	error = CKTmkNode((CKTcircuit*)ckt, &mynode); /*allocate the node*/
	if (error)
		return(error);

	error = (*(SPfrontEnd->IFnewUid))((GENERIC *)ckt, &uid, (IFuid*)NULL, name, UID_SIGNAL, (GENERIC**)mynode);  /* get a uid for it */

	if (error)
		return(error);

	mynode->name = uid;     /* set the info we have */
	mynode->type = SP_VOLTAGE;
	error = CKTlinkEq((CKTcircuit*)ckt, mynode); /* and link it in */
	if (node)
		*node = (GENERIC *)mynode; /* and finally, return it */
	return(OK);
}
