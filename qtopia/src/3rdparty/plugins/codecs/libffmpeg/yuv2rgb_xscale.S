@;
@; This file is an except of certain functions of the file yuv2rgb.c provided as a GNU
@; release, for more information see below.
@; 
@; The functions herein have been optimized by David Layman of the Intel Corporation
@; for the xscale processor
@; No warranties of fitness for any use are expressed or implied.
@;  
@; /**********************************************************************
@;** Copyright (C) 2000-2005 Trolltech AS and its licensors.
@;** All rights reserved.
@;**
@;** This file is part of the Qtopia Environment.
@;**
@;** This file may be distributed and/or modified under the terms of the
@;** GNU General Public License version 2 as published by the Free Software
@;** Foundation and appearing in the file LICENSE.GPL included in the
@;** packaging of this file.
@;**
@;** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
@;** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
@;**
@;** See http://www.trolltech.com/gpl/ for GPL licensing information.
@;** See below for additional copyright and license information
@;**
@;** Contact info@trolltech.com if any conditions of this licensing are
@;** not clear to you.
@;**
@;**********************************************************************/
@;/*
@; * yuv2rgb.c
@; *
@; * This file is part of xine, a unix video player.
@; *
@; * based on work from mpeg2dec:
@; * Copyright (C) 1999-2001 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
@; *
@; * This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
@; *
@; * mpeg2dec is free software; you can redistribute it and/or modify
@; * it under the terms of the GNU General Public License as published by
@; * the Free Software Foundation; either version 2 of the License, or
@; * (at your option) any later version.
@; *
@; * mpeg2dec is distributed in the hope that it will be useful,
@; * but WITHOUT ANY WARRANTY; without even the implied warranty of
@; * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@; * GNU General Public License for more details.
@; *
@; * You should have received a copy of the GNU General Public License
@; * along with c_this program; if not, write to the Free Software
@; * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
@; *
@; * $Id: yuv2rgb.c,v 1.3 2002/09/02 17:18:30 harlekin Exp $
@; */
@; 
@;
@; David Layman says...
@; we will keep the strategy of Scott Smith, making three separate sections 
@; depending on the step size, but
@; we will add a some new optimization strategies, that is:
@; 1. writing this in asm, always more efficient
@; 2. paying attention to xscale resource stalls
@; 3. loop unrolling
@; 4. preload
@; 5. write coalecing
@;

	.global scale_line_gen_xscale
	.global yuvrgbConvert
	.global yuvrgbCompute

	
@; C Function prototype
@; static void scale_line_gen_xscale (uint8_t *source, uint8_t *dest, int width, int step)
@;
@; r0 = source ptr
@; r1 = dest ptr
@; r2 = width
@; r3 = step
@; r4 = p1
@; r5 = p2
@; r6 = dx
@; r7 = output result 1
@; r8 = output result 2
@; r9 = output result 3
@; r10 = output result 4
@; r11 = modulo width by 4
@; r12 = scratch
	
scale_line_gen_xscale: .func
	
	@; store callee save registers, set up for return to lr
	stmdb sp,{r4-r12,lr}
	
	@; load first two bytes, sign extend
	ldrb r4,[r0],#1
	ldrb r5,[r0],#1

	@ ; make sure dx contents are 0
	mov r6,#0
	
	cmp r3,#32768
	blt expand
	cmp r3,#65536
	ble shrink
	b shrink_alot

expand:
@;*******************************************************************
@; This is to expand image, this could benefit from xscale tricks
@; but we are mainly optimizing for mobile platforms at this time
@; and it is likely we will alsways shrink image to fit screen
@; so we just implement this in (good quality) assembly code
@;*******************************************************************

@;
@;    while (width) {
@;      *dest = p1 + (((p2-p1) * dx)>>15);
@;
@;      dx += step;
@;      if (dx > 32768) {
@;	dx -= 32768;
@;	p1 = p2;
@;	p2 = *source++;
@;     }
@;      
@;      dest ++;
@;      width --;
@;    }
@;  }

a_start:
	@; p2 - p1
	sub r7,r5,r4
	@; (p2 - p1) * dx
	mul r12,r6,r7
	@; dx += step
	add r6,r6,r3
	@; dx > 32768 ?
	cmp r6,#32768
	@; add this to p1, shifted by 15
	add r7,r4,r12,asr #15
	@; if ( dx > 32768 ) dx -= 32768
	subgt r6,r6,#32768
	@; if ( dx > 32768 ) p1 = p2
	movgt r4,r5
	@; if ( dx > 32768 ) p2 = *source++;
	ldrgtb r5,[r0],#1

	@; maintain loop counter
	subs r2,r2,#1
	bgt a_start

	@; return to caller
	ldmdb sp,{r4-r12,pc}

shrink:
@;*******************************************************************
@; Some shrinkage, from 50% to 100% of original line
@;*******************************************************************
@;  else if (step <= 65536) {
@; while (width) {
@;   *dest = p1 + (((p2-p1) * dx)>>15);
@;
@;   dx += step;
@;   if (dx > 65536) {
@;	dx -= 65536;
@;	p1 = *source++;
@;	p2 = *source++;
@;    } else {
@;	dx -= 32768;
@;	p1 = p2;
@;	p2 = *source++;
@;   }
@;      
@;      dest ++;
@;      width --;
@;    }
@;
@; we are going to play a trick, we we adjust dx compensation down by 32768,
@; equivalent math, just all the adds and subs are smaller, eliminates the 
@; subtract of 32768

	@; reduce step by 32768
	sub r3,r3,#32768

	@; we are going to unroll loop by 4, so save off remainder
	@; and mask original count
	ands r11,r2,#0x3
	bic r2,r2,#0x3
	beq b_remainder

b_startby4:
	@; preload distance is at least 4 per loop, not always constant
	pld [r0,#32]

	@; first computation, result in r7
	@; p2-p1
	sub r7,r5,r4
	@; qty times dx
	mul r12,r6,r7
	@; adjust dx
	add r6,r6,r3
	@; bigger than 32768	
	cmp r6,#32768
	@; add this to p1, shifted down by 15
	add r7,r4,r12,asr #15
	@; if bigger, then replace both pixels, adjust dx
	@; else just replace p2
	mov r4,r5
	ldrgtb r4,[r0],#1
	ldrb r5,[r0],#1
	@; adjust dx
	subgt r6,r6,#32768

	@; second computation, result in r8
	@; p2-p1
	sub r8,r5,r4
	@; qty times dx
	mul r12,r6,r8
	@; adjust dx
	add r6,r6,r3
	@; bigger than 32768	
	cmp r6,#32768
	@; add this to p1, shifted down by 15
	add r8,r4,r12,asr #15
	@; if bigger, then replace both pixels, adjust dx
	@; else just replace p2
	mov r4,r5
	ldrgtb r4,[r0],#1
	ldrb r5,[r0],#1
	@; adjust dx
	subgt r6,r6,#32768

	@; third computation, result in r9
	@; p2-p1
	sub r9,r5,r4
	@; qty times dx
	mul r12,r6,r9
	@; adjust dx
	add r6,r6,r3
	@; bigger than 32768	
	cmp r6,#32768
	@; add this to p1, shifted down by 15
	add r9,r4,r12,asr #15
	@; if bigger, then replace both pixels, adjust dx
	@; else just replace p2
	mov r4,r5
	ldrgtb r4,[r0],#1
	ldrb r5,[r0],#1
	@; adjust dx
	subgt r6,r6,#32768

	@; fourth computation, result in r10
	@; p2-p1
	sub r10,r5,r4
	@; qty times dx
	mul r12,r6,r10
	@; adjust dx
	add r6,r6,r3
	@; bigger than 32768	
	cmp r6,#32768
	@; add this to p1, shifted down by 15
	add r10,r4,r12,asr #15
	@; if bigger, then replace both pixels, adjust dx
	@; else just replace p2
	mov r4,r5
	ldrgtb r4,[r0],#1
	ldrb r5,[r0],#1
	@; adjust dx
	subgt r6,r6,#32768

	@; we have computed 4 bytes for output, time to dump all at once
	@; this is a fine example of write coalescing
	@; a great xscale feature

	strb r7,[r1],#1
	strb r8,[r1],#1
	strb r9,[r1],#1
	strb r10,[r1],#1

	subs r2,r2,#4
	bgt b_startby4
	
b_remainder:
	movs r11,r11
	ldmeqdb sp,{r4-r12,pc}

b_remainder_loop:
	@; same as before, except one at a time, no write coelescing
	@; no loop unrolling, no preload
	@; first computation, result in r10
	@; p2-p1
	sub r7,r5,r4
	@; qty times dx
	mul r12,r6,r7
	@; adjust dx
	add r6,r6,r3
	@; bigger than 32768	
	cmp r6,#32768
	@; add this to p1, shifted down by 15
	add r7,r4,r12,asr #15
	@; if bigger, then replace both pixels, adjust dx
	@; else just replace p2
	mov r4,r5
	ldrgtb r4,[r0],#1
	ldrb r5,[r0],#1
	@; adjust dx
	subgt r6,r6,#32768

	@; output result
	strb r7,[r1],#1

	subs r11,r11,#1
	bgt b_remainder_loop

	@; return to caller
	ldmdb sp,{r4-r12,pc}

shrink_alot:
@;*******************************************************************
@; Lots of shrinkage
@;*******************************************************************
@;  
@;    while (width) {
@;      int offs;
@;      *dest = p1 + (((p2 - p1) * dx) >> 15);
@;      dx += step;
@;      offs=((dx-1)>>15);
@;      dx-=offs<<15;
@;      source+=offs-2;
@;      p1 = *source++;
@;      dest++;
@;      p2 = *source++;
@;      width--;
@;    }
@;
@;	same tricks as before, loop unrolling, write coalesing
@;   preload
@;
	@; we are going to unroll loop by 4, so save off remainder
	@; and mask original count
	ands r11,r2,#0x3
	bic r2,r2,#0x3
	beq c_remainder

c_startby4:
	@; preload distance is at least 8 per loop, not always constant
	pld [r0,#32]

	@; first computation, result in r7
	@; p2-p1
	sub r7,r5,r4
	@; qty times dx
	mul r12,r6,r7
	@; adjust dx, dx += step
	add r6,r6,r3
	@; offs = dx-1
	sub r14,r6,#1
	@; add this to p1, shifted down by 15
	add r7,r4,r12,asr #15
	@; offs >>= 15
	mov r14,r14,lsr #15
	@; source += offset - 2
	sub r14,r14,#2
	add r0,r0,r14
	@; p1 = *source++;
	ldrb r4,[r0],#1
	@; p2 = *source
	ldrb r5,[r0],#1	
	@; dx -= offs << 15
	sub r6,r6,r14,lsl #15

	@; second computation, result in r8
	@; p2-p1
	sub r8,r5,r4
	@; qty times dx
	mul r12,r6,r8
	@; adjust dx, dx += step
	add r6,r6,r3
	@; offs = dx-1
	sub r14,r6,#1
	@; add this to p1, shifted down by 15
	add r8,r4,r12,asr #15
	@; offs >>= 15
	mov r14,r14,lsr #15
	@; source += offset - 2
	sub r14,r14,#2
	add r0,r0,r14
	@; p1 = *source++;
	ldrb r4,[r0],#1
	@; p2 = *source
	ldrb r5,[r0],#1	
	@; dx -= offs << 15
	sub r6,r6,r14,lsl #15

	@; third computation, result in r9
	@; p2-p1
	sub r9,r5,r4
	@; qty times dx
	mul r12,r6,r9
	@; adjust dx, dx += step
	add r6,r6,r3
	@; offs = dx-1
	sub r14,r6,#1
	@; add this to p1, shifted down by 15
	add r9,r4,r12,asr #15
	@; offs >>= 15
	mov r14,r14,lsr #15
	@; source += offset - 2
	sub r14,r14,#2
	add r0,r0,r14
	@; p1 = *source++;
	ldrb r4,[r0],#1
	@; p2 = *source
	ldrb r5,[r0],#1	
	@; dx -= offs << 15
	sub r6,r6,r14,lsl #15

	@; fourth computation, result in r10

	@; decrement loop counter here, gives us a chance to
	@; wait for load latency
	subs r2,r2,#4

	@; p2-p1
	sub r10,r5,r4
	@; qty times dx
	mul r12,r6,r10
	@; adjust dx, dx += step
	add r6,r6,r3
	@; offs = dx-1
	sub r14,r6,#1
	@; add this to p1, shifted down by 15
	add r10,r4,r12,lsr #15
	@; offs >>= 15
	mov r14,r14,asr #15
	@; source += offset - 2
	sub r14,r14,#2
	add r0,r0,r14
	@; p1 = *source++;
	ldrb r4,[r0],#1
	@; p2 = *source
	ldrb r5,[r0],#1
	@; dx -= offs << 15
	sub r6,r6,r14,lsl #15

	@; write out results now, taking advantage of write coalescing
	strb r7,[r1],#1
	strb r8,[r1],#1
	strb r9,[r1],#1
	strb r10,[r1],#1

	bgt c_startby4

c_remainder:
	movs r11,r11
	ldmeqdb sp,{r4-r12,pc}
 
c_remainder_loop:
	@; p2-p1
	sub r7,r5,r4
	@; qty times dx
	mul r12,r6,r7
	@; adjust dx, dx += step
	add r6,r6,r3
	@; offs = dx-1
	sub r14,r6,#1
	@; add this to p1, shifted down by 15
	add r7,r4,r12,asr #15
	@; offs >>= 15
	mov r14,r14,lsr #15
	@; source += offset - 2
	sub r14,r14,#2
	add r0,r0,r14
	@; p1 = *source++;
	ldrb r4,[r0],#1
	@; p2 = *source
	ldrb r5,[r0],#1
	@; dx -= offs << 15
	sub r6,r6,r14,lsl #15

	@; store result
	strb r7,[r1],#1

	@; manage loop counter
	subs r11,r11,#1
	bgt c_remainder_loop

	@; return to caller
	ldmdb sp,{r4-r12,pc}

	.endfunc

@;*********************************************************************
@;
@; yuvrgbConvert
@; 
@; Made to nest inside yuv2rgb_c_16 - uses look up tables to convert
@; YUV to RGB
@;
@; yuvrgbConvert(uint8_t *y_buff,uint8_t *u_buff,uint8_t *v_buff,uint16_t *dst_buff,
@;               uint8_t *py,uint8_t *pu,uint8_t *pv,
@;               void *RedTable,void *BlueTable,void *GreenUTable,void *GreenVTable,
@;               int dest_width,int dest_height,int dy,int y_stride,int uv_stride,
@;               int step_dx,int step_dy,int rgb_stride,int uv_stretch)
@;
@; r0 = y_buffer
@; r1 = u_buffer
@; r2 = v_buffer
@; r3 = dest pointer
@; r4 = red pointer
@; r5 = green pointer
@; r6 = blue pointer
@; r7 = scratch
@; r8 = scratch
@; r9 = scratch
@; r10 = Base BlueTable
@; r11 = Base GreenUTable
@; r12 = Base GreenVTable
@; r14 = scratch
@;
@; *(sp) = save y_buff
@; *(sp+4) = save u_buff
@; *(sp+8) = save v_buff
@; *(sp+12) = save dst
@; *(sp+16) = local var,height
@; *(sp+60) = py
@; *(sp+64) = pu
@; *(sp+68) = pv
@; *(sp+72) = RedTable
@; *(sp+76) = BlueTable, later modified count
@; *(sp+80) = GreenUTable
@; *(sp+84) = GreenVTable
@; *(sp+88) = dest_width
@; *(sp+92) = dest_height
@; *(sp+96) = dy
@; *(sp+100) = y_stride
@; *(sp+104) = uv_stride
@; *(sp+108) = step_dx
@; *(sp+112) = step_dy
@; *(sp+116) = rgb_stride
@; *(sp+120) = uv_stretch
@;
yuvrgbConvert: .func	

	@; store callee save registers, set up for return to lr
	stmdb sp!,{r4-r12,lr}
        sub sp,sp,#20
	
	@; get width into r8
	ldr r8,[sp,#88]
	@; we are going to unroll by 8 pixels, so divide by 8
	mov r8,r8,asr #3
	
	@; get base pointer to BlueTable
	ldr r10,[sp,#76]
	@; get base pointer to GreenUTable
	ldr r11,[sp,#80]
	@; get base pointer to GreenVTable
	ldr r12,[sp,#84]
	
	@; save y_buffer
	str r0,[sp]
	@; save u_buffer
	str r1,[sp,#4]
	@; save v_buffer
	str r2,[sp,#8]
	@; save dest
	str r3,[sp,#12]
	@; zero out height counter
	mov r7,#0
	str r7,[sp,#16]

	@; store modified count
	str r8,[sp,#76]

	@; *********************************************************
	@; first chrominance pair
	@; *********************************************************
YUV_BY_4:

	@; get v
	ldrb r7,[r2],#1
	@; get u
	ldrb r14,[r1],#1
	@ get red base pointer
	ldr r9,[sp,#72]
	@; shift v
	mov r7,r7,lsl #2
	@; shift u
	mov r14,r14,lsl #2
	@; get red
	ldr r4,[r9,r7]
	@; get greenV
	ldr r5,[r12,r7]
	@; get greenU
	ldr r9,[r11,r14]
	@; get first y
	ldrb r7,[r0],#1
	@; get blue
	ldr r6,[r10,r14]

	@; add green components together, from above
	add r5,r5,r9
	@; shift y
	mov r7,r7,lsl #1
	@; get red
	ldrh r9,[r4,r7]
	@; get blue
	ldrh r14,[r6,r7]
	@; get green
	ldrh r8,[r5,r7]

	@ get second y
	ldrb r7,[r0],#1

	@ compute pixel
	add r9,r9,r14
	add r9,r9,r8
	
	@ store result
	strh r9,[r3],#2

	@; mult y by 2
	mov r7,r7,lsl #1
	@; get red
	ldrh r9,[r4,r7]
	@; get blue
	ldrh r14,[r6,r7]
	@; get green
	ldrh r8,[r5,r7]

	@ add R,G,B
	add r9,r9,r14
	add r9,r9,r8

	@; store result
	strh r9,[r3],#2

	@; *********************************************************
	@; second chrominance pair
	@; *********************************************************

	@; get v
	ldrb r7,[r2],#1
	@; get u
	ldrb r14,[r1],#1
	@ get red base pointer
	ldr r9,[sp,#72]
	@; shift v
	mov r7,r7,lsl #2
	@; shift u
	mov r14,r14,lsl #2
	@; get red
	ldr r4,[r9,r7]
	@; get greenV
	ldr r5,[r12,r7]
	@; get greenU
	ldr r9,[r11,r14]
	@; get first y
	ldrb r7,[r0],#1
	@; get blue
	ldr r6,[r10,r14]

	@; add green components together, from above
	add r5,r5,r9
	@; shift y
	mov r7,r7,lsl #1
	@; get red
	ldrh r9,[r4,r7]
	@; get blue
	ldrh r14,[r6,r7]
	@; get green
	ldrh r8,[r5,r7]

	@ get second y
	ldrb r7,[r0],#1

	@ compute pixel
	add r9,r9,r14
	add r9,r9,r8
	
	@ store result
	strh r9,[r3],#2

	@; mult y by 2
	mov r7,r7,lsl #1
	@; get red
	ldrh r9,[r4,r7]
	@; get blue
	ldrh r14,[r6,r7]
	@; get green
	ldrh r8,[r5,r7]

	@ add R,G,B
	add r9,r9,r14
	add r9,r9,r8

	@; store result
	strh r9,[r3],#2


	@; *********************************************************
	@; third chrominance pair
	@; *********************************************************

	@; get v
	ldrb r7,[r2],#1
	@; get u
	ldrb r14,[r1],#1
	@ get red base pointer
	ldr r9,[sp,#72]
	@; shift v
	mov r7,r7,lsl #2
	@; shift u
	mov r14,r14,lsl #2
	@; get red
	ldr r4,[r9,r7]
	@; get greenV
	ldr r5,[r12,r7]
	@; get greenU
	ldr r9,[r11,r14]
	@; get first y
	ldrb r7,[r0],#1
	@; get blue
	ldr r6,[r10,r14]

	@; add green components together, from above
	add r5,r5,r9
	@; shift y
	mov r7,r7,lsl #1
	@; get red
	ldrh r9,[r4,r7]
	@; get blue
	ldrh r14,[r6,r7]
	@; get green
	ldrh r8,[r5,r7]

	@ get second y
	ldrb r7,[r0],#1

	@ compute pixel
	add r9,r9,r14
	add r9,r9,r8
	
	@ store result
	strh r9,[r3],#2

	@; mult y by 2
	mov r7,r7,lsl #1
	@; get red
	ldrh r9,[r4,r7]
	@; get blue
	ldrh r14,[r6,r7]
	@; get green
	ldrh r8,[r5,r7]

	@ add R,G,B
	add r9,r9,r14
	add r9,r9,r8

	@; store result
	strh r9,[r3],#2

	@; *********************************************************
	@; fourth chrominance pair
	@; *********************************************************

	@; get v
	ldrb r7,[r2],#1
	@; get u
	ldrb r14,[r1],#1
	@ get red base pointer
	ldr r9,[sp,#72]
	@; shift v
	mov r7,r7,lsl #2
	@; shift u
	mov r14,r14,lsl #2
	@; get red
	ldr r4,[r9,r7]
	@; get greenV
	ldr r5,[r12,r7]
	@; get greenU
	ldr r9,[r11,r14]
	@; get first y
	ldrb r7,[r0],#1
	@; get blue
	ldr r6,[r10,r14]

	@; add green components together, from above
	add r5,r5,r9
	@; shift y
	mov r7,r7,lsl #1
	@; get red
	ldrh r9,[r4,r7]
	@; get blue
	ldrh r14,[r6,r7]
	@; get green
	ldrh r8,[r5,r7]

	@ get second y
	ldrb r7,[r0],#1

	@ compute pixel
	add r9,r9,r14
	add r9,r9,r8
	
	@ store result
	strh r9,[r3],#2

	@; mult y by 2
	mov r7,r7,lsl #1
	@; get red
	ldrh r9,[r4,r7]
	@; get blue
	ldrh r14,[r6,r7]
	@; get green
	ldrh r8,[r5,r7]

	@ add R,G,B
	add r9,r9,r14
	add r9,r9,r8

	@; store result
	strh r9,[r3],#2

	@; *********************************************************
	@; update loop count
	@; *********************************************************

	ldr r8,[sp,#76]
	subs r8,r8,#1
	str r8,[sp,#76]

	bgt YUV_BY_4
	
	@; **********************************************************
	@; if there is a single pair left, lets take care of it
	@; **********************************************************
	
	ldr r8,[sp,#88]
	ands r8,r8,#0x2
	beq testSingle

	@; *********************************************************
	@; last pair
	@; *********************************************************

	@; get v
	ldrb r7,[r2],#1
	@; get u
	ldrb r14,[r1],#1
	@ get red base pointer
	ldr r9,[sp,#72]
	@; shift v
	mov r7,r7,lsl #2
	@; shift u
	mov r14,r14,lsl #2
	@; get red
	ldr r4,[r9,r7]
	@; get greenV
	ldr r5,[r12,r7]
	@; get greenU
	ldr r9,[r11,r14]
	@; get first y
	ldrb r7,[r0],#1
	@; get blue
	ldr r6,[r10,r14]

	@; add green components together, from above
	add r5,r5,r9
	@; shift y
	mov r7,r7,lsl #1
	@; get red
	ldrh r9,[r4,r7]
	@; get blue
	ldrh r14,[r6,r7]
	@; get green
	ldrh r8,[r5,r7]

	@ get second y
	ldrb r7,[r0],#1

	@ compute pixel
	add r9,r9,r14
	add r9,r9,r8
	
	@ store result
	strh r9,[r3],#2

	@; mult y by 2
	mov r7,r7,lsl #1
	@; get red
	ldrh r9,[r4,r7]
	@; get blue
	ldrh r14,[r6,r7]
	@; get green
	ldrh r8,[r5,r7]

	@ add R,G,B
	add r9,r9,r14
	add r9,r9,r8

	@; store result
	strh r9,[r3],#2

    	@; ******************************************************************
	@; is there one more pixel to do ?
	@; ******************************************************************

testSingle:
	ldr r8,[sp,#88]
	ands r8,r8,#0x1
	beq scale_next

	@; get v
	ldrb r7,[r2],#1
	@; get u
	ldrb r14,[r1],#1
	@ get red base pointer
	ldr r9,[sp,#72]
	@; shift v
	mov r7,r7,lsl #2
	@; shift u
	mov r14,r14,lsl #2
	@; get red
	ldr r4,[r9,r7]
	@; get greenV
	ldr r5,[r12,r7]
	@; get greenU
	ldr r9,[r11,r14]
	@; get first y
	ldrb r7,[r0],#1
	@; get blue
	ldr r6,[r10,r14]

	@; add green components together, from above
	add r5,r5,r9
	@; shift y
	mov r7,r7,lsl #1
	@; get red
	ldrh r9,[r4,r7]
	@; get blue
	ldrh r14,[r6,r7]
	@; get green
	ldrh r8,[r5,r7]

	@ compute pixel
	add r9,r9,r14
	add r9,r9,r8
	
	@ store result
	strh r9,[r3],#2

scale_next:
	@; are we done?
	@; get dest_height
	ldr r9,[sp,#92]
	@; get dy
	ldr r8,[sp,#96]
	@; get step_dy
	ldr r14,[sp,#112]
	subs r9,r9,#1

	@; if done, then return
	addle sp,sp,#20
	ldmleia sp!,{r4-r12,pc}

	@; dy += step_dy
	add r8,r8,r14
	@; get dest
	ldr r7,[sp,#12]
	@; get rgb_stride
	ldr r14,[sp,#116]
	@; store dest_height
	str r9,[sp,#92]
	@; dest += rgb_stride
	add r3,r7,r14
	str r3,[sp,#12]
                                                                                                           
        @********************************************************************************
        @ Now we go to the next horizontal line, but we have to account for scaling
        @********************************************************************************
        @
        @ Original C Code:
        @
        @ do {
        @    dy -= 32768;
        @    _py += c_this->y_stride;
        @
        @    scale_line (_py, c_this->y_buffer,
        @              c_this->dest_width, c_this->step_dx);
        @
        @    if (height & 1 || !c_this->uv_stretch_y ) {
        @      _pu += c_this->uv_stride;
        @      _pv += c_this->uv_stride;
        @
        @      scale_line (_pu, c_this->u_buffer,
        @                  c_this->dest_width >> 1, c_this->step_dx);
        @      scale_line (_pv, c_this->v_buffer,
        @                  C_THIS->dest_width >> 1,c_this->step_dx);
        @      }
        @    height++;
        @    } while( dy>=32768);
        @ *********************************************************************************
        @

	@; NOTE ON REGISTERS
	@; r0 = py
        @; r1 = pu
        @; r2 = pv
	@; r3 = y_stride
        @; r14 = uv_stride
	@; r8 = dy
	@; r7 = height
	
	@; get py
	ldr r0,[sp,#60]
	@; get pu
	ldr r1,[sp,#64]
	@; get pv
	ldr r2,[sp,#68]
	@; get y_stride
	ldr r3,[sp,#100]
	@; get  uv_stride
	ldr r14,[sp,#104]
	@; dy is already in 8
	@; get height
	ldr r7,[sp,#16]

Start_Do_Scale:
	@; dy-=32768
	sub r8,r8,#32768
	@; py += y_stride
	add r0,r0,r3
	
	@; if height is odd, and !uv_stretch
        ands r9,r7,#1
        bgt scale_uv
	@; get uv_stretch_y
        ldr r9,[sp,#120]
	movs r9,r9
        beq scale_uv
	
	 @; inc height
        add r7,r7,#1
        cmp r8,#32768
        bge Start_Do_Scale
	b DoneDoWhile

scale_uv:
	@; pu += uv_stride
	add r1,r1,r14
	@; pv += uv_stride
	add r2,r2,r14

        @; inc height
        add r7,r7,#1
        cmp r8,#32768
        bge Start_Do_Scale

DoneDoWhile:
	@; we are done with the do loop, store important vars
	@; store py
	str r0,[sp,#60]
	@; store pu
	str r1,[sp,#64]
	@; store pv
	str r2,[sp,#68]
	@; store height
	str r7,[sp,#16]
	@; store dy
	str r8,[sp,#96]

	@; call scale line on y (chrominance)
	@; py is already in r0
	@; get y_buff in r1
	ldr r1,[sp] @; get y_buff
	@; get dest_width
	ldr r2,[sp,#88]
	@; get step_dx
	ldr r3,[sp,#108]

	@; save r2, r3 for later
	mov r7,r2
	mov r9,r3

	@; for chrominance	
	bl scale_line_gen_xscale
		
	@; scale u line
	@; get pu in r0
	ldr r0,[sp,#64]
	@; get u_buff in r1
	ldr r1,[sp,#4]
	@; get width / 2
	mov r2,r7,asr #1
	@; get step dx
	mov r3,r9
	
	@; for u line
	bl scale_line_gen_xscale

	@; scale v line
	@; get pv
	ldr r0,[sp,#68]
	@; get p_buff
	ldr r1,[sp,#8]
	@; get width / 2
	mov r2,r7,asr #1
	@; get step dx
	mov r3,r9

	bl scale_line_gen_xscale
	
	@; we now have to save off the vars we have held in registers, restore
	@; r0 - r4, and go back for another pass at line conversion
	
	@; get dest_width
	ldr r14,[sp,#88]
	@; get y_buff in r0
	ldr r0,[sp]
	@; get u_buff in r1
	ldr r1,[sp,#4]
	@; get v_buff in r2
	ldr r2,[sp,#8]
	@; get dest in r3
	ldr r3,[sp,#12]
	@; adjust dest width
	mov r14,r14,asr #3
	@; store in counter var
	str r14,[sp,#76]

        b YUV_BY_4

	.endfunc

	.end


