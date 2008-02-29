/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPRINTENGINE_PDF_P_H
#define QPRINTENGINE_PDF_P_H

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

#include "QtGui/qprintengine.h"

#ifndef QT_NO_PRINTER
#include "QtCore/qmap.h"
#include "QtGui/qmatrix.h"
#include "QtCore/qstring.h"
#include "QtCore/qvector.h"
#include "QtGui/qpaintengine.h"
#include "QtGui/qpainterpath.h"
#include "QtCore/qdatastream.h"

#include "private/qfontengine_p.h"
#include "private/qpdf_p.h"
#include "private/qpaintengine_p.h"

// #define USE_NATIVE_GRADIENTS

class QImage;
class QDataStream;
class QPen;
class QPointF;
class QRegion;
class QFile;
class QPdfEngine;

class QPdfEnginePrivate;

class QPdfEngine : public QPdfBaseEngine
{
    Q_DECLARE_PRIVATE(QPdfEngine)
public:
    QPdfEngine(QPrinter::PrinterMode m);
    virtual ~QPdfEngine();

    // reimplementations QPaintEngine
    bool begin(QPaintDevice *pdev);
    bool end();
    void drawPixmap (const QRectF & rectangle, const QPixmap & pixmap, const QRectF & sr);
    void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                   Qt::ImageConversionFlags flags = Qt::AutoColor);
    void drawTiledPixmap (const QRectF & rectangle, const QPixmap & pixmap, const QPointF & point);

    Type type() const;
    // end reimplementations QPaintEngine

    // reimplementations QPrintEngine
    bool abort() {return false;}
    bool newPage();
    QPrinter::PrinterState printerState() const {return state;}
    // end reimplementations QPrintEngine

    void setBrush();

    // ### unused, should have something for this in QPrintEngine
    void setAuthor(const QString &author);
    QString author() const;

    void setDevice(QIODevice* dev);

private:
    Q_DISABLE_COPY(QPdfEngine)

    QPrinter::PrinterState state;
};

class QPdfEnginePrivate : public QPdfBaseEnginePrivate
{
    Q_DECLARE_PUBLIC(QPdfEngine)
public:
    QPdfEnginePrivate(QPrinter::PrinterMode m);
    ~QPdfEnginePrivate();

    void newPage();

    int width() const {
        QRect r = paperRect();
        return qRound(r.width()*72./resolution);
    }
    int height() const {
        QRect r = paperRect();
        return qRound(r.height()*72./resolution);
    }

    void writeHeader();
    void writeTail();

    int addImage(const QImage &image, bool *bitmap, qint64 serial_no);
    int addBrushPattern(const QMatrix &matrix, bool *specifyColor, int *gStateObject);

    QMatrix pageMatrix() const;

private:
    Q_DISABLE_COPY(QPdfEnginePrivate)

#ifdef USE_NATIVE_GRADIENTS
    int gradientBrush(const QBrush &b, const QMatrix &matrix, int *gStateObject);
#endif

    void writeInfo();
    void writePageRoot();
    void writeFonts();
    void embedFont(QFontSubset *font);

    QVector<int> xrefPositions;
    QDataStream* stream;
    int streampos;

    int writeImage(const QByteArray &data, int width, int height, int depth,
                   int maskObject, int softMaskObject, bool dct = false);
    void writePage();

    int addXrefEntry(int object, bool printostr = true);
    void xprintf(const char* fmt, ...);
    inline void write(const QByteArray &data) {
        stream->writeRawData(data.constData(), data.size());
        streampos += data.size();
    }

    int writeCompressed(const char *src, int len);
    inline int writeCompressed(const QByteArray &data) { return writeCompressed(data.constData(), data.length()); }

    // various PDF objects
    int pageRoot, catalog, info, graphicsState, patternColorSpace;
    QVector<uint> pages;
    QHash<qint64, uint> imageCache;
};


#endif // QT_NO_PRINTER
#endif // QPRINTENGINE_PDF_P_H
