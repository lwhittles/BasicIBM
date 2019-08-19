#######################################################################

#######################################################################
import traceback
from numpy import *
from subprocess import call
import multiprocessing
import csv,sys
import time as time

################################
#Wrapper define function to be run
################################

mpi=False # set to True if using a cluster with MPI otherwise False to use python's multiprocessing

n_jobs=4 #number of simulations
pool_size = (multiprocessing.cpu_count()) #get the number of available CPUs
if pool_size>n_jobs:  #only use maximum necessary CPUs depending on number of jobs/runs
    pool_size = n_jobs
    
if mpi:
    from mpi4py import MPI
    comm = MPI.COMM_WORLD #communicator so every process can communicate with every other
    rank = comm.Get_rank() #
    size = comm.Get_size()

    #one to many communication
    comm.bcast(n_jobs,root=0)
    comm.bcast(mpi,root=0)
else:
    pass

#xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
#This function allows the compiled C code to be run
#in python and take a log of the simulation progress
# or errors
def Run_1_simulation(job):
    print 'in Run_1_simulation'
    write_reports=1
    if write_reports>0: #write output to file instead of terminal
        old = sys.stdout
        filename='reports/reportfile'+str(job)+'.dat'#name and open log file for simulation progress and errors
        sys.stdout = open(filename,'w')
        old.write(`sys.stdout`)
    #initialize external function options/parameters
    r=int(job)#current run/job
    j=0 #change random number seed/job ID
    #call the complied C code
    #for cluster exclude .exe        
    call(["./Main_desktop.exe", "fnumber=1"+str(j),\
          "my_id_0="+str(r),"randseq="+str(r)])
    print "finished randseed=",r,"interv=",j
    #close log files
    if write_reports>0:
        sys.stdout.close()
        sys.stdout = old
        sys.stdout
    return
##########################################
#create a wrapper function
def Run_1_simulation_wrapped(args):
    print 'Run_1_simulation_wrapped'
    try:
        return Run_1_simulation(*args)
    except:
        print('%s' % (traceback.format_exc()))   
###############################################
#set up parallel system
###############################################
jobs_list = list(range(n_jobs))#create a list of n simulations

################################################
#This function initializes multiprocessing    
def start_process():
    print 'Starting', multiprocessing.current_process().name
#################################################
#This function sets up the parallel processes     
def setup_parallel_process(jobs_list,mpi):
    print 'in setup_parallel_process'
    #create tuples to allow multiple args for multiprocessing
    jobs_tuple=[]
    for i in jobs_list:
        jobs_tuple.append((i,))
    
    ####################
    if mpi: #run using MPI        
        import mpi4py_map    
        mpi4py_map.map(Run_1_simulation_wrapped,jobs_tuple) #provide parameter values for function f and store any results        
        ##############        
    else:#run using python multiprocessing function
        print 'Sending out :', len(jobs_tuple), 'Jobs to', pool_size,' CPUs using multiprocessing'
        pool = multiprocessing.Pool(processes=pool_size,
                                    initializer=start_process)        
        pool.map(Run_1_simulation_wrapped, jobs_tuple) #map jobs to CPUs
        pool.close() # no more tasks
        pool.join()  # wrap up current tasks
        #take note of NULL values are failed simulation
    return 
       
###############################################
#This function runs and stores n simulations
def Run_n_simulations(n_jobs,mpi):
    print 'in Run_n_simulations'
    if mpi:
        call(["sbatch","--nodelist=smedcla[10,14]","run.sh"])
    else:
        jobs_list = list(range(n_jobs))
        setup_parallel_process(jobs_list,mpi)    
    return

def summarize_n_simulations(n_jobs): #summarize results 
    print 'summarizing model results'
    #initialise summary array
    ylabel=["Year","Total","Deaths","Births","UK born","Non-UK born"]
    basevals = None
    basevals_b = None
    count=0
    i=0
    for ran in range(0,n_jobs):#random seed used in nameing result files
        ##list of all base and interv file for each random seed
        files=[]
        files+=["summary_1"+str(i)+str(ran)+".txt" ] 
        filename = files[0]
        #read individual results files
        basearr = genfromtxt(filename, dtype=None, delimiter='\t')
        if basearr[0][1]>0: #guard against failed simulations
            count+=1
            if basevals_b is None:
                basevals_b = basearr
            else:
                basevals_b += basearr
                
    #Calculate the mean over all simulations  
    meanbasevals_b = basevals_b / count   
    #Write up the summary
    with open('mean_valiables_base.csv', 'wb') as fout:
        writer = csv.writer(fout, delimiter=',', lineterminator='\n')
        writer.writerow(ylabel)
        for x in meanbasevals_b:
            row=[]
            for y in x:
                 row.append(y)            
            writer.writerow(row)
    #returning some of the summary results is useful is you are going to
    #do some model fitting. Note this version does not do any fitting.
    population_size=meanbasevals_b[0][ylabel.index("Total")]
    return array([population_size])

#########################################
if __name__ == '__main__':
    #record start time
    start_time = time.time()#take not of similation start time
    #run the n parallel jobs
    Run_n_simulations(n_jobs,mpi)
    #summarize results
    soln=summarize_n_simulations(n_jobs)
    #record end time
    end_time = time.time()
    #print total duration of simulation
    print("Elapsed time was %g minutess" % ((end_time - start_time)/60.))
 

