#include "kernel.h"
#include <iostream>
#include <cmath>


Kernel::Kernel()
{
	this->m_filterMatrix = std::vector<float>(0);
	this->m_filterWidth = 0;
	this->m_filterHeight = 0;
}

bool Kernel::setGaussianFilter(const int height, const int width, const float stdDev)
{
    std::cout << "Building kernel (testing a Gaussian)..." << std::endl;

    if (height != width || height % 2 == 0 || width % 2 == 0) {
        std::cerr << "Invalid Height and Width" << std::endl;

        return false;
    }

    if (stdDev <= 0) {
        std::cerr << "Standard deviation value is not valid" << std::endl;
        std::cerr << "Standard deviation value must be positive" << std::endl;

        return false;
    }

    std::vector<float> kernel(width * height);
    // Random filter. this isnot useful for timing measuring
    //for (int i = 0; i < height; i++) {
    //    for (int j = 0; j < width; j++) {
    //    kernel[j + i * width] = 0.5;
    //    }
    //}

    float sum = 0.0;
    int middleHeight = static_cast<int>(height / 2);
    int middleWidth = static_cast<int>(width / 2);

    // Gaussian filter
    for (int i = -middleHeight; i <= middleHeight; i++) {
        for (int j = -middleWidth; j <= middleWidth; j++) {
            float cellValue = exp(- (i * i + j * j) / (2 * stdDev * stdDev)) / (2 * M_PI * stdDev * stdDev);
            kernel[(j + middleWidth) + ( i + middleHeight) * width] = cellValue;
            sum += cellValue;
        }
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            kernel[j + i * width] /= sum;
        }
    }

    m_filterMatrix = kernel;
    m_filterWidth = width;
    m_filterHeight = height;

    return true;
}

bool Kernel::buildKernelCommon(std::vector<float> &kernel, int max, int min, int height, int width)
{
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if ((i == static_cast<int>(height / 2)) && (j == static_cast<int>(width / 2))) {
                kernel[j + i * width] = max;
            }
            else {
                kernel[j + i * width] = min;
            }
        }
    }

    return true;
}

int Kernel::getKernelWidth() const
{
    return m_filterWidth;
}

int Kernel::getKernelHeight() const
{
    return m_filterHeight;
}

std::vector<float> Kernel::getKernel() const
{
    return this->m_filterMatrix;
}

void Kernel::printKernel() const
{
    int height = m_filterHeight;
    int width = m_filterWidth;
    std::cout << std::endl;
    std::cout << "=== Kernel ===" << std::endl;
     for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            std::cout << static_cast<float>(m_filterMatrix[j + i * width]) << " ";
        }
        std::cout << "" << std::endl;
    }
    std::cout << "==============" << std::endl;
    std::cout << std::endl;
}
