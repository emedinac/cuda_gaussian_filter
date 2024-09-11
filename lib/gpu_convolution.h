#ifndef GPU_CONVOLUTION_H_
#define GPU_CONVOLUTION_H_


bool runKernel(const float* srcImage, float* outImage, const float* mask, int width, int height, int paddedWidth, int paddedHeight, int filterWidth, int filterHeight);

#endif /* GPU_CONVOLUTION_H_ */
