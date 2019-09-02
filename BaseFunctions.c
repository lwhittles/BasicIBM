 /*----------------------------------------------------------------------------*
CHECK ALL EVENTS

This routine find the ealiest event for individual n and schedules it.

*/
#define SCHED(X,Y,Z)  { A[n].pending = X; EventSchedule(n,Y); return Z; }
Check_all_events(int n)
{ int i;
  i=Earliest(A[n].t)   ;
  SCHED(i, A[n].t[i],   i);      //Schedule the earliest event.
  return 1;                      //(Will never reach this.)
}

/*----------------------------------------------------------------------------*
INITIALIZE BINDS IN A

This routing initializes binding lists in A
*/
InitAbinds(int n){
  A[n].bto[0]=0;
  A[n].bto[1]=0;
  A[n].bfrom[0]=0;
  A[n].bfrom[1]=0;
	return 1;
}
/*----------------------------------------------------------------------------*
SEARCH

This routine finds the current location on array A of an individual with an ID
number n.

ENTRY: 'id' id number for the indidivual
EXIT:  'i' index of individual on array A

*/
Find_individual(id)
{int i;
  for (i=0;i<indiv+1;i++)
    { if (A[i].id==id){A[i].InFunction=fnc_Find_individual;
 return i;}
    }
}
/*----------------------------------------------------------------------------*
TRANSFER GROUP

This routine transfer and individual and all their information (including saved event
times) to a group. The routine then cancels the pending
event for that index number and re-schedules it using the new index number.

ENTRY: 'grp' is the new group to be assigned
       'n' is the current index number of the individual.

EXIT:  'm' is the new index number of the individual.
       'grp' in the new group fro the individual.
       individual n id deleted
       The old and new group counters are updated
       The funtion returns the new index of the individual
*/

TransferGroup(int n, int grp)
{//printf("Starting TransferGroup()...\n"); fflush(stdout);
     A[n].InFunction=fnc_TransferGroup;
     int m, yr,st,gid,q;
     gid = A[n].groupID;
     st =  A[n].strain;
     yr=(int)t-(int)t0;
     q = A[n].state;
     N[gid] -= 1;
     m=CCadd(grp);         //create a new individual in the settle immigrant group
     InitAbinds(m);
  	Check_all_events(n);  //update events
  	Transfer(m,n);        //Transfer individual and all information
                           //from the recent migrant to settle migrant group
  	A[m].groupID = grp;   //Update the group ID
     CCdel(A[n].groupID,n);//Remove duplicate individual from old group
     N[A[m].groupID] += 1;
    return m;
  	}

/*----------------------------------------------------------------------------*
TRANSFER

This routine transfer all information about an individual (including saved event
times) to a new identification number. The routine then cancels the pending
event for that index number and re-schedules it using the new index number.

ENTRY: 'n' is the new index number to be assigned, which has no event scheduled
       'n0' is the current index number of the individual.
       There is an event scheduled for 'n0'.

EXIT:  'n' is the new index number of the individual.
       The event scheduled for n0 is now re-scheduled under 'n' and all other
         data from 'n0' are transferred to 'n'. 'n0' no longer has an event
         scheduled and the index number is free to be used again.
*/

Transfer(int n, int n0)
{//printf("Starting Transfer()...\n"); fflush(stdout);
A[n].InFunction=fnc_Transfer;
  if(n!=n0)
    {DetachH(n0);
	A[n] = A[n0];
 	AttachH(n);
	EventRenumber(n, n0);     //Copy data and reschedule as 'n'.
    }
}
/*----------------------------------------------------------------------------*
INITIALIZE Groups
This function is called at the start of a simulation to initialize groups
ENTRY: 'mA' maximum population size currently defined in group.h
 	'nC' contains the number of groups.
       'Clowest' indexes the first individual in each group.
       'Emptyc[k]' contains the number of empty cells at the end of the group.
       'V[k]' contains the maximum probability for individuals in the group.
       'A[i].v' contains the probability of selection for each individual in
         the group. This is greater than zero but not greater than 'V[k]'.


EXIT:  nC groups are initialized
*/
GroupInit()
{int i,k,nx,r,mA2;
  mA2=mA;          //maximum population size currently defined in group.h
  nx=mA2/nC; //the number of individuals per group if they are eqaully distributed.
  r=mA2-nC*nx;  //the remainder
  k=1;
  for(i=0.;i<nC;i++) //for each group
    {Emptyc[i]=nx;  //initialize its size
      if(i>=nC-r) Emptyc[i] +=1; //distribute the remainder across higher numbered groups
      Clowest[i]=k; k+=Emptyc[i]; //initialize the group location
      V[i]=1;
    }
  Emptyc[nC]=0.; //close the lost of groups
  Clowest[nC]=mA2+1;
}
