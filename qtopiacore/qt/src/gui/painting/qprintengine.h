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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
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

        PPK_PaperSources,

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
