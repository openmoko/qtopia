/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/videodev.h>

#ifdef HAVE_V4L2

#include "formatconverter.h"
#include "bayerconverter.h"

namespace camera
{

/*
 * convert something to RGB32
 */

FormatConverter* FormatConverter::createFormatConverter(unsigned int type, int width, int height)
{
    switch (type)
    {
    case V4L2_PIX_FMT_RGB332:
    case V4L2_PIX_FMT_RGB555:
    case V4L2_PIX_FMT_RGB565:
    case V4L2_PIX_FMT_RGB555X:
    case V4L2_PIX_FMT_RGB565X:
    case V4L2_PIX_FMT_RGB24:
    case V4L2_PIX_FMT_RGB32:
    case V4L2_PIX_FMT_GREY:
    case V4L2_PIX_FMT_YVU410:
    case V4L2_PIX_FMT_YVU420:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_UYVY:
    case V4L2_PIX_FMT_YUV422P:
    case V4L2_PIX_FMT_YUV411P:
    case V4L2_PIX_FMT_Y41P:
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV21:
    case V4L2_PIX_FMT_YUV410:
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YYUV:
    case V4L2_PIX_FMT_HI240:
        return new NullConverter;

    case V4L2_PIX_FMT_SBGGR8:
        return new BayerConverter(width, height);
    }

    return new NullConverter;
}

void FormatConverter::releaseFormatConverter(FormatConverter* converter)
{
    delete converter;
}


// Null Converter
unsigned char* NullConverter::convert(unsigned char* src)
{
    return src;
}


}   // ns camera


#endif // HAVE_V4L2
