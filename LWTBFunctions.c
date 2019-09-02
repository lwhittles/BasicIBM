/*----------------------------------------------------------------------------*
PROGRESSION

This routine is dispatched when an individual progresses from latent to active disease.

ENTRY: 'n' indexes an individual who has just progressed.
       't' contains the current time.
       progression is scheduled for individual 'n'.

EXIT:
       'progressions' is incremented.
       Counters in 'A','L' are updated.
       Counters 'age1', 'age2', and 'agec' are updated.???
       death from TB or dormancy is scheduled for individual n

*/


Progress(int n)
{ int n2,yr,gid,tr,st,q,qv; dec age;
     A[n].InFunction=fnc_Progress;
	     st=A[n].strain;

        gid = A[n].groupID;                  // individual's current group ID
        A[n].state = qATB;                  // update current infection state


	      progressions += 1;                   //Increment the number of progressions.
        LTB[gid] -= 1;                           //Decrement the number of Latent
        ATB[gid] += 1;                          //Increment the number of Actives

  Check_all_events(n);                       //Update schedule for all events
  return 1;
}
