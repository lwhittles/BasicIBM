

/*----------------------------------------------------------------------------*
MAIN PROGRAM

Due to an MPI bug not allowing 'popen' etc to work, the TB program is
defined as a function which returns an array of output (rather than stand-alone
executable) for use with the fitting routine. 'define' statement is used to
control whether TB program is a stand-alone executable or function within
the fitting routine.

*/


int mainfx ( int argc, char *argv[] )

/******************************************************************************/
{ int i, j, k, l, n, sid;

  startsec = time(NULL);                     //Retrieve the wall-clock time. time() in time.h?

  if(fit5i==0) ErrorInit();                  //Trap system failures. In Error.c
  MainInit();                                //Start the main program. In Declarations.c
  EventInit();                               //Start the event queue.
  //FinalInit();                               //Start the final reports.
  ReportInit();                              //Start the output reports.

  A = (struct Indiv *)                       //Allocate array of individuals. struct Indiv defined in common.h
    calloc(indiv+NPSEUDO, sizeof(struct Indiv)+nEngines); //(Not static because of gcc bug
  if(A==0) Error(911.);                      //restricting such arrays to 2GB.)

  if(SUPER) maximm = 10000000;              //Adjust 'maximm' depending on
  else      maximm =  indiv-1;              //whether running on supercomp.
                                            //Adjusted by Tendai. NB maximm<<indiv in common.h .Changed to indiv-1 on 05/03/15

  Data();                                    //Read in appropriate data files
                                             //and store to arrays.
  my_id_0=(dec) my_id;
  gparam(argc, argv);                        //Collect parameters for this run
                                             //which have been specified on
                                             //command line.

  Param();                                   //Update variables/distributions
                                             //affected by parameters which
                                             //can change with each model run.
  //Births
  if(bcy[0]<=0.0001)                         //Calculate years per birth and
  { ypb = RT*100;                            //per immigrant at t=t0 for
    }//printf("Births are zero!\n"); }          //scheduling them regularly. If
  else ypb = 1./RT*100;//bcy[0];                      //births (immigrants) per year
                                             //Update time of last update for
  lup = t0;                                  //parameters sensitive to
                                             //calendar year.

  //randseq=(int) my_id_0+(int)currentrun*(int)numprocs;
  printf ( "\n" );
  printf ( "  The randseed is %d.\n", (int)randseq );
  rand0 = abs((int)randseq);                      //Start the random number sequence
  if(randseq>=0)  RandStart(rand0);          //from a specified or an arbitrary
  else rand0 = RandStartArb(rand0);          //place.

  EventStartTime(t0);                        //Initialize the event queues.

  t = t0;                                    //Set the starting time

  fpds=fopen(ftimesname, "w");

  InitPop();                                 //Set up initial population.
  BindInit();                                //Initilize Binding.

  snprintf(fname, sizeof(fname), "%s_%d%d.txt", fnamestem, (int)fnumber,(int)randseq);
  printf ( "\n" );
  printf ( "  The name is %d.\n", (int)fnumber );
  fptr=fopen(fname, "w");
  Report(argv[0]); pt = t;                   //Report initial conditions.

  BirthG();                                  //Start external event generators
  for(t=t0; t<t1; Dispatch())                //Main loop: process events,
    { if(t-pt<tgap) continue;                //reporting results periodically.
    pt = t; Report(argv[0]); }

  Report(argv[0]);                           //Get final report.
  fclose(fptr);
  fclose(fpds);

  free(A);                                   //caller.

  return 0;
}

/*----------------------------------------------------------------------------*
DISPATCH NEXT EVENT

All events pass through this routine. It picks the earliest event in the
queue, sets the time to match that event, and performs the operations called
for by that event. Typically that will result in other events being scheduled,
to be seen in the future as they arrive at the front of the queue.

ENTRY: The system is initialized with all events in the list ready for
         processing.
       't' contains the present time.
       't1' contains the ending time.

EXIT:  The next event has been processed and 'events' incremented, if the
         event's time is less than 't1'.
       't' is advanced to the next event, which may be an unprocessed event at
         time greater than 't1'.
*/

Dispatch()
{ int n; dec tw;

  tw = t;                                    //Remember the previous time.
  n = EventNext(); if(t>t1) return 0;          //Advance time to the next event. //Tendai changed return to return 0; to avoid warnings
  tstep(tw, t);                              //Record the size of the time step.
  events += 1;                               //Increment the events counter.
  switch(A[n].pending){                      //Process the event.
  case pDeath:    Death(n);        break;    //[death]
  case pBirth:    BirthG();        break;    //[birth generator]
  default: Error2(921.2,                      //[system error]
		  "`A[",n,"].pending=",A[n].pending);
  }}

/*----------------------------------------------------------------------------*
BIRTH GENERATOR

This routine initiates a birth and schedules the next birth, at regularly spaced
intervals, acting as the peripheral event generator for births.

ENTRY: 't' contains the current time.
       'ypb' is the number of years per birth. That is, the reciprocal of the
         annual birth rate.
       'rel' specifies the relative width of a random interval during which the next birth
         is allowed to happen. The next birth will sometime between 't+ypb-rel*ypb'
         and 't+ypb'. 'rel' is between zero and one.

EXIT:  A new individual has been born and added to the UK group.
       The next birth has been scheduled.
*/
struct Clock Cbirth ;                    //Create a new clock structure
BirthG()
{ int yr,n;
  yr=(int)(t-t0);
  n=CCadd(UK);
  Birth(n,t);                     //Create someone born in the UK.
  Cbirth.type=4;                          //Assign clock type
  Cbirth.rate=1/ypb;                      //Assign birth rate
  //Cbirth.rate=1/ypb_fixed;           //if rate is fixed
  //Cbirth.rate=bcy[yr];  		//if rate in based on data sotred in array bcy

  Cbirth.rel=1;                         //Assign relative width of a random
                                          //interval during which the next
                                          //birth is allowed to happen.
  Cbirth = ClockTick(Cbirth);             //call clock ticker
  A[BIRTH].pending = pBirth;              //Schedule the next birth.
  EventSchedule(BIRTH,Cbirth.next);

}

/*----------------------------------------------------------------------------*
BIRTH

This routine is dispatched when an individual is to be born. All newborns are
Uninfected; exit from the Uninfected compartment is by death or emigration from the study population.

ENTRY: 'n' indexes an individual being born.
       'b' contains the time of birth. Presently, this is the current time,
         though with some set up, it could be earlier than present (notably,
         'pmale' would have to be indexed differently).
       't' contains the current time.
       'A[n].state' contains the present state of the record (can be any state,
         including 0, which is not a state but marks records not yet assigned).
       'm1' contains the mortality rate for susceptible individuals (if app.).
       'em' contains the emigration rate.
       No event is scheduled for individual 'n'.

EXIT:  'Birth' contains a status code.
         0 The individual would die before the current time so no birth has been
            recorded and no event scheduled.
         1 Entry 'n' is initialized as a susceptible newborn and its first event
            is scheduled, either emigration or death.
       'A[n].state' marks a susceptible individual.
       Counters in 'N' are updated.
*/

int Birth(int n, dec b)
{ int y, s, v, e,rob; dec wd, wh,we,age;
  A[n].InFunction=fnc_Birth;

  y = (int)t - (int)t0;                        //Retrieve year index for arrays.
  if(n<Clowest[UK]) Error1(610.1, "n=",(dec)n);//Check for appropriate 'n', this
  if(n>indiv)    Error1(610.2, "n=",(dec)n); //routine does not allow immigrant
                                             //births or births to those with
                                             //index number >'indiv'.
  nbirths +=1;
  //A[n].sex = Rand()<pmale[y]? 0: 1;          //Assign the newborn's sex from data array.
  A[n].sex = Rand()<0.5? 0: 1;          //Assign the newborn's sex from random number generation.
  s = A[n].sex;
  age=0.;                                 //sent age
  rob=UK;				  //assign country of birth
  BasicInd(y,n,UK,age,s,UK, qUTB);                  //Set up basic individual.
  EventCancel(n);
  Check_all_events(n);                       //Update schedule for all events

}


/*----------------------------------------------------------------------------*
DEATH

This routine is dispatched when an individual dies, leaving the population.

ENTRY: 'n' indexes an individual who has just died.
       't' contains the current time.
       'A[n].state' contains the present state (can be any compartment).
       'A[n].tBirth' contains the time of birth.
       'popsize' current population size
       No event is scheduled for individual 'n'.

EXIT:  Either entry 'n' is sent to the Birth() function, to be initialized as a
        susceptible newborn and function returns '0' (DTYPE==0) or index number
       is recycled with 'Transfer' (DTYPE==1), no birth is generated and
       function returns '1'.
       'deaths' is incremented.
       Counters 'age1', 'age2', and 'agec' are updated.
       Counters in 'N',popsize are updated.
*/

#define DTYPE 0                              //Allows for non-constant population
                                             //size.
Death(int n)
{ int n2,yr,gid,tr,st,q,qv; dec age;
     A[n].InFunction=fnc_Death;
	st=A[n].strain;
  	yr=(int)(t-t0);                      //current year
	age = t-A[n].tBirth;                       //Compute the age at death.
        gid = A[n].groupID;                  // individual's current group ID
        q = A[n].state;                      // individual's current infection state

  	{ age1[0] += age; age2[0] += age*age;//Accumulate statistics for mean
    	agec[0] += 1; }                      //age and its variance.

	N[gid] -=1;                    //Decrement N[A[n].groupID][A[n].state].
	popsize-=1;	                     //updte population size
	deaths += 1;                         //Increment the number of deaths.

        BindDelete(A[n].bfrom);              //remove contact links
        BindDelete(A[n].bto);                //remove contact links
        DetachH(n);                          //Remove from the database.
        CCdel(gid,n);                        //remove individual
        if(DTYPE==0)                         //If population size is to be held
        { n=CCadd(UK);
        Birth(n, t);                         //constant, initiate a birth.
        return 0; }

  return 1;
}

/*------------------------------------------------
This finction checks the population size and increases births to keep it constant
ENTRY: 'Target_pop_size' Target fixed population size.

EXIT:  If the population is < Target_pop_size: a new
       individual 'n' is sent to the Birth() function, to be initialized as a
        susceptible newborn.
       If the population is > Target_pop_size: an individual 'n'
	is selected and sent to the Death() function, to be removed from the population.
       'deaths' is incremented.
       Counters 'age1', 'age2', and 'agec' are updated.
       Counters in 'N',popsize are updated.
*/
Check_population_size()
{
	int y,j,n,i,z;
	int grpcount,randgrp;

	z=CCgroup_size(NUK)+CCgroup_size(UK);      //Current populaton size

	if (z<Target_pop_size){                    //check if current population is too small
	for (i=0;i<(Target_pop_size-z);i++){       //create (Target_pop_size-z) new individuals
	n=CCadd(UK);
	Birth(n,t); }}                             //Create someone born in the UK.

	if (z>Target_pop_size){                   //check if current population is too large
	if ((z-Target_pop_size)>0){               //select (z-Target_pop_size) to repmove from the population
	 j=0;
  	 do{
	  do {randgrp=Rand()*2;
      	  n=CCsel(randgrp);} while (n==0);                     //select a random indiv from the whole population
      	 EventCancel(n);
      	 Death(n);                                //remove the individual via death
      	 j++;} while (j<(z-Target_pop_size));}}

}
/*----------------------------------------------------------------------------*
LIFESPAN DISTRIBUTION

This routine assigns a lifetime to an individual based on the present year, the
individual's sex and age, and other factors in the condition of the individual.
Various probability distributions may be selected. Exponentially distributed
ages, with a constant chance of death in any year, are included for calibration
with ordinary differential equation versions of this program.

ENTRY: 'sex' contains the individual's sex, 0=male, 1=female.
       'age' contains the individual's present age, years and fractions thereof.
       'mort' contains a mortality factor. For testing, this is the proportion
        of individuals who would die per year it deaths were strictly random
         (i.e., Poisson distributed in time).
       'lifedsn' defines the lifespan distribution computation:
         0 Exponential
         1 Gompertz
         2 Empirical life tables
       't' contains the present time.

EXIT:  'LifeDsn' contains the *remaining* life time (years until death) for the
        individual.

*/

static int lifedsn = 1;             //Type of longevity distribution to be used.

dec LifeDsn(int n,int sex, dec age, dec mort)
{ int yb, y,gid,q; dec w;
  gid=A[n].groupID;
  q=A[n].state;
  switch(lifedsn)
  {
    case 0: return 1./mort;                     //mortality
    //case 1: return Gompertz(sex,age,mort);   //Gompertz-Makeham death.
    case 1:                                  //Empirical life tables.
    {  yb = (int)(t-age);                    //Get year of birth.
       y  = yb-minBY; if(y<0) y=0.;            //Get array index for birth year.
	if (yb<=maxBY) w = RandF(A1, M1[y][sex], 122, age);//if value if available in data file
	else w=Expon(m1[q]);
       return w;
    }
    default: Error(922.0);                   //Incorrect life span selection.
  }
  return 0;                                  //(Will never reach this.)
}


/*----------------------------------------------------------------------------*
INITIALIZE STARTING POPULATION

This function sets up the intial population by looping through matrix 'n1981'
which holds numbers in the initial population by age, sex, and rob. For the SSA
version of model, 'ssa1981' is used for the proportions of SubSaharan Africans
among non-UK born in 1981 by age and sex.

Notes: In Param(), could multiply 1981 (if they are proportions) by initial pop
size, e.g. 'initpop'. Could make assignments deterministic since the numbers are
so large -- this would take some planning for dealing with remainders etc.

ENTRY: 'n1981' contains numbers by age, sex, and rob for individuals in the
         population at initialization in 1981.
       'ssa1981' contains the proportion of SSAs among non-UK born
         at population initialization.

EXIT:  The intial population is set up; each individual is assigned attributes
        and scheduled for exactly one event (other event times may be stored
        for an individual.
*/

InitPop()
{ int ii,a,s,i,n,st,rob,randrob,yr; dec age,wd,wh,we,wv,xs,tinf; int homl;
dec n1981_Total=0.;
  GroupInit();                               //Initialize groups
ii=0;
 yr=(int)(t-t0);

    for(a=0; a<121; a++){                        //age
    for(s=0; s<2;   s++){                      //sex
    for(rob=0; rob<2; rob++){   //region of birth
	for(i=0; i<n1981[a][s][rob]*0.1;i++){//scaled down
	if(ii>INDIV) Error(623);          //Check population size.
	age = a+Rand();                     //Assign age plus random bit.
	n=CCadd(rob);                        //Add new individual
  st = Rand()<0.00078? 0: 1;      //Assign infection status from random number generation.
	ii=ii+1;
	BasicInd(yr,n,rob,age,s,rob, st);                    //Set up basic individual.
        EventCancel(n);
  	Check_all_events(n);
	}}}}
}
/*----------------------------------------------------------------------------*
SET UP BASIC INDIVIDUAL FOR POPULATION INITIALIZATION

This function sets up a basic individual assigned to a death, emigration time.
This is similar to birth but processes individuals of any
age, sex, or region of birth (anyone being initialized when the model starts).
The scheduled event may change if a state other than 'Uninfected' is
assigned when diseaes state is assigned. This function is merely used to
get the individual initialized.

ENTRY:  'A[n]' individual is not scheduled for any event.
        'rob' is 0 for non-UK born , 1 for UK-born.
        'age' is the age of the individual in years.
        'sex' is 0 for males and 1 for females.

EXIT:   'A[n]' is in the Uninfected state and scheduled for its earliest
          event.

*/
BasicInd (int yr, int n, int rob, dec age, int s, int grp, int st)
{ dec wd, we, wv;
 int q;
  A[n].InFunction=fnc_BasicInd;
  popsize+=1;                           //update population size
  A[n].id = IDnew();                    //Assign unique ID number
  AttachH(n);                           //Attach to database
  InitAbinds(n);                        //initialize contact bindinds
  clear_times(n);                       //Initialize times
  A[n].v = 1;                           //Maximum probability to be chosen: Tendai
  A[n].rob = rob;                       //Set to rob
  A[n].groupID = grp;                   //Assign initial group
  A[n].tBirth = t-age;                  //Assign birth time from age.
  A[n].sex = s;                         //Assign sex.
  A[n].state= st;                     //Initialize TB Disease state
  q=A[n].state;
  A[n].strain=0;
  A[n].tDeath = wd = t+LifeDsn(n,s,age,m1[q]); //Assign time of death.
  if(wd<A[n].tBirth+age) Error(612.2);          //Check death time.
  if(wd<t) Error(850.);                      //check for errors

  Check_all_events(n);
 }


/*----------------------------------------------------------------------------*
REPORTING

This function is called periodically to display the cumulative number of
infections and other statistics. THIS FUNCTION SHOULD BE CALLED NO LESS THAN
ONCE PER YEAR, in current set-up, so that births per year ('bpy') and immigrants
per year ('ipy') can be updated at least each year. For generating mid-year
population estimates, this function should be called at least TWICE per year,
once at the beginning of the year and once mid-way through the year.

ENTRY: 'prog' contains the name of the program presently running.
       't' contains the current time.
       't0' contains the starting time.
       'N' contains the value of each dynamical variable, index by the state
         number ('qUTB', 'qPLTB', ...).
       'startsec' contains the wall-clock time of the start of the run.
       'kernel' defines the contagion kernel.
       'deaths' and 'events' contain the number of deaths and events since
         the counters were cleared.
       //-'relativetime' is set if the simulated times are to be reported relative
       //- to the simulated starting time.
       'rand0' defines the random number sequence used.

EXIT:  The next line of the report has been printed.
       'deaths' and 'events' are cleared.
*/

static int ReportFirst;
ReportInit() { ReportFirst = 1; } //To print headers set to 0;

Report(char *prog)
{
  int i,j,ac,r,y,yr; dec z,age;
  int z1,z2;

  //print to screen simulation infor and output labels
  if(ReportFirst==0)
  { ReportFirst = 1;
    printf("Dataset:     Simulation output of program '%s'\n", prog);
    printf("Kernel:      %s\n",
    kernel==0? "Mean field":
    kernel==1? "Cauchy":
    kernel==2? "Gaussian":
                 "Unspecified");
    printf("Sequence:    %lu\n\n", rand0);

    EventProfile("Initial");

    printf("Label t:	Time, in years and fractions thereof.\n");
    printf("Label N:	Total population size.\n");
    printf("Label Progs: Number of progressions since last report\n");
    printf("Label Deaths:  Number of deaths since last report.\n");
    printf("Label nbirths: Total number of births.\n");
    printf("Label UKborn: Total number of UK born\n");
    printf("Label NUKborn: Total number of non-UK born\n");

    printf("\n t \t  \tN \t  \t Progressions \t   \tDeaths  \tBirths \tUK \tNUK\n");
  }
    //Calculate result summarise
    z1=CCgroup_size(UK);
    z2=CCgroup_size(NUK);

   //Write results to screen.
    printf("%6.1f  \t%d	\t%d  	\t%d \t%d   \t%d  \t%d\n",
    t, popsize, progressions, deaths, nbirths,z1,z2);

  ///Write results to output file which is defined in Declarations.c
  fprintf(fptr,"%.0f\t%d\t%d\t%d\t%d\t%d\t%d\n",t, popsize, progressions, deaths, nbirths,z1,z2);

  fprintf(stderr, "  %.1f\r", t);            //Update status indicator.
  fflush(stdout); fflush(stderr);            //Make sure everything shows.
  deaths = progressions = events =nbirths = 0;      //Clear time-step counters.


/////////////////////////////////////////////////////////////////
//UPDATE TIME DEPENDENT PARAMETERS VALUES
/////////////////////////////////////////////////////////////////
  y = (int)t;                                //Get calendar (integer) year.
  if(y>lup){
  if((t>=interv-1))                                  //Check to see if parameters
  { //ypb = 1./bcy[y-(int)t0];                 //sensitive to calendar year
    //ypi = 1./interv_nimmigrants;}//immig[y-(int)t0];               //need updating.
  lup = y; }}
////////////////////////////////////////////////////////////////////////
  if((t-y)>0.3 && (t-y)<0.7 && y>1998)       //Since computation is expensive
  {                                          //only get population sizes when
                                             //necessary (mid-year).
  for(j=0;j<nC;j++){
    immid=Clowest[j+1]-Emptyc[j];
    yr = y-(int)t0;                          //Get year array index.
    for(i=Clowest[j]; i<immid; i++)                   //Loop through immigrants.
    { age = t-A[i].tBirth;                   //Get age and find age class.
      ac = age<15?0: age<45?1: age<65?2: 3; }}        //Increment correct compartment
                                             //for this individual.
  }}

/*----------------------------------------------------------------------------*
CLOSURE.

When processing is finished, this routine writes final statistics, prints
time trajectory of disease cases and returns an array with case notification
rates (or numbers of notifications) observed over the simulation.

ENTRY: 'startsec' contains the starting time, in the format of function
        'time'.
       'trho' and 'nrho' contain the total dispersal distance and the number
        of dispersal events.
       'tinfections' and 'linfections' contain the total number of infections
        targetted and the number that fell within the geographic area.
       'tstep' has accumulated statistics throughout the run.

EXIT:   Final statistics have been displayed.
        'out' contains the notification rates observed over the simulation.
*/

static dec nstep  =  0;                      //Number of steps
static dec tstep1 =  0;                      //Mean time step
static dec tstep2 =  0;                      //Standard deviation in time step
static dec tsmin  =  1E10;                   //Smallest time step
static dec tsmax  = -1E10;                   //Largest time step

static dec trho, nrho;                       //Statistics for local dispersal.
static dec tinfections, linfections;


FinalInit()
{
  nstep = tstep1 = tstep2 = 0;
  tsmin  =  1E10; tsmax  = -1E10;

  trho = nrho = 0;
  tinfections = linfections = 0;
}

Final()
{ int a,s,r,y,d,route,n,m,k; dec size,w;
  FILE *cases, *pop;

  printf("\n");
  size  = (indiv+3) * sizeof(struct Indiv);  //Tendai is this linked to the psuedo individuals?
  size += EventProfile("Final");

  tstepfin();
  { printf("Time steps:      Mean %s, Min %s, Max %s, SD %s, N %.0f\n",
            Tval(tstep1), Tval(tsmin), Tval(tsmax), Tval(tstep2), nstep); }

  if(nrho)
    printf("Dispersal:       Mean distance %.1f grid units.\n", trho/nrho);

  { printf("Infections:      Targeted %.0f, out of area %.0f, ratio %.2f%%\n",
      tinfections, tinfections-linfections,
      100.*(tinfections-linfections)/tinfections); }

  if(agec[0])
  { age1[0] /= agec[0]; age2[0] = sqrt(age2[0]/agec[0] - age1[0]*age1[0]);
    printf("All individuals: Mean age %.1f, SD %.1f, N %.0f\n",
      age1[0], age2[0], agec[0]); }

  if(agec[1])
  { age1[1] /= agec[1]; age2[1] = sqrt(age2[1]/agec[1] - age1[1]*age1[1]);
    printf("Disease-free:    Mean age %.1f, SD %.1f, N %.0f\n",
      age1[1], age2[1], agec[1]); }

  printf("\n");
  printf("Memory usage:    %.2f gigabytes\n", size/(1024*1024*1024));

  printf("Elapsed time:    %s\n",
    Tval((dec)(time(NULL)-startsec)/60/60/24/365.25));
  fprintf(stderr, "          \n");
  fflush(stdout); fflush(stderr);


 }
/*----------------------------------------------------------------------------*
TIMING STATISTICS

The system can take very small time steps or very large, depending on the number
and frequency of events. This routine keeps track of individual time steps for
statistical tracking, to be reported at the end of the run.

ENTRY: 't' contains the current time.
       'tn' contains the next instant of time.

EXIT:  Statistics have been accumulated for the time step.
*/

tstep(dec t, dec tn)
{ dec dt;

  dt = tn-t;                                 //Compute the length of the step.

  tstep1 += dt;                              //Accumulate the sum and sum of
  tstep2 += dt*dt;                           //squares.

  if(tsmin>dt) tsmin = dt;                   //Accumulate the minimum and
  if(tsmax<dt) tsmax = dt;                   //maximum.

  nstep  += 1;                               //Accumulate the count.
}

/*-------------------------------------------------------------------------
TIMING RESULTS

ENTRY:  'tstep' has accumulated statistics throughout the run.

EXIT:   'nstep'  contains the number of time steps.
        'tstep1' contains the length of the average time step, in years.
        'tstep2' contains the root-variance in length of the time steps.
        'tsmin'  contains the shortest time step.
        'tsmax'  contains the longest time step.

NOTE: Here it is called "root variance" rather than "standard deviation"
because the division is by 'n', not 'n-1'.
*/

tstepfin()
{
  if(nstep==0) return 0;  //Tendai changed return to return 0; to avoid warnings                       //Avoid division by zero.
  tstep1 /= nstep;                           //Compute the mean.
  tstep2 = tstep2/nstep - tstep1*tstep1;     //Compute the variance.
  tstep2 = sqrt(tstep2);                     //Convert to root-variance.
}
/*----------------------------------------------------------------------------*
CHECK FOR MONOTONICITY

This routine checks whether a table of cumulative probabilities is monotonically
increasing and optionally whether it is bracketed by 0 and 1.

ENTRY: 'p' is the table of cumulative pobabilities.
       'n' is the number of entries in the table.
       'b' is set if the table should begin with 0 and end with 1.
       'r1' and 'r2' contain two numbers that may help to identify the location
         of the error. If such numbers will not help, then either or both
         contain zero.

EXIT:  The routine returns if the table appears to be correct. If not, an error
         message is issued and the routine never returns.
*/

int monotone(dec p[], int n, int b, int r1, int r2)
{ int i;

  for(i=1; i<n; i++)                         //Make sure the sequence never
    if(p[i-1]>p[i])                          //decreases.
      Error3(621., "`",r1, "` ",r2, "` ",i);

  if(b && (p[0]!=0||p[n-1]!=1))              //If requested, make sure it begins
    Error2(622., "`",r1, "` ",r2);           //with 0 and ends with 1.

  return 0;                                  //(Will never reach this).
}

/*----------------------------------------------------------------------------*
SERVICE ROUTINES
These are parameter values that can be externally define/fed into the simulation
*/

char *pntab[] =                              //Table of parameter names.
{ "my_id_0","currentrun","fnumber", "randseq", 0 };

dec *patab[] =                               //Table of parameter addresses.
{ &my_id_0, &currentrun, &fnumber, &randseq, 0 };

#include "service.c"


/*============================================================================*
NOTES
_____________________________________________________
Before running on supercomputer:
1) Change 'SUPER' to '1'
2) In common.h change #define indiv to higher value
_____________________________________________________


*/
