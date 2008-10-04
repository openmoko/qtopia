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
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
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

#include <private/qprintengine_mac_p.h>
#include <qdebug.h>
#include <qthread.h>
#include <QtCore/qcoreapplication.h>

#ifndef QT_NO_PRINTER

QT_BEGIN_NAMESPACE

extern QSizeF qt_paperSizeToQSizeF(QPrinter::PaperSize size);
extern int qt_defaultDpi();

QMacPrintEngine::QMacPrintEngine(QPrinter::PrinterMode mode) : QPaintEngine(*(new QMacPrintEnginePrivate))
{
    Q_D(QMacPrintEngine);
    d->mode = mode;
    d->initialize();
}

bool QMacPrintEngine::begin(QPaintDevice *dev)
{
    Q_D(QMacPrintEngine);

    if (d->state == QPrinter::Idle && d->session == 0) // Need to reinitialize
        d->initialize();

    d->paintEngine->state = state;
    d->paintEngine->begin(dev);
    Q_ASSERT_X(d->state == QPrinter::Idle, "QMacPrintEngine", "printer already active");

    if (PMSessionValidatePrintSettings(d->session, d->settings, kPMDontWantBoolean) != noErr
        || PMSessionValidatePageFormat(d->session, d->format, kPMDontWantBoolean) != noErr) {
        d->state = QPrinter::Error;
        return false;
    }

    if (!d->outputFilename.isEmpty()) {
        QCFType<CFURLRef> outFile = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault,
                                                                  QCFString(d->outputFilename),
                                                                  kCFURLPOSIXPathStyle,
                                                                  false);
        if (PMSessionSetDestination(d->session, d->settings, kPMDestinationFile,
                                    kPMDocumentFormatPDF, outFile) != noErr) {
            qWarning("QMacPrintEngine::begin: Problem setting file [%s]", d->outputFilename.toUtf8().constData());
            return false;
        }
    }
    OSStatus status = noErr;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
        status = d->shouldSuppressStatus() ? PMSessionBeginCGDocumentNoDialog(d->session, d->settings, d->format)
                                           : PMSessionBeginCGDocument(d->session, d->settings, d->format);
    } else
#endif
    {
#ifndef Q_OS_MAC64
        status = d->shouldSuppressStatus() ? PMSessionBeginDocumentNoDialog(d->session, d->settings, d->format)
                                           : PMSessionBeginDocument(d->session, d->settings, d->format);
#endif
    }
    if (status != noErr) {
        d->state = QPrinter::Error;
        return false;
    }

    d->state = QPrinter::Active;
    setActive(true);
    d->newPage_helper();
    return true;
}

bool QMacPrintEngine::end()
{
    Q_D(QMacPrintEngine);
    if (d->state == QPrinter::Aborted)
        return true;  // I was just here a function call ago :)
    if(d->paintEngine->type() == QPaintEngine::CoreGraphics)
        static_cast<QCoreGraphicsPaintEngine*>(d->paintEngine)->d_func()->hd = 0;
    d->paintEngine->end();
    if (d->state != QPrinter::Idle) {
        if (d->shouldSuppressStatus()) {
            PMSessionEndPageNoDialog(d->session);
            PMSessionEndDocumentNoDialog(d->session);
        } else {
            PMSessionEndPage(d->session);
            PMSessionEndDocument(d->session);
        }
        PMRelease(d->session);
        d->session = 0;
    }
    d->state  = QPrinter::Idle;
    return true;
}

QPaintEngine *
QMacPrintEngine::paintEngine() const
{
    return d_func()->paintEngine;
}

Qt::HANDLE QMacPrintEngine::handle() const
{
    QCoreGraphicsPaintEngine *cgEngine = static_cast<QCoreGraphicsPaintEngine*>(paintEngine());
    return cgEngine->d_func()->hd;
}

void QMacPrintEnginePrivate::setPaperSize(QPrinter::PaperSize ps)
{
    Q_Q(QMacPrintEngine);
    QSizeF newSize = qt_paperSizeToQSizeF(ps);
    QCFType<CFArrayRef> formats;
    PMPrinter printer;

    if (PMSessionGetCurrentPrinter(session, &printer) == noErr
        && PMSessionCreatePageFormatList(session, printer, &formats) == noErr) {
        CFIndex total = CFArrayGetCount(formats);
        PMPageFormat tmp;
        PMRect paper;
        for (CFIndex idx = 0; idx < total; ++idx) {
            tmp = static_cast<PMPageFormat>(
                                        const_cast<void *>(CFArrayGetValueAtIndex(formats, idx)));
            PMGetUnadjustedPaperRect(tmp, &paper);
            int wMM = int((paper.right - paper.left) / 72 * 25.4 + 0.5);
            int hMM = int((paper.bottom - paper.top) / 72 * 25.4 + 0.5);
            if (newSize.width() == wMM && newSize.height() == hMM) {
                PMCopyPageFormat(tmp, format);
                // reset the orientation and resolution as they are lost in the copy.
                q->setProperty(QPrintEngine::PPK_Orientation, orient);
                if (PMSessionValidatePageFormat(session, format, kPMDontWantBoolean) != noErr) {
                    // Don't know, warn for the moment.
                    qWarning("QMacPrintEngine, problem setting format and resolution for this page size");
                }
                break;
            }
        }
    }
}

QPrinter::PaperSize QMacPrintEnginePrivate::paperSize() const
{
    PMRect paper;
    PMGetUnadjustedPaperRect(format, &paper);
    int wMM = int((paper.right - paper.left) / 72 * 25.4 + 0.5);
    int hMM = int((paper.bottom - paper.top) / 72 * 25.4 + 0.5);
    for (int i = QPrinter::A4; i < QPrinter::NPaperSize; ++i) {
        QSizeF s = qt_paperSizeToQSizeF(QPrinter::PaperSize(i));
        if (s.width() == wMM && s.height() == hMM)
            return (QPrinter::PaperSize)i;
    }
    return QPrinter::Custom;
}

QList<QVariant> QMacPrintEnginePrivate::supportedResolutions() const
{
    Q_ASSERT_X(session, "QMacPrinterEngine::supportedResolutions",
               "must have a valid printer session");
    UInt32 resCount;
    QList<QVariant> resolutions;
    PMPrinter printer;
    if (PMSessionGetCurrentPrinter(session, &printer) == noErr) {
        PMResolution res;
        OSStatus status = PMPrinterGetPrinterResolutionCount(printer, &resCount);
        if (status  == kPMNotImplemented) {
#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5)
            // *Sigh* we have to use the non-indexed version.
            if (PMPrinterGetPrinterResolution(printer, kPMMinSquareResolution, &res) == noErr)
                resolutions.append(int(res.hRes));
            if (PMPrinterGetPrinterResolution(printer, kPMMaxSquareResolution, &res) == noErr) {
                QVariant var(int(res.hRes));
                if (!resolutions.contains(var))
                    resolutions.append(var);
            }
            if (PMPrinterGetPrinterResolution(printer, kPMDefaultResolution, &res) == noErr) {
                QVariant var(int(res.hRes));
                if (!resolutions.contains(var))
                    resolutions.append(var);
            }
#endif
        } else if (status == noErr) {
            // According to the docs, index start at 1.
            for (UInt32 i = 1; i <= resCount; ++i) {
                if (PMPrinterGetIndexedPrinterResolution(printer, i, &res) == noErr)
                    resolutions.append(QVariant(int(res.hRes)));
            }
        } else {
            qWarning("QMacPrintEngine::supportedResolutions: Unexpected error: %ld", status);
        }
    }
    return resolutions;
}

bool QMacPrintEnginePrivate::shouldSuppressStatus() const
{
    if (suppressStatus == true)
        return true;
     
    // Supress displaying the automatic progress dialog if we are printing
    // from a non-gui thread.
    return (qApp->thread() != QThread::currentThread());
}

QPrinter::PrinterState QMacPrintEngine::printerState() const
{
    return d_func()->state;
}

bool QMacPrintEngine::newPage()
{
    Q_D(QMacPrintEngine);
    Q_ASSERT(d->state == QPrinter::Active);
    OSStatus err = d->shouldSuppressStatus() ? PMSessionEndPageNoDialog(d->session)
                                             : PMSessionEndPage(d->session);
    if (err != noErr)  {
        if (err == kPMCancel) {
            // User canceled, we need to abort!
            abort();
        } else {
            // Not sure what the problem is...
            qWarning("QMacPrintEngine::newPage: Cannot end current page. %ld", long(err));
            d->state = QPrinter::Error;
        }
        return false;
    }
    return d->newPage_helper();
}

bool QMacPrintEngine::abort()
{
    Q_D(QMacPrintEngine);
    if (d->state != QPrinter::Active)
        return false;
    bool ret = end();
    d->state = QPrinter::Aborted;
    return ret;
}

static inline int qt_get_PDMWidth(PMPageFormat pformat, bool fullPage,
                                  const PMResolution &resolution)
{
    int val = 0;
    PMRect r;
    qreal hRatio = resolution.hRes / 72;
    if (fullPage) {
        if (PMGetAdjustedPaperRect(pformat, &r) == noErr)
            val = qRound((r.right - r.left) * hRatio);
    } else {
        if (PMGetAdjustedPageRect(pformat, &r) == noErr)
            val = qRound((r.right - r.left) * hRatio);
    }
    return val;
}

static inline int qt_get_PDMHeight(PMPageFormat pformat, bool fullPage,
                                   const PMResolution &resolution)
{
    int val = 0;
    PMRect r;
    qreal vRatio = resolution.vRes / 72;
    if (fullPage) {
        if (PMGetAdjustedPaperRect(pformat, &r) == noErr)
            val = qRound((r.bottom - r.top) * vRatio);
    } else {
        if (PMGetAdjustedPageRect(pformat, &r) == noErr)
            val = qRound((r.bottom - r.top) * vRatio);
    }
    return val;
}


int QMacPrintEngine::metric(QPaintDevice::PaintDeviceMetric m) const
{
    Q_D(const QMacPrintEngine);
    int val = 1;
    switch (m) {
    case QPaintDevice::PdmWidth:
        if (d->hasCustomPaperSize) {
            val = qRound(d->customSize.width());
            if (d->hasCustomPageMargins) {
                val -= qRound(d->leftMargin + d->rightMargin);
            } else {
                QList<QVariant> margins = property(QPrintEngine::PPK_PageMargins).toList();
                val -= qRound(margins.at(0).toDouble() + margins.at(2).toDouble());
            }
        } else {
            val = qt_get_PDMWidth(d->format, property(PPK_FullPage).toBool(), d->resolution);
        }
        break;
    case QPaintDevice::PdmHeight:
        if (d->hasCustomPaperSize) {
            val = qRound(d->customSize.height());
            if (d->hasCustomPageMargins) {
                val -= qRound(d->topMargin + d->bottomMargin);
            } else {
                QList<QVariant> margins = property(QPrintEngine::PPK_PageMargins).toList();
                val -= qRound(margins.at(1).toDouble() + margins.at(3).toDouble());
            }
        } else {
            val = qt_get_PDMHeight(d->format, property(PPK_FullPage).toBool(), d->resolution);
        }
        break;
    case QPaintDevice::PdmWidthMM:
        val = metric(QPaintDevice::PdmWidth);
        val = int((val * 254 + 5 * d->resolution.hRes) / (10 * d->resolution.hRes));
        break;
    case QPaintDevice::PdmHeightMM:
        val = metric(QPaintDevice::PdmHeight);
        val = int((val * 254 + 5 * d->resolution.vRes) / (10 * d->resolution.vRes));
        break;
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmPhysicalDpiY: {
        PMPrinter printer;
        if(PMSessionGetCurrentPrinter(d->session, &printer) == noErr) {
            PMResolution resolution;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_5) {
                PMPrinterGetOutputResolution(printer, d->settings, &resolution);
            } else
#endif
            {
#ifndef Q_OS_MAC64
                PMPrinterGetPrinterResolution(printer, kPMCurrentValue, &resolution);
#endif
            }
            val = (int)resolution.vRes;
            break;
        }
        //otherwise fall through
    }
    case QPaintDevice::PdmDpiY:
        val = (int)d->resolution.vRes;
        break;
    case QPaintDevice::PdmDpiX:
        val = (int)d->resolution.hRes;
        break;
    case QPaintDevice::PdmNumColors:
        val = (1 << metric(QPaintDevice::PdmDepth));
        break;
    case QPaintDevice::PdmDepth:
        val = 24;
        break;
    default:
        val = 0;
        qWarning("QPrinter::metric: Invalid metric command");
    }
    return val;
}

void QMacPrintEnginePrivate::initialize()
{
    Q_ASSERT(!session);

    Q_Q(QMacPrintEngine);

    if (!paintEngine)
        paintEngine = new QCoreGraphicsPaintEngine();

    q->gccaps = paintEngine->gccaps;

    fullPage = false;

    if (PMCreateSession(&session) != noErr)
        session = 0;

    PMPrinter printer;
    if (session && PMSessionGetCurrentPrinter(session, &printer) == noErr) {
        QList<QVariant> resolutions = supportedResolutions();
        if (!resolutions.isEmpty() && mode != QPrinter::ScreenResolution) {
            if (resolutions.count() > 1 && mode == QPrinter::HighResolution) {
                int max = 0;
                for (int i = 0; i < resolutions.count(); ++i) {
                    int value = resolutions.at(i).toInt();
                    if (value > max)
                        max = value;
                }
                resolution.hRes = resolution.vRes = max;
            } else {
                resolution.hRes = resolution.vRes = resolutions.at(0).toInt();
            }
            if(resolution.hRes == 0)
                resolution.hRes = resolution.vRes = 600;
        } else {
            resolution.hRes = resolution.vRes = qt_defaultDpi();
        }
    }

    bool settingsInitialized = (settings != 0);
    bool settingsOK = !settingsInitialized ? PMCreatePrintSettings(&settings) == noErr : true;
    if (settingsOK && !settingsInitialized)
        settingsOK = PMSessionDefaultPrintSettings(session, settings) == noErr;


    bool formatInitialized = (format != 0);
    bool formatOK = !formatInitialized ? PMCreatePageFormat(&format) == noErr : true;
    if (formatOK) {
        if (!formatInitialized) {
            formatOK = PMSessionDefaultPageFormat(session, format) == noErr;
        }
        formatOK = PMSessionValidatePageFormat(session, format, kPMDontWantBoolean) == noErr;
    }


#ifndef Q_OS_MAC64
# if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    if (QSysInfo::MacintoshVersion < QSysInfo::MV_10_4)
# endif
    {
        if(paintEngine->type() == QPaintEngine::CoreGraphics) {
            CFStringRef strings[1] = { kPMGraphicsContextCoreGraphics };
            QCFType<CFArrayRef> contextArray = CFArrayCreate(kCFAllocatorDefault,
                    reinterpret_cast<const void **>(strings),
                    1, &kCFTypeArrayCallBacks);
            OSStatus err = PMSessionSetDocumentFormatGeneration(session, kPMDocumentFormatPDF,
                    contextArray, 0);
            if(err != noErr) {
                qWarning("QMacPrintEngine::initialize: Cannot set format generation to PDF: %ld", err);
                state = QPrinter::Error;
            }
        }
    }
#endif

    if (!settingsOK || !formatOK) {
        qWarning("QMacPrintEngine::initialize: Unable to initialize QPainter");
        state = QPrinter::Error;
    }

    QHash<QMacPrintEngine::PrintEnginePropertyKey, QVariant>::const_iterator propC;
    for (propC = valueCache.constBegin(); propC != valueCache.constEnd(); propC++) {
        q->setProperty(propC.key(), propC.value());
    }
}

bool QMacPrintEnginePrivate::newPage_helper()
{
    Q_Q(QMacPrintEngine);
    Q_ASSERT(state == QPrinter::Active);

    if (PMSessionError(session) != noErr) {
        q->abort();
        return false;
    }

    OSStatus status = shouldSuppressStatus() ? PMSessionBeginPageNoDialog(session, format, 0)
                                             : PMSessionBeginPage(session, format, 0);
    if(status != noErr) {
        state = QPrinter::Error;
        return false;
    }

    QRect page = q->property(QPrintEngine::PPK_PageRect).toRect();
    QRect paper = q->property(QPrintEngine::PPK_PaperRect).toRect();

    CGContextRef cgContext;
    OSStatus err = noErr;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
        err = PMSessionGetCGGraphicsContext(session, &cgContext);
    } else
#endif
    {
#ifndef Q_OS_MAC64
        err = PMSessionGetGraphicsContext(session, kPMGraphicsContextCoreGraphics,
                                          reinterpret_cast<void **>(&cgContext));
#endif
    }
    if(err != noErr) {
        qWarning("QMacPrintEngine::newPage: Cannot retrieve CoreGraphics context: %ld", long(err));
        state = QPrinter::Error;
        return false;
    }
    QCoreGraphicsPaintEngine *cgEngine = static_cast<QCoreGraphicsPaintEngine*>(paintEngine);
    cgEngine->d_func()->hd = cgContext;

    // Set the resolution as a scaling ration of 72 (the default).
    CGContextScaleCTM(cgContext, 72 / resolution.hRes, 72 / resolution.vRes);

    CGContextScaleCTM(cgContext, 1, -1);
    CGContextTranslateCTM(cgContext, 0, -paper.height());
    if (!fullPage)
        CGContextTranslateCTM(cgContext, page.x() - paper.x(), page.y() - paper.y());
    cgEngine->d_func()->orig_xform = CGContextGetCTM(cgContext);
    cgEngine->d_func()->setClip(0);
    cgEngine->state->dirtyFlags = QPaintEngine::AllDirty;
    cgEngine->syncState();
    return true;
}


void QMacPrintEngine::updateState(const QPaintEngineState &state)
{
    d_func()->paintEngine->updateState(state);
}

void QMacPrintEngine::drawRects(const QRectF *r, int num)
{
    Q_D(QMacPrintEngine);
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawRects(r, num);
}

void QMacPrintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_D(QMacPrintEngine);
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPoints(points, pointCount);
}

void QMacPrintEngine::drawEllipse(const QRectF &r)
{
    Q_D(QMacPrintEngine);
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawEllipse(r);
}

void QMacPrintEngine::drawLines(const QLineF *lines, int lineCount)
{
    Q_D(QMacPrintEngine);
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawLines(lines, lineCount);
}

void QMacPrintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QMacPrintEngine);
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPolygon(points, pointCount, mode);
}

void QMacPrintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    Q_D(QMacPrintEngine);
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPixmap(r, pm, sr);
}

void QMacPrintEngine::drawImage(const QRectF &r, const QImage &pm, const QRectF &sr, Qt::ImageConversionFlags flags)
{
    Q_D(QMacPrintEngine);
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawImage(r, pm, sr, flags);
}

void QMacPrintEngine::drawTextItem(const QPointF &p, const QTextItem &ti)
{
    Q_D(QMacPrintEngine);
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawTextItem(p, ti);
}

void QMacPrintEngine::drawTiledPixmap(const QRectF &dr, const QPixmap &pixmap, const QPointF &sr)
{
    Q_D(QMacPrintEngine);
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawTiledPixmap(dr, pixmap, sr);
}

void QMacPrintEngine::drawPath(const QPainterPath &path)
{
    Q_D(QMacPrintEngine);
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPath(path);
}


void QMacPrintEngine::setProperty(PrintEnginePropertyKey key, const QVariant &value)
{
    Q_D(QMacPrintEngine);

    d->valueCache.insert(key, value);
    if (!d->session)
        return;

    switch (key) {
    case PPK_CollateCopies:
        break;
    case PPK_ColorMode:
        break;
    case PPK_Creator:
        break;
    case PPK_DocumentName:
        break;
    case PPK_PageOrder:
        break;
    case PPK_PaperSource:
        break;
    case PPK_SelectionOption:
        break;
    case PPK_Resolution:  {
        PMPrinter printer;
        UInt32 count;
        if (PMSessionGetCurrentPrinter(d->session, &printer) != noErr)
            break;
        if (PMPrinterGetPrinterResolutionCount(printer, &count) != noErr)
            break;
        PMResolution resolution = { 0.0, 0.0 };
        PMResolution bestResolution = { 0.0, 0.0 };
        int dpi = value.toInt();
        int bestDistance = INT_MAX;
        for (UInt32 i = 1; i <= count; ++i) {  // Yes, it starts at 1
            if (PMPrinterGetIndexedPrinterResolution(printer, i, &resolution) == noErr) {
                if (dpi == int(resolution.hRes)) {
                    bestResolution = resolution;
                    break;
                } else {
                    int distance = qAbs(dpi - int(resolution.hRes));
                    if (distance < bestDistance) {
                        bestDistance = distance;
                        bestResolution = resolution;
                    }
                }
            }
        }
        PMSessionValidatePageFormat(d->session, d->format, kPMDontWantBoolean);
        break;
    }

    case PPK_FullPage:
        d->fullPage = value.toBool();
        break;
    case PPK_NumberOfCopies:
        PMSetCopies(d->settings, value.toInt(), false);
        break;
    case PPK_Orientation: {
        if (d->state == QPrinter::Active) {
            qWarning("QMacPrintEngine::setOrientation: Orientation cannot be changed during a print job, ignoring change");
        } else {
            QPrinter::Orientation newOrientation = QPrinter::Orientation(value.toInt());
            if (d->hasCustomPaperSize && (d->orient != newOrientation))
                d->customSize = QSizeF(d->customSize.height(), d->customSize.width());
            d->orient = newOrientation;
            PMOrientation o = d->orient == QPrinter::Portrait ? kPMPortrait : kPMLandscape;
            PMSetOrientation(d->format, o, false);
            PMSessionValidatePageFormat(d->session, d->format, kPMDontWantBoolean);
        }
        break; }
    case PPK_OutputFileName:
        d->outputFilename = value.toString();
        break;
    case PPK_PaperSize:
        d->setPaperSize(QPrinter::PaperSize(value.toInt()));
        break;
    case PPK_PrinterName: {
        OSStatus status = noErr;
        QCFType<CFArrayRef> printerList;
        status = PMServerCreatePrinterList(kPMServerLocal, &printerList);
        if (status == noErr) {
            CFIndex count = CFArrayGetCount(printerList);
            for (CFIndex i=0; i<count; ++i) {
                PMPrinter printer = static_cast<PMPrinter>(const_cast<void *>(CFArrayGetValueAtIndex(printerList, i)));
                QString name = QCFString::toQString(PMPrinterGetName(printer));
                if (name == value.toString()) {
                    status = PMSessionSetCurrentPMPrinter(d->session, printer);
                    break;
                }
            }
        }
        if (status != noErr)
            qWarning("QMacPrintEngine::setPrinterName: Error setting printer: %ld", long(status));
        break; }
    case PPK_SuppressSystemPrintStatus:
        d->suppressStatus = value.toBool();
        break;
    case PPK_CustomPaperSize:
    {
        PMOrientation orientation;
        PMGetOrientation(d->format, &orientation);
        d->hasCustomPaperSize = true;
        d->customSize = value.toSizeF();
        if (orientation != kPMPortrait)
            d->customSize = QSizeF(d->customSize.height(), d->customSize.width());
        break;
    }
    case PPK_PageMargins:
    {
        QList<QVariant> margins(value.toList());
        Q_ASSERT(margins.size() == 4);
        d->leftMargin = margins.at(0).toDouble();
        d->topMargin = margins.at(1).toDouble();
        d->rightMargin = margins.at(2).toDouble();
        d->bottomMargin = margins.at(3).toDouble();
        d->hasCustomPageMargins = true;
        break;
    }

    default:
        break;
    }
}

QVariant QMacPrintEngine::property(PrintEnginePropertyKey key) const
{
    Q_D(const QMacPrintEngine);
    QVariant ret;

    if (!d->session && d->valueCache.contains(key))
        return *d->valueCache.find(key);

    switch (key) {
    case PPK_CollateCopies:
        ret = false;
        break;
    case PPK_ColorMode:
        ret = QPrinter::Color;
        break;
    case PPK_Creator:
        break;
    case PPK_DocumentName:
        break;
    case PPK_FullPage:
        ret = d->fullPage;
        break;
    case PPK_NumberOfCopies:
        ret = 1;
        break;
    case PPK_Orientation:
        PMOrientation orientation;
        PMGetOrientation(d->format, &orientation);
        ret = orientation == kPMPortrait ? QPrinter::Portrait : QPrinter::Landscape;
        break;
    case PPK_OutputFileName:
        ret = d->outputFilename;
        break;
    case PPK_PageOrder:
        break;
    case PPK_PaperSource:
        break;
    case PPK_PageRect: {
        // PageRect is returned in device pixels
        QRect r;
        PMRect macrect, macpaper;
        qreal hRatio = d->resolution.hRes / 72;
        qreal vRatio = d->resolution.vRes / 72;
        if (d->hasCustomPaperSize) {
            r = QRect(0, 0, qRound(d->customSize.width() * hRatio), qRound(d->customSize.height() * vRatio));
            if (d->hasCustomPageMargins) {
                r.adjust(qRound(d->leftMargin * hRatio), qRound(d->topMargin * vRatio), 
                         -qRound(d->rightMargin * hRatio), -qRound(d->bottomMargin * vRatio));
            } else {
                QList<QVariant> margins = property(QPrintEngine::PPK_PageMargins).toList();
                r.adjust(qRound(margins.at(0).toDouble() * hRatio),
                         qRound(margins.at(1).toDouble() * vRatio),
                         -qRound(margins.at(2).toDouble() * hRatio),
                         -qRound(margins.at(3).toDouble()) * vRatio);
            }
        } else if (PMGetAdjustedPageRect(d->format, &macrect) == noErr
                   && PMGetAdjustedPaperRect(d->format, &macpaper) == noErr)
        {
            if (d->fullPage || d->hasCustomPageMargins) {
                r.setCoords(int(macpaper.left * hRatio), int(macpaper.top * vRatio),
                            int(macpaper.right * hRatio), int(macpaper.bottom * vRatio));
                r.translate(-r.x(), -r.y());
                if (d->hasCustomPageMargins) {
                    r.adjust(qRound(d->leftMargin * hRatio), qRound(d->topMargin * vRatio),
                             -qRound(d->rightMargin * hRatio), -qRound(d->bottomMargin * vRatio));
                }
            } else {
                r.setCoords(int(macrect.left * hRatio), int(macrect.top * vRatio),
                            int(macrect.right * hRatio), int(macrect.bottom * vRatio));
                r.translate(int(-macpaper.left * hRatio), int(-macpaper.top * vRatio));
            } 
        }
        ret = r;
        break; }
    case PPK_PaperSize:
        ret = d->paperSize();
        break;
    case PPK_PaperRect: {
        QRect r;
        PMRect macrect;
        if (d->hasCustomPaperSize) {
            r = QRect(0, 0, qRound(d->customSize.width()), qRound(d->customSize.height()));
        } else if (PMGetAdjustedPaperRect(d->format, &macrect) == noErr) {
            qreal hRatio = d->resolution.hRes / 72;
            qreal vRatio = d->resolution.vRes / 72;
            r.setCoords(int(macrect.left * hRatio), int(macrect.top * vRatio),
                        int(macrect.right * hRatio), int(macrect.bottom * vRatio));
            r.translate(-r.x(), -r.y());
        }
        ret = r;
        break; }
    case PPK_PrinterName: {
        CFIndex currIndex;
        PMPrinter unused;
        QCFType<CFArrayRef> printerList;
        OSStatus status = PMSessionCreatePrinterList(d->session, &printerList, &currIndex, &unused);
        if (status != noErr)
            qWarning("QMacPrintEngine::printerName: Problem getting list of printers: %ld", long(status));
        if (currIndex != -1 && printerList && currIndex < CFArrayGetCount(printerList)) {
            const CFStringRef name = static_cast<CFStringRef>(CFArrayGetValueAtIndex(printerList, currIndex));
            if (name)
                ret = QCFString::toQString(name);
        }
        break; }
    case PPK_Resolution: {
        ret = d->resolution.hRes;
        break;
    }
    case PPK_SupportedResolutions:
        ret = d->supportedResolutions();
        break;
    case PPK_CustomPaperSize:
        ret = d->customSize;
        break;
    case PPK_PageMargins:
    {
        QList<QVariant> margins;
        if (d->hasCustomPageMargins) {
            margins << d->leftMargin << d->topMargin
                    << d->rightMargin << d->bottomMargin;
        } else {
            PMPaperMargins paperMargins;
            PMPaper paper;
            PMGetPageFormatPaper(d->format, &paper);
            PMPaperGetMargins(paper, &paperMargins);
            margins << paperMargins.left << paperMargins.top
                    << paperMargins.right << paperMargins.bottom;
        }
        ret = margins;
        break;
    }
    default:
        break;
    }
    return ret;
}

QT_END_NAMESPACE

#endif // QT_NO_PRINTER
