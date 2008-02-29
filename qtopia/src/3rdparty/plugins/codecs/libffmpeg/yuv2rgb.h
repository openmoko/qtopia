/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/*
 * This appears to have been taken from Xine project and used
 * in Opie's player. I've adapted it slightly for use in the
 * libffmpeg plugin for Qtopia's mediaplayer.
 */

#ifndef HAVE_YUV2RGB_H
#define HAVE_YUV2RGB_h

#include <inttypes.h>

typedef struct yuv2rgb_s yuv2rgb_t;

typedef struct yuv2rgb_factory_s yuv2rgb_factory_t;

/*
 * function types for functions which can be replaced
 * by hardware-accelerated versions
 */

/* internal function use to scale yuv data */
typedef void (*scale_line_func_t) (uint8_t *source, uint8_t *dest, int width, int step);

typedef void (*yuv2rgb_fun_t) (yuv2rgb_t *c_this, uint8_t * image, uint8_t * py, uint8_t * pu, uint8_t * pv) ;

typedef void (*yuy22rgb_fun_t) (yuv2rgb_t *c_this, uint8_t * image, uint8_t * p);

typedef uint32_t (*yuv2rgb_single_pixel_fun_t) (yuv2rgb_t *c_this, uint8_t y, uint8_t u, uint8_t v);


/*
 * modes supported - feel free to implement yours
 */

#define MODE_8_RGB    1
#define MODE_8_BGR    2
#define MODE_15_RGB   3
#define MODE_15_BGR   4
#define MODE_16_RGB   5
#define MODE_16_BGR   6
#define MODE_24_RGB   7
#define MODE_24_BGR   8
#define MODE_32_RGB   9
#define MODE_32_BGR  10
#define	MODE_8_GRAY  11
#define MODE_PALETTE 12


#define FORMAT_YUV444	0
#define FORMAT_YUV422	1
#define FORMAT_YUV420	2
#define FORMAT_YUV411	3
#define FORMAT_YUV410	4


struct yuv2rgb_s {

  /*
   * configure converter for scaling factors
   */
  int (*configure) (yuv2rgb_t *c_this,
		    int source_width, int source_height,
		    int y_stride, int uv_stride,
		    int dest_width, int dest_height,
		    int rgb_stride, int format);

  /*
   * c_this is the function to call for the yuv2rgb and scaling process
   */
  yuv2rgb_fun_t     yuv2rgb_fun;

  /*
   * c_this is the function to call for the yuy2->rgb and scaling process
   */
  yuy22rgb_fun_t    yuy22rgb_fun;

  /*
   * c_this is the function to call for the yuv2rgb for a single pixel
   * (used for converting clut colors)
   */

  yuv2rgb_single_pixel_fun_t yuv2rgb_single_pixel_fun;

  /* private stuff below */

  int               source_width, source_height;
  int               y_stride, uv_stride;
  int               dest_width, dest_height;
  int               rgb_stride;
  int               step_dx, step_dy;
  int               do_scale;
#ifdef ARCH_XSCALE
  int               optimize;
#endif
  int               uv_stretch_x;
  int               uv_stretch_y;


  uint8_t          *y_buffer;
  uint8_t          *u_buffer;
  uint8_t          *v_buffer;
  void	           *y_chunk;
  void	           *u_chunk;
  void	           *v_chunk;

  void            **table_rV;
  void            **table_gU;
  int              *table_gV;
  void            **table_bU;

  uint8_t          *cmap;
  scale_line_func_t scale_line;
  
} ;

/*
 * convenience class to easily create a lot of converters
 */

struct yuv2rgb_factory_s {

  yuv2rgb_t* converter;

  int      mode;
  int      swapped;
  uint8_t *cmap;

  int      gamma;
  int      entry_size;

  uint32_t matrix_coefficients;

  void    *table_rV[256];
  void    *table_gU[256];
  int      table_gV[256];
  void    *table_bU[256];

  /* preselected functions for mode/swap/hardware */
  yuv2rgb_fun_t               yuv2rgb_fun;
  yuy22rgb_fun_t              yuy22rgb_fun;
  yuv2rgb_single_pixel_fun_t  yuv2rgb_single_pixel_fun;

};

yuv2rgb_factory_t *yuv2rgb_factory_init (int mode, int swapped, int gamma);



#endif
