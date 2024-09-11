#include <math.h>
#include <png++/png.hpp>


#include "image.h"
#include "gpu_convolution.h"


Image::Image()
{
	imWidth = 0;
	imHeight = 0;
}

int Image::getImageWidth() const
{
    return imWidth;
}

int Image::getImageHeight() const
{
    return imHeight;
}

int Image::getImageChannels() const
{
    return 1;
}

bool Image::setImage(const std::vector<float>& src, int width, int height)
{
    this->im = src;
    this->imWidth = width;
    this->imHeight = height;

    return true;
}

std::vector<float> Image::getImage() const
{
    return this->im;
}

bool Image::loadImage(const char *filename)
{
    // Load image
    png::image<png::gray_pixel> image(filename);

    // Build matrix from image
    imHeight = image.get_height();
    imWidth = image.get_width();
    std::vector<float> imageMatrix(imHeight * imWidth);

    for (unsigned int h = 0; h < image.get_height(); h++) {
        for (unsigned int w = 0; w < image.get_width(); w++) {
            imageMatrix[w + h * imWidth] = image[h][w];
        }
    }

    im = imageMatrix;

    return true;
}

bool Image::saveImage(const char *filename) const
{
    int height = this->getImageHeight();
    int width = this->getImageWidth();

   png::image<png::gray_pixel> imageFile(width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            imageFile[y][x] = im[x + y * width];
        }
    }
    imageFile.write(filename);

    std::cout << "Image saved in " << std::string(filename) << std::endl;

    return true;
}

std::vector<float> Image::buildReplicatePaddedImage(const int paddingHeight, const int paddingWidth) 
const{
    int height = this->getImageHeight();
    int width = this->getImageWidth();

    int padH = height + paddingHeight * 2;
    int padW = width + paddingWidth * 2;
    int imgH = height - 1;
    int imgW = width - 1;
    int idx = 0;

    std::vector<float> paddedImage(padH * padW);
    std::vector<float> srcImage = this->im;

    for (int h = 0; h < padH; h++) {
    	idx = h * padW;
    	for (int w = 0; w < padW; w++) {
            if ((h < paddingHeight) && (w < paddingWidth)) {
            	paddedImage[w + idx] = srcImage[0];
            }
            else if ((h > imgH) && (w > imgW)) {
               paddedImage[w + idx] = srcImage[(width - 1) + (height - 1) * width];
            }
            else if ((h < paddingHeight) && (w > imgW)) {
               paddedImage[w + idx] = srcImage[(width - 1) + (0) * width];
            }
            else if ((w < paddingWidth) && (h > imgH)) {
               paddedImage[w + idx] = srcImage[(0) + (height - 1) * width];
            }
            else if (h < paddingHeight) {
               paddedImage[w + idx] = srcImage[(w) + (0) * width];
            }
            else if (w < paddingWidth) {
               paddedImage[w + idx] = srcImage[(0) + (h) * width];
            }
            else if (h > imgH) {
               paddedImage[w + idx] = srcImage[(w) + (height - 1) * width];
            }
            else if (w > imgW) {
               paddedImage[w + idx] = srcImage[(width - 1) + (h) * width];
            }
            else {
               paddedImage[w + idx] = srcImage[(w - paddingWidth) + (h - paddingHeight) * width];
            }
        }
    }

    srcImage.clear();

    return paddedImage;
}

bool Image::applyFilter(Image& resultingImage, const Kernel& kernel) const
{
    std::cout << "Applying filter" << std::endl;
    std::vector<float> newImage = filterCore(kernel);
    resultingImage.setImage(newImage, imWidth, imHeight);
    std::cout << "Done!" << std::endl;
    newImage.clear();
    return true;
}

bool Image::applyFilter(const Kernel& kernel)
{
    std::cout << "Applying filter" << std::endl;
    std::vector<float> newImage = filterCore(kernel);
     if (newImage.empty()) {
        return false;
    }
    this->setImage(newImage, imWidth, imHeight);
    std::cout << "Done!" << std::endl;

    newImage.clear();

    return true;
}

std::vector<float> Image::filterCore(const Kernel& kernel) const
{
    // Get image dimensions
    int channels = this->getImageChannels();
    int height = this->getImageHeight();
    int width = this->getImageWidth();

    // Get filter dimensions
    int filterHeight = kernel.getKernelHeight();
    int filterWidth = kernel.getKernelWidth();

    // Checking image channels and kernel size
    if (channels != 1) {
        std::cerr << "Invalid number of image's channels" << std::endl;
          return std::vector<float>();
    }

     if (filterHeight == 0 || filterWidth == 0) {
        std::cerr << "Invalid filter dimension" << std::endl;
        return std::vector<float>();
    }

    // Input padding w.r.t. filter size
    auto t1 = std::chrono::high_resolution_clock::now();
    std::vector<float> paddedImage = buildReplicatePaddedImage(floor(filterHeight/2), floor(filterWidth/2));
    auto t2 = std::chrono::high_resolution_clock::now();
    auto paddingDuration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::cout << "Padding Execution time: " << paddingDuration << " μs" << std::endl;

    std::vector<float> newImage(height * width);

    // Get kernel matrix
    std::vector<float> mask = kernel.getKernel();

    // Get pointers to matrixes
    const float* maskPtr = {mask.data()};
    const float* paddedImagePtr = {paddedImage.data()};

    int paddedWidth = width + floor(filterWidth / 2) * 2;
    int s = floor(filterWidth/2);
    float pixelSum = 0;

    int filterRowIndex = 0;
    int sourceImgRowIndex = 0;
    int outImgRowIndex = 0;

    t1 = std::chrono::high_resolution_clock::now();
    // Apply convolution
    for (int i = s; i < height + s; i++) {
    	outImgRowIndex = (i - s) * width;
        for (int j = s; j < width + s; j++) {
        	for (int h = -s;  h <= s; h++) {
        		filterRowIndex = (h + s) * filterWidth;
                sourceImgRowIndex = (h + i) * paddedWidth;
                for (int w = -s; w <= s; w++) {
                	pixelSum += maskPtr[(w + s) + filterRowIndex] *
                                paddedImagePtr[w + j + sourceImgRowIndex];
                }
        	}
            if (pixelSum < 0) {
            	pixelSum = 0;
            }
            else if (pixelSum > 255) {
            	pixelSum = 255;
            }
            newImage[(j - s) + outImgRowIndex] = pixelSum;
            pixelSum = 0;
        }
    }
    t2 = std::chrono::high_resolution_clock::now();
    // Evaluating execution times
    auto filterDuration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::cout << "Sequential filtering execution time: " << filterDuration << " μs" << std::endl;

    paddedImage.clear();
    mask.clear();

    return newImage;
}

bool Image::multithreadFilter(Image& resultingImage, const Kernel& kernel)
{
    std::cout << "Applying multithread filter to image" << std::endl;

    // Get image dimensions
    // int channels = this->getImageChannels();
    int height = this->getImageHeight();
    int width = this->getImageWidth();

    // Get filter dimensions
    int filterHeight = kernel.getKernelHeight();
    int filterWidth = kernel.getKernelWidth();

    // Input padding w.r.t. filter size
    auto t1 = std::chrono::high_resolution_clock::now();
    std::vector<float> paddedImage = buildReplicatePaddedImage(floor(filterHeight/2), floor(filterWidth/2));
    auto t2 = std::chrono::high_resolution_clock::now();
    auto paddingDuration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::cout << "Applying Padding time: " << paddingDuration << " μs" << std::endl;

    std::vector<float> newImage(height * width);
    std::vector<float> Mkernel = kernel.getKernel();

    // Get pointers to matrixes
    const float* MkernelPtr = {Mkernel.data()};
    const float* paddedImagePtr = {paddedImage.data()};
    float* newImagePtr = {newImage.data()};

    bool result = false;
    result = runKernel(paddedImagePtr, newImagePtr, MkernelPtr, width, height,
                        width + floor(filterWidth / 2) * 2, height + floor(filterHeight / 2) * 2, filterWidth, filterHeight);

     if (!result) {
    	std::cerr << "Error while executing CUDA filtering" << std::endl;
    	paddedImage.clear();
    	newImage.clear();
    	Mkernel.clear();

    	return false;
    }

    resultingImage.setImage(newImage, imWidth, imHeight);

    std::cout << "Done!" << std::endl;

    paddedImage.clear();
    newImage.clear();
    Mkernel.clear();

    return true;
}
