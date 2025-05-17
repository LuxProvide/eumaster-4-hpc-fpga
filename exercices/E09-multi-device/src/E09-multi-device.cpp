#include <iostream>

// oneAPI headers
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/sycl.hpp>


#ifdef __SYCL_DEVICE_ONLY__
#define CL_CONSTANT __attribute__((opencl_constant))
#else
#define CL_CONSTANT
#endif

using namespace sycl;

#define PRINTF(format, ...)                                    \
  {                                                            \
    static const CL_CONSTANT char _format[] = format;          \
    ext::oneapi::experimental::printf(_format, ##__VA_ARGS__); \
  }

class FirstDeviceID;
class SecondDeviceID;

int main() {
  constexpr int N = 1000;
  bool passed = true;
  try {
    // Use compile-time macros to select either:
    //  - the FPGA emulator device (CPU emulation of the FPGA)
    //  - the FPGA device (a real FPGA)
    //  - the simulator device
      #if FPGA_SIMULATOR
          auto selector = sycl::ext::intel::fpga_simulator_selector_v;
      #elif FPGA_HARDWARE
          auto selector = sycl::ext::intel::fpga_selector_v;
      #else  // #if FPGA_EMULATOR
          auto selector = sycl::ext::intel::fpga_emulator_selector_v;
      #endif

    
    #if FPGA_HARDWARE
    /*
      *
      *
      * Define the context and the queue here
      *
      */
    #else
    sycl::queue q0 (selector);
    sycl::queue q1 (selector);
    #endif


    /*
      *
      *
      * Get the device and print their names
      *
      *
      */


    int host_array[N];
    std::memset(host_array, 0, sizeof(int)*N);
    /*
      *
      * Setup two array on both device global memories with size N/2
      * Transfer the first half of the host array on the first device
      * and the second half on the second array
      */

    std::cout << "Using USM device" << std::endl;
    {

    q0.submit([&](handler& h) {
       h.single_task<FirstDeviceID>([=](){ 
          for(unsigned int i = 0; i < N/2;i++)
            device_part1[i]+=1;
       });
     });

   q1.submit([&](handler& h) {
       h.single_task<SecondDeviceID>([=](){ 
          for(unsigned int i = 0; i < N/2;i++)
            device_part2[i]+=2;
       });
     });
    }

    q0.wait();
    q1.wait();

    /*
      *
      * Retireve the two half and update the host array
      *
      */

     for (int k = 0 ; k < N/2 ; k++){
        if(k<N/2 && host_array[k]!=1){
          std::cout<<"Expected host_array=1 but received host_array="<<host_array[k]<<std::endl;
          passed=false;
          break;
        }
        if(k>=N/2 && host_array[k]!=2){
          std::cout<<"Expected host_array=2 but received host_array="<<host_array[k]<<std::endl;
          passed=false;
          break;
        }
     }


  } catch (sycl::exception const &e) {
    // Catches exceptions in the host code.
    std::cerr << "Caught a SYCL host exception:\n" << e.what() << "\n";

    // Most likely the runtime couldn't find FPGA hardware!
    if (e.code().value() == CL_DEVICE_NOT_FOUND) {
      std::cerr << "If you are targeting an FPGA, please ensure that your "
                   "system has a correctly configured FPGA board.\n";
      std::cerr << "Run sys_check in the oneAPI root directory to verify.\n";
      std::cerr << "If you are targeting the FPGA emulator, compile with "
                   "-DFPGA_EMULATOR.\n";
    }
    std::terminate();
  }
  return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
