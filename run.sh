#!/bin/sh
# This file is called ~/psa.sh
#SBATCH --error=jobname_%a.err   # Replace jobname with the name you want to give to your error file, %A will add the job number and %a the array number 
#SBATCH --output=jobname_%a.out   # Replace jobname with the name you want to give to your output file; everything that shows up in your R console, will be printed in here
#SBATCH --nodes=5#50
#SBATCH --ntasks-per-node=12
echo "Nodelist: $SLURM_NODELIST"
module load Python/Python-2.7.5
module load openmpi/openmi-1.6.4
mpirun --mca btl_sm_use_knem 0 -np 30 full_mpi_code currentrun=0 fnumber=10 interv=2150
mpirun --mca btl_sm_use_knem 0 -np 30 full_mpi_code currentrun=0 fnumber=11 interv=2150
mpirun --mca btl_sm_use_knem 0 -np 30 full_mpi_code currentrun=0 fnumber=12 interv=2150
mpirun --mca btl_sm_use_knem 0 -np 30 full_mpi_code currentrun=0 fnumber=13 interv=2150
mpirun --mca btl_sm_use_knem 0 -np 30 full_mpi_code currentrun=0 fnumber=14 interv=2150
