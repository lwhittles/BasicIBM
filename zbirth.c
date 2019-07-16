/*BIRTH GENERATOR

ENTRY: 'ypb' is the number of years per birth. That is, the reciprocal of the
         annual birth rate.
       'w' specifies the width of a random interval during which the next birth
         is allowed to happen. The next birth will sometime between 't+ypb-w'
         and 't+ypb'. 'w' is non-negative and less or equal to 'ypb'.

EXIT:  A new individual has been born and added to the UK group.
       The next birth has been scheduled.

WORK:  'init' is 0 if this routine has never been called, 1 otherwise.
       'target' is the latest time for the next birth.
*/
BirthG(dec w)
{ static dec target, init;

  if(init==0) { target = t; init = 1; }   //Capture the time on the first call.

  Birth(CCadd(UK),t);                     //Create someone born in the UK.

  if(w<0||w>ypb) Error(851.);             //Generate a random variation in the
  if(w>0) w *= Rand();                    //time of next birth.

  if(target<t) target = t;                //Schedule an event for a bounded
  A[PBIRTH].pending = pBirth;             //distance in the future.
  EventSchedule(BIRTH,target+ypb-w);

  target += ypb;                          //Advance the latest time for the
}                                         //next event.
/*
Special considerations: The random number function 'Rand' should not generate
values of precisely 1. Random number generators typically generated numbers
0<=R<1, but this should be checked as part of the exit conditions of any new
random generator installed. There will not be a catastrophic failure if it
returns a 1, but could result in a birth being scheduled at the current time.
The scheduler should still work in that condition, but it seems slightly
slightly undesirable.

Also note that the code "if(target<t) target=t" is necessary to deal with
possible accumulated rounding error. Without it, the routine could fail by
trying to schedule a time in the past. This is an interesting example of a bug
that can hardly be removed by debugging, but must be removed by a priori proof-
of-correctness.
*/
