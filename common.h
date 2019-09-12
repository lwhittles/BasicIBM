/* DATA STRUCTURES COMMON AMONG THE MODULES

This module defines the main data structure in the program, the list of
individuals and their characteristics, also the main space-consuming entity in
the program.

Time is encoded in 8-byte double-precision floating point values ('dec'), which
is simple but a little extravagant. At the cost of a clarity time could be
encoded in 4-byte unsigned integers instead, with a single floating point number
defining the base year and time.
*/

#ifndef TYPEDEF
#define TYPEDEF
typedef double dec;
typedef short ints;
typedef unsigned char charu;
typedef unsigned int intu;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define indiv 2000000 //870000     //Maximum population size.
#define INDIV indiv   //Maximum population size.
//Initialize hashcoded names
#define ID indiv      //10000
#define NPSEUDO 2     //Number of pseudo individuals (clock driven)
#define Ncontacts 100 //Maximum number transmission and infection events per individual

//TB TRANSITIONS:
#define pDeath 0    //Pending death
#define pProgress 1 //Pending progression
#define pRegress 2  //Pending dormancy
#define pDeathTB 3  //Pending death from TB
#define MaxE 3      //Maximum pending event (non-clock)
//Clocks
#define pBirth MaxE + 1 //Pending birth

// FUTURE TIMES:
#define tDeath t[pDeath]       //Time for closure of this record
#define tProgress t[pProgress] //Time for progress????
#define tRegress t[pRegress]   //Time for Dormancy
#define tDeathTB t[pDeathTB]   // Time death from TB
#define tBirth t[pBirth]       //Time of initiation of this record
#define MaxT MaxE + 1          //highest t index

struct Indiv //STRUCTURE OF EACH RECORD                   BYTES
{
  dec v; //Probability of being chosen: Tendai
  //id and idn are required for contacts
  unsigned int id;      //individual's ID number: Tendai, LW: this could be intu
  unsigned int idn;     //individual's ID number: Tendai, LW: this could be intu
  dec t[MaxT + 1];      //Separate times for individual               64
  unsigned int groupID; //Group ID number, LW: this could be intu
  intu sex;             //Sex of this individual (0=female, 1=male)    1
  intu rob;             //Region of birth (0=Foreign-born, 1=UK-born)  1
  intu pending;         //Number of pending event                      1
  intu state;           //Number of present state
  intu strain;          // infecting strain
  intu InFunction;      //current function number
  // bto and bfrom are required for binding lists
  int bto[2];   //Binding list of infections by this individual
  int bfrom[2]; //Binding list of infections to other individual

} Aw; //                                            69 *

extern struct Indiv *A; //List of individuals.

// TB STATES:
#define qUTB 0  //1-1   //Uninfected
#define qLTB 1  //Latently Infected
#define qATB 2  //Actively Infected
#define qDTB 3  //Dormant infection
#define q0 qUTB //Lowest numbered state
#define q1 qDTB //Highest numbered state

// PROTOTYPES:
dec Rand();                      //Random number generator
dec Uniform(dec, dec);           //Uniform random number
dec Expon(dec);                  //Poisson events in time
dec Cauchy(dec, dec);            //Cauchy distribution
dec Gauss(dec, dec);             //Gaussian distribution
dec LogNormal(dec, dec);         //Lognormal distribution
dec RecovDsn(int, dec, dec);     //Recovery time from disease
dec LifeDsn(int, int, dec, dec); //Lifespan of individual
dec Gompertz(int, dec, dec);     //Lifespan of individual
char *Tval(dec);                 //Time conversion
unsigned long RandStartArb();    //Random number initializers
unsigned long RandStart(unsigned long);
dec Val(int, dec, dec[], dec[], int, int);
dec RandF(dec[], dec[], int, dec);
int Loc(dec[], int, int, dec);
dec Tdis(int, int, int, int, dec);
dec GetAge(int, int, int);
dec EmDsn(int, int, dec, dec);
int Error(dec);
int Error1(dec, char *, dec);
int Error2(dec, char *, dec, char *, dec);
int Error3(dec, char *, dec, char *, dec, char *, dec);
int StrainNum(int);

// LOCAL FUNCTIONS:
#define min(a, b) ((a) < (b) ? (a) : (b))         //Minimum
#define max(a, b) ((a) > (b) ? (a) : (b))         //Maximum
#define abs(a) ((a) >= 0 ? (a) : -(a))            //Absolute value
#define round(a) ((a) >= 0 ? (a) + 0.5 : (a)-0.5) //Rounding to integer

// Clarence Lehman, August 2009, modified by Adrienne Keen, 2010

//Functions used #for ease of debugging
#define fnc_Find_individual 1
#define fnc_Birth 2
#define fnc_Death 3
#define fnc_Transfer 4
#define fnc_BasicInd 5
#define fnc_VanDispatch 6
#define fnc_TransferGroup 7
#define fnc_Progress 8
#define fnc_Regress 9
#define fnc_DeathTB 10

// Clarence Lehman, August 2009, modified by Adrienne Keen, 2010, modified by Tendai 2016-
