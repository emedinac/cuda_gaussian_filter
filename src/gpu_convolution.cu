#include <chrono>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>


#include "gpu_convolution.h"
#include "cuda.h"

#define BLOCK_WIDTH 	32
#define BLOCK_HEIGHT	32

unsigned int divUp(const unsigned int& a, const unsigned int& b)
{
	if (a % b != 0) {
		return a / b + 1;
	}
	else {
		return a / b;
	}
}

const unsigned int MAX_FILTER_SIZE = 51;
__device__ __constant__ float d_cFilterKernel[MAX_FILTER_SIZE * MAX_FILTER_SIZE];

__global__ void applykernel(float* d_srcImagePtr, float* d_maskPtr, float* d_outImagePtr,
									int width, int height, int paddedWidth, int paddedHeight,
									int filterWidth, int filterHeight)
{
	const int s = floor(static_cast<float>(filterWidth) / 2);
	const int i = blockIdx.y * blockDim.y + threadIdx.y + s;
	const int j = blockIdx.x * blockDim.x + threadIdx.x + s;

	unsigned int filterRowIndex = 0;
	unsigned int srcImgRowIndex = 0;
	unsigned int srcImgIndex = 0;
	unsigned int maskIndex = 0;
	float pixelSum = 0;

	// Check out of bounds thread idx
	if( j >= s && j < paddedWidth - s &&
			i >= s && i < paddedHeight - s) {

		int outPixelPos = (j - s) + (i - s) * width;

		// Apply convolution
		for (int h = -s;  h <= s; h++) {
			filterRowIndex = (h + s) * filterWidth;
	    	srcImgRowIndex = (h + i) * paddedWidth;
	    	for (int w = -s; w <= s; w++) {
	    		srcImgIndex = w + j + srcImgRowIndex;
	    		maskIndex = (w + s) + filterRowIndex;
	    		pixelSum += d_srcImagePtr[srcImgIndex] * d_maskPtr[maskIndex];
	    	}
		}

		// Thresholding overflowing pixel's values
		if (pixelSum < 0) {
			pixelSum = 0;
		}
		else if (pixelSum > 255) {
			pixelSum = 255;
		}

		// Write pixel on the output image
		d_outImagePtr[outPixelPos] = pixelSum;
		pixelSum = 0;
	}
}

bool runKernel(const float* srcImage, float* outImage, const float* mask, int width, int height, 
                int paddedWidth, int paddedHeight, int filterWidth, int filterHeight) // magic happen here.
{
	std::cout << "Starting CUDA global memory convolution" << std::endl;

	const int blockWidth = BLOCK_WIDTH;
	const int blockHeight = BLOCK_HEIGHT;

	float *d_srcImagePtr;
	float *d_outImagePtr;
	float *d_maskPtr;

	const int srcImgSize = sizeof(float) * paddedWidth * paddedHeight;
	const int maskSize = sizeof(float) * filterWidth * filterHeight;
	const int outImageSize = sizeof(float) * width * height;

	int copyDuration = 0;
	auto t3 = std::chrono::high_resolution_clock::now();

	// Allocate device memory for images and filter
	cudaMalloc(reinterpret_cast<void**>(&d_srcImagePtr), srcImgSize);
	cudaMalloc(reinterpret_cast<void**>(&d_maskPtr), maskSize);
	cudaMalloc(reinterpret_cast<void**>(&d_outImagePtr), outImageSize);

	auto t4 = std::chrono::high_resolution_clock::now();
	copyDuration += std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

	// cudaError_t err = cudaGetLastError();

	t3 = std::chrono::high_resolution_clock::now();

	// Transfer data from host to device memory
	cudaMemcpy(d_srcImagePtr, srcImage, srcImgSize, cudaMemcpyHostToDevice);
	cudaMemcpy(d_maskPtr, mask, maskSize, cudaMemcpyHostToDevice);

	t4 = std::chrono::high_resolution_clock::now();
	copyDuration += std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

	// Allocates block size and grid size
	dim3 threadsPerBlock(blockWidth, blockHeight);
	dim3 blocksPerGrid(divUp(width, blockWidth), divUp(height, blockHeight));

	auto t1 = std::chrono::high_resolution_clock::now();

	applykernel<<<blocksPerGrid, threadsPerBlock>>>(d_srcImagePtr, d_maskPtr, d_outImagePtr, width,  height,  paddedWidth,  paddedHeight, filterWidth,  filterHeight);

	// Waits for threads to finish work
	cudaDeviceSynchronize();

	auto t2 = std::chrono::high_resolution_clock::now();
	auto filterDuration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	std::cout << "execution time: " << filterDuration << " μs" << std::endl;

	t3 = std::chrono::high_resolution_clock::now();

	// Transfer resulting image back
	cudaMemcpy(outImage, d_outImagePtr, outImageSize, cudaMemcpyDeviceToHost);

	t4 = std::chrono::high_resolution_clock::now();
	copyDuration += std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
	std::cout << "Copy time: " << copyDuration << " μs" << std::endl;

	cudaFree(d_srcImagePtr);
	cudaFree(d_maskPtr);
	cudaFree(d_outImagePtr);

	return true;
}