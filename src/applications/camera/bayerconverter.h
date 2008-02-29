/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __QTOPIA_CAMERA_BAYERCONVERTER_H
#define __QTOPIA_CAMERA_BAYERCONVERTER_H

#include "formatconverter.h"

namespace camera
{

class BayerConverter : public FormatConverter
{
public:
    BayerConverter(int width, int height);
    virtual ~BayerConverter();

    virtual unsigned char* convert(unsigned char* src);

private:

    int             m_width;
    int             m_height;
    unsigned char*  m_buf;
};

}

#endif  //__QTOPIA_CAMERA_BAYERCONVERTER_H

