/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/*
 * yuv2rgb.c
 *
 * This file is part of xine, a unix video player.
 *
 * based on work from mpeg2dec:
 * Copyright (C) 1999-2001 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
 *
 * mpeg2dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * mpeg2dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with c_this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: yuv2rgb.c,v 1.3 2002/09/02 17:18:30 harlekin Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "yuv2rgb.h"


static scale_line_func_t find_scale_line_func(int step);


const int32_t Inverse_Table_6_9[8][4] = {
  {117504, 138453, 13954, 34903}, /* no sequence_display_extension */
  {117504, 138453, 13954, 34903}, /* ITU-R Rec. 709 (1990) */
  {104597, 132201, 25675, 53279}, /* unspecified */
  {104597, 132201, 25675, 53279}, /* reserved */
  {104448, 132798, 24759, 53109}, /* FCC */
  {104597, 132201, 25675, 53279}, /* ITU-R Rec. 624-4 System B, G */
  {104597, 132201, 25675, 53279}, /* SMPTE 170M */
  {117579, 136230, 16907, 35559}  /* SMPTE 240M (1987) */
};


static void *my_malloc_aligned (size_t alignment, size_t size, void **chunk) {

  char *pMem;

  pMem = (char *)malloc(size+alignment);

  *chunk = pMem;

  while ((int) pMem % alignment)
    pMem++;

  return pMem;
}


static int yuv2rgb_configure (yuv2rgb_t *c_this, 
			      int source_width, int source_height,
			      int y_stride, int uv_stride,
			      int dest_width, int dest_height,
			      int rgb_stride) {
  /*
  printf ("yuv2rgb setup (%d x %d => %d x %d)\n", source_width, source_height,
	  dest_width, dest_height);
	  */
  c_this->source_width  = source_width;
  c_this->source_height = source_height;
  c_this->y_stride      = y_stride;
  c_this->uv_stride     = uv_stride;
  c_this->dest_width    = dest_width;
  c_this->dest_height   = dest_height;
  c_this->rgb_stride    = rgb_stride;
  
  if (c_this->y_chunk) {
    free (c_this->y_chunk);
    c_this->y_buffer = (uint8_t *)c_this->y_chunk = 0;
  }
  if (c_this->u_chunk) {
    free (c_this->u_chunk);
    c_this->u_buffer = (uint8_t *)c_this->u_chunk = 0;
  }
  if (c_this->v_chunk) {
    free (c_this->v_chunk);
    c_this->v_buffer = (uint8_t *)c_this->v_chunk = 0;
  }

  
  c_this->step_dx = source_width  * 32768 / dest_width;
  c_this->step_dy = source_height * 32768 / dest_height;
    
  c_this->scale_line = find_scale_line_func(c_this->step_dx);

  if ((source_width == dest_width) && (source_height == dest_height)) {
    c_this->do_scale = 0;

    /*
     * space for two y-lines (for yuv2rgb_mlib)
     * u,v subsampled 2:1
     */
    c_this->y_buffer = (uint8_t *)my_malloc_aligned (16, 2*dest_width, &c_this->y_chunk);
    if (!c_this->y_buffer)
      return 0;
    c_this->u_buffer = (uint8_t *)my_malloc_aligned (16, (dest_width+1)/2, &c_this->u_chunk);
    if (!c_this->u_buffer)
      return 0;
    c_this->v_buffer = (uint8_t *)my_malloc_aligned (16, (dest_width+1)/2, &c_this->v_chunk);
    if (!c_this->v_buffer)
      return 0;

  } else {
    c_this->do_scale = 1;
    
    /*
     * space for two y-lines (for yuv2rgb_mlib)
     * u,v subsampled 2:1
     */
    c_this->y_buffer = (uint8_t *)my_malloc_aligned (16, 2*dest_width, &c_this->y_chunk);
    if (!c_this->y_buffer)
      return 0;
    c_this->u_buffer = (uint8_t *)my_malloc_aligned (16, (dest_width+1)/2, &c_this->u_chunk);
    if (!c_this->u_buffer)
      return 0;
    c_this->v_buffer = (uint8_t *)my_malloc_aligned (16, (dest_width+1)/2, &c_this->v_chunk);
    if (!c_this->v_buffer)
      return 0;
  }
  return 1;
}


static void scale_line_gen (uint8_t *source, uint8_t *dest,
			    int width, int step) {

  /*
   * scales a yuv source row to a dest row, with interpolation
   * (good quality, but slow)
   */
  int p1;
  int p2;
  int dx;

  p1 = *source++;
  p2 = *source++;
  dx = 0;

  /*
   * the following code has been optimized by Scott Smith <ssmith@akamai.com>:
   *
   * ok now I have a meaningful optimization for yuv2rgb.c:scale_line_gen.
   * it removes the loop from within the while() loop by separating it out
   * into 3 cases: where you are enlarging the line (<32768), where you are
   * between 50% and 100% of the original line (<=65536), and where you are
   * shrinking it by a lot.  anyways, I went from 200 delivered / 100+
   * skipped to 200 delivered / 80 skipped for the enlarging case.  I
   * noticed when looking at the assembly that the compiler was able to
   * unroll these while(width) loops, whereas before it was trying to
   * unroll the while(dx>32768) loops.  so the compiler is better able to
   * deal with c_this code.
   */


  if (step < 32768) {
    while (width) {
      *dest = p1 + (((p2-p1) * dx)>>15);

      dx += step;
      if (dx > 32768) {
	dx -= 32768;
	p1 = p2;
	p2 = *source++;
      }
      
      dest ++;
      width --;
    }
  } else if (step <= 65536) {
    while (width) {
      *dest = p1 + (((p2-p1) * dx)>>15);

      dx += step;
      if (dx > 65536) {
	dx -= 65536;
	p1 = *source++;
	p2 = *source++;
      } else {
	dx -= 32768;
	p1 = p2;
	p2 = *source++;
      }
      
      dest ++;
      width --;
    }
  } else {
    while (width) {
      int offs;

      *dest = p1 + (((p2-p1) * dx)>>15);

      dx += step;
      offs=((dx-1)>>15);
      dx-=offs<<15;
      source+=offs-2;
      p1=*source++;
      p2=*source++;
      dest ++;
      width --;
    }
  }

}

/*
 * Interpolates 16 output pixels from 15 source pixels using shifts.
 * Useful for scaling a PAL mpeg2 dvd input source to 4:3 format on
 * a monitor using square pixels.
 * (720 x 576 ==> 768 x 576)
 */
static void scale_line_15_16 (uint8_t *source, uint8_t *dest,
			      int width, int ) {

  int p1, p2;

  while ((width -= 16) >= 0) {
    p1 = source[0];
    dest[0] = p1;
    p2 = source[1];
    dest[1] = (1*p1 + 7*p2) >> 3;
    p1 = source[2];
    dest[2] = (1*p2 + 7*p1) >> 3;
    p2 = source[3];
    dest[3] = (1*p1 + 3*p2) >> 2;
    p1 = source[4];
    dest[4] = (1*p2 + 3*p1) >> 2;
    p2 = source[5];
    dest[5] = (3*p1 + 5*p2) >> 3;
    p1 = source[6];
    dest[6] = (3*p2 + 5*p1) >> 3;
    p2 = source[7];
    dest[7] = (1*p1 + 1*p1) >> 1;
    p1 = source[8];
    dest[8] = (1*p2 + 1*p1) >> 1;
    p2 = source[9];
    dest[9] = (5*p1 + 3*p2) >> 3;
    p1 = source[10];
    dest[10] = (5*p2 + 3*p1) >> 3;
    p2 = source[11];
    dest[11] = (3*p1 + 1*p2) >> 2;
    p1 = source[12];
    dest[12] = (3*p2 + 1*p1) >> 2;
    p2 = source[13];
    dest[13] = (7*p1 + 1*p2) >> 3;
    p1 = source[14];
    dest[14] = (7*p2 + 1*p1) >> 3;
    dest[15] = p1;
    source += 15;
    dest += 16;
  }

  if ((width += 16) <= 0) return;
  *dest++ = source[0];
  if (--width <= 0) return;
  *dest++ = (1*source[0] + 7*source[1]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[1] + 7*source[2]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[2] + 3*source[3]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[3] + 3*source[4]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[4] + 5*source[5]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[5] + 5*source[6]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[6] + 1*source[7]) >> 1;
  if (--width <= 0) return;
  *dest++ = (1*source[7] + 1*source[8]) >> 1;
  if (--width <= 0) return;
  *dest++ = (5*source[8] + 3*source[9]) >> 3;
  if (--width <= 0) return;
  *dest++ = (5*source[9] + 3*source[10]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[10] + 1*source[11]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[11] + 1*source[12]) >> 2;
  if (--width <= 0) return;
  *dest++ = (7*source[12] + 1*source[13]) >> 3;
  if (--width <= 0) return;
  *dest++ = (7*source[13] + 1*source[14]) >> 3;
 
}


/*
 * Interpolates 53 output pixels from 45 source pixels using shifts.
 * Useful for scaling a NTSC mpeg2 dvd input source to 16:9 display
 * resulution
 * fullscreen resolution, or to 16:9 format on a monitor using square
 * pixels.
 * (720 x 480 ==> 848 x 480)
 */
static void scale_line_45_53 (uint8_t *source, uint8_t *dest,
			      int width, int ) {

  int p1, p2;

  while ((width -= 53) >= 0) {
    p1 = source[0];
    p2 = source[1];
    dest[0] = p1;
    dest[1] = (1*p1 + 7*p2) >> 3;
    p1 = source[2];
    dest[2] = (1*p2 + 3*p1) >> 2;
    p2 = source[3];
    dest[3] = (1*p1 + 1*p2) >> 1;
    p1 = source[4];
    dest[4] = (5*p2 + 3*p1) >> 3;
    p2 = source[5];
    dest[5] = (3*p1 + 1*p2) >> 2;
    p1 = source[6];
    dest[6] = (7*p2 + 1*p1) >> 3;
    dest[7] = p1;
    p2 = source[7];
    dest[8] = (1*p1 + 3*p2) >> 2;
    p1 = source[8];
    dest[9] = (3*p2 + 5*p1) >> 3;
    p2 = source[9];
    dest[10] = (1*p1 + 1*p2) >> 1;
    p1 = source[10];
    dest[11] = (5*p2 + 3*p1) >> 3;
    p2 = source[11];
    dest[12] = (3*p1 + 1*p2) >> 2;
    p1 = source[12];
    dest[13] = p2;
    dest[14] = (1*p2 + 7*p1) >> 3;
    p2 = source[13];
    dest[15] = (1*p1 + 3*p2) >> 2;
    p1 = source[14];
    dest[16] = (3*p2 + 5*p1) >> 3;
    p2 = source[15];
    dest[17] = (5*p1 + 3*p2) >> 3;
    p1 = source[16];
    dest[18] = (3*p2 + 1*p1) >> 2;
    p2 = source[17];
    dest[19] = (7*p1 + 1*p2) >> 3;
    dest[20] = p2;
    p1 = source[18];
    dest[21] = (1*p2 + 7*p1) >> 3;
    p2 = source[19];
    dest[22] = (3*p1 + 5*p2) >> 3;
    p1 = source[20];
    dest[23] = (1*p2 + 1*p1) >> 1;
    p2 = source[21];
    dest[24] = (5*p1 + 3*p2) >> 3;
    p1 = source[22];
    dest[25] = (3*p2 + 1*p1) >> 2;
    p2 = source[23];
    dest[26] = (7*p1 + 1*p2) >> 3;
    dest[27] = (1*p1 + 7*p2) >> 3;
    p1 = source[24];
    dest[28] = (1*p2 + 3*p1) >> 2;
    p2 = source[25];
    dest[29] = (3*p1 + 5*p2) >> 3;
    p1 = source[26];
    dest[30] = (1*p2 + 1*p1) >> 1;
    p2 = source[27];
    dest[31] = (5*p1 + 3*p2) >> 3;
    p1 = source[28];
    dest[32] = (7*p2 + 1*p1) >> 3;
    p2 = source[29];
    dest[33] = p1;
    dest[34] = (1*p1 + 7*p2) >> 3;
    p1 = source[30];
    dest[35] = (1*p2 + 3*p1) >> 2;
    p2 = source[31];
    dest[36] = (3*p1 + 5*p2) >> 3;
    p1 = source[32];
    dest[37] = (5*p2 + 3*p1) >> 3;
    p2 = source[33];
    dest[38] = (3*p1 + 1*p2) >> 2;
    p1 = source[34];
    dest[39] = (7*p2 + 1*p1) >> 3;
    dest[40] = p1;
    p2 = source[35];
    dest[41] = (1*p1 + 3*p2) >> 2;
    p1 = source[36];
    dest[42] = (3*p2 + 5*p1) >> 3;
    p2 = source[37];
    dest[43] = (1*p1 + 1*p2) >> 1;
    p1 = source[38];
    dest[44] = (5*p2 + 3*p1) >> 3;
    p2 = source[39];
    dest[45] = (3*p1 + 1*p2) >> 2;
    p1 = source[40];
    dest[46] = p2;
    dest[47] = (1*p2 + 7*p1) >> 3;
    p2 = source[41];
    dest[48] = (1*p1 + 3*p2) >> 2;
    p1 = source[42];
    dest[49] = (3*p2 + 5*p1) >> 3;
    p2 = source[43];
    dest[50] = (1*p1 + 1*p2) >> 1;
    p1 = source[44];
    dest[51] = (3*p2 + 1*p1) >> 2;
    p2 = source[45];
    dest[52] = (7*p1 + 1*p2) >> 3;
    source += 45;
    dest += 53;
  }

  if ((width += 53) <= 0) return;
  *dest++ = source[0];
  if (--width <= 0) return;
  *dest++ = (1*source[0] + 7*source[1]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[1] + 3*source[2]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[2] + 1*source[3]) >> 1;
  if (--width <= 0) return;
  *dest++ = (5*source[3] + 3*source[4]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[4] + 1*source[5]) >> 2;
  if (--width <= 0) return;
  *dest++ = (7*source[5] + 1*source[6]) >> 3;
  if (--width <= 0) return;
  *dest++ = source[6];
  if (--width <= 0) return;
  *dest++ = (1*source[6] + 3*source[7]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[7] + 5*source[8]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[8] + 1*source[9]) >> 1;
  if (--width <= 0) return;
  *dest++ = (5*source[9] + 3*source[10]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[10] + 1*source[11]) >> 2;
  if (--width <= 0) return;
  *dest++ = source[11];
  if (--width <= 0) return;
  *dest++ = (1*source[11] + 7*source[12]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[12] + 3*source[13]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[13] + 5*source[14]) >> 3;
  if (--width <= 0) return;
  *dest++ = (5*source[14] + 3*source[15]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[15] + 1*source[16]) >> 2;
  if (--width <= 0) return;
  *dest++ = (7*source[16] + 1*source[17]) >> 3;
  if (--width <= 0) return;
  *dest++ = source[17];
  if (--width <= 0) return;
  *dest++ = (1*source[17] + 7*source[18]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[18] + 5*source[19]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[19] + 1*source[20]) >> 1;
  if (--width <= 0) return;
  *dest++ = (5*source[20] + 3*source[21]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[21] + 1*source[22]) >> 2;
  if (--width <= 0) return;
  *dest++ = (7*source[22] + 1*source[23]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[22] + 7*source[23]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[23] + 3*source[24]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[24] + 5*source[25]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[25] + 1*source[26]) >> 1;
  if (--width <= 0) return;
  *dest++ = (5*source[26] + 3*source[27]) >> 3;
  if (--width <= 0) return;
  *dest++ = (7*source[27] + 1*source[28]) >> 3;
  if (--width <= 0) return;
  *dest++ = source[28];
  if (--width <= 0) return;
  *dest++ = (1*source[28] + 7*source[29]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[29] + 3*source[30]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[30] + 5*source[31]) >> 3;
  if (--width <= 0) return;
  *dest++ = (5*source[31] + 3*source[32]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[32] + 1*source[33]) >> 2;
  if (--width <= 0) return;
  *dest++ = (7*source[33] + 1*source[34]) >> 3;
  if (--width <= 0) return;
  *dest++ = source[34];
  if (--width <= 0) return;
  *dest++ = (1*source[34] + 3*source[35]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[35] + 5*source[36]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[36] + 1*source[37]) >> 1;
  if (--width <= 0) return;
  *dest++ = (5*source[37] + 3*source[38]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[38] + 1*source[39]) >> 2;
  if (--width <= 0) return;
  *dest++ = source[39];
  if (--width <= 0) return;
  *dest++ = (1*source[39] + 7*source[40]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[40] + 3*source[41]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[41] + 5*source[42]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[42] + 1*source[43]) >> 1;
  if (--width <= 0) return;
  *dest++ = (3*source[43] + 1*source[44]) >> 2;
 
}


/*
 * Interpolates 64 output pixels from 45 source pixels using shifts.
 * Useful for scaling a PAL mpeg2 dvd input source to 1024x768
 * fullscreen resolution, or to 16:9 format on a monitor using square
 * pixels.
 * (720 x 576 ==> 1024 x 576)
 */
static void scale_line_45_64 (uint8_t *source, uint8_t *dest,
			     int width, int ) {

  int p1, p2;

  while ((width -= 64) >= 0) {
    p1 = source[0];
    p2 = source[1];
    dest[0] = p1;
    dest[1] = (1*p1 + 3*p2) >> 2;
    p1 = source[2];
    dest[2] = (5*p2 + 3*p1) >> 3;
    p2 = source[3];
    dest[3] = (7*p1 + 1*p2) >> 3;
    dest[4] = (1*p1 + 3*p2) >> 2;
    p1 = source[4];
    dest[5] = (1*p2 + 1*p1) >> 1;
    p2 = source[5];
    dest[6] = (3*p1 + 1*p2) >> 2;
    dest[7] = (1*p1 + 7*p2) >> 3;
    p1 = source[6];
    dest[8] = (3*p2 + 5*p1) >> 3;
    p2 = source[7];
    dest[9] = (5*p1 + 3*p2) >> 3;
    p1 = source[8];
    dest[10] = p2;
    dest[11] = (1*p2 + 3*p1) >> 2;
    p2 = source[9];
    dest[12] = (5*p1 + 3*p2) >> 3;
    p1 = source[10];
    dest[13] = (7*p2 + 1*p1) >> 3;
    dest[14] = (1*p2 + 7*p1) >> 3;
    p2 = source[11];
    dest[15] = (1*p1 + 1*p2) >> 1;
    p1 = source[12];
    dest[16] = (3*p2 + 1*p1) >> 2;
    dest[17] = p1;
    p2 = source[13];
    dest[18] = (3*p1 + 5*p2) >> 3;
    p1 = source[14];
    dest[19] = (5*p2 + 3*p1) >> 3;
    p2 = source[15];
    dest[20] = p1;
    dest[21] = (1*p1 + 3*p2) >> 2;
    p1 = source[16];
    dest[22] = (1*p2 + 1*p1) >> 1;
    p2 = source[17];
    dest[23] = (7*p1 + 1*p2) >> 3;
    dest[24] = (1*p1 + 7*p2) >> 3;
    p1 = source[18];
    dest[25] = (3*p2 + 5*p1) >> 3;
    p2 = source[19];
    dest[26] = (3*p1 + 1*p2) >> 2;
    dest[27] = p2;
    p1 = source[20];
    dest[28] = (3*p2 + 5*p1) >> 3;
    p2 = source[21];
    dest[29] = (5*p1 + 3*p2) >> 3;
    p1 = source[22];
    dest[30] = (7*p2 + 1*p1) >> 3;
    dest[31] = (1*p2 + 3*p1) >> 2;
    p2 = source[23];
    dest[32] = (1*p1 + 1*p2) >> 1;
    p1 = source[24];
    dest[33] = (3*p2 + 1*p1) >> 2;
    dest[34] = (1*p2 + 7*p1) >> 3;
    p2 = source[25];
    dest[35] = (3*p1 + 5*p2) >> 3;
    p1 = source[26];
    dest[36] = (3*p2 + 1*p1) >> 2;
    p2 = source[27];
    dest[37] = p1;
    dest[38] = (1*p1 + 3*p2) >> 2;
    p1 = source[28];
    dest[39] = (5*p2 + 3*p1) >> 3;
    p2 = source[29];
    dest[40] = (7*p1 + 1*p2) >> 3;
    dest[41] = (1*p1 + 7*p2) >> 3;
    p1 = source[30];
    dest[42] = (1*p2 + 1*p1) >> 1;
    p2 = source[31];
    dest[43] = (3*p1 + 1*p2) >> 2;
    dest[44] = (1*p1 + 7*p2) >> 3;
    p1 = source[32];
    dest[45] = (3*p2 + 5*p1) >> 3;
    p2 = source[33];
    dest[46] = (5*p1 + 3*p2) >> 3;
    p1 = source[34];
    dest[47] = p2;
    dest[48] = (1*p2 + 3*p1) >> 2;
    p2 = source[35];
    dest[49] = (1*p1 + 1*p2) >> 1;
    p1 = source[36];
    dest[50] = (7*p2 + 1*p1) >> 3;
    dest[51] = (1*p2 + 7*p1) >> 3;
    p2 = source[37];
    dest[52] = (1*p1 + 1*p2) >> 1;
    p1 = source[38];
    dest[53] = (3*p2 + 1*p1) >> 2;
    dest[54] = p1;
    p2 = source[39];
    dest[55] = (3*p1 + 5*p2) >> 3;
    p1 = source[40];
    dest[56] = (5*p2 + 3*p1) >> 3;
    p2 = source[41];
    dest[57] = (7*p1 + 1*p2) >> 3;
    dest[58] = (1*p1 + 3*p2) >> 2;
    p1 = source[42];
    dest[59] = (1*p2 + 1*p1) >> 1;
    p2 = source[43];
    dest[60] = (7*p1 + 1*p2) >> 3;
    dest[61] = (1*p1 + 7*p2) >> 3;
    p1 = source[44];
    dest[62] = (3*p2 + 5*p1) >> 3;
    p2 = source[45];
    dest[63] = (3*p1 + 1*p2) >> 2;
    source += 45;
    dest += 64;
  }

  if ((width += 64) <= 0) return;
  *dest++ = source[0];
  if (--width <= 0) return;
  *dest++ = (1*source[0] + 3*source[1]) >> 2;
  if (--width <= 0) return;
  *dest++ = (5*source[1] + 3*source[2]) >> 3;
  if (--width <= 0) return;
  *dest++ = (7*source[2] + 1*source[3]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[2] + 3*source[3]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[3] + 1*source[4]) >> 1;
  if (--width <= 0) return;
  *dest++ = (3*source[4] + 1*source[5]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[4] + 7*source[5]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[5] + 5*source[6]) >> 3;
  if (--width <= 0) return;
  *dest++ = (5*source[6] + 3*source[7]) >> 3;
  if (--width <= 0) return;
  *dest++ = source[7];
  if (--width <= 0) return;
  *dest++ = (1*source[7] + 3*source[8]) >> 2;
  if (--width <= 0) return;
  *dest++ = (5*source[8] + 3*source[9]) >> 3;
  if (--width <= 0) return;
  *dest++ = (7*source[9] + 1*source[10]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[9] + 7*source[10]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[10] + 1*source[11]) >> 1;
  if (--width <= 0) return;
  *dest++ = (3*source[11] + 1*source[12]) >> 2;
  if (--width <= 0) return;
  *dest++ = source[12];
  if (--width <= 0) return;
  *dest++ = (3*source[12] + 5*source[13]) >> 3;
  if (--width <= 0) return;
  *dest++ = (5*source[13] + 3*source[14]) >> 3;
  if (--width <= 0) return;
  *dest++ = source[14];
  if (--width <= 0) return;
  *dest++ = (1*source[14] + 3*source[15]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[15] + 1*source[16]) >> 1;
  if (--width <= 0) return;
  *dest++ = (7*source[16] + 1*source[17]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[16] + 7*source[17]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[17] + 5*source[18]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[18] + 1*source[19]) >> 2;
  if (--width <= 0) return;
  *dest++ = source[19];
  if (--width <= 0) return;
  *dest++ = (3*source[19] + 5*source[20]) >> 3;
  if (--width <= 0) return;
  *dest++ = (5*source[20] + 3*source[21]) >> 3;
  if (--width <= 0) return;
  *dest++ = (7*source[21] + 1*source[22]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[21] + 3*source[22]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[22] + 1*source[23]) >> 1;
  if (--width <= 0) return;
  *dest++ = (3*source[23] + 1*source[24]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[23] + 7*source[24]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[24] + 5*source[25]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[25] + 1*source[26]) >> 2;
  if (--width <= 0) return;
  *dest++ = source[26];
  if (--width <= 0) return;
  *dest++ = (1*source[26] + 3*source[27]) >> 2;
  if (--width <= 0) return;
  *dest++ = (5*source[27] + 3*source[28]) >> 3;
  if (--width <= 0) return;
  *dest++ = (7*source[28] + 1*source[29]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[28] + 7*source[29]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[29] + 1*source[30]) >> 1;
  if (--width <= 0) return;
  *dest++ = (3*source[30] + 1*source[31]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[30] + 7*source[31]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[31] + 5*source[32]) >> 3;
  if (--width <= 0) return;
  *dest++ = (5*source[32] + 3*source[33]) >> 3;
  if (--width <= 0) return;
  *dest++ = source[33];
  if (--width <= 0) return;
  *dest++ = (1*source[33] + 3*source[34]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[34] + 1*source[35]) >> 1;
  if (--width <= 0) return;
  *dest++ = (7*source[35] + 1*source[36]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[35] + 7*source[36]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[36] + 1*source[37]) >> 1;
  if (--width <= 0) return;
  *dest++ = (3*source[37] + 1*source[38]) >> 2;
  if (--width <= 0) return;
  *dest++ = source[38];
  if (--width <= 0) return;
  *dest++ = (3*source[38] + 5*source[39]) >> 3;
  if (--width <= 0) return;
  *dest++ = (5*source[39] + 3*source[40]) >> 3;
  if (--width <= 0) return;
  *dest++ = (7*source[40] + 1*source[41]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[40] + 3*source[41]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[41] + 1*source[42]) >> 1;
  if (--width <= 0) return;
  *dest++ = (7*source[42] + 1*source[43]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[42] + 7*source[43]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[43] + 5*source[44]) >> 3;
 
}


/*
 * Interpolates 16 output pixels from 9 source pixels using shifts.
 * Useful for scaling a PAL mpeg2 dvd input source to 1280x1024 fullscreen
 * (720 x 576 ==> 1280 x XXX)
 */
static void scale_line_9_16 (uint8_t *source, uint8_t *dest,
			     int width, int ) {

  int p1, p2;

  while ((width -= 16) >= 0) {
    p1 = source[0];
    p2 = source[1];
    dest[0] = p1;
    dest[1] = (1*p1 + 1*p2) >> 1;
    p1 = source[2];
    dest[2] = (7*p2 + 1*p1) >> 3;
    dest[3] = (3*p2 + 5*p1) >> 3;
    p2 = source[3];
    dest[4] = (3*p1 + 1*p2) >> 2;
    dest[5] = (1*p1 + 3*p2) >> 2;
    p1 = source[4];
    dest[6] = (5*p2 + 3*p1) >> 3;
    dest[7] = (1*p2 + 7*p1) >> 3;
    p2 = source[5];
    dest[8] = (1*p1 + 1*p2) >> 1;
    p1 = source[6];
    dest[9] = p2;
    dest[10] = (3*p2 + 5*p1) >> 3;
    p2 = source[7];
    dest[11] = (7*p1 + 1*p2) >> 3;
    dest[12] = (1*p1 + 3*p2) >> 2;
    p1 = source[8];
    dest[13] = (3*p2 + 1*p1) >> 2;
    dest[14] = (1*p2 + 7*p1) >> 3;
    p2 = source[9];
    dest[15] = (5*p1 + 3*p2) >> 3;
    source += 9;
    dest += 16;
  }

  if ((width += 16) <= 0) return;
  *dest++ = source[0];
  if (--width <= 0) return;
  *dest++ = (1*source[0] + 1*source[1]) >> 1;
  if (--width <= 0) return;
  *dest++ = (7*source[1] + 1*source[2]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[1] + 5*source[2]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[2] + 1*source[3]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[2] + 3*source[3]) >> 2;
  if (--width <= 0) return;
  *dest++ = (5*source[3] + 3*source[4]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[3] + 7*source[4]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[4] + 1*source[5]) >> 1;
  if (--width <= 0) return;
  *dest++ = source[5];
  if (--width <= 0) return;
  *dest++ = (3*source[5] + 5*source[6]) >> 3;
  if (--width <= 0) return;
  *dest++ = (7*source[6] + 1*source[7]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[6] + 3*source[7]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[7] + 1*source[8]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[7] + 7*source[8]) >> 3;

}


/*
 * Interpolates 12 output pixels from 11 source pixels using shifts.
 * Useful for scaling a PAL vcd input source to 4:3 display format.
 */
static void scale_line_11_12 (uint8_t *source, uint8_t *dest,
			     int width, int ) {

  int p1, p2;

  while ((width -= 12) >= 0) {
    p1 = source[0];
    p2 = source[1];
    dest[0] = p1;
    dest[1] = (1*p1 + 7*p2) >> 3;
    p1 = source[2];
    dest[2] = (1*p2 + 7*p1) >> 3;
    p2 = source[3];
    dest[3] = (1*p1 + 3*p2) >> 2;
    p1 = source[4];
    dest[4] = (3*p2 + 5*p1) >> 3;
    p2 = source[5];
    dest[5] = (3*p1 + 5*p2) >> 3;
    p1 = source[6];
    dest[6] = (1*p2 + 1*p1) >> 1;
    p2 = source[7];
    dest[7] = (5*p1 + 3*p2) >> 3;
    p1 = source[8];
    dest[8] = (5*p2 + 3*p1) >> 3;
    p2 = source[9];
    dest[9] = (3*p1 + 1*p2) >> 2;
    p1 = source[10];
    dest[10] = (7*p2 + 1*p1) >> 3;
    p2 = source[11];
    dest[11] = (7*p1 + 1*p2) >> 3;
    source += 11;
    dest += 12;
  }

  if ((width += 12) <= 0) return;
  *dest++ = source[0];
  if (--width <= 0) return;
  *dest++ = (1*source[0] + 7*source[1]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[1] + 7*source[2]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[2] + 3*source[3]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[3] + 5*source[4]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[4] + 5*source[5]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[5] + 1*source[6]) >> 1;
  if (--width <= 0) return;
  *dest++ = (5*source[6] + 3*source[7]) >> 3;
  if (--width <= 0) return;
  *dest++ = (5*source[7] + 3*source[8]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[8] + 1*source[9]) >> 2;
  if (--width <= 0) return;
  *dest++ = (7*source[9] + 1*source[10]) >> 3;


}


/*
 * Interpolates 24 output pixels from 11 source pixels using shifts.
 * Useful for scaling a PAL vcd input source to 4:3 display format
 * at 2*zoom.
 */
static void scale_line_11_24 (uint8_t *source, uint8_t *dest,
			     int width, int ) {

  int p1, p2;

  while ((width -= 24) >= 0) {
    p1 = source[0];
    p2 = source[1];
    dest[0] = p1;
    dest[1] = (1*p1 + 1*p2) >> 1;
    dest[2] = (1*p1 + 7*p2) >> 3;
    p1 = source[2];
    dest[3] = (5*p2 + 3*p1) >> 3;
    dest[4] = (1*p2 + 7*p1) >> 3;
    p2 = source[3];
    dest[5] = (3*p1 + 1*p2) >> 2;
    dest[6] = (1*p1 + 3*p2) >> 2;
    p1 = source[4];
    dest[7] = (3*p2 + 1*p1) >> 2;
    dest[8] = (3*p2 + 5*p1) >> 3;
    p2 = source[5];
    dest[9] = (7*p1 + 1*p2) >> 3;
    dest[10] = (3*p1 + 5*p2) >> 3;
    p1 = source[6];
    dest[11] = p2;
    dest[12] = (1*p2 + 1*p1) >> 1;
    dest[13] = p1;
    p2 = source[7];
    dest[14] = (5*p1 + 3*p2) >> 3;
    dest[15] = (1*p1 + 7*p2) >> 3;
    p1 = source[8];
    dest[16] = (5*p2 + 3*p1) >> 3;
    dest[17] = (1*p2 + 3*p1) >> 2;
    p2 = source[9];
    dest[18] = (3*p1 + 1*p2) >> 2;
    dest[19] = (1*p1 + 3*p2) >> 2;
    p1 = source[10];
    dest[20] = (7*p2 + 1*p1) >> 3;
    dest[21] = (3*p2 + 5*p1) >> 3;
    p2 = source[11];
    dest[22] = (7*p1 + 1*p2) >> 3;
    dest[23] = (1*p1 + 1*p2) >> 1;
    source += 11;
    dest += 24;
  }

  if ((width += 24) <= 0) return;
  *dest++ = source[0];
  if (--width <= 0) return;
  *dest++ = (1*source[0] + 1*source[1]) >> 1;
  if (--width <= 0) return;
  *dest++ = (1*source[0] + 7*source[1]) >> 3;
  if (--width <= 0) return;
  *dest++ = (5*source[1] + 3*source[2]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[1] + 7*source[2]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[2] + 1*source[3]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[2] + 3*source[3]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[3] + 1*source[4]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[3] + 5*source[4]) >> 3;
  if (--width <= 0) return;
  *dest++ = (7*source[4] + 1*source[5]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[4] + 5*source[5]) >> 3;
  if (--width <= 0) return;
  *dest++ = source[5];
  if (--width <= 0) return;
  *dest++ = (1*source[5] + 1*source[6]) >> 1;
  if (--width <= 0) return;
  *dest++ = source[6];
  if (--width <= 0) return;
  *dest++ = (5*source[6] + 3*source[7]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[6] + 7*source[7]) >> 3;
  if (--width <= 0) return;
  *dest++ = (5*source[7] + 3*source[8]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[7] + 3*source[8]) >> 2;
  if (--width <= 0) return;
  *dest++ = (3*source[8] + 1*source[9]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[8] + 3*source[9]) >> 2;
  if (--width <= 0) return;
  *dest++ = (7*source[9] + 1*source[10]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[9] + 5*source[10]) >> 3;
  if (--width <= 0) return;
  *dest++ = (7*source[10] + 1*source[11]) >> 3;


}


/*
 * Interpolates 8 output pixels from 5 source pixels using shifts.
 * Useful for scaling a PAL svcd input source to 4:3 display format.
 */
static void scale_line_5_8 (uint8_t *source, uint8_t *dest,
			    int width, int ) {

  int p1, p2;

  while ((width -= 8) >= 0) {
    p1 = source[0];
    p2 = source[1];
    dest[0] = p1;
    dest[1] = (3*p1 + 5*p2) >> 3;
    p1 = source[2];
    dest[2] = (3*p2 + 1*p1) >> 2;
    dest[3] = (1*p2 + 7*p1) >> 3;
    p2 = source[3];
    dest[4] = (1*p1 + 1*p2) >> 1;
    p1 = source[4];
    dest[5] = (7*p2 + 1*p1) >> 3;
    dest[6] = (1*p2 + 3*p1) >> 2;
    p2 = source[5];
    dest[7] = (5*p1 + 3*p2) >> 3;
    source += 5;
    dest += 8;
  }

  if ((width += 8) <= 0) return;
  *dest++ = source[0];
  if (--width <= 0) return;
  *dest++ = (3*source[0] + 5*source[1]) >> 3;
  if (--width <= 0) return;
  *dest++ = (3*source[1] + 1*source[2]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[1] + 7*source[2]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[2] + 1*source[3]) >> 1;
  if (--width <= 0) return;
  *dest++ = (7*source[3] + 1*source[4]) >> 3;
  if (--width <= 0) return;
  *dest++ = (1*source[3] + 3*source[4]) >> 2;


}


/*
 * Interpolates 4 output pixels from 3 source pixels using shifts.
 * Useful for scaling a NTSC svcd input source to 4:3 display format.
 */
static void scale_line_3_4 (uint8_t *source, uint8_t *dest,
			    int width, int ) {

  int p1, p2;

  while ((width -= 4) >= 0) {
    p1 = source[0];
    p2 = source[1];
    dest[0] = p1;
    dest[1] = (1*p1 + 3*p2) >> 2;
    p1 = source[2];
    dest[2] = (1*p2 + 1*p1) >> 1;
    p2 = source[3];
    dest[3] = (3*p1 + 1*p2) >> 2;
    source += 3;
    dest += 4;
  }

  if ((width += 4) <= 0) return;
  *dest++ = source[0];
  if (--width <= 0) return;
  *dest++ = (1*source[0] + 3*source[1]) >> 2;
  if (--width <= 0) return;
  *dest++ = (1*source[1] + 1*source[2]) >> 1;


}


/* Interpolate 2 output pixels from one source pixel. */

static void scale_line_1_2 (uint8_t *source, uint8_t *dest,
			    int width, int ) {
  int p1, p2;

  p1 = *source;
  while ((width -= 4) >= 0) {
    *dest++ = p1;
    p2 = *++source;
    *dest++ = (p1 + p2) >> 1;
    *dest++ = p2;
    p1 = *++source;
    *dest++ = (p2 + p1) >> 1;
  }

  if ((width += 4) <= 0) return;
  *dest++ = source[0];
  if (--width <= 0) return;
  *dest++ = (source[0] + source[1]) >> 1;
  if (--width <= 0) return;
  *dest++ = source[1];
 

}

			
/*
 * Scale line with no horizontal scaling. For NTSC mpeg2 dvd input in
 * 4:3 output format (720x480 -> 720x540)
 */
static void scale_line_1_1 (uint8_t *source, uint8_t *dest,
			    int width, int ) {

  memcpy(dest, source, width);
}

			
static scale_line_func_t find_scale_line_func(int step) {
  static struct {
    int			src_step;
    int			dest_step;
    scale_line_func_t	func;
    char	       *desc;
  } scale_line[] = {
    { 15, 16, scale_line_15_16, "dvd 4:3(pal)" },
    { 45, 64, scale_line_45_64, "dvd 16:9(pal), fullscreen(1024x768)" },
    {  9, 16, scale_line_9_16,  "dvd fullscreen(1280x1024)" },
    { 45, 53, scale_line_45_53, "dvd 16:9(ntsc)" },
    { 11, 12, scale_line_11_12, "vcd 4:3(pal)" },
    { 11, 24, scale_line_11_24, "vcd 4:3(pal) 2*zoom" },
    {  5,  8, scale_line_5_8,   "svcd 4:3(pal)" },
    {  3,  4, scale_line_3_4,   "svcd 4:3(ntsc)" },
    {  1,  2, scale_line_1_2,   "2*zoom" },
    {  1,  1, scale_line_1_1,   "non-scaled" },
  };
  int i;

  for (i = 0; i < sizeof(scale_line)/sizeof(scale_line[0]); i++) {
    if (step == scale_line[i].src_step*32768/scale_line[i].dest_step) {
      printf("yuv2rgb: using %s optimized scale_line\n", scale_line[i].desc);
      return scale_line[i].func;
    }
  }
  printf("yuv2rgb: using generic scale_line with interpolation\n");
  return scale_line_gen;

}


static void scale_line_2 (uint8_t *source, uint8_t *dest,
			  int width, int step) {
  int p1;
  int p2;
  int dx;

  p1 = *source; source+=2;
  p2 = *source; source+=2;
  dx = 0;

  while (width) {

    *dest = (p1 * (32768 - dx) + p2 * dx) / 32768;

    dx += step;
    while (dx > 32768) {
      dx -= 32768;
      p1 = p2;
      p2 = *source;
      source+=2;
    }

    dest ++;
    width --;
  }
}

static void scale_line_4 (uint8_t *source, uint8_t *dest,
			  int width, int step) {
  int p1;
  int p2;
  int dx;

  p1 = *source; source+=4;
  p2 = *source; source+=4;
  dx = 0;

  while (width) {

    *dest = (p1 * (32768 - dx) + p2 * dx) / 32768;

    dx += step;
    while (dx > 32768) {
      dx -= 32768;
      p1 = p2;
      p2 = *source;
      source+=4;
    }

    dest ++;
    width --;
  }
}


#define RGB(i)							\
	U = pu[i];						\
	V = pv[i];						\
	r = (uint16_t *) c_this->table_rV[V];					\
	g = (uint16_t *) (((uint8_t *)c_this->table_gU[U]) + c_this->table_gV[V]);	\
	b = (uint16_t *) c_this->table_bU[U];

#define DST1(i)					\
	Y = py_1[2*i];                          \
	dst_1[2*i] = r[Y] + g[Y] + b[Y];	\
	Y = py_1[2*i+1];			\
	dst_1[2*i+1] = r[Y] + g[Y] + b[Y];

#define DST2(i)					\
	Y = py_2[2*i];				\
	dst_2[2*i] = r[Y] + g[Y] + b[Y];	\
	Y = py_2[2*i+1];			\
	dst_2[2*i+1] = r[Y] + g[Y] + b[Y];



/* This is exactly the same code as yuv2rgb_c_32 except for the types of */
/* r, g, b, dst_1, dst_2 */
static void yuv2rgb_c_16 (yuv2rgb_t *c_this, uint8_t * _dst,
			  uint8_t * _py, uint8_t * _pu, uint8_t * _pv)
{
  int U, V, Y;
  uint8_t * py_1, * py_2, * pu, * pv;
  uint16_t * r, * g, * b;
  uint16_t * dst_1, * dst_2;
  int width, height, dst_height;
  int dy;

  if (c_this->do_scale) {
    scale_line_func_t scale_line = c_this->scale_line;

    scale_line (_pu, c_this->u_buffer,
		c_this->dest_width >> 1, c_this->step_dx);
    scale_line (_pv, c_this->v_buffer,
		c_this->dest_width >> 1, c_this->step_dx);
    scale_line (_py, c_this->y_buffer, 
		c_this->dest_width, c_this->step_dx);

    dy = 0;
    dst_height = c_this->dest_height;

    for (height = 0;; ) {
      dst_1 = (uint16_t*)_dst;
      py_1  = c_this->y_buffer;
      pu    = c_this->u_buffer;
      pv    = c_this->v_buffer;

      width = c_this->dest_width >> 3;

      do {
	  RGB(0);
	  DST1(0);

	  RGB(1);
	  DST1(1);
      
	  RGB(2);
	  DST1(2);

	  RGB(3);
	  DST1(3);

	  pu += 4;
	  pv += 4;
	  py_1 += 8;
	  dst_1 += 8;
      } while (--width);

      dy += c_this->step_dy;
      _dst += c_this->rgb_stride;

      while (--dst_height > 0 && dy < 32768) {

	memcpy (_dst, (uint8_t*)_dst-c_this->rgb_stride, c_this->dest_width*2); 

	dy += c_this->step_dy;
	_dst += c_this->rgb_stride;
      }

      if (dst_height <= 0)
	break;

      do {
          dy -= 32768;
          _py += c_this->y_stride;

          scale_line (_py, c_this->y_buffer, 
                      c_this->dest_width, c_this->step_dx);

          if (height & 1) {
              _pu += c_this->uv_stride;
              _pv += c_this->uv_stride;
	  
              scale_line (_pu, c_this->u_buffer,
                          c_this->dest_width >> 1, c_this->step_dx);
              scale_line (_pv, c_this->v_buffer,
                          c_this->dest_width >> 1, c_this->step_dx);
	  
          }
          height++;
      } while( dy>=32768);
    }
  } else {
    height = c_this->source_height >> 1;
    do {
      dst_1 = (uint16_t*)_dst;
      dst_2 = (uint16_t*)( (uint8_t *)_dst + c_this->rgb_stride );
      py_1 = _py;
      py_2 = _py + c_this->y_stride;
      pu   = _pu;
      pv   = _pv;
      width = c_this->source_width >> 3;
      do {
	RGB(0);
	DST1(0);
	DST2(0);

	RGB(1);
	DST2(1);
	DST1(1);

	RGB(2);
	DST1(2);
	DST2(2);

	RGB(3);
	DST2(3);
	DST1(3);

	pu += 4;
	pv += 4;
	py_1 += 8;
	py_2 += 8;
	dst_1 += 8;
	dst_2 += 8;
      } while (--width);

      _dst += 2 * c_this->rgb_stride; 
      _py += 2 * c_this->y_stride;
      _pu += c_this->uv_stride;
      _pv += c_this->uv_stride;

    } while (--height);
  }
}


static int div_round (int dividend, int divisor)
{
  if (dividend > 0)
    return (dividend + (divisor>>1)) / divisor;
  else
    return -((-dividend + (divisor>>1)) / divisor);
}

static uint32_t yuv2rgb_single_pixel_16 (yuv2rgb_t *c_this, uint8_t y, uint8_t u, uint8_t v)
{
  uint16_t * r, * g, * b;

  r = (uint16_t *) c_this->table_rV[v];
  g = (uint16_t *) (((uint8_t *)c_this->table_gU[u]) + c_this->table_gV[v]);
  b = (uint16_t *) c_this->table_bU[u];

  return r[y] + g[y] + b[y];
}

static void yuy22rgb_c_16 (yuv2rgb_t *c_this, uint8_t * _dst, uint8_t * _p)
{
  int U, V, Y;
  uint8_t * py_1, * pu, * pv;
  uint16_t * r, * g, * b;
  uint16_t * dst_1;
  int width, height;
  int dy;

  /* FIXME: implement unscaled version */

  scale_line_4 (_p+1, c_this->u_buffer,
		c_this->dest_width >> 1, c_this->step_dx);
  scale_line_4 (_p+3, c_this->v_buffer,
		c_this->dest_width >> 1, c_this->step_dx);
  scale_line_2 (_p, c_this->y_buffer, 
		c_this->dest_width, c_this->step_dx);
  
  dy = 0;
  height = c_this->dest_height;

  for (;;) {
    dst_1 = (uint16_t*)_dst;
    py_1  = c_this->y_buffer;
    pu    = c_this->u_buffer;
    pv    = c_this->v_buffer;
    
    width = c_this->dest_width >> 3;
    
    do {
      RGB(0);
      DST1(0);

      RGB(1);
      DST1(1);
      
      RGB(2);
      DST1(2);
      
      RGB(3);
      DST1(3);

      pu += 4;
      pv += 4;
      py_1 += 8;
      dst_1 += 8;
    } while (--width);
    
    dy += c_this->step_dy;
    _dst += c_this->rgb_stride;

    while (--height > 0 && dy < 32768) {
      
      memcpy (_dst, (uint8_t*)_dst-c_this->rgb_stride, c_this->dest_width*2); 

      dy += c_this->step_dy;
      _dst += c_this->rgb_stride;
    }
    
    if (height <= 0)
      break;

    _p += c_this->y_stride*2*(dy>>15);
    dy &= 32767;
    
    scale_line_4 (_p+1, c_this->u_buffer,
		  c_this->dest_width >> 1, c_this->step_dx);
    scale_line_4 (_p+3, c_this->v_buffer,
		  c_this->dest_width >> 1, c_this->step_dx);
    scale_line_2 (_p, c_this->y_buffer,
		  c_this->dest_width, c_this->step_dx);
  }
}

#ifdef __arm__
#include "yuv2rgb_arm.c"
#endif

yuv2rgb_factory_t* yuv2rgb_factory_init (int mode, int swapped, 
					 int gamma) {

  yuv2rgb_factory_t *c_this;

  c_this = (yuv2rgb_factory_t *) malloc (sizeof (yuv2rgb_factory_t));

  c_this->mode                = mode;
  c_this->swapped             = swapped;




  c_this->matrix_coefficients = 6;

  int i;
  uint8_t table_Y[1024];
  uint32_t * table_32 = 0;
  uint16_t * table_16 = 0;
  uint8_t * table_8 = 0;
  int entry_size = 0;
  void *table_r = 0, *table_g = 0, *table_b = 0;
  int shift_r = 0, shift_g = 0, shift_b = 0;

  int crv = Inverse_Table_6_9[c_this->matrix_coefficients][0];
  int cbu = Inverse_Table_6_9[c_this->matrix_coefficients][1];
  int cgu = -Inverse_Table_6_9[c_this->matrix_coefficients][2];
  int cgv = -Inverse_Table_6_9[c_this->matrix_coefficients][3];

  for (i = 0; i < 1024; i++) {
    int j;

    j = (76309 * (i - 384 - 16) + 32768) >> 16;
    j = (j < 0) ? 0 : ((j > 255) ? 255 : j);
    table_Y[i] = j;
  }

  table_16 = (uint16_t *)malloc ((197 + 2*682 + 256 + 132) * sizeof (uint16_t));

  entry_size = sizeof (uint16_t);
  table_r = table_16 + 197;
  table_b = table_16 + 197 + 685;
  table_g = table_16 + 197 + 2*682;

  if (swapped) {
    switch (mode) {
    case MODE_15_BGR: shift_r =  8; shift_g =  5; shift_b = 2; break;
    case MODE_16_BGR:	shift_r =  8; shift_g =  5; shift_b = 3; break;
    case MODE_15_RGB:	shift_r =  2; shift_g =  5; shift_b = 8; break;
    case MODE_16_RGB:	shift_r =  3; shift_g =  5; shift_b = 8; break;
    }
  } else {
    switch (mode) {
    case MODE_15_BGR:	shift_r =  0; shift_g =  5; shift_b = 10; break;
    case MODE_16_BGR:	shift_r =  0; shift_g =  5; shift_b = 11; break;
    case MODE_15_RGB:	shift_r = 10; shift_g =  5; shift_b =  0; break;
    case MODE_16_RGB:	shift_r = 11; shift_g =  5; shift_b =  0; break;
    }
  }

  for (i = -197; i < 256+197; i++)
    ((uint16_t *)table_r)[i] = (table_Y[i+384] >> 3) << shift_r;

  for (i = -132; i < 256+132; i++) {
    int j = table_Y[i+384] >> (((mode==MODE_16_RGB) || (mode==MODE_16_BGR)) ? 2 : 3);
    if (swapped)
      ((uint16_t *)table_g)[i] = (j&7) << 13 | (j>>3);
    else
      ((uint16_t *)table_g)[i] = j << 5;
  }
  for (i = -232; i < 256+232; i++)
    ((uint16_t *)table_b)[i] = (table_Y[i+384] >> 3) << shift_b;
  
  for (i = 0; i < 256; i++) {
    c_this->table_rV[i] = (((uint8_t *) table_r) +
			 entry_size * div_round (crv * (i-128), 76309));
    c_this->table_gU[i] = (((uint8_t *) table_g) +
			 entry_size * div_round (cgu * (i-128), 76309));
    c_this->table_gV[i] = entry_size * div_round (cgv * (i-128), 76309);
    c_this->table_bU[i] = (((uint8_t *)table_b) +
			 entry_size * div_round (cbu * (i-128), 76309));
  }
  c_this->gamma = 0;
  c_this->entry_size = entry_size;

  // yuv2rgb_set_gamma 
  for (int i = 0; i < 256; i++) {
      (uint8_t *)c_this->table_rV[i] += c_this->entry_size*(gamma - c_this->gamma);
      (uint8_t *)c_this->table_gU[i] += c_this->entry_size*(gamma - c_this->gamma);
      (uint8_t *)c_this->table_bU[i] += c_this->entry_size*(gamma - c_this->gamma);
  }
  c_this->gamma = gamma;





  /*
   * auto-probe for the best yuv2rgb function
   */
  c_this->yuv2rgb_fun = NULL;
#ifdef __arm__
  yuv2rgb_init_arm ( c_this );
  
  if(c_this->yuv2rgb_fun != NULL)
  	printf("yuv2rgb: using arm4l assembler for colorspace transform\n" );
#endif
  if (c_this->yuv2rgb_fun == NULL) {
    printf ("yuv2rgb: using unaccelerated colorspace conversion\n");
    c_this->yuv2rgb_fun = yuv2rgb_c_16;
  }

  yuv2rgb_t *converter = c_this->converter = (yuv2rgb_t *) malloc (sizeof (yuv2rgb_t));
  converter->y_chunk = converter->y_buffer = 0;
  converter->u_chunk = converter->u_buffer = 0;
  converter->v_chunk = converter->v_buffer = 0;
  converter->table_rV                 = c_this->table_rV;
  converter->table_gU                 = c_this->table_gU;
  converter->table_gV                 = c_this->table_gV;
  converter->table_bU                 = c_this->table_bU;
  converter->yuv2rgb_fun              = c_this->yuv2rgb_fun;
  converter->yuy22rgb_fun             = yuy22rgb_c_16;
  converter->yuv2rgb_single_pixel_fun = yuv2rgb_single_pixel_16;
  converter->configure                = yuv2rgb_configure;

  return c_this;
}





