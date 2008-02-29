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
#ifndef VIDEO_SCALE_CONTEXT_H
#define VIDEO_SCALE_CONTEXT_H


extern "C" {
#include "avcodec.h"
#include "avformat.h"
};

#include <qtopia/mediaplayerplugininterface.h>
#include "yuv2rgb.h"


//#define USE_LIB_AVCODEC_IMG_RESAMPLE


class VideoScaleContext {
public:
#ifdef USE_LIB_AVCODEC_IMG_RESAMPLE
    AVPicture outputPic1;
    AVPicture outputPic2;

    VideoScaleContext() {
	videoScaleContext2 = 0;
	outputPic1.data[0] = 0;
	outputPic2.data[0] = 0;
    }

    virtual ~VideoScaleContext() {
	free();
    }

    void free() {
	if ( videoScaleContext2 )
	    img_resample_close(videoScaleContext2);
	videoScaleContext2 = 0;
	if ( outputPic1.data[0] )
	    avpicture_free(&outputPic1);
	outputPic1.data[0] = 0;
	if ( outputPic2.data[0] )
	    avpicture_free(&outputPic2);
	outputPic2.data[0] = 0;
    }
#else
    VideoScaleContext() {
    }

    virtual ~VideoScaleContext() {
    }
#endif

    void init() {
	scaleContextDepth = -1;
	scaleContextInputWidth = -1;
	scaleContextInputHeight = -1;
	scaleContextPicture1Width = -1;
	scaleContextPicture2Width = -1;
	scaleContextOutputWidth = -1;
	scaleContextOutputHeight = -1;
	scaleContextLineStride = -1;
    }

    bool configure(int w, int h, int outW, int outH, AVFrame *picture, int lineStride, int fmt, ColorFormat outFmt ) {
#ifdef USE_LIB_AVCODEC_IMG_RESAMPLE
	int colorMode = -1;
	switch ( outFmt ) {
	    case RGB565:   colorMode = PIX_FMT_RGB565; break;
	    case BGR565:   colorMode = PIX_FMT_RGB565; break;
	    case RGBA8888: colorMode = PIX_FMT_RGBA32; break;
	    case BGRA8888: colorMode = PIX_FMT_RGBA32; break;
	};
	scaleContextFormat = fmt;
	scaleContextDepth = colorMode;
	if ( scaleContextInputWidth != w || scaleContextInputHeight != h
				|| scaleContextOutputWidth != outW || scaleContextOutputHeight != outH ) {
	    scaleContextInputWidth = w;
	    scaleContextInputHeight = h;
	    scaleContextOutputWidth = outW;
	    scaleContextOutputHeight = outH;
	    scaleContextLineStride = lineStride;
	    free();
	    if ( !(videoScaleContext2 = img_resample_init(outW, outH, w, h)) )
		return false;
	    if ( avpicture_alloc(&outputPic1, scaleContextFormat, scaleContextOutputWidth, scaleContextOutputHeight) < 0 )
		return false;
	    if ( avpicture_alloc(&outputPic2, scaleContextDepth, scaleContextOutputWidth, scaleContextOutputHeight) < 0 )
		return false;
	}
	return true;
#else
	int pw1 = picture->linesize[0];
	int pw2 = picture->linesize[1];

	int colorMode = -1;
	switch ( outFmt ) {
	    case RGB565:   colorMode = MODE_16_RGB; break;
	    case BGR565:   colorMode = MODE_16_BGR; break;
	    case RGBA8888: colorMode = MODE_32_RGB; break;
	    case BGRA8888: colorMode = MODE_32_BGR; break;
	};
	if ( colorMode != scaleContextDepth ) {
	    scaleContextDepth = colorMode;
	    videoScaleContext = yuv2rgb_factory_init( colorMode, 0, 0 );
	}
	// Check if any colour space conversion variables have changed
	// since the last decoded frame which will require
	// re-initialising the colour space tables 
	if ( scaleContextInputWidth != w || scaleContextInputHeight != h ||
	    scaleContextPicture1Width != pw1 || scaleContextPicture2Width != pw2 ||
	    scaleContextOutputWidth != outW || scaleContextOutputHeight != outH ||
	    scaleContextLineStride != lineStride || scaleContextFormat != fmt ) {

	    scaleContextInputWidth = w;
	    scaleContextInputHeight = h;
	    scaleContextPicture1Width = pw1;
	    scaleContextPicture2Width = pw2;
	    scaleContextOutputWidth = outW;
	    scaleContextOutputHeight = outH;
	    scaleContextLineStride = lineStride;
	    scaleContextFormat = fmt;

	    int format = 0;
	    switch ( fmt ) {
		case PIX_FMT_YUV444P:
		    format = FORMAT_YUV444;
		    break;
		case PIX_FMT_YUV422P:
		    format = FORMAT_YUV422;
		    break;
		case PIX_FMT_YUV420P:
		    format = FORMAT_YUV420;
		    break;
		case PIX_FMT_YUV411P:
		    format = FORMAT_YUV411;
		    break;
		case PIX_FMT_YUV410P:
		    format = FORMAT_YUV410;
		    break;
		default:
		    qDebug("format not supported: %i", fmt );
		    return false;
		    break;
	    };

	    qDebug("reconfiguring scale context");
	    videoScaleContext->converter->configure( videoScaleContext->converter, w, h, pw1, pw2, outW, outH, lineStride, format );
//	    qDebug("configured yuv convert context with - input: %i x %i  pic lines: %i %i, output: %i x %i, linestride: %i",
//			 w, h, pw1, pw2, outW, outH, lineStride );
	}
	return true;
#endif
    }

    void convert(uint8_t *output, AVFrame *picture) {
#ifdef USE_LIB_AVCODEC_IMG_RESAMPLE
	if ( !videoScaleContext2 || !picture || !outputPic1.data[0] || !outputPic2.data[0] )
	    return;
	img_resample(videoScaleContext2, &outputPic1, (AVPicture*)picture);
	img_convert(&outputPic2, scaleContextDepth, &outputPic1, scaleContextFormat, scaleContextOutputWidth, scaleContextOutputHeight);
	int offset = 0;
	for ( int i = 0; i < scaleContextOutputHeight; i++ ) {
	    memcpy( output, outputPic2.data[0] + offset, outputPic2.linesize[0] );
	    output += scaleContextLineStride;
	    offset += outputPic2.linesize[0]; 
	}
#else
	uint8_t *y = picture->data[0];
	uint8_t *u = picture->data[1];
	uint8_t *v = picture->data[2];
	videoScaleContext->converter->yuv2rgb_fun( videoScaleContext->converter, output, y, u, v );
#endif
    }

private:
#ifdef USE_LIB_AVCODEC_IMG_RESAMPLE
    ImgReSampleContext *videoScaleContext2;
#else
    yuv2rgb_factory_t *videoScaleContext;
#endif
    int scaleContextDepth;
    int scaleContextInputWidth;
    int scaleContextInputHeight;
    int scaleContextPicture1Width;
    int scaleContextPicture2Width;
    int scaleContextOutputWidth;
    int scaleContextOutputHeight;
    int scaleContextLineStride;
    int scaleContextFormat;
};


#endif // VIDEO_SCALE_CONTEXT_H

