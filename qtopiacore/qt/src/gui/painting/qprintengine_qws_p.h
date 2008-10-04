/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QPRINTENGINE_QWS_P_H
#define QPRINTENGINE_QWS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qprinter.h"

#ifndef QT_NO_PRINTER

#include "QtGui/qprintengine.h"
#include "QtCore/qbytearray.h"
#include "private/qpaintengine_p.h"

QT_BEGIN_NAMESPACE

class QtopiaPrintEnginePrivate;
class QRasterPaintEngine;
class QPrinterPrivate;
class QImage;

class QtopiaPrintEngine : public QPaintEngine, public QPrintEngine
{
    Q_DECLARE_PRIVATE(QtopiaPrintEngine)
public:
    QtopiaPrintEngine(QPrinter::PrinterMode mode);

    // override QWSPaintEngine
    bool begin(QPaintDevice *dev);
    bool end();
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    void drawTextItem(const QPointF &p, const QTextItem &ti);
    QPaintEngine::Type type() const { return QPaintEngine::X11; }

    QPaintEngine *paintEngine() const;

    void updateState(const QPaintEngineState &state);

    QRect paperRect() const;
    QRect pageRect() const;

    bool newPage();
    bool abort();

    QPrinter::PrinterState printerState() const;

    int metric(QPaintDevice::PaintDeviceMetric metricType) const;

    QVariant property(PrintEnginePropertyKey key) const;
    void setProperty(PrintEnginePropertyKey key, const QVariant &value);

private:
    friend class QPrintDialog;
    friend class QPageSetupDialog;

    void clearPage();
    void flushPage();
};

class QtopiaPrintBuffer
{
public:
    QtopiaPrintBuffer( bool bigEndian=FALSE ) { _bigEndian = bigEndian; }
    ~QtopiaPrintBuffer() {}

    const QByteArray& data() const { return _data; }

    int size() const { return _data.size(); }

    void clear() { _data.clear(); }

    void append( char value ) { _data.append( value ); }
    void append( short value );
    void append( int value );
    void append( const QByteArray& array ) { _data.append( array ); }

    void patch( int posn, int value );

    void pad();

private:
    QByteArray _data;
    bool _bigEndian;
};

#define	QT_QWS_PRINTER_DEFAULT_DPI	   200

class QtopiaPrintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QtopiaPrintEngine)
public:
    QtopiaPrintEnginePrivate(QPrinter::PrinterMode m) :
	mode(m),
	printerState(QPrinter::Idle),
	orientation(QPrinter::Portrait),
	paperSize(QPrinter::A4),
	pageOrder(QPrinter::FirstPageFirst),
	colorMode(QPrinter::GrayScale),
	paperSource(QPrinter::OnlyOne),
        resolution(QT_QWS_PRINTER_DEFAULT_DPI),
        paintEngine(0),
	numCopies(1),
        outputToFile(false),
        fullPage(false),
        collateCopies(false),
	pageNumber(0),
	pageImage(0),
	partialByte(0),
	partialBits(0)
    {
    }
    ~QtopiaPrintEnginePrivate();

    void initialize();

    QPrinter::PrinterMode mode;

    QString printerName;
    QString outputFileName;
    QString printProgram;
    QString docName;
    QString creator;

    QPrinter::PrinterState printerState;

    QPrinter::Orientation orientation;
    QPrinter::PaperSize paperSize;
    QPrinter::PageOrder pageOrder;
    QPrinter::ColorMode colorMode;
    QPrinter::PaperSource paperSource;

    int resolution;
    QPaintEngine *paintEngine;
    int numCopies;

    bool outputToFile;
    bool fullPage;
    bool collateCopies;

    int pageNumber;

    QImage *pageImage;

    QtopiaPrintBuffer buffer;

    // Definitions that are only relevant to G3FAX output.
    int ifdPatch;
    int partialByte;
    int partialBits;
    void writeG3FaxHeader();
    void writeG3FaxPage();
    int writeG3IFDEntry( int tag, int type, int count, int value );
    void writeG3Code( int code, int bits );
    void writeG3WhiteRun( int len );
    void writeG3BlackRun( int len );
    void writeG3EOL();
};

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_QWS_P_H
