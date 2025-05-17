#!/bin/bash -l


# Test example provided by LXP

EXAMPLES=( E01-first-compilation
	   E02-enqueue-kernel
	   E03-transfer-data
	   E04-buffer-transfer
	   E05-dot-product
	   E06-mat-transpose
	   E07-shared-memory
	   E08-pipe
	   E09-multi-device
	   E10-convolution)




for code in "${EXAMPLES[@]}"; do

DIR=$(find $PWD -name "$code")
LAUNCHER="launcher_${code}.sh"
cat << EOF > ${LAUNCHER}
#!/bin/bash -l
#SBATCH --chdir=${DIR}                     # 
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
cmake -DBUILD=SOL -DUSER_FPGA_FLAGS="-Xsfast-compile -Xsparallel=128" .. && make VERBOSE=3 fpga
EOF
chmod +x ${LAUNCHER}
cat ${LAUNCHER}
sbatch ${LAUNCHER}
done



