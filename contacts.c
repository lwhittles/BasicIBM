/*===============================================================================
UNIQUE INDIVIDUAL IDENTIFIERS
Many applications benefit from a unique code for each individual that is never
reassigned during the simulation. These routines assign such identification
codes and provide access to the records of individuals based on their
identification codes.
The database of all active individuals is array 'A[n]'. The ordering of that
database is arbitrary, but for rapid access typically records are grouped
together based on the categories their individuals occupy. A hashcode table
'H[i]' is managed by these routines to provide rapid access to the location of a
data record based on the individual's unique identification code, wherever that
record is in array 'A[n]'.
The hash code table 'H[i]' can be any size, but when it is proportional in
number of entries to 'A[n]', then it provides order-one access to individual
records. That is, it will take the same number of probes to locate an individual
whether there are 100 individuals in the database or 100 million. In the common
case where the two arrays are equal in number of entries and the hashing
algorithm distributes things reasonably randomly, then an individual whose
record is in the array will be located about 6/10 of the time in a single probe
<m>\big(1/(e-1)\big)</m>, about 3/10 of the time in two probes, and 1/10 of the
time in three or more probes. The probability of multiple probes decreases
rapidly, with chance less than 1/1000 of five or more probes.
Hash coding is among the earliest order-one algorithms discovered [Ref] and many
variations are now known. The variation used here, with a separate table, works
well when the hash coding routines cannot assign the location of the records,
and when the records move around during this simulation. Moreover, with a serial
assignments of integers as unique identification codes, a complex randomization
method for hashcoding is not important and a simple modulus division can
suffice, as in the algorithms below.

*/
int H[ID];                     //Hash codes

/*
Assign unique identification
This routine assigns a new individual identifier that has not been used before.
The present routine assigns in numeric order, but that property should not be
relied upon.
ENTRY: No significant conditions.
EXIT: 'IDnew' returns the a new individual identifier.*/
static int id;

int IDnew() { return ++id; }
/*-------------------------------------------------------------------------------
Locate individual known to be present
Given an individual identification, this routine locates the individual in the
database array. If the individual is not present, a fatal error is recorded and
the program terminates.
ENTRY: 'id' contains the individual identifier.
UNIQUE IDENTIFIERS AND CONTACT LISTS
EXIT: 'L' returns the index of the individual in the database array.

*/
int Li(int id)
{int n;
  for (n=H[id%ID]; ; n=A[n].idn)
 { if(n==0) Error(999.);
 if(id==A[n].id) return n; } }
/*NOTE: Perhaps should include a counter to guard against accidental infinite
  loops in the list of links, in this routine and the one below.*/

/*-------------------------------------------------------------------------------
Locate individual if present
Given an individual identification, this routine locates the individual in the
database array.
ENTRY: 'id' contains the individual identifier.
EXIT: 'Lz' returns the index of the individual in the database array, or zero
if the individual is no longer in the database.

*/
int Lz(int id)
{ int n;
  for(n=H[id%ID]; ; n=A[n].idn)
    { if(n==0) return 0;
      if(id==A[n].id) return n; } 
}

/*-------------------------------------------------------------------------------
Attach individual
Given a database location containing an individual identification, this routine
adds the individual's location to the hash table, attaching the individual to
the database.
ENTRY: 'n' contains the location in the database.
'A[n].id' contains the individual identifier, which is not yet linked
into the hash table.
EXIT: The individual is linked.

*/
void AttachH(int n)
{ int id=A[n].id;
 if(Lz(id)) Error(997.);
 A[n].idn=H[id%ID]; H[id%ID]=n;
}

/*-------------------------------------------------------------------------------
Detach individual
Given a database location containing an individual identification, this routine
removes the individual's location from the hash table, detaching the individual
from the database.
ENTRY: 'n' contains the location in the database.
'A[n].id' contains the individual identifier.
EXIT: The individual is unlinked and may be moved or deleted.

*/
void DetachH(int n)
{ int id, nprev;
 id=A[n].id; n=H[id%ID]; if(n==0) Error(998.);
 if(id==A[n].id) { H[id%ID]=A[n].idn; A[n].idn=0; return; }

 for(nprev=n, n=A[n].idn; n; nprev=n, n=A[n].idn)
   if(id==A[n].id) { A[nprev].idn=A[n].idn; A[n].idn=0; return; }
 //Error(997.); //what is the purpose for this error?
}

/*UNIQUE IDENTIFIERS AND CONTACT LISTS
-------------------------------------------------------------------------------
Hashing overhead
ENTRY: No significant conditions.
EXIT: 'HashProfile' returns the size of the hash table
*/
int HashProfile() { return sizeof(H); }

/*UNIQUE IDENTIFIERS AND CONTACT LISTS
===============================================================================
BIND RECORDS TOGETHER
This set of routines is to bind individual records together for interconnections
of any variety---for example to efficiently track which individuals were
infected by a given individual in an epidemiological microscale model.
The routines use a global array of structures for binding, 'B[n]', which
augments the main data structure of active individuals, 'A[n]'. Row 'B[0]' has
a special use, but in all subsequent rows, the first integer is a linkage to
other rows in 'B[n]' and the second integer is a unique individual identifier.
Any other information, such as a timestamp and the type of binding, can also be
included in 'B[n]'.
The array can carry hundreds of millions of entries, but for illustration
consider an example of only eight entries, or rows, initially set up with all
rows available for use and with no timestamps applied, as follows.
LINK ID TIME
B[0] 1 7 0.000000000
B[1] 2 0 0.000000000
B[2] 3 0 0.000000000
B[3] 4 0 0.000000000
B[4] 5 0 0.000000000
B[5] 6 0 0.000000000
B[6] 7 0 0.000000000
B[7] 0 0 0.000000000
The first row, 'B[0]', has a special use with two integers, 1 and 7. Those are
indexes into 'B[n]' of the first row in the list of available rows and the last
row, respectively. (The list of available rows is called the "available list"
for short). Therefore, in this example the first available row for use is 'B[1]'
and the last available row is 'B[7]'. In between, the first integer in each row
is the number of the next row in turn, terminating with 0 when there are no more
rows.
In addition, in the records of 'A[n]', the database for individuals can carry
any number of binding lists, each managed buy a row of two integers. As in the
first row above, the left integer indexes the first element in the list and the
right integer indexes the last element. Supposing that 'b[0]' lists who
contacted this individual, and 'c[1]' lists who this individual contacted, then
at the outset these two rows would be null, as follows.
b[0] 0 0
b[1] 0 0
Now suppose that the individual in question is contacted by individual 10. Then
row 1 in 'B' would be marked with the individual identifier 10 and row 2 would
become the next row in the available list. 'B' would be updated as follows.
LINK ID TIME
B[0] 2 7 0.000000000 <--
B[1] 0 10 0.123762388 <--
B[2] 3 0 0.000000000
B[3] 4 0 0.000000000
B[4] 5 0 0.000000000
B[5] 6 0 0.000000000
B[6] 7 0 0.000000000
B[7] 0 0 0.000000000
Concurrently, the individual's data structure 'b[0]' would be updated to
indicate that that individual was contacted by the individual recorded in row
'B[1]'. There are two 1s in the data structure, indicating that the first
element is also the last.
b[0] 1 1 <--
UNIQUE IDENTIFIERS AND CONTACT LISTS
b[1] 0 0
Next suppose that the individual in question is contacted by individual 11. Then
row 2 would be marked with the individual identifier 11 and row 3 would become
the next row in the available list. 'B' would be updated as follows.
LINK ID TIME
B[0] 3 7 0.000000000 <--
B[1] 2 10 0.123762388 <--
B[2] 0 11 0.248300635 <--
B[3] 4 0 0.000000000
B[4] 5 0 0.000000000
B[5] 6 0 0.000000000
B[6] 7 0 0.000000000
B[7] 0 0 0.000000000
Similarly, the individual's data structure 'b[0]' would be updated to show that
role two is the last in the list, as follows.
b[0] 1 2 <--
b[1] 0 0
Now suppose that the individual in question contacts three other individuals,
12, 13, and 14. Row 3, 4, and 5 would be used to record those individuals,
resulting in row 6 being the next element in the available list.
LINK ID TIME
B[0] 6 7 0.000000000 <--
B[1] 2 10 0.123762388
B[2] 0 11 0.248300635
B[3] 4 12 0.373780248 <--
B[4] 5 13 0.499083751 <--
B[5] 0 14 0.624200552 <--
B[6] 7 0 0.000000000
B[7] 0 0 0.000000000
Now the individual's data structure 'b[1]' would show row 3 as the first
individual contacted and row 5 as the last.
b[0] 1 2
b[1] 3 5 <--
Finally for illustration, suppose that the entire list 'b[0]' is discarded. It
would be placed at the top of the available list, with its last entry indexing
the former first element of the available list, and its first entry becoming the
first element on the new available list, as follows.
LINK ID TIME
B[0] 1 7 0.000000000 <--
B[1] 2 10 0.123762388 <--
B[2] 6 11 0.248300635 <--
B[3] 4 12 0.373780248
B[4] 5 13 0.499083751
B[5] 0 14 0.624200552
B[6] 7 0 0.000000000
B[7] 0 0 0.000000000
The data structure 'b[0]' would be back to zeros, indicating that it is empty.
b[0] 0 0 <--
b[1] 3 5
The routines in this module implement algorithms to manage such lists.
*/
//#include "common.h"
//#include "protos.h"

/*UNIQUE IDENTIFIERS AND CONTACT LISTS*/
#define BN INDIV*10 ////Number of contacts for testing.

struct Bind //Structure of records in array 'B',
{ int k[2]; //Link and individual identifer.
 dec time; //Creation time (optinal).
 char spec[4]; }; //Codes describing this record.

struct Bind B[BN]; //Lists of all bindings.
extern dec t; //Simulated time.
/*------------------------------------------------------------------------------
Display set of bindings
This routine displays a list one element at a time, in order. It is called
multiple times to display the full list. This routine is an example for other
related routines that note the contents of lists.
ENTRY: 'id' contains the identification number to be displayed. If negative, a
new list is about to arrive, and if zero, the list is complete.
EXIT: The list element has been displayed.

*/
void BindList(int id)
{ if (id<0) printf("Bound:");
 else if(id>0) printf(" %d\n", id);
 else printf("\n"); }

/*------------------------------------------------------------------------------
Initialize data structure
This routine initialises the data structure used to bind records together. It must
be called before any operations with the data structure begin.
When completed, the first integer in the data structure, 'B[0].k[0]', indexes
the first element in the list of available elements. The second integer,
'B[0].k[1]', indexes the last element in that list. In each later element, the
first integer, 'B[i].k[0]' with 'i' greater than 0, indexes the next element in
the list, with the last element of the list having 0 in that location. The
second integer of each element, 'B[i].k[1]', is insignificant immediately after
initialization.
ENTRY: No significant additions.
EXIT: The data structure 'B' has been initialized.
*/
void BindInit()
{ int i;
  for(i=0; i<BN-1; i++) B[i].k[0]=i+1; //Sequence the empty list in order.
 B[0].k[1]=BN-1; } //Record the last element.

/*------------------------------------------------------------------------------
Bind a new record, chronological order
This routine adds a new element to the end of any binding list, creating a list
in chronological order. It removes one row from the available list in data
structure 'B' and adds it to a specified binding list. The present version of
the routine tracks the individual identification number and the simulated time,
but can include other information. The present version of the routine also halts
if the total number of contacts exceeds a pre-specified limit, but the data
structure 'B' could be reallocated whenever more memory is needed.
The new binding is added to the list in an order-one operation.
ENTRY: 'b' is the list to be updated, with 'b[0]' indexing the list's first
row and 'b[1]' indexing its last.
'id' is the individual identification code to be added, which may or may
not already be in the list. Duplicates in the list are allowed by this
routine.
't' is the current simulated time.
Data structure 'B' is initialized.
EXIT: The new binding is at the end of list 'b', and 'b[1]' records its index
into array 'B'.
*/
void Bind(int b[2], int id)
{
int j=B[0].k[0]; if(j==0) Error(980.); //Locate the first available entry,
 B[0].k[0]=B[j].k[0]; //if any is available, and remove

 if(B[0].k[0]==0) B[0].k[1]=0; //it from the available list.

 B[j].time=t; //Prepare the new entry and link it
 B[j].k[0]=0; B[j].k[1]=id; //to the end of the list.

 if(b[1]==0) b[1]=j;
 else B[b[1]].k[0]=j;

 b[1]=j; if(b[0]==0) b[0]=j; } //Record it in the database record.

/*----------------------------------------------------------------------------
Bind a new record, reverse chronological order
This routine adds a new row to the beginning of any binding list, creating a
list in reverse chronological order. Other than the order, it operates the same
as 'Bind'.
The new binding is added to the list in an order-one operation.
ENTRY: 'b' is the list to be updated, with 'b[0]' indexing the list's first
element and 'b[1]' indexing its last.
'id' is the individual identification code to be added, which may or may
not already be in the list. Duplicates in the list are allowed by this
routine.
't' is the current simulated time.
Data structure 'B' is initialized.
EXIT: The new binding is at the beginning of list 'b', and 'b[0]' records its index
into array 'B'.
*/
void BindReverse(int b[2], int id)
{ int j=B[0].k[0]; if(j==0) Error(980.); //Locate the first available entry,
 B[0].k[0]=B[j].k[0]; //if any is available, and remove
 if(B[0].k[0]==0) B[0].k[1]=0; //it from the available list.

 B[j].time=t; //Prepare the new entry and link it
 B[j].k[0]=b[0]; B[j].k[1]=id; //to the front of the list.
 b[0]=j; if(b[1]==0) b[1]=j; } //Record it in the database record.

/*-----------------------------------------------------------------------------
Discard a list of bindings, releasing all elements
This routine removes a binding list, making all rows of the list available for
subsequent use elsewhere in the system.
The data contents of the rows are not cleared after being released, that being
left until they are reused. Also, elements are put back at the front of the
available list to keep memory usage more localized and hence somewhat faster in
memory-cached machines.
ENTRY: 'b' defines the list to be deleted.
Data structure 'B' is initialized.
EXIT: The list defined by 'b' on entry has been discarded.
Data structure 'B' is updated.
*/
void BindDelete(int b[2])
{ if(b[0]==0) return; ////Ignore empty lists.
 B[b[1]].k[0]=B[0].k[0]; B[0].k[0]=b[0]; //Attach the list being discarded
 if(B[0].k[1]==0) B[0].k[1]=b[1]; //to the available list.
 b[0]=b[1]=0;   //Clear pointers in the database.
}
/*------------------------------------------------------------------------------
Scan through the list noting each individual
This is a service routine that merely scans through a list and notifies a
specified routine of the contents the list.
ENTRY: 'b' defines the list to be scanned.
'Note' identifies the subroutine to be passed the contents of each list
element as an integer. If negative, a new list is beginning. If zero,
the present list he is completed.
Data structure 'B' is initialized.
*/
void BindNote(int b[2], void Note(int id))
{ int i; 
 Note(-1); //Mark the beginning.
 for(i=b[0]; i>0; i=B[i].k[0]) ////Pass all list elements in order.
 Note(B[i].k[1]);
 Note(0); } ////Mark the end.

/*------------------------------------------------------------------------------
Determine whether a specified individual is in the list
This routine scans a list to see how many times, if any, a specified individual
is present.
ENTRY: 'b' is the list to be checked.
'id' is the individual identification code to be checked.
Data structure 'B' is initialized.
EXIT: 'BindCount' tells how many times the specified individual appears in
the list.
*//*
int BindCount(int b[2], int id)
{ int k=0,i;

 for(i=b[0]; i>0; i=B[i].k[0])
 if(B[i].k[1]==id) k+=1;

 return k; }*/
//returns the total number of contacts within a specified period of time
int BindCount(int b[2], dec tn)
{ int z=0,i;
 for(i=b[0]; i>0; i=B[i].k[0])
 if((t-B[i].time)<tn) z+=1;
 return z; }

//returns an array with contacts within a specified period of time
int *BindTrace(int b[2], dec tn)
{ int j=b[0]; if(j==0) Error(980.); //Locate the first available entry
int z=BindCount(b,tn);
static int c[1000];
int i=0;
while(j>0){  
//if((t-B[j].time)<tn) printf("Time=%f  id=%d j=%d\n",B[j].time,B[j].k[1],j);
c[i]=B[j].k[1];
j=B[j].k[0];
i+=1;
}
//printf("\n");
return c;
}

/*------------------------------------------------------------------------------
Binding overhead
ENTRY: No significant conditions.
EXIT: 'BindProfile' returns the size of the binding tables.

*/
int BindProfile() { return sizeof(B); }



/*Laboratory Notebook
File set 57B55AD7
2016/08/17 11:51 (Wednesday)
Clarence L. Lehman*/
