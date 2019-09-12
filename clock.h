/*-------------------------------------------------------------------------*
CLOCK FOR PERIPHERAL EVENTS

Any number of "clocks" may be running to schedule peripheral events that are
not generated within the program. For example, immigration could be a
peripheral event that happens at fixed or random periodic intervals. This
routine handles the timing of the next tick for any clock.

ENTRY: Structure 'c' defines the clock, as described below.
       'c.type' is the type of clock.
         0 General based on an arbitrary distribution of waiting times.
         1 Simple exponential, fixed mean time (special case of 0 for
             convenience).
         2 Simple exponential, fixed number of events (not implemented yet).
         3 Periodic with arbitrary distribution of noise.
         4 Periodic with optional uniform noise (special case of 3 for
             convenience).
       'c.rate' is, for types 1 to 4, the average number of events per unit
         time, which must be greater than zero. The average time between events
         is thus '1/rate'.
       'c.rel' is, for types 3 to 4, the relative width, from 0 to 1, of a
         random interval during which the next event will occur. It will be
         sometime between 't+1/rate-1/rate*rel' and 't+1/rate'.
       't' is the current time.

EXIT:  'ClockTick' returns an updated structure. In particular, 'c.next' is
         the time to be scheduled for the next tick.

Note: Tendai for 1 gives back on avarage c.rate events per unit time. i.e observed c.rate varies
*/

struct Clock
{
  int type;   //Type of clock (see above)
  dec next;   //Scheduled time of the next tick
  dec rate;   //Events per unit time, types 0-3 (see above)
  dec rel;    //Relative width of uniform noise, types 2-3 (see above)
  dec target; //Latest time for the next tick, types 2-3 (see above)
  dec *x, *y; //Cumulative distribution, types 3-4 (see 'RandF')
  int nxy;
}; //Number of elements in 'x' and 'y', types 3-4 (see 'RandF')

struct Clock ClockTick(struct Clock c)
{
  dec v, w;

  switch (c.type)
  {
  case 0: //GENERAL CUMULATIVE DISTRIBUTION.
    c.next = t + RandF(c.x, c.y, c.nxy, 0.);
    break;

  case 1:
    c.next = t + Expon(c.rate); //FIXED TIME EXPONENTIAL.
    break;
    //  case 3:                               //PERIODIC WITH GENERAL NOISE.
    //    w = c.rel;
    //    if(w<0||w>1) Error(851.);           //Generate a random variation in the
    //    if(w>0) w *=                        //timing, if requested.
    //      RandF(c.x,c.y,c.nxy,0.)/x[nxy-1];
    //    if(c.target<t) c.target  = t;
    //    v = 1./c.rate; c.target += v;       //Advance the latest time for the
    //    c.next = c.target-v*w;              //next event.
    //    break;

  case 4: //PERIODIC WITH UNIFORM NOISE.
    w = c.rel;
    if (w < 0 || w > 1)
      Error(851.); //Generate a random variation in the
    if (w > 0)
      w *= Rand(); //timing, if requested.
    if (c.target < t)
      c.target = t;
    v = 1. / c.rate;
    c.target += v;             //Advance the latest time for the
    c.next = c.target - v * w; //next event.
    break;

  default:
    Error(852.);
  }

  return c;
}
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
