# CUDA C++ convolution.

This repository contains a CUDA C++ application that applies a kernel filter. I used a default set of values to represent the convolution kernel (simulating only one CNN layer).

## Compile the application

```
sudo apt install libpng++-dev
make
```
to clean the exe. Please run:
```
make
```

# Usage: 

```
./bin/kernel_convolution image_path kernel_size std_gaussian
```

Some examples:

```
./bin/kernel_convolution data/gauss.png 3 1
./bin/kernel_convolution data/gauss.png 5 3
./bin/kernel_convolution data/gauss.png 15 10
```
