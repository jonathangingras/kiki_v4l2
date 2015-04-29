/* Copyright (c) 2014-2015, Jonathan Gingras
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 */

/* 
 * based on http://en.wikipedia.org/wiki/YUV
 */
#include <kiki_v4l2.h>

#define WRAP_TO_255(x) ( x >= 0xFF ? 0xFF : ( x <= 0x00 ? 0x00 : x ) )

void kiki_v4l2_YUV422toRGB888(kiki_v4l2_image_size_t image_size, raw_t *dst, raw_t *src) {
  int line, column;
  raw_t *y = src, *u = src + 1, *v = src + 3;

  for (line = 0; line < image_size.height; ++line) {
    for(column = 0; column < image_size.width; ++column) {
      *dst++ = WRAP_TO_255((double)*y + 1.402*((double)*v-128.0));
      *dst++ = WRAP_TO_255((double)*y - 0.344*((double)*u-128.0) - 0.714*((double)*v-128.0));      
      *dst++ = WRAP_TO_255((double)*y + 1.772*((double)*u-128.0));
      y += 2;
      if((column & 1) == 1) {
        u += 4;
        v += 4;
      }
    }
  }
}