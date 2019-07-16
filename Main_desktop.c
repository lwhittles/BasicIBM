#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include "fileio.h"
#include "Declarations.c"
#include "groups.c"
#include "contacts.c"
#include "clock.h"
#include "BaseFunctions.c"

/******************************************************************************/

int firstseed;
dec currentrun=0;
int master = 0;
int my_id=0;
dec my_id_0=0;
int numprocs=1;
//#define main dec *mainiac                    //Make this main not the real main, for
                                             //use with fitting routine. Comment
                                             //out for independent executable.

#ifdef  main
static int fit5i = 1;                        //Flag set when linked with fitting.
#else
static int fit5i = 0;                        //Flag set when not linked.
#endif
static int fitm  = 2;                        //Flag set when fitting to rates,
                                             //(0=numbers, 1=rates, 2=clust
                                             //and 3=overall rates).


#include "MainModelFunctions.c"
int main ( int argc, char *argv[] )
{mainfx ( argc, argv );}
