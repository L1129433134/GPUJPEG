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

#ifndef JPEG_ENCODER
#define JPEG_ENCODER

#include "jpeg_table.h"
#include "jpeg_writer.h"

/**
 * JPEG encoder structure
 */
struct jpeg_encoder
{  
    // Image width
    int width;
    // Image height
    int height;
    // Component count
    int comp_count;
    // Quality level (0-100)
    int quality;
    
    // Source image data in device memory
    uint8_t* d_data_source;
    
    // Preprocessed data in device memory
    uint8_t* d_data;
    
    // Data after DCT and quantization in device memory
    int16_t* d_data_quantized;
    
    // Table for luminance [0] and chrominance [1] color component
    struct jpeg_table* table[2];
    
    // JPEG writer structure
    struct jpeg_writer* writer;
};

/**
 * Create JPEG encoder
 * 
 * @param width  Width of encodable images
 * @param height  Height of encodable images
 * @return encoder structure if succeeds, otherwise NULL
 */
struct jpeg_encoder*
jpeg_encoder_create(int width, int height, int quality);

/**
 * Compress image by encoder
 * 
 * @param encoder  Encoder structure
 * @param image  Source image data
 * @param image_compressed  Pointer to variable where compressed image data buffer will be placed
 * @param image_compressed_size  Pointer to variable where compressed image size will be placed
 * @return 0 if succeeds, otherwise nonzero
 */
int
jpeg_encoder_encode(struct jpeg_encoder* encoder, uint8_t* image, uint8_t** image_compressed, int* image_compressed_size);

/**
 * Destory JPEG encoder
 * 
 * @param encoder  Encoder structure
 * @return 0 if succeeds, otherwise nonzero
 */
int
jpeg_encoder_destroy(struct jpeg_encoder* encoder);

#endif // JPEG_ENCODER