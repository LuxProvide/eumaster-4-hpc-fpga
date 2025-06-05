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


class DotProductID;

void DotProduct(const double *vec_a_in, const double *vec_b_in, double *vec_c_out,
               int len) {
// Define your dot product here
}



int main() {
  bool passed = true;
  const int kVectSize = 256;
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


    double* host_vec_a = new double[kVectSize];
    double* host_vec_b = new double[kVectSize];
    double* host_vec_c = new double[1];
    for (int i = 0; i < kVectSize; i++) {
      host_vec_a[i] = i;
      host_vec_b[i] = (kVectSize - i);
    }


    std::cout << "Using buffer version" << std::endl;
    {
     sycl::buffer buffer_vec_a{host_vec_a,sycl::range(kVectSize)}; 
     sycl::buffer buffer_vec_b{host_vec_b,sycl::range(kVectSize)};
     sycl::buffer buffer_vec_c{host_vec_c,sycl::range(1)};
     q.submit([&](handler& h) {
       sycl::accessor access_vec_a{buffer_vec_a,h,sycl::read_only}; 
       sycl::accessor access_vec_b{buffer_vec_b,h,sycl::read_only};  
       sycl::accessor access_vec_c{buffer_vec_c,h,sycl::write_only,sycl::no_init};  
       h.single_task<DotProductID>([=]() {
          // Call the the function here
          DotProduct(&access_vec_a[0],&access_vec_b[0],&access_vec_c[0],kVectSize);
       });
     });
    }
    // verify that correctness
    double expected = 0;
    for (int i = 0; i < kVectSize; i++) {
      expected += (host_vec_a[i] * host_vec_b[i]);
    }

    if (*host_vec_c != expected) {
      std::cout << "expected=" << expected << ": result " << *host_vec_c << std::endl;
      passed = false;
    }else{
      std::cout << "A.B=" << host_vec_c << std::endl;
    }

    delete[] host_vec_a;
    delete[] host_vec_b;
    delete[] host_vec_c;

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
