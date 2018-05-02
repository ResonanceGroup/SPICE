/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include "cktdefs.h"
#include <stdio.h>
#include "sperror.h"
#include "trandefs.h"
#include "util.h"
#include "suffix.h"

extern SPICEanalysis *analInfo[];
extern int ANALmaxnum;



/*
	This is the master routine for actually running SPICE jobs.
	CKTdoJob takes a task and performs all of the analyses described in it.
	CKTdoJob is free to re-arrange the analyses within the task at its discretion, combining them,
	sorting them for reduced computation time, or other optimizations
	desired.
	In the case of this routine, it extracts the values set in the "options" analysis and
	sets them in the circuit structure.
	Then, depending on the value of the restart parameter and the state saed in the task structure,
	it either jumps to a specific analysis or steps through the analyses in it perferred order.
	Note that many of the analyses are special cased in this routine, such as extracting any sensitivity
	analysis and setting it up to be performed during the other analyses as appropriate.

	This function instrUots the simulator to perform the analyses within the specified task that have
	been requested by previous fuoction calls.
	The restart parameter has several different values which determine how the simulator will
	proceed. If restart is RESUME, the simulator should attempt to resume hom where it stopped if possible
	even if that is in the middle of a paused or aborted analysis, and failing that, restm the aborted
	simulation without remMing the entire task. If restart is RESTART, the simulator should throw away
	any analysis results obtained so far for this task and start all over again. If restart is SKIPTONEXT,
	the simulator should give up on the analysis in progress, but attempt to continue with any other
	scheduled analyses in this task. When starting an analysis from the beginning, the value RESTART
	should be given.
*/
///<param name = "*ckt"> Circuit to operate on </param>
///<param name = "reset"> Restart or continue? </param>
///<param name = "*task"> Task to perform </param>
int CKTdoJob(GENERIC *inCkt, int reset, GENERIC *inTask)
{
	CKTcircuit	*ckt = (CKTcircuit *)inCkt;
	TSKtask	*task = (TSKtask *)inTask;
	JOB		*job;
	double	startTime;
	int		error, i, error2;

#ifdef HAS_SENSE2
	int		senflag;
	static int	sens_num = -1;

	/* Sensitivity is special */
	if (sens_num < 0)
	{
		for (i = 0; i < ANALmaxnum; i++)
		{
			if (!strcmp("SENS2", analInfo[i]->public.name))
				break;
		}

		sens_num = i;
	}
#endif

	startTime = (*(SPfrontEnd->IFseconds))();

	ckt->CKTtemp = task->TSKtemp;
	ckt->CKTnomTemp = task->TSKnomTemp;
	ckt->CKTmaxOrder = task->TSKmaxOrder;
	ckt->CKTintegrateMethod = task->TSKintegrateMethod;
	ckt->CKTbypass = task->TSKbypass;
	ckt->CKTdcMaxIter = task->TSKdcMaxIter;
	ckt->CKTdcTrcvMaxIter = task->TSKdcTrcvMaxIter;
	ckt->CKTtranMaxIter = task->TSKtranMaxIter;
	ckt->CKTnumSrcSteps = task->TSKnumSrcSteps;
	ckt->CKTnumGminSteps = task->TSKnumGminSteps;
	ckt->CKTminBreak = task->TSKminBreak;
	ckt->CKTabstol = task->TSKabstol;
	ckt->CKTpivotAbsTol = task->TSKpivotAbsTol;
	ckt->CKTpivotRelTol = task->TSKpivotRelTol;
	ckt->CKTreltol = task->TSKreltol;
	ckt->CKTchgtol = task->TSKchgtol;
	ckt->CKTvoltTol = task->TSKvoltTol;
	ckt->CKTgmin = task->TSKgmin;
	ckt->CKTdelmin = task->TSKdelmin;
	ckt->CKTtrtol = task->TSKtrtol;
	ckt->CKTdefaultMosL = task->TSKdefaultMosL;
	ckt->CKTdefaultMosW = task->TSKdefaultMosW;
	ckt->CKTdefaultMosAD = task->TSKdefaultMosAD;
	ckt->CKTdefaultMosAS = task->TSKdefaultMosAS;
	ckt->CKTfixLimit = task->TSKfixLimit;
	ckt->CKTnoOpIter = task->TSKnoOpIter;
	ckt->CKTtryToCompact = task->TSKtryToCompact;
	ckt->CKTbadMos3 = task->TSKbadMos3;
	ckt->CKTkeepOpInfo = task->TSKkeepOpInfo;
	ckt->CKTtroubleNode = 0;
	ckt->CKTtroubleElt = NULL;
#ifdef NEWTRUNC
	ckt->CKTlteReltol = task->TSKlteReltol;
	ckt->CKTlteAbstol = task->TSKlteAbstol;
#endif /* NEWTRUNC */

	error = 0;

	if (reset)
	{

		ckt->CKTdelta = 0.0;
		ckt->CKTtime = 0.0;
		ckt->CKTcurrentAnalysis = 0;

#ifdef HAS_SENSE2
		senflag = 0;
		if (sens_num < ANALmaxnum)
		{

			for (job = task->jobs; !error && job; job = job->JOBnextJob)
			{
				if (job->JOBtype == sens_num)
				{
					senflag = 1;
					ckt->CKTcurJob = job;
					ckt->CKTsenInfo = (SENstruct *)job;
					error = (*(analInfo[sens_num]->an_func))(ckt, reset);
				}
			}
		}

		if (ckt->CKTsenInfo && (!senflag || error))
			FREE(ckt->CKTsenInfo);
#endif

		/* normal reset */
		if (!error)
			error = CKTunsetup(ckt);
		if (!error)
			error = CKTsetup(ckt);
		if (!error)
			error = CKTtemp(ckt);
		if (error)
			return error;
	}

	error2 = OK;

	/* Analysis order is important */
	for (i = 0; i < ANALmaxnum; i++)
	{

#ifdef HAS_SENSE2
		if (i == sens_num)
			continue;
#endif

		for (job = task->jobs; job; job = job->JOBnextJob)
		{
			if (job->JOBtype == i)
			{
				ckt->CKTcurJob = job;
				error = OK;
				if (analInfo[i]->an_init)
					error = (*(analInfo[i]->an_init))(ckt, job);
				if (!error && analInfo[i]->do_ic)
					error = CKTic(ckt);
				if (!error)
					error = (*(analInfo[i]->an_func))(ckt, reset);
				if (error)
					error2 = error;
			}
		}
	}

	ckt->CKTstat->STATtotAnalTime += (*(SPfrontEnd->IFseconds))() - startTime;

#ifdef HAS_SENSE2
	if (ckt->CKTsenInfo)
		SENdestroy(ckt->CKTsenInfo);
#endif

	return(error2);
}

