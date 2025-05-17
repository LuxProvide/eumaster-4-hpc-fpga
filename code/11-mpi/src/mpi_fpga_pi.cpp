#include <mpi.h>
// oneAPI headers
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/sycl.hpp>
#include <iomanip>  // setprecision library
#include <iostream>
#include <numeric> 


using namespace sycl;
constexpr int master = 0;

////////////////////////////////////////////////////////////////////////
//
// Each MPI ranks compute the number Pi partially on target device using SYCL.
// The partial result of number Pi is returned in "results".
//
////////////////////////////////////////////////////////////////////////
void mpi_native(double* results, int rank_num, int num_procs,
                long total_num_steps, queue& q) {

  double dx = 1.0f / (double)total_num_steps;
  long items_per_proc = total_num_steps / size_t(num_procs);
  // The size of amount of memory that will be given to the buffer.
  //range<1> num_items{items_per_proc};

  // Buffers are used to tell SYCL which data will be shared between the host
  // and the devices.
  buffer<double, 1> results_buf(results,
                               range<1>(items_per_proc));

  // Submit takes in a lambda that is passed in a command group handler
  // constructed at runtime.
  q.submit([&](handler& h) {
    // Accessors are used to get access to the memory owned by the buffers.
    accessor results_accessor(results_buf,h,write_only);
    // Each kernel calculates a partial of the number Pi in parallel.
    h.parallel_for(range<1>(items_per_proc), [=](id<1> k) {
      double x = ((double)(rank_num * items_per_proc + k))  * dx ;
      results_accessor[k] = (4.0f * dx) / (1.0f + x * x);
    });
  });
}


int main(int argc, char** argv) {
  long num_steps = 1000000;
  char machine_name[MPI_MAX_PROCESSOR_NAME];
  int name_len=0;
  int id=0;
  int num_procs=0;
  double pi=0.0;
  double t1, t2;
  try {
  // Use compile-time macros to select either:
  //   - the FPGA emulator device (CPU emulation of the FPGA)
  //   - the FPGA device (a real FPGA)
  //   - the simulator device
  #if FPGA_SIMULATOR
  auto selector = ext::intel::fpga_simulator_selector_v;
  #elif FPGA_HARDWARE
  auto selector = ext::intel::fpga_selector_v;
  #elif FPGA_EMULATOR
  auto selector = ext::intel::fpga_emulator_selector_v;
  #else 
  auto selector = sycl::cpu_selector_v;
  #endif

  property_list q_prop{property::queue::in_order()};
  queue myQueue{selector,q_prop};

  // Start MPI.
  if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
    std::cout << "Failed to initialize MPI\n";
    exit(-1);
  }

  // Create the communicator, and retrieve the number of MPI ranks.
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  // Determine the rank number.
  MPI_Comm_rank(MPI_COMM_WORLD, &id);

  // Get the machine name.
  MPI_Get_processor_name(machine_name, &name_len);

  if(id == master) t1 = MPI_Wtime();

  std::cout << "Rank #" << id << " runs on: " << machine_name
            << ", uses device: "
            << myQueue.get_device().get_info<info::device::name>() << "\n";

  int num_step_per_rank = num_steps / num_procs;
  double* results_per_rank = new double[num_step_per_rank];

  // Initialize an array to store a partial result per rank.
  for (size_t i = 0; i < num_step_per_rank; i++) results_per_rank[i] = 0.0;

  // Calculate the Pi number partially by multiple MPI ranks.
  mpi_native(results_per_rank, id, num_procs, num_steps, myQueue);

  double local_sum = 0.0;
  for(unsigned int i = 0; i < num_step_per_rank; i++){
    local_sum += results_per_rank[i];
  }

  // Master rank performs a reduce operation to get the sum of all partial Pi.
  MPI_Reduce(&local_sum, &pi, 1, MPI_DOUBLE, MPI_SUM, master, MPI_COMM_WORLD);

  if (id == master) {
    t2 = MPI_Wtime(); 
    std::cout << "mpi native:\t\t";
    std::cout << std::setprecision(10) << "PI =" << pi << std::endl;
    std::cout << "Elapsed time is " << t2-t1 << std::endl;
  }

  delete[] results_per_rank;

  MPI_Finalize();

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

  return 0;
}
