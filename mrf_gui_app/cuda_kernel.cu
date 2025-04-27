// cuda_kernel.cu
#include <cuda_runtime.h>
#include <iostream>

__global__ void squareKernel(float* d_out, float* d_in, int size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) {
        d_out[idx] = d_in[idx] * d_in[idx];
    }
}

extern "C" void squareArray(float* h_out, float* h_in, int size) {
    float* d_in;
    float* d_out;
    size_t bytes = size * sizeof(float);

    cudaMalloc(&d_in, bytes);
    cudaMalloc(&d_out, bytes);

    cudaMemcpy(d_in, h_in, bytes, cudaMemcpyHostToDevice);

    int blockSize = 256;
    int gridSize = (size + blockSize - 1) / blockSize;
    squareKernel<<<gridSize, blockSize>>>(d_out, d_in, size);

    cudaMemcpy(h_out, d_out, bytes, cudaMemcpyDeviceToHost);

    cudaFree(d_in);
    cudaFree(d_out);
}
