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




for (( i=0; i<"${#EXAMPLES[@]}"; i++ )); do
if [[ -f "${EXAMPLES[i]}/src-solution/${EXAMPLES[i]}.cpp" ]]; then
rm "${EXAMPLES[i]}/src-solution/${EXAMPLES[i]}.cpp"
fi
code=${EXAMPLES[i]}
EXTRA_MODULES=""
EXTRA_FLAGS=""
EXTRA_PARAMS=""
if [[ "${code}" == "E10-convolution" ]];then
	EXTRA_MODULES="OpenCV"
	EXTRA_FLAGS="-DUSER_FLAGS=\"-lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc\""
	EXTRA_PARAMS="../original_image.png"
fi


DIR=$(find $PWD -name "$code")
LAUNCHER="launcher_${code}.sh"
cat << EOF > ${LAUNCHER}
#!/bin/bash -l
#SBATCH --chdir=$(basename $DIR)           # 
#SBATCH --nodes=1                          # number of nodes
#SBATCH --ntasks=1                         # number of tasks
#SBATCH --cpus-per-task=128                # number of cores per task
#SBATCH --time=24:00:00                    # time (HH:MM:SS)
#SBATCH --account=lxp                      # project account
#SBATCH --partition=fpga                   # partition
#SBATCH --qos=default                      # QOS

module --force purge
module load env/staging/2023.1
module load CMake ${EXTRA_MODULES}
module load intel-oneapi/2024.1.0
module load 520nmx/20.4
module load jemalloc

# echo "Create building directory"
# mkdir -p build_ex && find build_ex -mindepth 1 -delete && cd build_ex
# echo "Building"
# Uncomment to run the exercice 
# cmake .. -DBUILD=EX ${EXTRA_FLAGS} && make fpga_emu
# ./$(basename $DIR).fpga_emu ${EXTRA_PARAMS}
# echo "Create building directory"
# mkdir -p build_sol && find build_sol -mindepth 1 -delete && cd build_sol
# echo "Building"
# Uncomment to run the solution
# cmake .. -DPASSWORD="XXXXXX" -DBUILD=SOL ${EXTRA_FLAGS} && make fpga_emu
# ./$(basename $DIR).fpga_emu ${EXTRA_PARAMS}
# Uncomment to execute the FPGA image
# cd fpga_image 
# export JEMALLOC_PRELOAD=\$(jemalloc-config --libdir)/libjemalloc.so.\$(jemalloc-config --revision)
# LD_PRELOAD=\${JEMALLOC_PRELOAD} ./$(basename $DIR).fpga ${EXTRA_PARAMS}
EOF
chmod +x ${LAUNCHER}
done



