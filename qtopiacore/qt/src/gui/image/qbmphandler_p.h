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

#ifndef QBMPHANDLER_P_H
#define QBMPHANDLER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qimageiohandler.h"

#ifndef QT_NO_IMAGEFORMAT_BMP

struct BMP_FILEHDR {                     // BMP file header
    char   bfType[2];                    // "BM"
    qint32  bfSize;                      // size of file
    qint16  bfReserved1;
    qint16  bfReserved2;
    qint32  bfOffBits;                   // pointer to the pixmap bits
};

struct BMP_INFOHDR {                     // BMP information header
    qint32  biSize;                      // size of this struct
    qint32  biWidth;                     // pixmap width
    qint32  biHeight;                    // pixmap height
    qint16  biPlanes;                    // should be 1
    qint16  biBitCount;                  // number of bits per pixel
    qint32  biCompression;               // compression method
    qint32  biSizeImage;                 // size of image
    qint32  biXPelsPerMeter;             // horizontal resolution
    qint32  biYPelsPerMeter;             // vertical resolution
    qint32  biClrUsed;                   // number of colors used
    qint32  biClrImportant;              // number of important colors
};

class QBmpHandler : public QImageIOHandler
{
public:
    QBmpHandler();
    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const;
    void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;

private:
    bool readHeader();
    enum State {
        Ready,
        ReadHeader,
        Error
    };
    State state;
    BMP_FILEHDR fileHeader;
    BMP_INFOHDR infoHeader;
    int startpos;
};

#endif

#endif // QBMPHANDLER_P_H
