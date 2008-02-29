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

#ifndef QXPMHANDLER_P_H
#define QXPMHANDLER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qimageiohandler.h"

#ifndef QT_NO_IMAGEFORMAT_XPM

class QXpmHandler : public QImageIOHandler
{
public:
    QXpmHandler();
    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    static bool canRead(QIODevice *device);

    QByteArray name() const;

    QVariant option(ImageOption option) const;
    void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;

private:
    bool readHeader();
    bool readImage(QImage *image);
    enum State {
        Ready,
        ReadHeader,
        Error
    };
    State state;
    int width;
    int height;
    int ncols;
    int cpp;
    QByteArray buffer;
    int index;
    QString fileName;
};

#endif // QT_NO_IMAGEFORMAT_XPM

#endif // QXPMHANDLER_P_H
