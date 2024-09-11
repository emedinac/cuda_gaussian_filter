#include <iostream>
#include <chrono>
#include "image.h"
#include "kernel.h"

#define OUTPUT_FOLDER   "output/"
#define OUTPUT_EXT       ".png"

int main(int argc, char **argv)
{
	std::cout << "===== Multithread kernel convolution =====" << std::endl;

	// Check command line parameters
	if (argc < 4) {
		std::cerr << "Usage: " << argv[0] << " filter_type image_path cuda_mem_tye" << std::endl;
	    std::cerr << "image path " << std::endl;
	    std::cerr << "kernel size e.g. 3,5,7,..." << std::endl;
		std::cerr << "std for the gaussian kernel" << std::endl;
	    return 1;
	}

    Kernel filter = Kernel();
	filter.setGaussianFilter(atoi(argv[2]), atoi(argv[2]), std::stof(argv[3]));
	filter.printKernel();

	Image img;
	bool loadResult = img.loadImage(argv[1]);
	if (!loadResult) {
		std::cerr << "Unable to load image " << argv[1] << std::endl;
		return 1;
	}

	Image newMtImg;
	Image newNpImg;

	// Init the CUDA device
	cudaFree(0);

	// Executing multithread filtering for each image
	auto t1 = std::chrono::high_resolution_clock::now();
	bool cudaResult = img.multithreadFilter(newMtImg, filter);
	auto t2 = std::chrono::high_resolution_clock::now();

	std::cout << std::endl;

	auto t3 = std::chrono::high_resolution_clock::now();
	bool Result = img.applyFilter(newNpImg, filter);
	auto t4 = std::chrono::high_resolution_clock::now();

	std::cout << std::endl;

	// Evaluating execution times and save results
	if (cudaResult) {
		auto multithreadDuration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		std::cout << "Total CUDA Execution time: " << multithreadDuration << " μs" << std::endl;
		newMtImg.saveImage(std::string(std::string(OUTPUT_FOLDER) +  "result" + std::string(OUTPUT_EXT)).c_str());
	}

	if (Result) {
		auto singleDuration = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
		std::cout << "Total CPU Execution time: " << singleDuration << " μs" << std::endl;
	}
}
