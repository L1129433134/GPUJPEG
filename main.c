/* 
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

#include "jpeg_common.h"
#include "jpeg_encoder.h"
#include "jpeg_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <strings.h>

void
print_help() 
{
    printf(
        "jpeg_compress [options] img.rgb\n"
        "   -h, --help\t\tprint help\n"
        "   -s, --size\t\timage size in pixels, e.g. 1920x1080\n"
        "   -q, --quality\t\tquality level 1-100 (default 75)\n"
    );
}

int
main(int argc, char *argv[])
{       
    struct option longopts[] = {
        {"help",    no_argument,       0, 'h'},
        {"size",    required_argument, 0, 's'},
        {"quality", required_argument, 0, 'q'},
    };

    // Parameters
    int width = 0;
    int height = 0;
    int quality = 75;
    
    // Parse command line
    char ch = '\0';
    int optindex = 0;
    char* pos = 0;
    while ( (ch = getopt_long(argc, argv, "hs:q:", longopts, &optindex)) != -1 ) {
        switch (ch) {
        case 'h':
            print_help();
            return 0;
        case 's':
            width = atoi(optarg);
            pos = strstr(optarg, "x");
            if ( pos == NULL || width == 0 || (strlen(pos) >= strlen(optarg)) ) {
                print_help();
                return -1;
            }
            height = atoi(pos + 1);
            break;
        case 'q':
            quality = atoi(optarg);
            if ( quality <= 0 )
                quality = 1;
            if ( quality > 100 )
                quality = 100;
            break;
        case '?':
            return -1;
        default:
            print_help();
            return -1;
        }
    }
	argc -= optind;
	argv += optind;
    
    // Source image must be presented
    if ( argc < 2 ) {
        printf("Please supply source and destination image filename!\n");
        print_help();
        return -1;
    }
    
    // Create encoder
    struct jpeg_encoder* encoder = jpeg_encoder_create(width, height, quality);
    if ( encoder == NULL ) {
        fprintf(stderr, "Failed to create encoder!\n");
        return -1;
    }
    
    // Encode images
    for ( int index = 0; index < argc; index += 2 ) {
        TIMER_INIT();    
        
        TIMER_START();
    
        // Load image
        uint8_t* image = NULL;
        if ( jpeg_image_load_from_file(argv[index], width, height, &image) != 0 ) {
            fprintf(stderr, "Failed to load image [%s]!\n", argv[index]);
            return -1;
        }
        
        TIMER_STOP_PRINT("Load Image:     ");
        TIMER_START();
            
        // Encode image
        uint8_t* image_compressed = NULL;
        int image_compressed_size = 0;
        if ( jpeg_encoder_encode(encoder, image, &image_compressed, &image_compressed_size) != 0 ) {
            fprintf(stderr, "Failed to encode image [%s]!\n", argv[index]);
            return -1;
        }
        
        TIMER_STOP_PRINT("Encode Image:   ");
        TIMER_START();
        
        // Save image
        if ( jpeg_image_save_to_file(argv[index + 1], image_compressed, image_compressed_size) != 0 ) {
            fprintf(stderr, "Failed to save image [%s]!\n", argv[index]);
            return -1;
        }
        
        TIMER_STOP_PRINT("Save Image:     ");
        
        printf("Compressed Size: %d bytes\n", image_compressed_size);
        
        // Destroy image
        jpeg_image_destroy(image);
    }
    
    // Destroy encoder
    jpeg_encoder_destroy(encoder);
    
	return 0;
}