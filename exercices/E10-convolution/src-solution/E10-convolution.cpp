#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

// oneAPI headers
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/sycl.hpp>


#ifdef __SYCL_DEVICE_ONLY__
#define CL_CONSTANT __attribute__((opencl_constant))
#else
#define CL_CONSTANT
#endif

#define pi 3.14159265359
#define KERNEL_SIZE 3

class ConvolutionID;

using namespace sycl;

float* load_image(const char* im_path, int* Nx, int* Ny){
       cv::Mat image = cv::imread(im_path, cv::IMREAD_GRAYSCALE);

       if (image.empty()) {
           std::cerr << "Failed to load image!" << std::endl;
           return nullptr;
       }

       // Convert to float (optional: normalize to [0,1])
       cv::Mat imageFloat;
       image.convertTo(imageFloat, CV_32F); // normalize to 0-1 range

       // Allocate and copy to float*
       *Nx = imageFloat.rows;
       *Ny = imageFloat.cols;
       int size = imageFloat.rows * imageFloat.cols;
       float* buffer = new float[size];
       std::memcpy(buffer, imageFloat.ptr<float>(), size * sizeof(float));

       return buffer;
}

void save_image(const char* im_path,float* buffer, const int Nx, const int Ny){

        // Create a CV_32F Mat from the buffer (no copy, wraps memory)
        cv::Mat floatImage(Nx, Ny, CV_32F, buffer);

        // Convert to 8-bit grayscale for saving or displaying
        cv::Mat grayImage;
        floatImage.convertTo(grayImage, CV_8U); // scale from [0,1] → [0,255]

        // Save or display
        cv::imwrite(im_path, grayImage);
}

int main(int argc, char* argv[]) {

  // Check if exactly one argument is passed (excluding program name)
  if (argc != 2) {
      std::cerr << "Usage: " << argv[0] << " <filename>\n";
      return 1;
  }
  const char* filename = argv[1];
  std::ifstream file(filename);

  // Check if the file can be opened
  if (!file) {
      std::cerr << "Error: '" << filename << "' is not a valid file.\n";
      return EXIT_FAILURE;
  }

  int Nx;
  int Ny;
  float* buffer = load_image(filename,&Nx,&Ny);
  float* bufferf = (float*)malloc(sizeof(float)*Nx*Ny);
  std::cout<<"The image has "<< Nx <<" rows and "<< Ny <<" cols"<<std::endl; 

  float kernel[(KERNEL_SIZE*KERNEL_SIZE)]={0, 1,0,
                                           1,-4,1,
                                           0, 1,0};
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

    sycl::queue queue (selector);

    auto device = queue.get_device();
    std::cout << "Running on device: "
              << device.get_info<sycl::info::device::name>().c_str()
              << std::endl;

    {
        sycl::buffer<float,2> img_buffer{buffer,sycl::range<2>(Nx,Ny)};
        sycl::buffer<float,2> imgf_buffer{bufferf,sycl::range<2>(Nx,Ny)};
        sycl::buffer<float> kernel_buffer{kernel,sycl::range((KERNEL_SIZE*KERNEL_SIZE))};
        queue.submit([&](sycl::handler &h) {
            sycl::accessor aimg{img_buffer, h, sycl::read_only};
            sycl::accessor akernel{kernel_buffer,h, sycl::read_only};
            sycl::accessor aimgf{imgf_buffer, h,sycl::write_only, sycl::no_init};
            h.single_task<ConvolutionID>([=]() {

            // Copy to local array
            float local_kernel[(KERNEL_SIZE*KERNEL_SIZE)];
            #pragma unroll KERNEL_SIZE*KERNEL_SIZE
            for(int i = 0; i<=(KERNEL_SIZE*KERNEL_SIZE); i++){
                   local_kernel[i]=akernel[i]; 
            }
            float sum = 0;
            int center = (KERNEL_SIZE-1)/2;
            int ii, jj;
            for (int i = center; i<(Nx-center); i++){
                for (int j = center; j<(Ny-center); j++){
                    sum = 0;
                    #pragma unroll KERNEL_SIZE*KERNEL_SIZE
                    for(int k = 0; k<(KERNEL_SIZE*KERNEL_SIZE); k++){
	                jj = (k%j) + j - center;
	                ii = (k/i) + i - center;
	                sum+=aimg[ii][jj]*local_kernel[k];
                    }
                    aimgf[i][j] = sum;
                }
            }

            });

        });

  }

  std::cout<< " Saving image  " << std::endl;
  save_image("output.png",bufferf,Nx,Ny);
  free(buffer);
  free(bufferf);





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
