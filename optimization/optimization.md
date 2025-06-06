# Optimizing SYCL programs for Intel® FPGA cards

Optimizing SYCL code for Intel FPGAs requires a combination of understanding the FPGA hardware, the SYCL programming model, and the specific compiler features provided by Intel. Here are some general guidelines to optimize Intel FPGA SYCL code.

Compared to OpenCL, the Intel® oneAPI DPC++ compiler has enhanced features to detect possible optimizations( vectorization, static coalescing, etc ...). Nonetheless, some rules need to be followed to make sure the compiler is able to apply these optimizations. 

!!! warning "Optimizing your design"
    As this course/workshop is only an introduction to the Intel® oneAPI for FPGA programming, we can't unfortunately provide all existing and possible optimizations. Many more optimizations can be found in the Intel official documentation.

## Loop optimization

Loop unrolling is an optimization technique that aims to increase parallelism and, consequently, the throughput of certain computational tasks, particularly when implemented in hardware environments such as FPGAs. 


1. **Pipelining Synergy**: Loop unrolling often goes hand in hand with pipelining in FPGAs. When loops are unrolled, each unrolled iteration can be pipelined, leading to even greater throughput enhancements.

2. **Resource Utilization**: While loop unrolling can significantly speed up operations, it also consumes more FPGA resources, like Logic Elements (LEs) and registers, because of the duplicated hardware. Hence, there's a trade-off between speed and resource utilization.

3. **Memory Access**: Unrolling loops that involve memory operations can lead to increased memory bandwidth utilization. In cases where memory bandwidth is a bottleneck, unrolling can provide substantial performance improvements.

4. **Latency & Throughput**: Loop unrolling doesn't necessarily reduce the latency of a single loop iteration (the time taken for one iteration to complete), but it can significantly improve the throughput (number of completed operations per unit time).

5. **Reduction in Control Logic**: Unrolling can reduce the overhead associated with the loop control logic, such as incrementing the loop counter and checking the loop termination condition.

    <figure markdown>
        ![](https://res.cloudinary.com/dxzx2bxch/image/upload/v1604459708/posts/X14770-loop-pipelining_v42eza.svg)
       <figcaption>[Loop Optimization in HLS](https://www.zzzdavid.tech/loop_opt/)</figcaption>
    </figure>

* Unrolling loops will help to reduce the Initialization Interval (II) as you can notice on the previous figure.

!!! tig "Increasing throughput with loop unrolling"
    === "How to unroll loops"
        * Unrolling loop can be done using the `#pragma unroll <N>`
        * `<N>` is the unroll factor
        * `#pragma unroll 1` : prevent a loop in your kernel from unrolling
        * `#pragma unroll` : let the offline compiler decide how to unroll the loop 
        ```cpp
        handler.single_task<class example>([=]() {
            #pragma unroll
                for (int i = 0; i < 10; i++) {
                    acc_data[i] += i;
                }
            #pragma unroll 1
            for (int k = 0; k < N; k++) {
                #pragma unroll 5
                for (int j = 0; j < N; j++) {
                    acc_data[j] = j + k;
                }
            }
        });
        ```

    === "Question"
        * Consider the following code that you can find at `oneAPI-samples/DirectProgramming/C++SYCL_FPGA/Tutorials/Features/loop_unroll`
        * Note that Intel did not consider data alignment which could impact performance
        * We included `#include <boost/align/aligned_allocator.hpp>` to create aligned std::vector
        * The following SYCL code has been already compiled for you, execute it on the FPGA nodes for several data input size and record the throughput and kernel time
        * What do you observe ?
        ```cpp linenums="1"
        --8<-- "./code/09-loop_unroll/src/loop_unroll.cpp"
        ```
    === "Solution"

        <div align="center">

        | Unroll factor   | kernel execution time (ms)   | Throughput (GFlops) |
        |:---------------:|:----------------------------:|:-------------------:|
        |       1         |             77               |        0.447        |
        |       2         |             58               |        0.591        |
        |       4         |             43               |        0.804        |
        |       8         |             40               |        0.857        |
        |       16        |             39               |        0.882        |

        </div>

        * Increasing the unroll factor improves throughput    
        * Nonetheless, unrolling large loops should be avoided as it would require a large amount of hardware
        !!! warning "Recording kernel time"
            * In this example, we have also seen how to record kernel time.
            * Using the property `property::queue::enable_profiling{}`` adds the requirement that the runtime must capture profiling information for the command groups that are submitted from the queue 
            * You can the capture  the start & end time using the following two commands:
                - `double start = e.get_profiling_info<info::event_profiling::command_start>();`
                - `double end = e.get_profiling_info<info::event_profiling::command_end>();`

!!! warning "Caution with nested loops"
    * Loop unrolling involves replicating the hardware of a loop body multiple times and reducing the trip count of a loop. Unroll loops to reduce or eliminate loop control overhead on the FPGA. 
    * Loop-unrolling can be used to eliminate nested-loop structures.
    * However avoid unrolling the outer-loop which will lead to **Resource Exhaustion** and dramatically increase offline compilation

## SIMD Work Items for ND-Range kernels

* ND-range kernel should use instead of classical data-parallel kernels
* The work-group size needs to be set using the attribute `[[sycl::reqd_work_group_size(1, 1, REQD_WG_SIZE)]]`
* To specify the number of SIMD work_items, you will need to add the following attribute `[[intel::num_simd_work_items(NUM_SIMD_WORK_ITEMS)]]`
* Note that **NUM_SIMD_WORK_ITEMS** should divide evenly **REQD_WG_SIZE**
* The supported values for **NUM_SIMD_WORK_ITEMS**  are 2, 4, 8, and 16

!!! example "Example"
    ```cpp linenums="1"
    ...
    h.parallel_for<VectorAddID>(
    sycl::nd_range<1>(sycl::range<1>(2048), sycl::range<1>(128)),        
        [=](sycl::nd_item<1> it) 
        [[intel::num_simd_work_items(8),
        sycl::reqd_work_group_size(1, 1, 128)]] {
        auto gid = it.get_global_id(0);
        accessor_c[gid] = accessor_a[gid] + accessor_b[gid];
        });
    });
    ...
    ```

    * The **128** work-items are evenly distributed among **8** SIMD lanes

    * 128/8 = 16 wide vector operation

    * The offline compiler coalesces 8 loads to optimize (reduce) the access to memory in case there are no data dependencies


## Loop coalescing

Utilize the `loop_coalesce` attribute to instruct the Intel® oneAPI DPC++/C++ Compiler to merge nested loops into one, preserving the loop's original functionality. By coalescing loops, you can minimize the kernel's area consumption by guiding the compiler to lessen the overhead associated with loop management.

!!! example "Coalesced two loops"
    === "Using the loop_coalesce attribute"
    ```cpp
    [[intel::loop_coalesce(2)]]
    for (int i = 0; i < N; i++)
       for (int j = 0; j < M; j++)
          sum[i][j] += i+j;
    ```
    === "Equivalent code"
    ```cpp
    int i = 0;
    int j = 0;
    while(i < N){
      sum[i][j] += i+j;
      j++;
      if (j == M){
        j = 0;
        i++;
      }
    }
    ```


## Memory 

### Static coalescing

* Static coalescing is performed by the Intel® oneAPI DPC++/C++ Compiler contiguous accesses to global memory can be merged into a single wide access.

* For static memory coalescing to occur, your code should be structured so that the compiler can detect a linear access pattern at compile time. The initial kernel code depicted in the previous figure can leverage static memory coalescing, as all indices into buffers a and b increase with offsets recognizable during compilation.


<figure markdown>
![](https://www.intel.com/content/dam/docs/us/en/optimization-guide/2023-1/9253AEE0-2330-480D-941B-B1D61496F8D0-low.png)
  <figcaption><a href=https://www.intel.com/content/www/us/en/docs/oneapi-fpga-add-on/optimization-guide/2023-1/static-memory-coalescing.html>FPGA Optimization Guide for Intel® oneAPI Toolkits</a> -- Figure 17-21 </figcaption>
</figure>

### Data structure alignment

In order to performance, structure alignment can be modified to be properly aligned. By default, the offline compiler aligns these elements based on:

* The alignment should be a power of two.
* The alignment should be a multiple of the least common multiple (LCM) of the word-widths of the structure member sizes.

Let's take a simple but clear example to understand why alignment is so important.

![](./images/alignment.png)


* Each element of MyStruct has 12 bytes  due to padding

* Recall that each transaction between the user kernel design and the memory controller is 512 bits wide to enable DMA

* We have therefore 64/12 = 5.333 => alignment is far from optimal as the 6th element of MyStruct will be split between two 64-byte regions


![](./images/struct_padding.png)

* Removing all padding will definitely reduce the size 

* Padding can be removed by adding  the “packed” attribute, i.e, “__attribute__((packed))” in your kernel

* Each element of MyStruct will have therefore 9 bytes

* However, 64/9 = 7.111 => we still have some elements in multiple 64-bytes region and the alignment is sub-optimal


![](./images/struct_remove_padding.png)

* To improve performance, align structure  such all elements belongs to a single 64-byte regions

* Padding can still be removed by adding  the “packed” attribute, i.e, “__attribute__((packed))” 

* Transaction size is 64 bytes, the minimum alignment which is also a multiple of the transaction size is 16 

* Enforce a 16-byte alignment with `__attribute__((aligned(16)))`

![](./images/struct_align.png)

!!! example "Removing padding and changing structure alignment"
    === "Code"
    
        * The following code show the impact of changing the alignmement and padding using three scenarii:

             * Default alignment and padding 

             * Removing padding

             * Changing alignment 

        ```cpp linenums="1"
        --8<-- "./code/10-alignment/src/alignment.cpp"
        ```
    === "Execution time"

        <div align=center>

        | Scenario                      | Processing time (seconds)  |
        |: ---------------------------: | :-------------------------:|
        | Default alignment and padding | 14.33                      |
        | Removing padding              | 6.35                       |
        | Changing alignment            | 0.03                       |

        </div>



### Memory


|           Type           |                    Access                   |         Hardware         |
|:------------------------:|:-------------------------------------------:|:------------------------:|
| Host memory              | read/write only by host                     | DRAM                     |
| Global memory (device)   | read/write by host and work-items           | FPGA DRAM (HBM, DDR,QDR) |
| Local memory (device)    | read/write only by work-group               | RAM blocks               |
| Constant memory (device) | read/write by host read only  by work-items | FPGA DRAM  RAM blocks    |
| Private memory device    | read/write by single work-item only         | RAM blocks Registers     |

* Transfers between host memory and global device memory should leverage DMA for efficiency.

* Of all memory types on FPGAs, accessing device global memory is the slowest.

* In practice, using local device memory is advisable to reduce global memory accesses.

#### Local memory in ND-Range kernels

* You can improve memory access by using local and private memory.
* When you define a private array, group local memory, or a local accessor, the Intel® oneAPI DPC++/C++ Compiler generates kernel memory in the hardware. This kernel memory is often termed on-chip memory since it originates from memory resources, like RAM blocks, present on the FPGA.
* Local or private memory is a fast memory that should be favored when resources allow.

!!! example "Private memory"
    ```cpp linenums="1"
    ...
    q.submit([&](handler &h) {
    // Create an accessor for device global memory from buffer buff
    accessor acc(buff, h, write_only);
    cgh.single_task([=]() {
         // Declare a private array
         int T[N];
         // Write to private memory
         for (int i = 0; i < N; i++)
            T[i] = i;
         // Read from private memory and write to global memory through the accessor
         for (int i = 0; i < N; i+=2)
            acc[i] = T[i] + T[i+1];
         });
    }); 
    ...
    ```
* Two ways to define local memory for work-groups:

    - If you have function scope local data using group_local_memory_for_overwrite, the Intel® oneAPI DPC++/C++ Compiler statically sizes the local data that you define within a function body at compilation time.

    - For accessors in the local space, the host assigns their memory sizes dynamically at runtime. However, the compiler must set these physical memory sizes at compilation time. By default, that size is 16 kB. 

!!! example "Local memory"
    === "Using sycl::local_accessor"
        ```cpp linenums="1"
        ...
        q.submit([&](handler &h) {
            h.parallel_for(
                nd_range<1>(range<1>(256), range<1>(16)), [=](nd_item<1> item)
                [[intel::max_work_group_size(1, 1, 16)]] { 
                int local_id = item.get_local_id();
                sycl::local_accessor<float,2> lmem{B, h};
                lmem[local_id] = local_id++;
                });
            });
        ... 
        ```
    === "Using `group_local_memory` functions"
        ```cpp linenums="1"
        ...
        q.submit([&](handler &h) {
            h.parallel_for(
                nd_range<1>(range<1>(256), range<1>(16)), [=](nd_item<1> item) {
                int local_id = item.get_local_id();
                auto ptr = group_local_memory_for_overwrite<int[16]>(item.get_group());
                auto& ref = *ptr;
                ref[local_id] = local_id++ ;
                });
            });
        ... 
        ```

    * The ND-Range kernel has 16 workgroups with 16 work items for each group.
    * A group-local variable (int[16]) is created for each group and shared through a multi_ptr to all work-items of the same group

* Use sycl::local_accessor when data needs to be retained and shared within the same work-group and when precise control over data access and synchronization is necessary. Opt for group_local_memory_for_overwrite when you need temporary storage that can be efficiently reused across multiple work-groups, without persistence or heavy synchronization overhead.


#### Kernel Memory System

* Before diving into the different settings, we need to introduce some definitions:

    * <u>Port</u>: a memory port serves as a physical access point to memory, connecting to one or more load-store units (LSUs) within the datapath. An LSU can interface with multiple ports, and a port can be linked to multiple LSUs.

    * <u>Bank</u>: a memory bank is a division within the kernel memory system, holding a unique subset of the kernel's data. All data is distributed across these banks, and every memory system has at least one bank.

    * <u>Replicate</u>: a memory bank replicate is a copy of the data within a memory bank, with each replicate containing identical data. Replicates are independently accessible, and every memory bank includes at least one replicate.

    * <u>Private Copy</u>: a private copy is a version of data within a replicate, created for nested loops to support concurrent outer loop iterations. Each outer loop iteration has its own private copy, allowing different data per iteration.

#### Settings memory banks

* Local data can be stored  in separate  local memory banks for parallel memory accesses
* Number of banks of a local memory can be adjusted (e.g., to increase the parallel access) 
* Add the following attributes `[[intel::numbanks(#NB), intel::bankwidth(#BW)]]`:  
    * `#NB` : number of banks 
    * `#BW` : bankwidth to be considered in bytes
* <u>Ex</u>: `[[intel::numbanks(8), intel::bankwidth(16)]] int lmem[8][4]`; 
* All rows accessible in parallel with numbanks(8) 
* Different configurations patterns can be adopted 

<figure markdown>
  ![](./images/banks.png)
  <figcaption> [[intel::numbanks(8), intel::bankwidth(16)]] lmem[8][4] </br> (source: Intel)  </figcaption>
</figure>

!!! warning "Masking the last index"
    * Intel's documentation states that "To enable parallel access, you must mask the dynamic access on the lower array index"
    ```cpp linenums="1"
    [[intel::numbanks(8), intel::bankwidth(16)]] int lmem[8][4];
    #pragma unroll
    for (int i = 0; i < 4; i+=2) {
        lmem[i][x & 0x3] = ...;
    } 
    ```
#### Local memory replication

!!! example "Example"
    <div style="position:relative;">
    <div style="display:inline-block; width:50%;">
    ```cpp linenums="1"
    [[intel::fpga_memory,
    intel::singlepump,
    intel::max_replicates(3)]] int lmem[16]; 
    lmem[waddr] = lmem[raddr] +
                  lmem[raddr + 1] +
                  lmem[raddr + 2]; 
    ```

    * The offline compiler can replicate the local memory
    * This allows to create multiple ports 
    * Behaviour: 
        * All read ports will be accessed in parallel 
        * All write ports are connected together
        * Data between replicate is identical 
    * Parallel access to all ports is possible but consumes more hardware resources
    * `[[intel::max_replicates(N)]]` control the replication factor


    </div>
    <div style="display:inline-block; width:50%;float:right;">
    ![](./images/replicates.png){ align=right width=300 }
    </div>
    </div>

