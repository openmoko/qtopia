/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __QTOPIA_CAMERA_FORMATCONVERTER_H
#define __QTOPIA_CAMERA_FORMATCONVERTER_H

#include <QList>

namespace camera
{

/*
 * convert something to RGB32
 */

class FormatConverter
{
public:

    virtual ~FormatConverter() {}

    virtual unsigned char* convert(unsigned char* src) = 0;

    static FormatConverter* createFormatConverter(unsigned int type, int width, int height);
    static void releaseFormatConverter(FormatConverter* converter);
    static QList<unsigned int> supportedFormats();
};


class NullConverter : public FormatConverter
{
public:

    virtual unsigned char* convert(unsigned char* src);
};

}   // ns camera

#endif  //__QTOPIA_CAMERA_FORMATCONVERTER_H
