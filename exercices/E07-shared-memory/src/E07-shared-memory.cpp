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


<<<<<<<< HEAD:exercices/E07-shared-memory/src/E07-shared-memory.cpp
class VecProductPID;
========
class DotProductID;

void DotProduct(const double *vec_a_in, const double *vec_b_in, double *vec_c_out,
               int len) {
// Define your dot product here
}

>>>>>>>> 65d2f804a19776777ecfb37e5336cea6e5972f72:exercices/E05-dot-product/src/E05-dot-product.cpp


int main() {
  bool passed = true;
  const int kVectSize = 256;
  const int blockSize = 16;
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


    float host_vec_a[kVectSize];
    float host_vec_b[kVectSize];
    float host_vec_c[kVectSize*kVectSize];
    for (int i = 0; i < kVectSize; i++) {
      host_vec_a[i] = i;
      host_vec_b[i] = (kVectSize - i);
    }


    std::cout << "Using buffer version" << std::endl;
    {
     sycl::buffer buffer_vec_a{host_vec_a,sycl::range(kVectSize)}; 
     sycl::buffer buffer_vec_b{host_vec_b,sycl::range(kVectSize)};
     sycl::buffer<float,2> buffer_vec_c{host_vec_c,sycl::range<2>(kVectSize,kVectSize)};
     q.submit([&](handler& h) {
       sycl::accessor access_vec_a{buffer_vec_a,h,sycl::read_only}; 
       sycl::accessor access_vec_b{buffer_vec_b,h,sycl::read_only};  
       sycl::accessor access_vec_c{buffer_vec_c,h,sycl::write_only,sycl::no_init};  
<<<<<<<< HEAD:exercices/E07-shared-memory/src/E07-shared-memory.cpp
       sycl::local_accessor<float,1> shared_mem{blockSize, h};
       h.parallel_for<VecProductPID>(sycl::nd_range<1>({kVectSize,blockSize}),[=](sycl::nd_item<1> item){ 
         int m = item.get_global_id()[0];
         int i = item.get_local_id()[0];
         /*
          *
          *  Define the kernel code here
          *
          *
          */
========
       h.single_task<DotProductID>([=]() {
          // Call the the function here
          DotProduct(&access_vec_a[0],&access_vec_b[0],&access_vec_c[0],kVectSize);
>>>>>>>> 65d2f804a19776777ecfb37e5336cea6e5972f72:exercices/E05-dot-product/src/E05-dot-product.cpp
       });
     });
    }


    // verify that correctness
    for (int i = 0; i < kVectSize; i++) {
      for (int j =0; j < kVectSize; j++){
          float expected = host_vec_a[i] * host_vec_b[j];
          if (expected != host_vec_c[i*kVectSize+j]) {
            std::cout << "expected=" << expected << ": result " << host_vec_c << std::endl;
            passed = false;
            break;
          }
      }
    }

<<<<<<<< HEAD:exercices/E07-shared-memory/src/E07-shared-memory.cpp
========
    if (host_vec_c != expected) {
      std::cout << "expected=" << expected << ": result " << host_vec_c << std::endl;
      passed = false;
    }else{
      std::cout << "A.B=" << host_vec_c << std::endl;
      std::cout << "Success"<< std::endl;
    }
>>>>>>>> 65d2f804a19776777ecfb37e5336cea6e5972f72:exercices/E05-dot-product/src/E05-dot-product.cpp

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
