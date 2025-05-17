#include <iostream>
#include <algorithm>
#include <random>

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

class MatTransposeID;


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

    constexpr size_t N = 512;
    std::vector<float> mat(N * N);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(0.0, 1.0);

    // Generate random values
    std::generate(mat.begin(), mat.end(), [&dist, &mt]() {
      return dist(mt);
    });

    std::vector<float> copy_mat(mat);


    std::cout << "Using buffer version" << std::endl;
    {
     sycl::buffer<float,2> buffer_mat{mat.data(),sycl::range<2>(N,N)};
     q.submit([&](handler& h) {
       sycl::accessor access_mat{buffer_mat,h}; 
       /*
          *
          *
          *
          *  Define and call the kernel here
          *
          *
          *
          *
        */
     });
    }


    // verify that correctness
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++){
          if (mat[i*N+j] != copy_mat[j*N+i]) {
            std::cout << "expected=" << copy_mat[j*N+i] << ": result " <<  mat[i*N+j] << std::endl;
            passed = false;
            break;
          }
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
