int prnt = 0; //on and off switch fro printing output

struct Indiv *A; //State of each individual, including their
                 //characterisitics, saved event times, etc.
//Initialize time
#define T0 1981         //Start time of model, years.
#define T1 2050         //End time of model, years. The simulation \
                        //ends -before- reaching this year.
dec interv = T1 - 500.; //Time of intervention start
#define RT (T1 - T0)    //Running time of model, calendar years.
/* Population initialization */
int maximm;                   //Maximum immigrants in pop'n at any time. (non-UK born: tendai)
int Target_pop_size = 700000; //Population size if population size will be constant
//
#define SUPER 0  //Notes whether model is run on supercomputer, \
                 //0=no, 1=yes (this changes population sizes).
#define DPARAM 1 //Allows model to accept disease progression \
                 //parameters (4 in this version), 0=no, 1=yes.
#define REC 1    //Flag for whether tallying recent transmissions.
//Psuedo individuals
#define BIRTH (indiv + 1) //Index used for scheduling births.
#define nEngines 1
//Groups
#define NUK 0         //Array index for non-UK born.
#define UK 1          //Array index for UK-born.
#define nC 2          //number of groups
#define TotalPgrps nC //total population subgroups

//Sex
#define M 0 //Array index for males.
#define F 1 //Array index for females.

#define E 0.0000000001; //Small number added to some event times to \
                        //ensure they happen in the future.

#define AC 122               //Age classes for mortality data.
#define BY (2010 - 1870 + 1) //Number of birth cohorts for mortality data.
#define maxBY 2010
#define minBY 1870
//Counters
int N[TotalPgrps];             //Current number in each group
int UTB[TotalPgrps];           //Current number uninfected in each group
int LTB[TotalPgrps];           //Current number with latent TB in each group
int ATB[TotalPgrps];           //Current number with active TB in each group
int DTB[TotalPgrps];           //Current number with dormant TB in each group
dec age1[2], age2[2], agec[2]; //Accumulators for 1st and 2nd moments of age.

int deaths;       //Current number of deaths.
int nbirths;      //Current number of births
int progressions; //Current number of progressions.
int regressions;  // Current number of regressions
int deathsTB;
int events; //Current number of events dispatched.

int immid; //Next available ID number for immigrants.
int ukbid; //Next available ID number for UK-born.
int stid;  //Next available ID for new strain types.
int repid; //Next available ID for 'repc3' case report.

int popsize; //current population size
int uninfecteds;
int actives;
int latents;
int dormants;
extern dec t; //Current time (Managed by 'EventSchedule').
dec pt;       //Time of previous report.
dec t0 = T0;  //Beginning time of simulation.
dec t1 = T1;  //End time of simulation.
int lup;      //Year of last update to birth & immigration
              //rates, which are sensitive to calendar year.

unsigned long startsec; //Starting clock time, seconds of Unix.
unsigned long rand0;    //Starting random number seed.

//int *InfectedBy_array;
FILE *cc; //Create files pointers for output of
FILE *rc; //cumulative cases (all) and reported cases.

/*----------------------------------------------------------------------------*

PARAMETERS AND CONTROL VARIABLES
*/

/* Population initialization */
dec n1981[121][2][2]; //Numbers in each age/sex/rob category at
                      //population initialization, 1981.

/* Mortality */
dec A1[AC]; //Holds ages 0-121 which correspond to the

//cumulative probabilities in M1.
dec M1[BY][2][AC]; //Cumulative probabilities of death by a given
                   //birth cohort, sex and age.
/*These are old mortality rates used to generate lifetimes with exponential
distribution, left to keep compatibility with testing version of model */
dec m1[1]; //Mortality of uninf ind's

//*Birth and migration */
dec bcy[RT];             //Births by calendar year. 24/11/16 I have updated birth witha nd additional 6 yrs to 2015. note fmt below also have +6
dec pmale[RT];           //Portion of newborns who are male by year.
dec ypb, ypi;            //Years per birth, years per immigrant.
dec ypb_fixed = 0.00025; // for testing this can be fixed else use ypb

/*Reporting options*/
dec relativetime = 0; //Set for relative time reporting.
dec randseq = 0;      //Random number sequence (set with 'randseq=N').
dec tgap = 1;         //0.5;             //Time between reports, years.
dec kernel = 0;       //Contagion kernel, 0=Panmictic, 1=Cauchy.
dec sigma = 1;        //Width of contagion kernel, where applicable.

//create an output file to store simulation results
char fnamestem[] = "summary";
dec fnumber = 12.;
char fname[BUFSIZ];
FILE *fptr, *fpds;
char ftimesname[] = "diseasetocare.txt";

struct IO fmt[] = //Format statements for input/output.
    {
        /*00*/ {(dec *)bcy, {-'i', RT + 7}},
        /*01*/ {(dec *)pmale, {-'i', RT}},
        /*02*/ {(dec *)n1981, {-'a', 121, -'s', 2, -'r', 2}, {-'s', -'a', -'R', 1, 0, 1}},
        /*03*/ {(dec *)M1, {-'b', BY, -'s', 2, -'a', AC}, {-'s', -'b', -'A'}},
};

/*----------------------------------------------------------------------------*
DATA PROCESSING AND ARRAY INITIALIZATION

This function reads data files into appropriate arrays and also copies smaller
amounts of data, coded here instead of read from files, into appropriate
arrays.

ENTRY: Files are set up for 'FileIO', including:
       'births.txt' contains the number of births.
       'mort.txt' is a file of mortality data (life tables).
       'casefat.txt' holds case fatality rates.

EXIT:  'A1' contains ages which accompany 'M1' for calls to 'RandF'
       All data read in through FileIO() is stored properly, including:
        arrays 'bcy', 'mort', 'cft' contain data for births, mortality and case
        fatality due to TB.
*/

void Data()
{
  int i, j, k, temp, a, s, y, r, ac, st;
  dec temp2;
  //-printf("Entering Data() routine....\n");

  for (i = 0; i < AC; i++) //Create array of ages which correspond
  {
    A1[i] = i; //to cumulative probabilities of death
  }            //in M1 (A1) and cumulative probabilities

  m1[qUTB] = 0.01; //Mortality of uninf ind

  //The /60 below is temporary in getting this started under Cygwin: Tendai
#define RSCALE (SUPER ? "r|" : "r|=n/1")   //Scaling factor 1/5 for laptops.
  FileIO("births.txt", fmt[0], RSCALE);    //Read birth data.
  FileIO("propmale.txt", fmt[1], "r|");    //Fraction of births that are male.
  FileIO("n1981_new.txt", fmt[2], RSCALE); //Read 'n1981' as integers.
  FileIO("mort.txt", fmt[3], "r|");        //Mortality data.

  for (i = 0; i < BY; i++)             //Audit the cumulative mortality
    for (s = 0; s < 2; s++)            //data to make sure each table
      monotone(M1[i][s], AC, 1, i, s); //increases from 0 to 1.
}
/*----------------------------------------------------------------------------*
PARAMETER CHANGING

This function updates variables associated with parameters that change with
each model run. This routine must come after any change to parameters, e.g.
through the 'gparam' function. Currently no parameters are changed so the function can be ignored

ENTRY:
EXIT:
*/
void Param()
{
  int a, s, r;

  dec ep = 0.00000000000001; //Check that 'ehiv' and 'df' are not
}

/*----------------------------------------------------------------------------*
MAIN INITIALIZATION

This routine should be called each time the program is reused, to clear static
variables for the next run. The function was added to allow the fitting routine,
to implemented parallel, replicate runs of the TB program. This would not be necessary if the program
were called as independent executable, as before.
*/

MainInit()
{
  int i, j, k, l, m, n, dt, route, st;
  //Initialize counters
  for (j = 0; j < TotalPgrps; j++) //Groups
    N[j] = 0;
  popsize = 0; //current population size
}
