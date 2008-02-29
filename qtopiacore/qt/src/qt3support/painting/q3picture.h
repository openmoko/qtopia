/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3PICTURE_H
#define Q3PICTURE_H

#include <QtGui/qpicture.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3Picture : public QPicture
{
public:
    Q3Picture() : QPicture(-1) { }
    Q3Picture(const QPicture &pic) : QPicture(pic) { }
    bool load(QIODevice *dev, const char *format = 0);
    bool load(const QString &fileName, const char *format = 0);
    bool save(QIODevice *dev, const char *format = 0);
    bool save(const QString &fileName, const char *format = 0);
};

QT_END_HEADER

#endif // Q3PICTURE_H
