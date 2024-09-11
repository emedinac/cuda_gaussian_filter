#ifndef IMAGE_H_
#define IMAGE_H_

#include <vector>
#include <thread>
#include "kernel.h"

class Image
{
    public:
        Image();
        ~Image() {
            im.clear();
            std::vector<float>().swap(im);
        }

        int getImageWidth() const;
        int getImageHeight() const;
        int getImageChannels() const;
        bool setImage(const std::vector<float>& src, int width, int height);

        std::vector<float> getImage() const;

        bool loadImage(const char *filename);
        bool saveImage(const char *filename) const;
        bool applyFilter(Image& resultingImage, const Kernel& kernel) const;
        bool applyFilter(const Kernel& kernel);
        bool multithreadFilter(Image& resultingImage, const Kernel& kernel);

    private:
        std::vector<float> filterCore(const Kernel& kernel) const;
        std::vector<float> buildReplicatePaddedImage(const int paddingHeight,
                                                    const int paddingWidth) const;
        std::vector<float> buildZeroPaddingImage(const int paddingHeight,
                                                const int paddingWidth) const;

        std::vector<float> im;
        int imWidth;
        int imHeight;
};

#endif /* IMAGE_H_ */
