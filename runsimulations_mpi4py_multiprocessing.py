#######################################################################

#######################################################################
#from multiprocessing import cpu_count
import traceback
from numpy import *
from subprocess import call
import numpy as np
import multiprocessing
import random
from math import ceil, floor,isnan
import csv,sys
from scipy.optimize import leastsq
import scipy.stats as ss
from copy import deepcopy
from time import *
import time as time
from copy import copy
import itertools as IT
from functools import partial

################################
#Wrapper define function to be run
################################

mpi=False

n_samples=4 #number of simulations
pool_size = (multiprocessing.cpu_count())
if pool_size>n_samples:
    pool_size = n_samples
    
if mpi:
    from mpi4py import MPI
    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    size = comm.Get_size()
    
    numberofcases=Nsamples=size
    #else:
    numberofcases=Nsamples=int(n_samples)
	
    #if rank==0:
    comm.bcast(numberofcases,root=0)
    comm.bcast(mpi,root=0)
else:
    numberofcases=Nsamples=int(n_samples)                      
    

#xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
def do_calculation(case):
    print 'in do_calculation'
    write_reports=1
    if write_reports>0:
        old = sys.stdout
        filename='reports/reportfile'+str(case)+'.dat'
        sys.stdout = open(filename,'w')
        old.write(`sys.stdout`)        
    r=case*1
    j=0
    #for cluster exclude .exe        
    call(["./Main_desktop.exe", "fnumber=1"+str(j),\
          "my_id_0="+str(r),"randseq="+str(r)])
    print "finished randseed=",r,"interv=",j
    if write_reports>0:
        sys.stdout.close()
        sys.stdout = old
        sys.stdout
        
    return
def do_calculation_wrapped(args):
    print 'do_calculation_wrapped'
    
    try:
        return do_calculation(*args)
    except:
        print('%s' % (traceback.format_exc()))   
###############################################
#set up parallel system
###############################################
start_time = time.time()

cases = list(range(numberofcases))

def start_process():    
    print 'Starting', multiprocessing.current_process().name
def setup_multiprocess(cases1,mpi):
    print 'in setup_multiprocess'
    cases=[] # create tuples to allow multi args for multiprocessing
    for i in cases1:
        cases.append((i,))
    #print cases
    if mpi:        
        import mpi4py_map    
        mpi4py_map.map(do_calculation_wrapped,cases) #provide parameter values for function f and store any results        
        ##############        
    else:
        print 'Sending out :', len(cases), 'Jobs to', pool_size,' CPUs using multiprocessing'
        pool = multiprocessing.Pool(processes=pool_size,
                                    initializer=start_process)        
        pool.map(do_calculation_wrapped, cases)
        pool.close() # no more tasks
        pool.join()  # wrap up current tasks
        #take note of NULL values from failed simulation
        ##############

    return 
       

###############################################
        
###############################################
def solver(n_samples,mpi):
    print 'in solver'
    if mpi:
        call(["sbatch","--nodelist=smedcla[10,14]","run.sh"])#sbatch --nodelist=smedcla[10-14,16-21,23-24,58-60,64-67,73-81]  run.sh
    else:
        cases = list(range(n_samples))
        setup_multiprocess(cases,mpi)
    
    return

def model_summary(n_samples): #summarize results 
    print 'summrizing model results'
    ylabel=["Year","Total","Deaths","Births","UK born","Non-UK born"]
    basevals = None
    basevals_b = None
    count=0
    i=0
    for ran in range(0,n_samples):
        files=[]
        files+=["summary_1"+str(i)+str(ran)+".txt" ] ##list of all base and interv file for each random seed
        filename = files[0]
       
        basearr = np.genfromtxt(filename, dtype=None, delimiter='\t')
        if basearr[0][1]>0: #guard against failed simulations
            count+=1
            if basevals_b is None:
                basevals_b = basearr
            else:
                basevals_b += basearr

    meanbasevals_b = basevals_b / count   #mean over all simulations  

    with open('mean_valiables_base.csv', 'wb') as fout:
        writer = csv.writer(fout, delimiter=',', lineterminator='\n')
        writer.writerow(ylabel)
        for x in meanbasevals_b:
            row=[]
            for y in x:
                 row.append(y)            
            writer.writerow(row)
    population_size=meanbasevals_b[0][ylabel.index("Total")]
    return array([population_size])

#########################################
if __name__ == '__main__':
    ##Initial run to establish starting point
    solver(n_samples,mpi)
    soln=model_summary(n_samples)
    print 'soln',soln
    end_time = time.time()
    print("Elapsed time was %g minutess" % ((end_time - start_time)/60.))
 

