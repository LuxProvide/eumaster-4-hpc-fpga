#!/bin/bash -l
#SBATCH --nodes=5
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=02:00:00
#SBATCH --partition=fpga
#SBATCH --account=lxp
#SBATCH --qos=default

module load env/staging/2023.1
#module load intel-compilers
module load intel-oneapi
module load 520nmx
module load jemalloc
export I_MPI_OFI_PROVIDER=verbs
export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
EXE="./fpga_image/mpi_fpga_pi.fpga"
srun --mpi=pspmi --export=ALL,LD_PRELOAD=${JEMALLOC_PRELOAD} $EXE



