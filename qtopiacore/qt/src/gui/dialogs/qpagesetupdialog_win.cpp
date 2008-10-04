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

#include "qpagesetupdialog.h"

#ifndef QT_NO_PRINTDIALOG
#include <qapplication.h>

#include <private/qprintengine_win_p.h>
#include <private/qabstractpagesetupdialog_p.h>

QT_BEGIN_NAMESPACE

class QPageSetupDialogPrivate : public QAbstractPageSetupDialogPrivate
{
};

QPageSetupDialog::QPageSetupDialog(QPrinter *printer, QWidget *parent)
    : QAbstractPageSetupDialog(*(new QPageSetupDialogPrivate), printer, parent)
{

}

int QPageSetupDialog::exec()
{
    Q_D(QPageSetupDialog);

    if (d->printer->outputFormat() != QPrinter::NativeFormat)
        return Rejected;

    QWin32PrintEngine *engine = static_cast<QWin32PrintEngine*>(d->printer->paintEngine());
    QWin32PrintEnginePrivate *ep = static_cast<QWin32PrintEnginePrivate *>(engine->d_ptr);

    PAGESETUPDLG psd;
    memset(&psd, 0, sizeof(PAGESETUPDLG));
    psd.lStructSize = sizeof(PAGESETUPDLG);

    // we need a temp DEVMODE struct if we don't have a global DEVMODE
    HGLOBAL hDevMode;
    int devModeSize;
    if (!ep->globalDevMode) {
        QT_WA( { devModeSize = sizeof(DEVMODEW) + ((DEVMODEW *) ep->devMode)->dmDriverExtra; },
               { devModeSize = sizeof(DEVMODEA) + ((DEVMODEA *) ep->devMode)->dmDriverExtra; });
        hDevMode = GlobalAlloc(GHND, devModeSize);
        if (hDevMode) {
            void *dest = GlobalLock(hDevMode);
            memcpy(dest, ep->devMode, devModeSize);
            GlobalUnlock(hDevMode);
        }
        psd.hDevMode = hDevMode;
    } else {
        psd.hDevMode = ep->devMode;
    }

    HGLOBAL *tempDevNames = ep->createDevNames();
    psd.hDevNames = tempDevNames;

    QWidget *parent = parentWidget();
    parent = parent ? parent->window() : qApp->activeWindow();
    Q_ASSERT(!parent ||parent->testAttribute(Qt::WA_WState_Created));
    psd.hwndOwner = parent ? parent->winId() : 0;

    QRect paperRect = d->printer->paperRect();
    QRect pageRect = d->printer->pageRect();

    psd.Flags = PSD_MARGINS;
    double multiplier = 1;
    switch (QLocale::system().measurementSystem()) {
    case QLocale::MetricSystem:
        psd.Flags |= PSD_INHUNDREDTHSOFMILLIMETERS;
        multiplier = 1;
        break;
    case QLocale::ImperialSystem:
        psd.Flags |= PSD_INTHOUSANDTHSOFINCHES;
        multiplier = 25.4/10;
        break;
    }

    QRect marginRect = ep->getPageMargins();
    psd.rtMargin.left   = marginRect.left()   / multiplier;
    psd.rtMargin.top    = marginRect.top()    / multiplier;
    psd.rtMargin.right  = marginRect.width()  / multiplier;;
    psd.rtMargin.bottom = marginRect.height() / multiplier;;

    bool result = PageSetupDlg(&psd);
    if (result) {
        ep->readDevnames(psd.hDevNames);
        ep->readDevmode(psd.hDevMode);

        QRect theseMargins = QRect(psd.rtMargin.left   * multiplier,
                                   psd.rtMargin.top    * multiplier,
                                   psd.rtMargin.right  * multiplier,
                                   psd.rtMargin.bottom * multiplier);

        if (theseMargins != marginRect) {
            ep->setPageMargins(psd.rtMargin.left   * multiplier,
                               psd.rtMargin.top    * multiplier,
                               psd.rtMargin.right  * multiplier,
                               psd.rtMargin.bottom * multiplier);
        }

        extern QPrinter::PaperSize mapDevmodePaperSize(int s);
        QPrinter::PaperSize pSize;

        QT_WA( {
                pSize = mapDevmodePaperSize(ep->devModeW()->dmPaperSize);
        }, {
                pSize = mapDevmodePaperSize(ep->devModeA()->dmPaperSize);
        } );
        if (pSize != QPrinter::Custom)
            ep->has_custom_paper_size = false;

        // copy from our temp DEVMODE struct
        if (!ep->globalDevMode && hDevMode) {
            void *src = GlobalLock(hDevMode);
            memcpy(ep->devMode, src, devModeSize);
            GlobalUnlock(hDevMode);
        }
    }

    if (!ep->globalDevMode && hDevMode)
        GlobalFree(hDevMode);
    GlobalFree(tempDevNames);
    return result;
}

QT_END_NAMESPACE
#endif
