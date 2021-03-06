/* k_vxdemo.c - VxWorks Demo MIB interface to target System */

/* Copyright 1984-1993 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/*
modification history
--------------------
01a,12oct93,jag  written
*/

/*
DESCRIPTION

This module contains the necessary routines to interface the SNMP agent with
the WRS Demo MIB extension definition.  The MIB is defined in concise MIB 
format in the file vxdemomib.my.  This file is the input to the MIB compiler 
which generates the following:  The data structures that need to be added
to the SNMP agent to support the new MIB extension; The necessary header files; 
The functions headers (or stubs optional) that need be written to support the
MIB extension.

The software that needs to be written to support the MIB extension is divided
in two layers.  The first layer is called "v_" and the second layer is called
"k_".  The corresponding layers in the vxWorks demo MIB are v_vxdemo.c and 
k_vxdemo.c.  The functionality of each layer is described in the corresponding
module.

The SNMP agent is responsible for the communication protocol, proper 
encapsulation of the MIB variables, validation of OIDs and the invokation of
the "v_" routines.  The "v_" layer validates the value and operation of the
MIB variable and calls the respective "k_" routine.  The "k_" routine does the
actual operation on the variable.

The groups in the vxWorks Demo MIB sysconfig, systasks, sysmemory and sysnfs.
Each have a set of "v_" and "k_" routines that support the group.  On a per
group and tables within a group the k_groupID_get and k_groupID_set routines
need to be provided.

The k_ logic provides a set of routines on a per group bases.  The k_groupID_get
routine reads all the MIB variables in the group into a structure generated
by the MIB compiler and returns a pointer to it.  The k_groupID_set routine is
passed the structure generated by the MIB compiler, with a valid vector
field structure, and the request type (ADD_MODIFY or DELETE).  The MIB
variables in the group are updated accordingly.  The set routine only needs
to be provided for variables that can be modified.  The structures generated
by the compiler which are used to transfer data between the v_ and k_ layer
are machine dependent.  Data structures that are returned by the k_groupID_get
routines must be static, furthermore a special considerantion must be observed
when ASCII strings correnspond to MIB variables.  ASCII strings must be 
returned in octect structures.  Octet structures contain pointers to character
arrays, the result is that Octet structures and character arrays must be
declared static as well.  The linkage of this structures is done at 
initialization time.
*/


/* includes */

#include <vxWorks.h>
#include <stdio.h>
#include <string.h>
#include <snmp.h>
#include <snmpuser.h>
#include <diag.h>

#include <remLib.h>
#include <taskLib.h>
#include <symLib.h>
#include <nfsLib.h>
#include <semLib.h>
#include <dllLib.h>
#include <string.h>
#include <a_out.h>
#include <sysSymTbl.h>
#include <symbol.h>
#include <iosLib.h>
#include <private/taskLibP.h>    /* Task Status Definitions */
#include <snmpd.h>
#include <nfsDrv.h>
#include <memLib.h>
#include <taskHookLib.h>

#define STATIC  
/* defines */

#define  MAXTASKS	500   /* Assume Maximum # of tasks in the system */
#define  MAXNFS	        200   /* Assume Maximum # of files systems mounted */
#define  DISPLAY_STR_SZ 255

/* globals */

/* Static variables used for the system configuration group get routine. */

static sysconfig_t   sysVariables;		/* MIB Generated Structure */
static OctetString   sNameOctet;                /* Referenced by sysVariables */
static unsigned char userName  [DISPLAY_STR_SZ]; /* Referenced by sNameOctet */
static OctetString   sPasswOctet;		/* Referenced by sysVariables */
static unsigned char userPassw [DISPLAY_STR_SZ]; /* Referenced by sPasswOctet */


/* Static variables used for the task table group get routine. */

static taskEntry_t    taskEntryVars;	       /* MIB Generated Structure */
static OctetString    tNameOctet;	       /* Referenced by taskEntryVars */
static char           tName  [DISPLAY_STR_SZ]; /* Referenced by tNameOctet */
static OctetString    tMainOctet;	       /* Referenced by taskEntryVars */
static char           tMain  [DISPLAY_STR_SZ]; /* Referenced by tMainOctet */

/* Internal variables used by the task support routines */

STATIC int           taskIdList [MAXTASKS];   /* List of task Ids */
STATIC int           numTasks;		      /* Number of tasks in the list */


/* Static variables used for the system memory group get routine. */

static sysmemory_t    memVars;		/* MIB Generated Structure */


/* Static variables used for the NFS group get routine. */

static sysnfs_t    nfsVars;		/* MIB Generated Structure */


/* Static variables used for the NFS table group get routine. */

static nfsEntry_t     nfsEntryVars;	/* MIB Generated Structure */
static OctetString    nHostNameOctet;	/* Referenced by nfsEntryVars */
					/* Referenced by nHostNameOctet */
static char           HostName        [DISPLAY_STR_SZ];

static OctetString    nHostFileOctet;	/* Referenced by nfsEntryVars */
					/* Referenced by nHostFileOctet */
static char           HostFileSysName [DISPLAY_STR_SZ];

static OctetString    nLocalFileOctet;	/* Referenced by nfsEntryVars */
					/* Referenced by nLocalFileOctet */
static char           LocalFileSysName [DISPLAY_STR_SZ];

/* Internal variables used by the NFS table support routines */

STATIC unsigned long  nfsHandles [MAXNFS];   /* List of NFS handles */
STATIC int            numMounted;            /* Number of handles in the list */

/*******************************************************************************
*
* vxDemoDeleteHook -  Task Delete Hook
*
* This routine is invoked by vxWorks each time a task is deleted in the system.
* At the time the task is deleted a trap is sent to the SNMP Manager.
*
* RETURNS: N/A
* 
* SEE ALSO: N/A
*/
void vxDemoDeleteHook
	(
	WIND_TCB *pTcb
	)
	{
	OID 	    * pObject;	 /* Ptr to Object ID */
	static OID    inst;	 /* Table entry index to return to agent */
	static unsigned long    
		     oidbuff;    /* OID buffer for inst variable */
	static long   taskId;    /* Buffer for the Task ID */

	taskId = (long) pTcb;	 /* Task ID is the address of the TCB */

	inst.length = 1;	 /* Format Table Index information */
	inst.oid_ptr = &oidbuff;
	inst.oid_ptr [0] = taskId;

				 /* Get the Object ID */
    	pObject = MakeOIDFromDot ("taskId");

				 /* Send SNMP Trap */
	do_trap (ENTERPRISE_TRAP, 1, MakeVarBindWithValue (pObject, &inst, 
		 INTEGER_TYPE, &taskId), MakeOIDFromDot("vxTaskDeleted"));
	}

/*******************************************************************************
*
* k_vxdemo_initialize -  Initialize all the module structures.
*
* Link the MIB structures with their corresponding Octet structures and 
* character arrays.  The code used to generate the specified trap in the
* vxdemo.my is enabled here.
* This routine must be invoked by the SNMP agent before the
* MIB variables can be accessed.
*
* RETURNS: OK.  If resources were to be allocated it could return ERROR.
* 
* SEE ALSO: N/A
*/
int k_vxdemo_initialize
    (
    )
    {

    /* Link MIB structures with corresponding Octet structures */

    sysVariables.sysUserName                    = &sNameOctet;
    sysVariables.sysUserName->octet_ptr         = userName;
    sysVariables.sysUserPassw                   = &sPasswOctet;
    sysVariables.sysUserPassw->octet_ptr        = userPassw;

    taskEntryVars.taskName         	        = &tNameOctet;
    taskEntryVars.taskName->octet_ptr           = tName;
    taskEntryVars.taskMain         	        = &tMainOctet;
    taskEntryVars.taskMain->octet_ptr           = tMain;

    nfsEntryVars.nfsHostName 		        = &nHostNameOctet;
    nfsEntryVars.nfsHostName->octet_ptr	        = HostName;
    nfsEntryVars.nfsHostFileSysName  		= &nHostFileOctet;
    nfsEntryVars.nfsHostFileSysName->octet_ptr 	= HostFileSysName;
    nfsEntryVars.nfsLocalFileSysName            = &nLocalFileOctet;
    nfsEntryVars.nfsLocalFileSysName->octet_ptr = LocalFileSysName;

    /* Set up task Delete Hook */
    if (taskDeleteHookAdd ((FUNCPTR) vxDemoDeleteHook) == ERROR)
	printf ("vxDemoDeleteHook fail installation. SNMP Trap not enable\n");

    return (OK);
    }

/*******************************************************************************
*
* k_vxdemo_terminate -  Free all resource allocated by this service.
*
* This routine frees all the resources that have been allocated by the MIB 
* service.  In this routine the task hook intalled at initialization
* is removed.
*
* RETURNS: OK, unless it failes in deallocating a resource.
* 
* SEE ALSO: N/A
*/
int k_vxdemo_terminate
    (
    )
    {
    taskDeleteHookDelete ((FUNCPTR) vxDemoDeleteHook);
    return (OK);
    }

/*******************************************************************************
*
* k_sysconfig_get -  Read the sysconfig group variables.
*
* Access the vxWorks variables and copies their value into the MIB generated
* structure.
*
* RETURNS: Pointer to a sysconfig structure.
* 
* SEE ALSO: remCurIdGet.
*/
sysconfig_t * k_sysconfig_get 
    (
    int           id,
    ContextInfo * contextInfo,
    int           reqVar
    )
    {
    sysVariables.sysState = D_sysState_system_running;  /* System is Running */
    remCurIdGet (userName, userPassw);   /* Get data from vxWorks */

    sysVariables.sysUserName->length     = strlen (userName);
    sysVariables.sysUserPassw->length    = strlen (userPassw);

    SET_ALL_VALID(sysVariables.valid);   /* Set all variable to valid */
    return (&sysVariables);
    }

/*******************************************************************************
*
* snmpReboot -  print reboot message and issue vxWorks reboot command.
*
* This routine is spawned as a task by the SNMP agent.  Its job is to print
* a message to the system console, and issue the vxWork's reboot command.
*
* RETURNS: N/A
*
* SEE ALSO: taskDelay and reboot.
*/
void snmpReboot ()
    {
    printf("\007\007SNMP System Reboot Command in progress\n");
    taskDelay (200);	/* Wait 200 ticks before Reboot */
    reboot (0);		/* Reboot System */
    }

/*******************************************************************************
*
* k_sysconfig_set -  Set a sysconfig group variable to the requested value.
*
* This routine checks that the variable to be set has been marked valid by
* v_ code.   The corresponding action or setting of the variable is taken.
*
* RETURNS: Successful = NO_ERROR. Failed =  GEN_ERROR
* 
* SEE ALSO: taskSpawn, remCurIdGet, and remCurIdSet.
*/
int k_sysconfig_set 
    (
    sysconfig_t * sysVarToSet,
    ContextInfo * contextInfo,
    int           state
    )
    {

    /*  Check the valid vector to determine which variable is to be modified. */

    if (VALID(I_sysState, sysVarToSet->valid)) 
	{
	if (sysVarToSet->sysState == D_sysState_system_reboot)
	    {
	    if (taskSpawn ((char *)NULL, 150, VX_NO_STACK_FILL, 1024, 
			   (FUNCPTR) snmpReboot, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
			   == ERROR)
		return (GEN_ERROR);
	    }
	return (NO_ERROR);
	}
    else 
	if (VALID(I_sysUserName, sysVarToSet->valid)) 
	    {
	    remCurIdGet (userName, userPassw);      /* Get data from vxWorks */

	    (void) strncpy (userName, 
			    (char *) sysVarToSet->sysUserName->octet_ptr,
			    (int) sysVarToSet->sysUserName->length);
	    userName [sysVarToSet->sysUserName->length] = '\0';

	    remCurIdSet (userName, userPassw);      /* Set data from vxWorks */

	    return (NO_ERROR);
	    }
	else 
	    if (VALID(I_sysUserPassw, sysVarToSet->valid)) 
		{
		remCurIdGet (userName, userPassw);  /* Get data from vxWorks */

		(void) strncpy (userPassw, 
				(char *)sysVarToSet->sysUserPassw->octet_ptr,
				(int) sysVarToSet->sysUserPassw->length);
		userPassw [sysVarToSet->sysUserPassw->length] = '\0';

		remCurIdSet (userName, userPassw);  /* Set data from vxWorks */

		return (NO_ERROR);
		}

    return (GEN_ERROR);
    }


/*******************************************************************************
*
* intComp -  Compare two integer values.
*
* This routine compares integer value 1 agains integer value 2.
*
* RETURNS: 0 if v1 == v2, < 0 if v1 < v2 and > 0 if v1 > v2
* 
* SEE ALSO: N/A
*/

int intComp 
    (
    const void * pValue1,
    const void * pValue2
    )
    {
    return (*((int *)pValue1) - *((int *)pValue2));
    }

/*******************************************************************************
*
* k_taskEntry_get -  Returns the task information for the requested task.
*
* This routine read the task table from the system and returns the information
* corresponding to the requested task.   The request for the task can still
* fail if the task is no longer executing in the system.
*
* RETURNS: Pointer to a taskEtnry structure.
* 
* SEE ALSO: taskIdListGet, taskInfoGet, taskIdVerify and symFindByValueAndType.
*/
taskEntry_t * k_taskEntry_get 
    (
    int            reqId,         /* SNMP PDU ID used for catching */
    ContextInfo  * contextInfo,
    int	           reqVar,
    int            searchType,    /* Request for EXACT or NEXT */
    long           taskId
    )
    {
    int             ix;                 /* General Index */
    SYM_TYPE        stype;		/* Symbol Type */
    TASK_DESC       td;                 /* Task Descriptor */

    /* 
     * The catching startegy is described in several steps here. First if the
     * search type is EXACT the catch can be checked righ away since the task
     * ID has been specified.  In the case of a NEXT the ID is not know in
     * advace and some work need to be done before the catch can be checked.
     */

    if (searchType == NEXT)			/* get the next request */
	{
	/* get tasks in the system and sort them by taskId */

	numTasks = taskIdListGet (taskIdList, MAXTASKS);
	qsort ((void *) taskIdList, numTasks, sizeof (long), intComp);

	/* find the next task greater than the one passed in */

	for (ix = 0 ; ix < numTasks ; ix++)
	    {
	     if ((taskIdList [ix] > taskId) && 
		 (taskInfoGet (taskIdList [ix], &td) == OK))
		break;
	    }

	if (ix >= numTasks)
	    return ((taskEntry_t *)NULL);   	/* no more tasks */

	taskId = taskIdList [ix];
	}
     else
	{

	if (taskInfoGet (taskId, &td) == ERROR)
	    return ((taskEntry_t *)NULL);       /* Task ID no longer valid */
	}

    /* Fill the Task Entry information */

    taskEntryVars.taskId           	= taskId;
    strcpy (tName, td.td_name);
    taskEntryVars.taskName->length     = strlen (tName);

    symFindByValueAndType (sysSymTbl, (int)td.td_entry, tMain, 
			   (int *)&td.td_entry, &stype, 
			   N_EXT | N_TEXT, N_EXT | N_TEXT);
    taskEntryVars.taskMain->length     = strlen (tMain);

    taskEntryVars.taskPriority     	= td.td_priority;

    switch (td.td_status)
	{
	case WIND_READY:
	    taskEntryVars.taskStatus   = D_taskStatus_task_ready;
	    break;
	    
	case WIND_SUSPEND:
	    taskEntryVars.taskStatus   = D_taskStatus_task_suspended;
	    break;

	case WIND_PEND:
	case WIND_DELAY:
	    taskEntryVars.taskStatus   = D_taskStatus_task_delay;
	    break;

	case WIND_DEAD:
	    taskEntryVars.taskStatus   = D_taskStatus_task_deleted;
	}

    taskEntryVars.taskOptions      	 = td.td_options;
    taskEntryVars.taskStackPtr     	 = (unsigned long) td.td_sp;
    taskEntryVars.taskStackBase    	 = (unsigned long) td.td_pStackBase;
    taskEntryVars.taskStackPos     	 = (unsigned long) td.td_sp;
    taskEntryVars.taskStackEnd     	 = (unsigned long) td.td_pStackEnd;
    taskEntryVars.taskStackSize    	 = td.td_stackSize;
    taskEntryVars.taskStackSizeUsage     = td.td_stackCurrent;
    taskEntryVars.taskStackMaxUsed 	 = td.td_stackHigh;
    taskEntryVars.taskStackFree    	 = td.td_stackMargin;
    taskEntryVars.taskErrorStatus  	 = td.td_errorStatus;

    SET_ALL_VALID(taskEntryVars.valid);

    return (&taskEntryVars);            /* Ptr to task entry information */
    }

/*******************************************************************************
*
* k_taskEntry_set -  Create or deletes a task, options & state can be changed.
*
* This routine allows for manipulation of tasks.  Tasks can be created or
* deleted.  Tasks state and options can also be changed.
*
* RETURNS: NO_ERROR if successful otherwise GEN_ERROR
* 
* SEE ALSO: symFindByName, taskSpawn, taskDelete, taskOptionsSet, taskResume,
*	    taskIsSuspended, taskSuspend, and taskPrioritySet.
*/
int k_taskEntry_set 
    (
    taskEntry_t * ptaskEntry,       /* Ptr to table Entry */
    ContextInfo * contextInfo,
    int           reqState          /*  Request to lower layer (INDEX) */
    )
    {
    char 	* pentryRoutine; 
    SYM_TYPE      stype;
    int           tOptions;
    char   	  tString [DISPLAY_STR_SZ];   /* Temporary String Variable */


    /* 
     *  Entries that have a taskId of Zero are used to spawn new tasks.  Non
     *  zero ids are used to change a task status, options, or to delete it.
     */

    if (ptaskEntry->taskId == 0)
	{
	if (reqState != ADD_MODIFY)
	    return (GEN_ERROR);

	/* Verify that the entry point specified exists in the system. */

	(void) strcpy (tString, "_");
        (void) strncat (tString, (char *)ptaskEntry->taskMain->octet_ptr,
                (int) ptaskEntry->taskMain->length);
        tString [ptaskEntry->taskMain->length + 1] = '\0';

	if (symFindByName (sysSymTbl, tString, &pentryRoutine, &stype) == ERROR)
	    return (GEN_ERROR);
	  
        (void) strncpy (tString, (char *)ptaskEntry->taskName->octet_ptr,
                (int) ptaskEntry->taskName->length);
        tString [ptaskEntry->taskName->length] = '\0';

	if (taskSpawn (tString, ptaskEntry->taskPriority,
		       ptaskEntry->taskOptions, ptaskEntry->taskStackSize,
		       (FUNCPTR) pentryRoutine, 
		       0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
	    return (GEN_ERROR);
	}				/* TaskId == 0 */
    else
	{
	if (reqState == ADD_MODIFY)
	    {
	    /* Check if the PRIORITY of the task is to be changed */

	    if (VALID(I_taskPriority, ptaskEntry->valid)) 
		{
		if (taskPrioritySet (ptaskEntry->taskId, 
				     ptaskEntry->taskPriority) == ERROR)
		    return (GEN_ERROR);
		}

	    /* Check if the STATUS of the task is to be changed */

	    if (VALID(I_taskStatus, ptaskEntry->valid)) 
		{
		/* Check if the task is to be suspended */

		if (ptaskEntry->taskStatus == D_taskStatus_task_suspended)
		   {
		   if (taskSuspend (ptaskEntry->taskId) == ERROR)
		       return (GEN_ERROR); /* Task Doesn't exits */
		   }
		else
		   {
		   /* Check if the task is to be made ready */

		    if (ptaskEntry->taskStatus == D_taskStatus_task_ready)
			{
			if (taskResume (ptaskEntry->taskId) == ERROR)
			    return (GEN_ERROR); /* Task Doesn't exits */
			}
		    else
			return (GEN_ERROR);   /* Invalid State Change Request */
		   }
		}

	    /* 
	     * Check if the OPTIONS of the task are to be changed.  The only
	     * option that can be changed is VX_UNBREAKABLE ON/OFF.
	     */

	    if (VALID(I_taskOptions, ptaskEntry->valid)) 
		{
		if (taskOptionsGet (ptaskEntry->taskId, &tOptions) == ERROR)
		    return (GEN_ERROR); /* Task Doesn't exits */

		if ((tOptions & VX_UNBREAKABLE) == 0)
		    {
		    if (taskOptionsSet (ptaskEntry->taskId, (~tOptions), 
					(tOptions | VX_UNBREAKABLE)) == ERROR)
			return (GEN_ERROR); /* Task Doesn't exits */
		    }
		else
		    {
		    tOptions &= ~VX_UNBREAKABLE;
		    if (taskOptionsSet (ptaskEntry->taskId, VX_UNBREAKABLE,
					tOptions) == ERROR)
			return (GEN_ERROR); /* Task Doesn't exits */
		    }
		}
	    }
	else
	    {  /* DELETE a Task Request */

	    if (taskDelete (ptaskEntry->taskId) == ERROR)
		return (GEN_ERROR);
	    }
	}

    return (NO_ERROR);
    }


/*******************************************************************************
*
* k_sysmemory_get -  Reads the statistics from the System Memory Partition.
*
* The vxWorks system memory partion is access and the statistics are returned.
*
* RETURNS: Pointer to a sysmemory structure.
*
* SEE ALSO: memPartInfoGet
*/
sysmemory_t * k_sysmemory_get 
    (
    int           reqId,         /* SNMP PDU ID used for catching */
    ContextInfo * contextInfo,
    int           reqVar         /* Request for EXACT or NEXT */
    )
    {
    MEM_PART_STATS  memStats;

    memPartInfoGet (memSysPartId, &memStats);

    memVars.numBytesFree       = memStats.numBytesFree;
    memVars.numBlocksFree      = memStats.numBlocksFree;

    if (memVars.numBlocksFree > 0)
	memVars.avgBlockSizeFree = memVars.numBytesFree / memVars.numBlocksFree;

    memVars.maxBlockSizeFree   = memStats.maxBlockSizeFree;

    memVars.numBytesAlloc      = memStats.numBytesAlloc;
    memVars.numBlocksAlloc     = memStats.numBlocksAlloc;
    memVars.avgBlockSizeAlloc  = memVars.numBytesAlloc / memVars.numBlocksAlloc;

    SET_ALL_VALID(memVars.valid);
    return (&memVars);
    }



/*******************************************************************************
*
* k_sysnfs_get -  The NFS group and user IDs are read.
*
* The vxWorks efective user and group IDs are accesse and returned.
*
* RETURNS: Pointer to the NFS structure.
*
* SEE ALSO: nfsAuthUnixGet
*/
sysnfs_t * k_sysnfs_get 
    (
    int            reqId,         /* SNMP PDU ID used for catching */
    ContextInfo  * contextInfo,
    int            reqVar         /* Request for EXACT or NEXT */
    )
    {
    char machname [AUTH_UNIX_FIELD_LEN];/* host name where client is */
    int uid;                            /* client's UNIX effective uid */
    int gid;                            /* client's current group ID */
    int len;                            /* element length of aup_gids */
    int aup_gids [MAX_GRPS];

    nfsAuthUnixGet (machname, &uid, &gid, &len, aup_gids);
	
    nfsVars.nfsUserId  = uid;
    nfsVars.nfsGroupId = gid;

    SET_ALL_VALID(nfsVars.valid);
    return (&nfsVars);
    }

/*******************************************************************************
*
* k_sysnfs_set -  Change the NFS user ID or the NFS group ID.
*
* The NFS user ID or NFS group ID is changed. 
*
* RETURNS: NO_ERROR, This is because nfsAuthUnixSet returns a void.
*
* SEE ALSO: nfsAuthUnixSet.
*/
int k_sysnfs_set 
    (
    sysnfs_t    * pnfsVars,         /* Ptr to nfs Variables */
    ContextInfo * contextInfo,
    int           reqState          /*  Request */
    )
    {
    char machname [AUTH_UNIX_FIELD_LEN];/* host name where client is */
    int uid;                            /* client's UNIX effective uid */
    int gid;                            /* client's current group ID */
    int len;                            /* element length of aup_gids */
    int aup_gids [MAX_GRPS];

    nfsAuthUnixGet (machname, &uid, &gid, &len, aup_gids);

    if (VALID(I_nfsUserId, pnfsVars->valid))
	uid = pnfsVars->nfsUserId;

    if (VALID(I_nfsGroupId, pnfsVars->valid))
	gid = pnfsVars->nfsGroupId;

    nfsAuthUnixSet (machname, uid, gid, len, aup_gids);

    return (NO_ERROR);
    }


/*******************************************************************************
*
* k_nfsEntry_get -  Read the information of an NFS Device in the system.
*
* This routine allows for the NFS local file system name, remote file system
* name etc, to be retrived from vxWorks.
*
* RETURNS: A pointer to an nfsEntry structure or NULL if not found.
* 
* SEE ALSO: nfsDevListGet, hostGetByName, 
*/
nfsEntry_t * k_nfsEntry_get 
    (
    int            reqId,         /* SNMP PDU ID used for catching */
    ContextInfo  * contextInfo,
    int            reqVar,
    int            searchType,    /* Request for EXACT or NEXT */
    long  	   entryIndex
    )
    {
    int          ix;		 /* Generic Index */
    NFS_DEV_INFO nfsDev;

    /*  Collect information of all the NFS devices currently mounted.  */

    numMounted = nfsDevListGet (nfsHandles, MAXNFS);

    if (numMounted <=  0)
	return ((nfsEntry_t *)NULL);	/* No NFS devices Mounted */

    /* Sort NFS Table Entries */

    qsort ((void *) nfsHandles, numMounted, sizeof (long), intComp);

    if (searchType == NEXT)
	{
	/* Find the NFS Index greater than the one passed */
	
	for (ix = 0 ; ix < numMounted ; ix++)
	    {
		if ((nfsHandles [ix] > entryIndex) &&
    		    (nfsDevInfoGet (nfsHandles [ix], &nfsDev) == OK))
		    break;
	    }

	/* Check for END of table condition */

	if (ix >= numMounted)
	    return ((nfsEntry_t *)NULL);

	entryIndex = nfsHandles [ix];	/* Entry Index Found */

	}
    else
	{
	/* Get Device information for the specified Index */

	if (nfsDevInfoGet (entryIndex, &nfsDev) == ERROR)
	    return ((nfsEntry_t *)NULL);
	}

    /* Fill the device information structure */

    nfsEntryVars.nfsIndex 	= entryIndex; /* Fix Index for 1 - N */
    						 /* File System Mounted */
    nfsEntryVars.nfsState 	= D_nfsState_nfs_mounted;
    strcpy (HostName, nfsDev.hostName);
    nfsEntryVars.nfsHostName->length  	= strlen (HostName);

    /* Get Host IP Address */

    nfsEntryVars.nfsHostIpAddr = hostGetByName (HostName); 

    strcpy (HostFileSysName, nfsDev.hostName);
    strcat (HostFileSysName, ":");
    strcat (HostFileSysName, nfsDev.remFileSys);
    nfsEntryVars.nfsHostFileSysName->length      = strlen (HostFileSysName);

    strcpy (LocalFileSysName, nfsDev.locFileSys);
    nfsEntryVars.nfsLocalFileSysName->length     = strlen (LocalFileSysName);

    SET_ALL_VALID(nfsEntryVars.valid);

    return (&nfsEntryVars);
    }

/*******************************************************************************
*
* k_nfsEntry_set -  Alter, create or delete an NFS device in the system.
*
* This routine allows for a NFS device in the system to be changed or deleted.
* NFS devices can also be created by this routine.  When devices are created
* if the host does not exists in the host table it will be added.   For hosts
* which require a route, the route must be added through the MIB-II.
*
* RETURNS: NO_ERROR if successfull otherwise GEN_ERROR.
* 
* SEE ALSO: nfsDevInfoGet, nfsUnmount, nfsMount, inet_ntoa_b, and hostAdd,
*/
int k_nfsEntry_set 
    (
    nfsEntry_t  * pnfsEntry,        /* Ptr to table Entry */
    ContextInfo * contextInfo,
    int           reqState          /*  Request to lower layer (INDEX) */
    )
    {
    char             thName      [80];
    char             tlocalFile  [80];
    char             thostFile   [80];
    char             tIpString   [40];
    struct in_addr   hostIpAd;
    NFS_DEV_INFO     nfsDev;

    if (reqState == DELETE)
	{			    /* Unmount the NFS Device */

	if (nfsDevInfoGet (pnfsEntry->nfsIndex, &nfsDev) == ERROR)
	    return (GEN_ERROR);

	if (nfsUnmount (nfsDev.locFileSys) == ERROR)
	    return (GEN_ERROR);

	return (NO_ERROR);
	}

    /*
     *  The ADD_MODIFY state is used to add NFS mount entries.
     */

    if (reqState == ADD_MODIFY)
	{			    /* ADD_MODIFY Request */

	if (pnfsEntry->nfsIndex != 0) /* Can't mod an existing entry */
	    return (GEN_ERROR);

	/* If the host does not exist in the host table add it */

	strncpy (thName, pnfsEntry->nfsHostName->octet_ptr,
		 pnfsEntry->nfsHostName->length);
	thName [pnfsEntry->nfsHostName->length] = '\0';

    	hostIpAd.s_addr = hostGetByName (thName);
	if (hostIpAd.s_addr == ERROR)
	    {
	    inet_ntoa_b (pnfsEntry->nfsHostIpAddr, tIpString);
	    if (hostAdd (thName, tIpString) == ERROR)
		return (GEN_ERROR);
	    }

	strncpy (thostFile, pnfsEntry->nfsHostFileSysName->octet_ptr,
		 pnfsEntry->nfsHostFileSysName->length);
	thostFile [pnfsEntry->nfsHostFileSysName->length] = '\0';

	strncpy (tlocalFile, pnfsEntry->nfsLocalFileSysName->octet_ptr,
		 pnfsEntry->nfsLocalFileSysName->length);
	tlocalFile [pnfsEntry->nfsLocalFileSysName->length] = '\0';

	if (nfsMount (thName, thostFile, tlocalFile) == OK)
	    return (NO_ERROR);
	}

    return (GEN_ERROR);
    }

