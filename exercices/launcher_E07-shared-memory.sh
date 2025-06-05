#!/bin/bash -l
#SBATCH --chdir=E07-shared-memory           # 
#SBATCH --nodes=1                          # number of nodes
#SBATCH --ntasks=1                         # number of tasks
#SBATCH --cpus-per-task=128                # number of cores per task
#SBATCH --time=24:00:00                    # time (HH:MM:SS)
#SBATCH --account=p200911                      # project account
#SBATCH --qos=default                      # QOS
#Uncomment fpga or cpu
##SBATCH --reservation=eumaster4hpc-cpu
##SBATCH --partition=cpu
#SBATCH --reservation=eumaster4hpc-fpga
#SBATCH --partition=fpga

module --force purge
module load env/staging/2023.1
module load CMake 
module load intel-oneapi/2024.1.0
module load 520nmx/20.4
module load jemalloc

# echo "Create building directory"
# mkdir -p build_ex && find build_ex -mindepth 1 -delete && cd build_ex
# echo "Building"
# Uncomment to run the exercice 
# cmake .. -DBUILD=EX  && make fpga_emu
# ./E07-shared-memory.fpga_emu 
# echo "Create building directory"
# mkdir -p build_sol && find build_sol -mindepth 1 -delete && cd build_sol
# echo "Building"
# Uncomment to run the solution
# cmake .. -DPASSWORD="XXXXXX" -DBUILD=SOL  && make fpga_emu
# ./E07-shared-memory.fpga_emu 
# Uncomment to execute the FPGA image
# cd fpga_image 
# export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
# LD_PRELOAD=${JEMALLOC_PRELOAD} ./E07-shared-memory.fpga 
