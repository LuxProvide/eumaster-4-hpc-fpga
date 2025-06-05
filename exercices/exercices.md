# Hands-on session


 * This hands-on session is designed to provide you with practical, interactive experience. It allows you to apply theoretical knowledge directly by working with real tools or code, thereby deepening your understanding, building practical skills, and boosting confidence in the subject matter.


* Using the resources seen this morning, i.e.,
    - [Compiling SYCL programs](compile.md)
    - [Developing SYCL programs](writing.md)
    - [Reporting & Profiling SYCL programs](writing.md)
    - [Optimizing SYCL programs](optimization.md)  

## Access to MeluXina

- This workshop **IS NOT** a session on how to connect to MeluXina

- You **SHOULD ALL** have access to MeluXina before starting the workshop 

- Please note that if you did not register in time, we **WILL NOT** onboard you during the workshop

- You **SHOULD** all have received a Welcome Email providing all the necessary guidelines to connect MeluXina

- If you did not setup your connection to MeluXina in time, please follow the [documentation](https://docs.lxp.lu/first-steps/connecting/) or contact the [service desk](https://servicedesk.lxp.lu/)

## Setup

- You need first to obtain an interactive job on the fpga partition and load a module
```bash
salloc -A p200911 --reservation=eumaster4hpc-cpu -t 00:30:00 -q short -p cpu -N1
# OR 
# salloc -A p200911 --reservation=eumaster4hpc-fpga -t 00:30:00 -q short -p fpga -N1
module load git-lfs
```
- Clone first the repository and enter the `exercices` folder
```bash
git lfs clone --depth 1 https://gitlab.lxp.lu/emmanuel.kieffer/eumaster-4-hpc-fpga.git
cd eumaster-4-hpc-fpga.git/exercices
```

!!! warning "Reservations"
    Please use only the two following reservations which have dedicated resources for this event:

    - `#SBATCH --reservation=eumaster4hpc-fpga` for accessing the 15 reserved FPGA nodes

    - `#SBATCH --reservation=eumaster4hpc-cpu` for accessing the 30 reserved CPU nodes

    While emulation can be done using both reservations, FPGA image provided to you can only be executed using the `eumaster4hpc-fpga` reservation.



## Exercices

### E01-first-compilation
!!! example "First compilation"
    === "Instruction"
        - Go to `./exercices/E01-first-compilation`
        - Add the minimun required code in `src/01-first-compilation.cpp` to select the FPGA device and create a sycl queue
        - Execute your code with the FPGA emulator
        - Execute the FPGA image in the `fpga_image` folder
    === "Code description"
         ```cpp linenums="1"
         --8<-- "./exercices/E01-first-compilation/src/E01-first-compilation.cpp"
         ```
    === "Emulation (test + solution)"
        - To execute your code, uncomment the following lines in `launcher_E01-first-compilation.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        echo "Building"
        cmake .. -DBUILD=EX && make fpga_emu
        ./E01-first-compilation.fpga_emu
        ```
        - To test the solution, uncomment the following lines in `launcher_E01-first-compilation.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        cmake .. -DPASSWORD="XXXXXX" -DBUILD=SOL  && make fpga_emu
        ./E01-first-compilation.fpga_emu
        ```
    === "Hardware (Solution)"
        - To run the fpga image, uncomment the following lines in `launcher_E01-first-compilation.sh`
        - Make sure that the header `#SBATCH -p fpga` is present
        ```bash
        export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
        cd fpga_image
        LD_PRELOAD=${JEMALLOC_PRELOAD} ./E01-first-compilation.fpga
        ```


### E02-enqueue-kernel

!!! example "Enqueuing a single task kernel"
    === "Instruction"
        - Go to `./exercices/E02-enqueue-kernel`
        - Add the minimun required code in `src/02-enqueue-kernel.cpp` to enqueue a single-task kernel. The body of the kernel should contain the following:
        ```bash
        PRINTF("Result1: Hello, World!\n");
        PRINTF("Result2: %%\n");
        PRINTF("Result3: %d\n", x);
        PRINTF("Result4: %u\n", 123);
        PRINTF("Result5: %.2f\n", y);
        PRINTF("Result6: print slash_n \\n \n");
        PRINTF("Result7: Long: %ld\n", 650000L);
        PRINTF("Result8: Preceding with blanks: %10d \n", 1977);                                                  
        PRINTF("Result9: Preceding with zeros: %010d \n", 1977);                                                  
        PRINTF("Result10: Some different radices: %d %x %o %#x %#o \n", 100,                                      
               100, 100, 100, 100);
        PRINTF("Result11: ABC%c\n", 'D');
        ```
        - Execute your code with the FPGA emulator
        - Execute the FPGA image in the `fpga_image` folder
    === "Code description"
         ```cpp linenums="1"
         --8<-- "./exercices/E02-enqueue-kernel/src/E02-enqueue-kernel.cpp"
         ```
    === "Emulation (test + solution)"
        - To execute your code, uncomment the following lines in `launcher_E02-enqueue-kernel.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        echo "Building"
        cmake .. -DBUILD=EX && make fpga_emu
        ./E02-enqueue-kernel.fpga_emu
        ```
        - To test the solution, uncomment the following lines in `launcher_E02-enqueue-kernel.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        cmake .. -DPASSWORD="XXXXXX" -DBUILD=SOL  && make fpga_emu
        ./E02-enqueue-kernel.fpga_emu
        ```
    === "Hardware (Solution)"
        - To run the fpga image, uncomment the following lines in `launcher_E02-enqueue-kernel.sh`
        - Make sure that the header `#SBATCH -p fpga` is present
        ```bash
        export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
        cd fpga_image
        LD_PRELOAD=${JEMALLOC_PRELOAD} ./E02-enqueue-kernel.fpga
        ```

### E03-transfer-data
!!! example "Transferring data to the device"
    === "Instruction"
        - Go to `./exercices/E03-transfer-data`
        - Transfer the data from `host_vec_a` and `host_vec_b` to the device by using the `memcpy` function
        - Execute your code with the FPGA emulator
        - Execute the FPGA image in the `fpga_image` folder
    === "Code description"
         ```cpp linenums="1"
         --8<-- "./exercices/E03-transfer-data/src/E03-transfer-data.cpp"
         ```
    === "Emulation (test + solution)"
        - To execute your code, uncomment the following lines in `launcher_E03-transfer-data.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        echo "Building"
        cmake .. -DBUILD=EX && make fpga_emu
        ./E03-transfer-data.fpga_emu
        ```
        - To test the solution, uncomment the following lines in `launcher_E03-transfer-data.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        cmake .. -DPASSWORD="XXXXXX" -DBUILD=SOL  && make fpga_emu
        ./E03-transfer-data.fpga_emu
        ```
    === "Hardware (Solution)"
        - To run the fpga image, uncomment the following lines in `launcher_E03-transfer-data.sh`
        - Make sure that the header `#SBATCH -p fpga` is present
        ```bash
        export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
        cd fpga_image
        LD_PRELOAD=${JEMALLOC_PRELOAD} ./E03-transfer-data.fpga
        ```

### E04-buffer-transfer
!!! example "Using buffers to transfer data to kernels"
    === "Instruction"
        - Go to `./exercices/E04-buffer-transfer`
        - Create the buffers and accessors for the two host array `host_vec_a` and `host_vec_b`
        - Execute your code with the FPGA emulator
        - Execute the FPGA image in the `fpga_image` folder
    === "Code description"
         ```cpp linenums="1"
         --8<-- "./exercices/E04-buffer-transfer/src/E04-buffer-transfer.cpp"
         ```
    === "Emulation (test + solution)"
        - To execute your code, uncomment the following lines in `launcher_E04-buffer-transfer.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        echo "Building"
        cmake .. -DBUILD=EX && make fpga_emu
        ./E04-buffer-transfer.fpga_emu
        ```
        - To test the solution, uncomment the following lines in `launcher_E04-buffer-transfer.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        cmake .. -DPASSWORD="XXXXXX" -DBUILD=SOL  && make fpga_emu
        ./E04-buffer-transfer.fpga_emu
        ```
    === "Hardware (Solution)"
        - To run the fpga image, uncomment the following lines in `launcher_E04-buffer-transfer.sh`
        - Make sure that the header `#SBATCH -p fpga` is present
        ```bash
        export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
        cd fpga_image
        LD_PRELOAD=${JEMALLOC_PRELOAD} ./E04-buffer-transfer.fpga
        ```

### E05-dot-product
!!! example "Dot-product kernel"
    === "Instruction"
        - Go to `./exercices/E05-dot-product`
        - Create a dot product and call it from the kernel to compute the dot-product between host_vec_a and host_vec_b
        - Execute your code with the FPGA emulator
        - Execute the FPGA image in the `fpga_image` folder
    === "Code description"
         ```cpp linenums="1"
         --8<-- "./exercices/E05-dot-product/src/E05-dot-product.cpp"
         ```
    === "Emulation (test + solution)"
        - To execute your code, uncomment the following lines in `launcher_E05-dot-product.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        echo "Building"
        cmake .. -DBUILD=EX && make fpga_emu
        ./E05-dot-product.fpga_emu
        ```
        - To test the solution, uncomment the following lines in `launcher_E05-dot-product.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        cmake .. -DPASSWORD="XXXXXX" -DBUILD=SOL  && make fpga_emu
        ./E05-dot-product.fpga_emu
        ```
    === "Hardware (Solution)"
        - To run the fpga image, uncomment the following lines in `launcher_E05-dot-product.sh`
        - Make sure that the header `#SBATCH -p fpga` is present
        ```bash
        export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
        cd fpga_image
        LD_PRELOAD=${JEMALLOC_PRELOAD} ./E05-dot-product.fpga
        ```
### E06-mat-transpose
!!! example "Matrix transpose kernel"
    === "Instruction"
        - Go to `./exercices/E06-mat-transpose`
        - Create a data-parallel kernel transposing a NxN matrix
        - Execute your code with the FPGA emulator
        - Execute the FPGA image in the `fpga_image` folder
    === "Code description"
         ```cpp linenums="1"
         --8<-- "./exercices/E06-mat-transpose/src/E06-mat-transpose.cpp"
         ```
    === "Emulation (test + solution)"
        - To execute your code, uncomment the following lines in `launcher_E06-mat-transpose.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        echo "Building"
        cmake .. -DBUILD=EX && make fpga_emu
        ./E06-mat-transpose.fpga_emu
        ```
        - To test the solution, uncomment the following lines in `launcher_E06-mat-transpose.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        cmake .. -DPASSWORD="XXXXXX" -DBUILD=SOL  && make fpga_emu
        ./E06-mat-transpose.fpga_emu
        ```
    === "Hardware (Solution)"
        - To run the fpga image, uncomment the following lines in `launcher_E06-mat-transpose.sh`
        - Make sure that the header `#SBATCH -p fpga` is present
        ```bash
        export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
        cd fpga_image
        LD_PRELOAD=${JEMALLOC_PRELOAD} ./E06-mat-transpose.fpga
        ```

### E07-shared-memory
!!! example "Shared memory example"
    === "Instruction"
        - Go to `./exercices/E07-shared-memory`
        - The operation that generates a matrix from two vectors is commonly referred to as the **outer product** (also known as the **tensor product** or **dyadic product**) of two vectors.
        - For two vectors, **a** and **b**, the outer product results in a matrix where each element is the product of the corresponding components of the vectors.
        - If:

            - **a** is an $(N \times 1)$ column vector: $a = \begin{bmatrix} 
                                                                a_1 \\ 
                                                                a_2 \\
                                                                \vdots \\ 
                                                                a_N \end{bmatrix}$
  
            - **b** is an ($1 \times N)$  row vector: $b = \begin{bmatrix} b_1 & b_2 & . . .  & b_N \end{bmatrix}$

        - Then the outer product of **a** and **b**, denoted by \( a \otimes b \), results in an \( m \times n \) matrix:
            
            $$a \otimes b = \begin{bmatrix}
            a_1 b_1 & a_1 b_2 & \cdots & a_1 b_n \\
            a_2 b_1 & a_2 b_2 & \cdots & a_2 b_n \\
            \vdots & \vdots & \ddots & \vdots \\
            a_m b_1 & a_m b_2 & \cdots & a_m b_n
            \end{bmatrix}$$

        - This matrix has the element ($i, j$) equal to the product $a_i \times b_j$.
        - Compute the outer-product by completing the kernel and use shared memory / group memory to minimize access to the device global memory

        ![](./images/sharedmemory.png){width=80%}

        - Execute your code with the FPGA emulator
        - Execute the FPGA image in the `fpga_image` folder
    === "Code description"
         ```cpp linenums="1"
         --8<-- "./exercices/E07-shared-memory/src/E07-shared-memory.cpp"
         ```
    === "Emulation (test + solution)"
        - To execute your code, uncomment the following lines in `launcher_E07-shared-memory.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        echo "Building"
        cmake .. -DBUILD=EX && make fpga_emu
        ./E07-shared-memory.fpga_emu
        ```
        - To test the solution, uncomment the following lines in `launcher_E07-shared-memory.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        cmake .. -DPASSWORD="XXXXXX" -DBUILD=SOL  && make fpga_emu
        ./E07-shared-memory.fpga_emu
        ```
    === "Hardware (Solution)"
        - To run the fpga image, uncomment the following lines in `launcher_E07-shared-memory.sh`
        - Make sure that the header `#SBATCH -p fpga` is present
        ```bash
        export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
        cd fpga_image
        LD_PRELOAD=${JEMALLOC_PRELOAD} ./E07-shared-memory.fpga
        ```

### E08-pipe
!!! example "Fibonnaci ping-pong"
    === "Instruction"
        - Go to `./exercices/E08-pipe`
        - Using two kernels and pipes, compute the Fibonnaci sequence using both kernels by exchanging everytime the value computing by one kernel with the other one. 
        - <u>**Fibonacci Formula**</u>:
          - $F(n)$ is the nth Fibonacci number
          - $F(0) = 0$ and $F(1) = 1$ are the base cases
          - $F(n) = F(n-1) + F(n-2)$
        ![](./images/pipe.png)
        - Execute your code with the FPGA emulator
        - Execute the FPGA image in the `fpga_image` folder
    === "Code description"
         ```cpp linenums="1"
         --8<-- "./exercices/E08-pipe/src/E08-pipe.cpp"
         ```
    === "Emulation (test + solution)"
        - To execute your code, uncomment the following lines in `launcher_E08-pipe.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        echo "Building"
        cmake .. -DBUILD=EX && make fpga_emu
        ./E08-pipe.fpga_emu
        ```
        - To test the solution, uncomment the following lines in `launcher_E08-pipe.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        cmake .. -DPASSWORD="XXXXXX" -DBUILD=SOL  && make fpga_emu
        ./E08-pipe.fpga_emu
        ```
    === "Hardware (Solution)"
        - To run the fpga image, uncomment the following lines in `launcher_E08-pipe.sh`
        - Make sure that the header `#SBATCH -p fpga` is present
        ```bash
        export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
        cd fpga_image
        LD_PRELOAD=${JEMALLOC_PRELOAD} ./E08-pipe.fpga
        ```

### E09-multi-device
!!! example "Multi-device computations"
    === "Instruction"
        - Go to `./exercices/E09-multi-device`
        - You are given an host_array and needs to transfer the first half into device 1 and the second part to device 2
        - The first device will add 1 to each value of the first half while the second one will add 2 to each value of the second half 
        - Finally, retrieve from the two half from each device and update the original host_array
        - Execute your code with the FPGA emulator
        - Execute the FPGA image in the `fpga_image` folder
    === "Code description"
         ```cpp linenums="1"
         --8<-- "./exercices/E09-multi-device/src/E09-multi-device.cpp"
         ```
    === "Emulation (test + solution)"
        - To execute your code, uncomment the following lines in `launcher_E09-multi-device.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        echo "Building"
        cmake .. -DBUILD=EX && make fpga_emu
        ./E09-multi-device.fpga_emu
        ```
        - To test the solution, uncomment the following lines in `launcher_E09-multi-device.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        cmake .. -DPASSWORD="XXXXXX" -DBUILD=SOL  && make fpga_emu
        ./E09-multi-device.fpga_emu
        ```
    === "Hardware (Solution)"
        - To run the fpga image, uncomment the following lines in `launcher_E09-multi-device.sh`
        - Make sure that the header `#SBATCH -p fpga` is present
        ```bash
        export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
        cd fpga_image
        LD_PRELOAD=${JEMALLOC_PRELOAD} ./E09-multi-device.fpga
        ```
        <u> Question </u>: Why do you still get a warning for alignment ?


### E10-convolution
!!! example "Convolution"
    === "Instruction"
        - Go to `./exercices/E10-convolution`
        - Create a ndrange sycl kernel to apply a convolution kernel on a png image

        ![](./images/convolution.png){width="80%"}

        - Execute your code with the FPGA emulator
        - Execute the FPGA image in the `fpga_image` folder
    === "Code description"
         ```cpp linenums="1"
         --8<-- "./exercices/E10-convolution/src/E10-convolution.cpp"
         ```
    === "Emulation (test + solution)"
        - To execute your code, uncomment the following lines in `launcher_E10-convolution.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        echo "Building"
        cmake .. -DBUILD=EX -DUSER_FLAGS="-lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc" && make fpga_emu
        ./E10-convolution.fpga_emu
        ```
        - To test the solution, uncomment the following lines in `launcher_E10-convolution.sh`
        ```bash
        echo "Create building directory"
        mkdir -p build && find build -mindepth 1 -delete && cd build
        cmake .. -DPASSWORD="XXXXXX" -DBUILD=SOL -DUSER_FLAGS="-lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc"  && make fpga_emu
        ./E10-convolution.fpga_emu
        ```
    === "Hardware (Solution)"
        - To run the fpga image, uncomment the following lines in `launcher_E10-convolution.sh`
        - Make sure that the header `#SBATCH -p fpga` is present
        ```bash
        export JEMALLOC_PRELOAD=$(jemalloc-config --libdir)/libjemalloc.so.$(jemalloc-config --revision)
        cd fpga_image
        LD_PRELOAD=${JEMALLOC_PRELOAD} ./E10-convolution.fpga
        ```
