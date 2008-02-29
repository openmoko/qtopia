/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
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

#ifdef ARCH_XSCALE

extern "C"
{
// Optimized functions for scaling down

void scale_line_gen_xscale (uint8_t *source, uint8_t *dest, int width, int step);

int yuvrgbConvert(uint8_t *y_buff,uint8_t *u_buff,uint8_t *v_buff,uint16_t *dst_buff,
                   uint8_t *py,uint8_t *pu,uint8_t *pv,
                   void *RedTable,void *BlueTable,void *GreenUTable,void *GreenVTable,
                   int dest_width,int dest_height,int dy,int y_stride,int uv_stride,
                   int step_dx,int step_dy,int rgb_stride,int uv_stretch);
}
#endif

#ifdef ARCH_XSCALE
static scale_line_func_t find_scale_line_func(int step, int optimize);
#else
static scale_line_func_t find_scale_line_func(int step);
#endif


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


static void *my_malloc_aligned (size_t alignment, size_t size, void **chunk)
{
    char *pMem = (char *)malloc(size+alignment);

    *chunk = pMem;

    while ((int) pMem % alignment)
	pMem++;

    return pMem;
}


/*
    fourcc:    bpp: IEEE:      plane sizes: (w=width h=height of original image)
    444P       24   YUV 4:4:4  Y: w * h  U,V: w * h
    YUY2,UYVY  16   YUV 4:2:2  Y: w * h  U,V: (w/2) * h      [MJPEG]
    YV12,I420  12   YUV 4:2:0  Y: w * h  U,V: (w/2) * (h/2)  [MPEG, h263]
    411P       12   YUV 4:1:1  Y: w * h  U,V: (w/4) * h      [DV-NTSC, CYUV]
    YVU9,IF09   9   YUV 4:1:0  Y: w * h  U,V: (w/4) * (h/4)  [Sorenson, Indeo]
*/

static int yuv2rgb_configure (yuv2rgb_t *c_this, int source_width, int source_height,
			      int y_stride, int uv_stride,
			      int dest_width, int dest_height,
			      int rgb_stride, int format)
{
  c_this->source_width  = source_width;
  c_this->source_height = source_height;
  c_this->y_stride      = y_stride;
  c_this->uv_stride     = uv_stride;
  c_this->dest_width    = dest_width;
  c_this->dest_height   = dest_height;
  c_this->rgb_stride    = rgb_stride;

printf("yuv2rgb config - w: %d h: %d dw: %d dh: %d\n", source_width, source_height, dest_width, dest_height);

  switch ( format ) {
    case FORMAT_YUV444:
      c_this->uv_stretch_x = 0;
      c_this->uv_stretch_y = 0;
      break;
    case FORMAT_YUV422:
      c_this->uv_stretch_x = 1;
      c_this->uv_stretch_y = 0;
      break;
    case FORMAT_YUV420:
      c_this->uv_stretch_x = 1;
      c_this->uv_stretch_y = 1;
      break;
    case FORMAT_YUV411:
      c_this->uv_stretch_x = 2;
      c_this->uv_stretch_y = 0;
      break;
    case FORMAT_YUV410:
      c_this->uv_stretch_x = 2;
      c_this->uv_stretch_y = 2;
      break;
  }
  
  if (c_this->y_chunk) {
    free (c_this->y_chunk);
    c_this->y_chunk = 0;
    c_this->y_buffer = (uint8_t *)c_this->y_chunk;
  }
  if (c_this->u_chunk) {
    free (c_this->u_chunk);
    c_this->u_chunk = 0;
    c_this->u_buffer = (uint8_t *)c_this->u_chunk;
  }
  if (c_this->v_chunk) {
    free (c_this->v_chunk);
    c_this->v_chunk = 0;
    c_this->v_buffer = (uint8_t *)c_this->v_chunk;
  }

  
  c_this->step_dx = source_width  * 32768 / dest_width;
  c_this->step_dy = source_height * 32768 / dest_height;
  
#ifdef ARCH_XSCALE
    // If source larger than destination, use optimized scaling method
    // Otherwise, use regular scaling method
    if( source_width > dest_width || source_height > dest_height ) {
        c_this->optimize = 1;
        printf( "yuv2rgb config - using xscale optimized scaling method\n" );
    } else {
        c_this->optimize = 0;
        printf( "yuv2rgb config - using regular scaling method\n" );
    }
#endif

#ifdef ARCH_XSCALE
  c_this->scale_line = find_scale_line_func(c_this->step_dx, c_this->optimize);
#else
  c_this->scale_line = find_scale_line_func(c_this->step_dx);
#endif

  if ((source_width == dest_width) && (source_height == dest_height) && (format == FORMAT_YUV420) ) {
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

static void scale_line_gen (uint8_t *source, uint8_t *dest, int width, int step)
{
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
   * deal with this code.
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
      *dest = p1 + (((p2 - p1) * dx) >> 15);
      dx += step;
      offs=((dx-1)>>15);
      dx-=offs<<15;
      source+=offs-2;
      p1 = *source++;
      dest++;
      p2 = *source++;
      width--;
    }
  }
}

/*
 * Interpolates 8 output pixels from 5 source pixels using shifts.
 * Useful for scaling a PAL svcd input source to 4:3 display format.
 */
static void scale_line_5_8 (uint8_t *source, uint8_t *dest, int width, int )
{
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
static void scale_line_3_4 (uint8_t *source, uint8_t *dest, int width, int )
{
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

static void scale_line_1_2 (uint8_t *source, uint8_t *dest, int width, int )
{
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
static void scale_line_1_1 (uint8_t *source, uint8_t *dest, int width, int )
{
  memcpy(dest, source, width);
}

			
#ifdef ARCH_XSCALE
static scale_line_func_t find_scale_line_func(int step, int optimize)
#else
static scale_line_func_t find_scale_line_func(int step)
#endif
{
  static struct {
    int			src_step;
    int			dest_step;
    scale_line_func_t	func;
    char	       *desc;
  } scale_line[] = {
    {  5,  8, scale_line_5_8,   "svcd 4:3(pal)" }, // No tr
    {  3,  4, scale_line_3_4,   "svcd 4:3(ntsc)" },
    {  1,  2, scale_line_1_2,   "2*zoom" }, // No tr
    {  1,  1, scale_line_1_1,   "non-scaled" },
  };
  unsigned int i;

  for (i = 0; i < sizeof(scale_line)/sizeof(scale_line[0]); i++) {
    if (step == scale_line[i].src_step*32768/scale_line[i].dest_step) {
      printf("yuv2rgb: using %s optimized scale_line\n", scale_line[i].desc);
      return scale_line[i].func;
    }
  }
#ifdef ARCH_XSCALE
  if( optimize ) {
      printf( "yuv2rgb: using xscale optimized scale_line\n" );
      return scale_line_gen_xscale;
  }
#endif
  printf("yuv2rgb: using generic scale_line with interpolation\n");
  return scale_line_gen;
}


#define RGB(i,type)							\
	U = pu[i];						\
	V = pv[i];						\
	r = (type *) c_this->table_rV[V];					\
	g = (type *) (((uint8_t *)c_this->table_gU[U]) + c_this->table_gV[V]);	\
	b = (type *) c_this->table_bU[U];

#define RGB16(i) RGB(i,uint16_t)
#define RGB32(i) RGB(i,uint32_t)

#define _DST(i,dst,x) \
	Y = py_##dst[2*i+x]; \
	dst_##dst[2*i+x] = r[Y] + g[Y] + b[Y];

#define DST(i,dst) \
	_DST(i,dst,0) \
	_DST(i,dst,1) \

#define DST1(i) DST(i,1)
#define DST2(i) DST(i,2)


static void yuv2rgb_c_32 (yuv2rgb_t *c_this, uint8_t * _dst,
			  uint8_t * _py, uint8_t * _pu, uint8_t * _pv)
{
  int U, V, Y;
  uint8_t  * py_1, * py_2, * pu, * pv;
  uint32_t * r, * g, * b;
  uint32_t * dst_1, * dst_2;
  int width, height, dst_height;
  int dy;

  if (c_this->do_scale) {
    scale_line_func_t scale_line = c_this->scale_line;

    scale_line (_pu, c_this->u_buffer,
		c_this->dest_width >> c_this->uv_stretch_x, c_this->step_dx);
    scale_line (_pv, c_this->v_buffer,
		c_this->dest_width >> c_this->uv_stretch_x, c_this->step_dx);
    scale_line (_py, c_this->y_buffer, 
		c_this->dest_width, c_this->step_dx);

    dy = 0;
    dst_height = c_this->dest_height;

    for (height = 0;; ) {
      dst_1 = (uint32_t*)_dst;
      py_1  = c_this->y_buffer;
      pu    = c_this->u_buffer;
      pv    = c_this->v_buffer;

      width = c_this->dest_width >> 3;

      do {
	  RGB32(0);
	  DST1(0);

	  RGB32(1);
	  DST1(1);
      
	  RGB32(2);
	  DST1(2);

	  RGB32(3);
	  DST1(3);

	  pu += 4;
	  pv += 4;
	  py_1 += 8;
	  dst_1 += 8;
      } while (--width);

      int edge_pixels = c_this->dest_width % 8;
      for ( int i = 0; i < (edge_pixels-1); i+=2 ) {
	  RGB32(0);
	  DST1(0);
	  pu += 1;
	  pv += 1;
	  py_1 += 2;
	  dst_1 += 2;
      }
      if (c_this->dest_width % 1) {
	  RGB32(0);
	  _DST(0,1,0);
      }

      dy += c_this->step_dy;
      _dst += c_this->rgb_stride;

      while (--dst_height > 0 && dy < 32768) {

	memcpy(_dst, (uint8_t*)_dst-c_this->rgb_stride, c_this->dest_width*4); 

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

          if (height & 1 || !c_this->uv_stretch_y ) {
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
      dst_1 = (uint32_t*)_dst;
      dst_2 = (uint32_t*)( (uint8_t *)_dst + c_this->rgb_stride );
      py_1 = _py;
      py_2 = _py + c_this->y_stride;
      pu   = _pu;
      pv   = _pv;

      width = c_this->source_width >> 3;
      do {
	RGB32(0);
	DST1(0);
	DST2(0);

	RGB32(1);
	DST2(1);
	DST1(1);

	RGB32(2);
	DST1(2);
	DST2(2);

	RGB32(3);
	DST2(3);
	DST1(3);
      
	pu += 4;
	pv += 4;
	py_1 += 8;
	py_2 += 8;
	dst_1 += 8;
	dst_2 += 8;
      } while (--width);

      int edge_pixels = c_this->source_width % 8;
      for ( int i = 0; i < (edge_pixels-1); i+=2 ) {
	RGB32(0)
	DST1(0)
	DST2(0)
	pu += 1;
	pv += 1;
	py_1 += 2;
	py_2 += 2;
	dst_1 += 2;
	dst_2 += 2;
      }
      if (c_this->source_width % 1) {
	  RGB32(0);
	  _DST(0,1,0);
      }

      _dst += 2 * c_this->rgb_stride; 
      _py += 2 * c_this->y_stride;
      _pu += c_this->uv_stride;
      _pv += c_this->uv_stride;

    } while (--height);
  }
}


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
		c_this->dest_width >> c_this->uv_stretch_x, c_this->step_dx);
    scale_line (_pv, c_this->v_buffer,
		c_this->dest_width >> c_this->uv_stretch_x, c_this->step_dx);
    scale_line (_py, c_this->y_buffer, 
		c_this->dest_width, c_this->step_dx);

    dy = 0;
    dst_height = c_this->dest_height;

#ifdef ARCH_XSCALE
   if ( !c_this->optimize || c_this->step_dy < 32768 )
    {
#endif
    for (height = 0;; ) {
      dst_1 = (uint16_t*)_dst;
      py_1  = c_this->y_buffer;
      pu    = c_this->u_buffer;
      pv    = c_this->v_buffer;

      width = c_this->dest_width >> 3;

      do {
	  RGB16(0);
	  DST1(0);

	  RGB16(1);
	  DST1(1);
      
	  RGB16(2);
	  DST1(2);

	  RGB16(3);
	  DST1(3);

	  pu += 4;
	  pv += 4;
	  py_1 += 8;
	  dst_1 += 8;
      } while (--width);

      int edge_pixels = c_this->dest_width % 8;
      for ( int i = 0; i < (edge_pixels-1); i+=2 ) {
	  RGB16(0);
	  DST1(0);
	  pu += 1;
	  pv += 1;
	  py_1 += 2;
	  dst_1 += 2;
      }
      if (c_this->dest_width % 1) {
	  RGB16(0);
	  _DST(0,1,0);
      }
      
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

          if (height & 1 || !c_this->uv_stretch_y ) {
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
#ifdef ARCH_XSCALE
   } // end if use old or dy < 32768
else
   {    
   // dkl - this algorithm was written in assembly, it only handles the scale case of shrinking the
   //  image, otherwise we use the old method above                                                                                                       
	yuvrgbConvert(c_this->y_buffer,c_this->u_buffer,c_this->v_buffer,(uint16_t *)_dst,
            _py,_pu,_pv,c_this->table_rV,c_this->table_bU,c_this->table_gU,c_this->table_gV,
            c_this->dest_width,dst_height,dy,c_this->y_stride,c_this->uv_stride,
            c_this->step_dx,c_this->step_dy,c_this->rgb_stride,c_this->uv_stretch_y);
   }
#endif

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
	RGB16(0);
	DST1(0);
	DST2(0);

	RGB16(1);
	DST2(1);
	DST1(1);

	RGB16(2);
	DST1(2);
	DST2(2);

	RGB16(3);
	DST2(3);
	DST1(3);

	pu += 4;
	pv += 4;
	py_1 += 8;
	py_2 += 8;
	dst_1 += 8;
	dst_2 += 8;
      } while (--width);

      int edge_pixels = c_this->source_width % 8;
      for ( int i = 0; i < (edge_pixels-1); i+=2 ) {
	RGB16(0);
	DST1(0);
	DST2(0);
	pu += 1;
	pv += 1;
	py_1 += 2;
	py_2 += 2;
	dst_1 += 2;
	dst_2 += 2;
      }
      if (c_this->source_width % 1) {
	  RGB16(0);
	  _DST(0,1,0);
      }

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

/*
#include <linux/config.h>
#ifdef CONFIG_ARM
#include "yuv2rgb_arm.c"
#endif
*/

yuv2rgb_factory_t* yuv2rgb_factory_init (int mode, int swapped, int gamma)
{
    yuv2rgb_factory_t *c_this;

    c_this = (yuv2rgb_factory_t *) malloc (sizeof (yuv2rgb_factory_t));

    c_this->mode = mode;
    c_this->swapped = swapped;


    c_this->matrix_coefficients = 6;

    int i;
    uint8_t table_Y[1024];
    uint32_t * table_32 = 0;
    uint16_t * table_16 = 0;
    //uint8_t * table_8 = 0;
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





    switch (mode) {
	case MODE_32_RGB:
	case MODE_32_BGR:
	    table_32 = (uint32_t*)malloc ((197 + 2*682 + 256 + 132) * sizeof (uint32_t));

	    entry_size = sizeof (uint32_t);
	    table_r = table_32 + 197;
	    table_b = table_32 + 197 + 685;
	    table_g = table_32 + 197 + 2*682;

	    if (swapped) {
		switch (mode) {
		    case MODE_32_RGB: shift_r =  8; shift_g = 16; shift_b = 24; break;
		    case MODE_32_BGR: shift_r = 24; shift_g = 16; shift_b =  8; break;
		}
	    } else {
		switch (mode) {
		    case MODE_32_RGB: shift_r = 16; shift_g =  8; shift_b =  0; break;
		    case MODE_32_BGR: shift_r =  0; shift_g =  8; shift_b = 16; break;
		}
	    }

	    for (i = -197; i < 256+197; i++)
		((uint32_t *) table_r)[i] = table_Y[i+384] << shift_r;
	    for (i = -132; i < 256+132; i++)
		((uint32_t *) table_g)[i] = table_Y[i+384] << shift_g;
	    for (i = -232; i < 256+232; i++)
		((uint32_t *) table_b)[i] = table_Y[i+384] << shift_b;
	    break;

	case MODE_15_BGR:
	case MODE_16_BGR:
	case MODE_15_RGB:
	case MODE_16_RGB:
	    table_16 = (uint16_t*)malloc ((197 + 2*682 + 256 + 132) * sizeof (uint16_t));

	    entry_size = sizeof (uint16_t);
	    table_r = table_16 + 197;
	    table_b = table_16 + 197 + 685;
	    table_g = table_16 + 197 + 2*682;

	    if (swapped) {
		switch (mode) {
		    case MODE_15_BGR: shift_r =  8; shift_g =  5; shift_b = 2; break;
		    case MODE_16_BGR: shift_r =  8; shift_g =  5; shift_b = 3; break;
		    case MODE_15_RGB: shift_r =  2; shift_g =  5; shift_b = 8; break;
		    case MODE_16_RGB: shift_r =  3; shift_g =  5; shift_b = 8; break;
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
	    break;

	default:
	    fprintf (stderr, "mode %d not supported by yuv2rgb\n", mode); // No tr
	    abort();
    }




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
	uint8_t *rV_table = (uint8_t *)c_this->table_rV;
	uint8_t *gU_table = (uint8_t *)c_this->table_gU;
	uint8_t *bU_table = (uint8_t *)c_this->table_bU;
	rV_table[i] += c_this->entry_size*(gamma - c_this->gamma);
	gU_table[i] += c_this->entry_size*(gamma - c_this->gamma);
	bU_table[i] += c_this->entry_size*(gamma - c_this->gamma);
    }
    c_this->gamma = gamma;





    /*
    * auto-probe for the best yuv2rgb function
    */
    c_this->yuv2rgb_fun = NULL;

/*
    #ifdef CONFIG_ARM
    yuv2rgb_init_arm ( c_this );
    if(c_this->yuv2rgb_fun != NULL)
	printf("yuv2rgb: using arm4l assembler for colorspace transform\n" );
    #endif
*/

    if (c_this->yuv2rgb_fun == NULL) {
	printf ("yuv2rgb: using unaccelerated colorspace conversion\n");
	if ( mode == MODE_32_RGB || mode == MODE_32_BGR)
	    c_this->yuv2rgb_fun = yuv2rgb_c_32;
	else
	    c_this->yuv2rgb_fun = yuv2rgb_c_16;
    }

    yuv2rgb_t *converter = c_this->converter = (yuv2rgb_t *) malloc (sizeof (yuv2rgb_t));
    converter->y_chunk = converter->y_buffer = 0;
    converter->u_chunk = converter->u_buffer = 0;
    converter->v_chunk = converter->v_buffer = 0;
    converter->table_rV = c_this->table_rV;
    converter->table_gU = c_this->table_gU;
    converter->table_gV = c_this->table_gV;
    converter->table_bU = c_this->table_bU;
    converter->yuv2rgb_fun = c_this->yuv2rgb_fun;
    converter->configure = yuv2rgb_configure;

    return c_this;
}





