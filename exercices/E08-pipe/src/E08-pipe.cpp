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

class PingID;
class PongID;

int main() {
  bool passed = true;
  const int N = 40;
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

    // create the device queue
    sycl::queue q(selector);

    auto device = q.get_device();

    std::cout << "Running on device: "
              << device.get_info<sycl::info::device::name>().c_str()
              << std::endl;



    std::cout << "Using USM device" << std::endl;
    using pipeA = ext::intel::pipe<      
                class Ping2Pong,
                long,           
                1>;             

    using pipeB = ext::intel::pipe<      
                class Pong2Ping, 
                long,            
                1>;              
     q.submit([&](handler& h) {
       h.single_task<PingID>([=](){ 
        /*
        *
        * Add the code for the Ping kernel
        *
        *
        */
       });
     });

     q.submit([&](handler& h) {
       h.single_task<PongID>([=](){ 
        /*
        *
        * Add the code for the Pong kernel
        *
        *
        */
       });
     });

     q.wait();

     //verify that correctness
     //long host_vec[N];
     //q.memcpy(host_vec, device_vec, N * sizeof(long)).wait();
     //for (int k = 0 ; k < N ; k++){
     //     std::cout <<"FIB("<<k+1<<")=" << host_vec[k] << std::endl;
     //}
     //
     //if (host_vec[N-1] != fib_40) {
     //  std::cout << "expected=" << fib_40 << ": result " << host_vec[N-1] << std::endl;
     //  passed = false;
     //}


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
