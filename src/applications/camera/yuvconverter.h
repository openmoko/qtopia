/****************************************************************************
**
** Copyright (C) 2007-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef __QTOPIA_CAMERA_YUVCONVERTER_H
#define __QTOPIA_CAMERA_YUVCONVERTER_H

#include "formatconverter.h"

namespace camera
{

class YUVConverter : public FormatConverter
{
public:
    YUVConverter(unsigned int type, int width, int height);
    virtual ~YUVConverter();

    virtual unsigned char* convert(unsigned char* src);

private:

    unsigned int   m_type;  // V4L2_PIX_FMT_YUV_*
    int             m_width;
    int             m_height;
    unsigned char*  m_buf;
    int ui,vi,y1i,y2i;

};

}

#endif  //__QTOPIA_CAMERA_YUYVCONVERTER_H

