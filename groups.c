/*==============================================================================
GROUP MANAGEMENT

Consider a large number of individuals that occur in a relatively small number
of groups. For example, consider ten million individuals in a simulation
assigned to 120 different years of birth combined with 5 different regions of
birth, making 600 groups. And suppose the probability of being selected for some
event depends jointly on the year of birth and region of birth. This module
contains Order-1 methods for (1) selecting individuals from any group, (2)
adding individuals to a group, and (3) deleting them. The running time is
independent of the number of individuals and nearly independent of the number of
groups.

An application is human disease models which need "age dependent mixing."
Transmission of common colds, for example, can be more likely between similar
age classes, since individuals of similar ages frequent similar
locales---day-care children are together, grade school, middle school, high
school, business places, nursing homes, and so forth---all these may have
approximately uniform mixing within and reduced mixing between groups. An
age-dependent mixing strategy can substitute for a more accurate but unknown
contact network.

Below is a hypothetical example of the probability of transmission of a cold
among age classes. The horizontal axis is the age of a susceptible individual,
the vertical axis represents relative probability of infection from an infected
four-year-old:

        :
       :::                  :
    ..:::::................:::.................:::.........................
    0    5   10   15   20   25   30   35   40   45   50   55   60   65   70


In this example, four-year olds have the highest probability of transmitting to
other four-year olds and to nearby ages, but also an increased probability of
transmitting to those the age of their parents, around 24-years old, and their
grandparents, around 44-years old, in this hypothetical example. Such
distributions would be empirically estimated and a group selected, for example,
with the routine 'RandF'.

The present module allows continuous or discontinuous variations in probability
of being selected within a class, which need not be used if equal probabilities
within a class are a good enough approximation.

STRUCTURE OF THE MODULE

Data structures for the simple version, which has a uniform probability within a
group, are as follows:

'A' is a one-dimensional array of all individuals, in order by group, but in no
particular order within group. Each individual carries its probability of being
selected.

'C' is a one-dimensional array of indexes identifying the lowest-numbered
individual in 'A' for each group, structured so that 'C[0]' is the index of the
lowest numbered individual in the first group and that 'C[j+1]-C[j]' is the
number of individuals in group 'j'.

'E' is a corresponding one-dimensional array identifying the number of empty
cells at the end of each group. Deletions typically increase the number of empty
cells and additions draw from them, where possible.

'V' is the maximum probability of selection for any individual allowed in the
class.

'nA' and 'mA' are the current number and maximum number of individuals in 'A',
respectively.

'nC' is the current number and maximum number of groups.

Three externally callable routines are included, 'CCsel', to select an
individual randomly from a specified group, 'CCadd', to add individuals, and
'CCdel', to delete individuals. Runtimes of all three are independent of the
number of individuals being analysed. 'CCsel' will typically be combined with
'RandF', which also is independent of the number of individuals. This entire
module, therefore, is Order-1 on individuals, which is the most crucial
variable.

EXAMPLE OF OPERATION

Here is a step-by-step example starting with an empty list of 27 entries and
filling it with individuals in random order, then deleting each of them in a
different random order. There are 52 possible individuals, each with a fixed
"name", 'A' through 'Z' or lower case 'a' through 'z'. Each is assigned a group,
as follows, which organizes the list:

    'A-J'    Group 0
    'K-T'    Group 1
    'U-Z'    Group 2
    'a-j'    Group 3
    'k-t'    Group 4
    'u-z'    Group 5

In the list below, the array 'A' starts with 27 empty slots, denoted by '-'. The
six groups, marked 0 through 5, have an initial allocation of 4 slots each,
leaving 3 slots at the end, in the non-group marked 6. Entries are added at the
first available slot for their group, until that group is filled. Then entries
cascade to the right, resting in the first available slot.

This routine is Order-1 on individuals. That is, its timing is the same whether
there are 27 individuals in the list or 27 million. It depends only on the
number of groups, and weakly on them.

    ---------------------------  A[i]
    0   1   2   3   4   5   6    C[i]
    4   4   4   4   4   4   3    E[i]
            v                         (Add A[09]='U' group=2 nA=01)
    --------U------------------  A[i]
    0   1   2   3   4   5   6    C[i]
    4   4   3   4   4   4   3    E[i]
                        v             (Add A[21]='z' group=5 nA=02)
    --------U-----------z------  A[i]
    0   1   2   3   4   5   6    C[i]
    4   4   3   4   4   3   3    E[i]
        v                             (Add A[05]='O' group=1 nA=03)
    ----O---U-----------z------  A[i]
    0   1   2   3   4   5   6    C[i]
    4   3   3   4   4   3   3    E[i]
         v                            (Add A[06]='P' group=1 nA=04)
    ----OP--U-----------z------  A[i]
    0   1   2   3   4   5   6    C[i]
    4   2   3   4   4   3   3    E[i]
             v                        (Add A[10]='Z' group=2 nA=05)
    ----OP--UZ----------z------  A[i]
    0   1   2   3   4   5   6    C[i]
    4   2   2   4   4   3   3    E[i]
          v                           (Add A[07]='M' group=1 nA=06)
    ----OPM-UZ----------z------  A[i]
    0   1   2   3   4   5   6    C[i]
    4   1   2   4   4   3   3    E[i]
                         v            (Add A[22]='x' group=5 nA=07)
    ----OPM-UZ----------zx-----  A[i]
    0   1   2   3   4   5   6    C[i]
    4   1   2   4   4   2   3    E[i]
                v                     (Add A[13]='g' group=3 nA=08)
    ----OPM-UZ--g-------zx-----  A[i]
    0   1   2   3   4   5   6    C[i]
    4   1   2   3   4   2   3    E[i]
                          v           (Add A[23]='v' group=5 nA=09)
    ----OPM-UZ--g-------zxv----  A[i]
    0   1   2   3   4   5   6    C[i]
    4   1   2   3   4   1   3    E[i]
    v                                 (Add A[01]='B' group=0 nA=10)
    B---OPM-UZ--g-------zxv----  A[i]
    0   1   2   3   4   5   6    C[i]
    3   1   2   3   4   1   3    E[i]
              v                       (Add A[11]='W' group=2 nA=11)
    B---OPM-UZW-g-------zxv----  A[i]
    0   1   2   3   4   5   6    C[i]
    3   1   1   3   4   1   3    E[i]
                           v          (Add A[24]='w' group=5 nA=12)
    B---OPM-UZW-g-------zxvw---  A[i]
    0   1   2   3   4   5   6    C[i]
    3   1   1   3   4   0   3    E[i]
                 v                    (Add A[14]='i' group=3 nA=13)
    B---OPM-UZW-gi------zxvw---  A[i]
    0   1   2   3   4   5   6    C[i]
    3   1   1   2   4   0   3    E[i]
           v                          (Add A[08]='S' group=1 nA=14)
    B---OPMSUZW-gi------zxvw---  A[i]
    0   1   2   3   4   5   6    C[i]
    3   0   1   2   4   0   3    E[i]
     v                                (Add A[02]='H' group=0 nA=15)
    BH--OPMSUZW-gi------zxvw---  A[i]
    0   1   2   3   4   5   6    C[i]
    2   0   1   2   4   0   3    E[i]
      v                               (Add A[03]='G' group=0 nA=16)
    BHG-OPMSUZW-gi------zxvw---  A[i]
    0   1   2   3   4   5   6    C[i]
    1   0   1   2   4   0   3    E[i]
                  v                   (Add A[15]='d' group=3 nA=17)
    BHG-OPMSUZW-gid-----zxvw---  A[i]
    0   1   2   3   4   5   6    C[i]
    1   0   1   1   4   0   3    E[i]
                   v                  (Add A[16]='h' group=3 nA=18)
    BHG-OPMSUZW-gidh----zxvw---  A[i]
    0   1   2   3   4   5   6    C[i]
    1   0   1   0   4   0   3    E[i]
            v                         (Add A[09]='N' group=1 nA=19)
    BHG-OPMSNZWUgidh----zxvw---  A[i]
    0   1    2  3   4   5   6    C[i]
    1   0    0  0   4   0   3    E[i]
             v                        (Add A[10]='R' group=1 nA=20)
    BHG-OPMSNRWUZidhg---zxvw---  A[i]
    0   1     2  3   4  5   6    C[i]
    1   0     0  0   3  0   3    E[i]
                     v                (Add A[18]='o' group=4 nA=21)
    BHG-OPMSNRWUZidhgo--zxvw---  A[i]
    0   1     2  3   4  5   6    C[i]
    1   0     0  0   2  0   3    E[i]
                      v               (Add A[19]='n' group=4 nA=22)
    BHG-OPMSNRWUZidhgon-zxvw---  A[i]
    0   1     2  3   4  5   6    C[i]
    1   0     0  0   1  0   3    E[i]
                 v                    (Add A[14]='Y' group=2 nA=23)
    BHG-OPMSNRWUZYdhginozxvw---  A[i]
    0   1     2   3   4 5   6    C[i]
    1   0     0   0   0 0   3    E[i]
                            v         (Add A[25]='u' group=5 nA=24)
    BHG-OPMSNRWUZYdhginozxvwu--  A[i]
    0   1     2   3   4 5    6   C[i]
    1   0     0   0   0 0    2   E[i]
                      v               (Add A[19]='f' group=3 nA=25)
    BHG-OPMSNRWUZYdhgifonxvwuz-  A[i]
    0   1     2   3    4 5    6  C[i]
    1   0     0   0    0 0    1  E[i]
              v                       (Add A[11]='T' group=1 nA=26)
    BHG-OPMSNRTUZYWhgifdnovwuzx  A[i]
    0   1      2   3    4 5    6 C[i]
    1   0      0   0    0 0    0 E[i]
                    ^                 (Del A[17]='g' group=3 nA=26)
    BHG-OPMSNRTUZYWhdif-novwuzx  A[i]
    0   1      2   3    4 5    6 C[i]
    1   0      0   1    0 0    0 E[i]
                              ^       (Del A[27]='x' group=5 nA=25)
    BHG-OPMSNRTUZYWhdif-novwuz-  A[i]
    0   1      2   3    4 5    6 C[i]
    1   0      0   1    0 1    0 E[i]
                            ^         (Del A[25]='u' group=5 nA=24)
    BHG-OPMSNRTUZYWhdif-novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    1   0      0   1    0 2    0 E[i]
                 ^                    (Del A[14]='Y' group=2 nA=23)
    BHG-OPMSNRTUZW-hdif-novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    1   0      1   1    0 2    0 E[i]
                     ^                (Del A[18]='i' group=3 nA=22)
    BHG-OPMSNRTUZW-hdf--novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    1   0      1   2    0 2    0 E[i]
          ^                           (Del A[07]='M' group=1 nA=21)
    BHG-OPTSNR-UZW-hdf--novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    1   1      1   2    0 2    0 E[i]
                     ^                (Del A[18]='f' group=3 nA=20)
    BHG-OPTSNR-UZW-hd---novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    1   1      1   3    0 2    0 E[i]
         ^                            (Del A[06]='P' group=1 nA=19)
    BHG-ORTSN--UZW-hd---novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    1   2      1   3    0 2    0 E[i]
      ^                               (Del A[03]='G' group=0 nA=18)
    BH--ORTSN--UZW-hd---novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    2   2      1   3    0 2    0 E[i]
                ^                     (Del A[13]='Z' group=2 nA=17)
    BH--ORTSN--UW--hd---novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    2   2      2   3    0 2    0 E[i]
           ^                          (Del A[08]='S' group=1 nA=16)
    BH--ORTN---UW--hd---novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    2   3      2   3    0 2    0 E[i]
               ^                      (Del A[12]='U' group=2 nA=15)
    BH--ORTN---W---hd---novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    2   3      3   3    0 2    0 E[i]
           ^                          (Del A[08]='N' group=1 nA=14)
    BH--ORT----W---hd---novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    2   4      3   3    0 2    0 E[i]
                    ^                 (Del A[17]='d' group=3 nA=13)
    BH--ORT----W---h----novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    2   4      3   4    0 2    0 E[i]
    ^                                 (Del A[01]='B' group=0 nA=12)
    H---ORT----W---h----novwz--  A[i]
    0   1      2   3    4 5    6 C[i]
    3   4      3   4    0 2    0 E[i]
                           ^          (Del A[24]='w' group=5 nA=11)
    H---ORT----W---h----novz---  A[i]
    0   1      2   3    4 5    6 C[i]
    3   4      3   4    0 3    0 E[i]
                          ^           (Del A[23]='v' group=5 nA=10)
    H---ORT----W---h----noz----  A[i]
    0   1      2   3    4 5    6 C[i]
    3   4      3   4    0 4    0 E[i]
    ^                                 (Del A[01]='H' group=0 nA=09)
    ----ORT----W---h----noz----  A[i]
    0   1      2   3    4 5    6 C[i]
    4   4      3   4    0 4    0 E[i]
        ^                             (Del A[05]='O' group=1 nA=08)
    ----TR----W----h----noz----  A[i]
    0   1     2    3    4 5    6 C[i]
    4   4     4    4    0 4    0 E[i]
                        ^             (Del A[21]='n' group=4 nA=07)
    ----TR----W----h----o-z----  A[i]
    0   1     2    3    4 5    6 C[i]
    4   4     4    4    1 4    0 E[i]
                        ^             (Del A[21]='o' group=4 nA=06)
    ----TR----W----h------z----  A[i]
    0   1     2    3    4 5    6 C[i]
    4   4     4    4    2 4    0 E[i]
                   ^                  (Del A[16]='h' group=3 nA=05)
    ----TR----W-----------z----  A[i]
    0   1     2    3   4  5    6 C[i]
    4   4     4    4   3  4    0 E[i]
                          ^           (Del A[23]='z' group=5 nA=04)
    ----TR----W----------------  A[i]
    0   1     2    3   4  5   6  C[i]
    4   4     4    4   3  4   1  E[i]
         ^                            (Del A[06]='R' group=1 nA=03)
    ----T----W-----------------  A[i]
    0   1    2    3   4   5   6  C[i]
    4   4    4    4   4   4   1  E[i]
        ^                             (Del A[05]='T' group=1 nA=02)
    --------W------------------  A[i]
    0   1   2    3   4   5   6   C[i]
    4   4   4    4   4   4   2   E[i]
            ^                         (Del A[09]='W' group=2 nA=01)
    ---------------------------  A[i]
    0   1   2   3   4   5   6    Clowest[i]
    4   4   4   4   4   4   3    E[i]
*/
#define mA   indiv+1           //Maximum population size
//#define nC   3               //Maximum number of groups in 'C'. NUK;UK
static int   Clowest[nC+2];  //Array of groups.
static int   Emptyc[nC+2];   //Number of empty cells trailing each group.
static dec   V[nC+2];        //Maximum probability in each group.
static int   nA = 0;         //Current number active in 'A'.
//static int   emax  = 5;    //Maximum number of empty slots per group.
//static dec   draws = 0;    //Number of random draws.


/*------------------------------------------------------------------------------
1. SELECT INDIVIDUAL

ENTRY: 'k' contains the group to be sampled.
       'nC' contains the number of groups.
       'Clowest' indexes the first individual in each group.
       'Emptyc[k]' contains the number of empty cells at the end of the group.
       'V[k]' contains the maximum probability for individuals in the group.
       'A[i].v' contains the probability of selection for each individual in
         the group. This is greater than zero but not greater than 'V[k]'.

EXIT:  'CCsel' indexes the individual selected. If zero, the group is empty.
*/

int CCsel(int k)
{ int h, n; dec r;
  if(Clowest[0]==0||k>=nC) { return 0;}         //Guard against null cases.
  h = Clowest[k+1]-Clowest[k]-Emptyc[k];     //Determine how many occupy the  
  if(h<=0) { return 0; }                        //group.
   while(1)                                   //Select an individual randomly
    { n = Clowest[k]+h*Rand();               //and use it if it corresponds to
      if(A[n].id==0) { return 0;}  //Skip deleted individuals
      return n;      

      /*if(A[n].v==V[k]){return n;}            //the maximal probability.      
      r = Rand();                            //Otherwise use it only in propor-
      if(r<A[n].v/V[k]) {return n; } */        //tion to its relative probability.
    }
} //Tendai: Note that the relative probability allows one to rescale the probabilities to between 0 and 1 instead of between 0 and maximum probability.

/*------------------------------------------------------------------------------
2. ADD INDIVIDUAL

ENTRY: 'k' contains the group for the new individual.
       'Aw' contains the formatted record for the new individual. In
         particular, 'Aw.v' contains the probability for the individual.
       'A' contains the list of individuals, ordered by group.
       'Clowest' indexes the first individual in each group.
       'Emptyc' contains the number of empty cells in each group.
       'V[k]' contains the maximum probability for individuals in the group.
       'nW' contains the summed probabilities in all groupes.
       'mA' contains the maximum number of individuals that may reside in 'A'.
       'nA' contains the current number of individuals in 'A'.
       'nC' contains the number of groups.

EXIT:  'CCadd' contains the index of an available entry in 'A' where the
         individual should be added. If zero, none can be added.
       'A', 'C', and 'E' are updated to include space for the new individual.
       'nA' is increased by one.

Note: This routine must check for empty slots both in higher and lower numbered
groups, otherwise the array may not be able to be packed full. Here is an
example of how it works:

    Add 'Z2' to group 2, shifting from higher-numbered groups:
       0                 1              2           3              4
      A0 B0 C0 D0 -- -- E1 F1 G1 H1 I1 J2 K2 L2 M2 N3 O3 P3 Q3 R3 -- -- --
      A0 B0 C0 D0 -- -- E1 F1 G1 H1 I1 J2 K2 L2 M2 -- O3 P3 Q3 R3 N3 -- --
      A0 B0 C0 D0 -- -- E1 F1 G1 H1 I1 J2 K2 L2 M2 Z2 O3 P3 Q3 R3 N3 -- --
       0                 1              2              3              4

    Add 'Z2' to group 2, shifting from lower-numbered groups:
       0                 1              2           3              4
      A0 B0 C0 D0 -- -- E1 F1 G1 H1 I1 J2 K2 L2 M2 N3 O3 P3 Q3 R3 -- -- --
      A0 B0 C0 D0 -- I1 E1 F1 G1 H1 -- J2 K2 L2 M2 N3 O3 P3 Q3 R3 -- -- --
      A0 B0 C0 D0 -- I1 E1 F1 G1 H1 Z2 J2 K2 L2 M2 N3 O3 P3 Q3 R3 -- -- --
       0             1              2               3              4
*/

int CCadd(int k)
{ int i, m, n, d,j;
  if(Clowest[0]==0) return 0;                //Guard against null cases.
  if(nA>=mA)        return 0;
  if(Aw.v>V[k]) Error(831.0);                //Guard against invalid additions.

  for(d=0; k-d>=0||k+d+1<=nC; d++)           //Search forward and backward
  { i = k-d;   if(i>=0  && Emptyc[i]) break; //simultaneously for the nearest
    i = k+d+1; if(i<=nC && Emptyc[i]) break; //group with an empty slot.
    i = -1; }
//printf("i=%d,k=%d, Emptyc[k]=%d\n",i,k,Emptyc[k]); 
  if(i<0) return 0;                          
  
  if(k<=i && i<=nC)                          //If there is a slot at the present
  { j=i;
	for(j=i; j>=k; j--)                         //location, use it, or if forward,
    {//printf("j=%d\n",j); 
	Emptyc[j] -= 1; 
      m = Clowest[j+1]-Emptyc[j]-1;          //cascade it back to the current
//printf("m=%d,j=%d,k=%d, Clowest[j+1]=%d,Clowest[j]=%d,Chighest[k]=%d,Emptyc[j]=%d\n",m,j,k,Clowest[j+1],Clowest[j],Clowest[j+1]-Emptyc[j],Emptyc[j]); 
      if(j==k) { nA += 1; 			
		      return m; }        //location.
      n = Clowest[j]; if(m!=n) Transfer(m, n); //Swap position
      Emptyc[j-1] += 1;  Clowest[j] += 1; 

	} 
	}  
  
  if(i<k && i>=0)                            //Otherwise, if there is an open
  { j=i;
for(j=i; j<=k; j++)                         //earlier in the list of
    { Emptyc[j] -= 1; 
      m = Clowest[j+1]-1;                    //groups, cascade it forward
      if(j==k) { nA += 1; return m; }        //to the current location.
      n = Clowest[j+2]-1; 
	//n=Clowest[2+1]-Emptyc[j+1];
	if(m!=n) Transfer(m, n);
      Emptyc[j+1] += 1; Clowest[j+1] -= 1; } }

  return 0;                                  //Indicate the list is full.
}


/*------------------------------------------------------------------------------
3. DELETE INDIVIDUAL

ENTRY: 'n' indexes the individual being deleted. 'A[n]' is ready for reuse.
       'k' contains the group for former individual 'n'.
       'A' contains the list of individuals, ordered by group.
       'Clowest' indexes the first individual in each group.
       'Emptyc' contains the number of empty cells in each group.
       'nW' contains the summed probabilities in all groups.
       'nA' contains the current number of individuals in 'A'.
       'nC' contains the number of classes.
       'emax' contains the maximum number of empty slots per group.

EXIT:  'CCdel' is zero if the operation failed.
       'A', 'C', and 'E' are updated to exclude the deleted individual.
       'nA' is decreased by one.

Note: When necessary, this routine cascades empty slots to higher-numbered
groups. That always works, but it could also choose to cascade them to
lower-numbered groups if those could accept them, to keep the open slots
reasonably balanced. But it is not clear that the added time to check would be
worth it. That is something for possible later evaluation.
*/

int CCdel(int k, int n)
{ int h, i, m;
  if(Clowest[0]==0||k>nC) return 0;                  //Guard against null cases.
  h = Clowest[k+1]-Clowest[k]; if(h<=0) return 0;
  if(Clowest[k+1]-Clowest[k]-Emptyc[k]<0) return 0;  //Skip empty groups:Tendai
  Emptyc[k] += 1; m = Clowest[k+1]-Emptyc[k];        //Find the highest occupied index in group k i.e m  
  if(n==m||m==0)                                     //If n is not the highest indexed individual corrected from n!=m on 06/10/16
    {A[n].id=0; clear_times(n);}                     //Otherwise clear n's ID and clear all event
  else{Transfer(n, m); clear_times(m); A[m].id=0;}   //tranfer m into deleted position and update index times
  nA -= 1; return 1;                                 //Return with success.
}

/*Added by Tendai
4.Function that clears all the saved times when an individual is deleted

ENTRY: 'n' indexes the individual being deleted. 
       'MaxT' size of t array

EXIT:  'A[n].t' All times are set to zero
       
 */
int clear_times(int n)
{ int i;
  for(i=0; i<MaxT; i++) A[n].t[i] = 0;
return 1;
}

/*------------------------------------------------------------------------------
5. GROUP SIZE

ENTRY: 'k' contains the group to be sampled.
       'nC' contains the number of groups.
       'Clowest' indexes the first individual in each group.
       'Emptyc[k]' contains the number of empty cells at the end of the group.
       
EXIT:  'CCgroup_size' calculates and returns the size of group k. If zero, the group is empty.
*/

int CCgroup_size(int k)
{int h;
  if(Clowest[0]==0||k>=nC) { return 0;}         //Guard against null cases.
  h = Clowest[k+1]-Clowest[k]-Emptyc[k];     //Determine how many occupy the group  
return h;
}


/* CLARENCE LEHMAN AND ADRIENNE KEEN, JUNE 2011.

Both authors contributed equally to this algorithm. It began in May 2010 with an
Order-1 method by AK for handling two groups. An algorithm for any number of
groups was jointly conceived in February 2011 and first coded and tested then.
It was Order-1 on the number of individuals, which was the primary design goal.
Selection had a small coefficient but addition and deletion had a larger one. In
June 2011, both authors jointly and simultaneously realized how to reduce the
coefficient on addition and deletion to a very small value, which also made it
effectively Order-1 on the number of groups. Later in June, AK conceived a
simple way to efficiently incorporate the "sieve method" for variable
probabilities within groups. The founding idea for the algorithm was by AK and
CL coded the final version.

*/


