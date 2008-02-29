/*
 * yuv2rgb_arm.c
 * Copyright (C) 2000-2001 Project OPIE.
 * All Rights Reserved.
 *
 * Author: Robert Griebl <sandman@handhelds.org>
 *
 * This file is part of OpiePlayer2.
 *
 * OpiePlayer2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * OpiePlayer2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with c_this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This is now also a part of Qtopia as a Qtopia MediaPlayer plugin.
 * If is being used by the libffmpegplugin.
 * It appears that this code originated from the xine player.
 * I found this in the OpiePlayer2 which is based on xine and have
 * adapted it for use in the libffmpegplugin instead. I removed the
 * dependancies on the xine header files and functions/defines etc
 * so that this can be compiled without needing xine.
 * Having this routine in asm on the arm is fairly important to good
 * perfornace so this is a fairly critical piece of code. Also the
 * iDCT is another important section of code which has asm optimizations
 * for the arm processor in libffmpeg/libav/libavcodec which improves
 * performance considerably.
 */

#include <linux/config.h>
#ifdef CONFIG_ARM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


struct dummy {
	uint8_t *yuv [3];
	int stride [3];
};


#ifdef __cplusplus
extern "C" 
#endif
void convert_yuv420_rgb565(struct dummy *picture, unsigned char *results, int w, int h);


static void arm_rgb16 (yuv2rgb_t *c_this, uint8_t * _dst,
               uint8_t * _py, uint8_t * _pu, uint8_t * _pv)
{
	if ( !c_this-> do_scale ) {
		struct dummy d;
		d. yuv [0] = _py;
		d. yuv [1] = _pu;
		d. yuv [2] = _pv;
		d. stride [0] = c_this-> y_stride;
		d. stride [1] = d. stride [2] =  c_this-> uv_stride;

//		printf( "calling arm (%dx%d)\n", c_this-> dest_width, c_this-> dest_height );

		convert_yuv420_rgb565 ( &d, _dst, c_this->dest_width, c_this->dest_height );

//		printf ( "arm done\n" );
    }
    else {
		int U, V, Y;
		uint8_t * py_1, * py_2, * pu, * pv;
		uint16_t * r, * g, * b;
		uint16_t * dst_1, * dst_2;
		int width, height, dst_height;
		int dy;

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
		    for ( int i = 0; i < edge_pixels; i++ )
		      *dst_1++ = 0;

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
    }
}
                       


void yuv2rgb_init_arm (yuv2rgb_factory_t *c_this) {

  if (c_this->swapped) 
    return; /*no swapped pixel output upto now*/

  switch (c_this->mode) {
  case MODE_16_RGB:
    c_this->yuv2rgb_fun = arm_rgb16;
    break;
  }
}



#endif
