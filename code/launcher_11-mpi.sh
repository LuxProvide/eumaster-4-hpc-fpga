#!/bin/bash -l
#SBATCH --chdir=/project/home/lxp/ekieffer/Training/epicure-hpc-fpga/code/11-mpi                     # 
#SBATCH --nodes=1                          # number of nodes
#SBATCH --ntasks=1                         # number of tasks
#SBATCH --cpus-per-task=128                # number of cores per task
#SBATCH --time=24:00:00                    # time (HH:MM:SS)
#SBATCH --account=lxp                      # project account
#SBATCH --partition=fpga                   # partition
#SBATCH --qos=default                      # QOS

module --force purge
module load env/staging/2023.1
module load CMake
module load intel-oneapi/2024.1.0
module load 520nmx/20.4

echo "Create building directory"
mkdir -p build && find build -mindepth 1 -delete && cd build
echo "Building fpga image"
cmake -DUSER_FPGA_FLAGS="-cxx=$(mpiicpx -show) -Xsfast-compile -Xsparallel=128" .. && make VERBOSE=3 fpga
