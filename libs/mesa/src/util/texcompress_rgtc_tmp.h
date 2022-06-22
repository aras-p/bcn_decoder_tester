/*
 * Copyright (C) 2011 Red Hat Inc.
 *
 * block compression parts are:
 * Copyright (C) 2004  Roland Scheidegger   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author:
 *    Dave Airlie
 */

/* included by texcompress_rgtc to define byte/ubyte compressors */

void TAG(fetch_texel_rgtc)(unsigned srcRowStride, const TYPE *pixdata,
	                   unsigned i, unsigned j, TYPE *value, unsigned comps)
{
   TYPE decode;
   const TYPE *blksrc = (pixdata + ((srcRowStride + 3) / 4 * (j / 4) + (i / 4)) * 8 * comps);
   const TYPE alpha0 = blksrc[0];
   const TYPE alpha1 = blksrc[1];
   const char bit_pos = ((j&3) * 4 + (i&3)) * 3;
   const unsigned char acodelow = blksrc[2 + bit_pos / 8];
   const unsigned char acodehigh = (3 + bit_pos / 8) < 8 ? blksrc[3 + bit_pos / 8] : 0;
   const unsigned char code = (acodelow >> (bit_pos & 0x7) |
      (acodehigh  << (8 - (bit_pos & 0x7)))) & 0x7;

   if (code == 0)
      decode = alpha0;
   else if (code == 1)
      decode = alpha1;
   else if (alpha0 > alpha1)
      decode = ((alpha0 * (8 - code) + (alpha1 * (code - 1))) / 7);
   else if (code < 6)
      decode = ((alpha0 * (6 - code) + (alpha1 * (code - 1))) / 5);
   else if (code == 6)
      decode = T_MIN;
   else
      decode = T_MAX;

   *value = decode;
}
