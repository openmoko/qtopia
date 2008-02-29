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

#ifndef QPRINTENGINE_H
#define QPRINTENGINE_H

#include <QtCore/qvariant.h>
#include <QtGui/qprinter.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_PRINTER

class Q_GUI_EXPORT QPrintEngine
{
public:
    virtual ~QPrintEngine() {}
    enum PrintEnginePropertyKey {
        PPK_CollateCopies,
        PPK_ColorMode,
        PPK_Creator,
        PPK_DocumentName,
        PPK_FullPage,
        PPK_NumberOfCopies,
        PPK_Orientation,
        PPK_OutputFileName,
        PPK_PageOrder,
        PPK_PageRect,
        PPK_PageSize,
        PPK_PaperRect,
        PPK_PaperSource,
        PPK_PrinterName,
        PPK_PrinterProgram,
        PPK_Resolution,
        PPK_SelectionOption,
        PPK_SupportedResolutions,

        PPK_WindowsPageSize,
        PPK_FontEmbedding,
        PPK_SuppressSystemPrintStatus,
        
        PPK_Duplex,       

        PPK_CustomBase = 0xff00
    };

    virtual void setProperty(PrintEnginePropertyKey key, const QVariant &value) = 0;
    virtual QVariant property(PrintEnginePropertyKey key) const = 0;

    virtual bool newPage() = 0;
    virtual bool abort() = 0;

    virtual int metric(QPaintDevice::PaintDeviceMetric) const = 0;

    virtual QPrinter::PrinterState printerState() const = 0;

#ifdef Q_WS_WIN
    virtual HDC getPrinterDC() const { return 0; }
    virtual void releasePrinterDC(HDC) const { }
#endif

};

#endif // QT_NO_PRINTER

QT_END_HEADER

#endif // QPRINTENGINE_H
