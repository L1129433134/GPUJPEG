/**
 * Copyright (c) 2011, Martin Srom
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
 
#include "jpeg_preprocessor.h"
#include "jpeg_util.h"

#define RGB_8BIT_THREADS 256

/**
 * Kernel - Copy image source data into three separated component buffers
 *
 * @param d_c1  First component buffer
 * @param d_c2  Second component buffer
 * @param d_c3  Third component buffer
 * @param d_source  Image source data
 * @param pixel_count  Number of pixels to copy
 * @return void
 */
__global__ void d_rgb_to_comp(uint8_t* d_c1, uint8_t* d_c2, uint8_t* d_c3, const uint8_t* d_source, int pixel_count)
{
    int x  = threadIdx.x;
    int gX = blockDim.x * blockIdx.x;

    __shared__ unsigned char s_data[RGB_8BIT_THREADS * 3];

    if ( (x * 4) < RGB_8BIT_THREADS * 3 ) {
        int* s = (int*)d_source;
        int* d = (int*)s_data;
        d[x] = s[((gX * 3) >> 2) + x];
    }
    __syncthreads();

    int offset = x * 3;
    int r1 = (int)(s_data[offset]);
    int r2 = (int)(s_data[offset + 1]);
    int r3 = (int)(s_data[offset + 2]);
    int globalOutputPosition = gX + x;
    if ( globalOutputPosition < pixel_count ) {
        d_c1[globalOutputPosition] = (uint8_t)r1;
        d_c2[globalOutputPosition] = (uint8_t)r2;
        d_c3[globalOutputPosition] = (uint8_t)r3;
    }
}

/** Documented at declaration */
int
jpeg_preprocessor_process(struct jpeg_encoder* encoder, uint8_t* image)
{
    int pixel_count = encoder->width * encoder->height;
    int alignedSize = (pixel_count / RGB_8BIT_THREADS + 1) * RGB_8BIT_THREADS * 3;
    
    // Copy image to device memory
    if ( cudaSuccess != cudaMemcpy(encoder->d_data_source, image, pixel_count * 3 * sizeof(uint8_t), cudaMemcpyHostToDevice) )
        return NULL;
        
    // Kernel
    dim3 threads (RGB_8BIT_THREADS);
    dim3 grid (alignedSize / (RGB_8BIT_THREADS * 3));
    assert(alignedSize % (RGB_8BIT_THREADS * 3) == 0);

    uint8_t* d_c1 = &encoder->d_data[0 * pixel_count];
    uint8_t* d_c2 = &encoder->d_data[1 * pixel_count];
    uint8_t* d_c3 = &encoder->d_data[2 * pixel_count];
    d_rgb_to_comp<<<grid, threads>>>(d_c1, d_c2, d_c3, encoder->d_data_source, pixel_count);
    
    cudaError cuerr = cudaThreadSynchronize();
    if ( cuerr != cudaSuccess ) {
        fprintf(stderr, "Preprocessing failed: %s!\n", cudaGetErrorString(cuerr));
        return -1;
    }
    
    return 0;
}