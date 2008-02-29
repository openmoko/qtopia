/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include <qtopianamespace.h>

#include <QDebug>
#include <QFile>
#include <QKeyEvent>
#include <QLayout>
#include <QProcess>
#include <QScrollBar>
#include <QTextBrowser>
#include <QDesktopWidget>
#include <QTimer>
#include "securityinfo.h"

SecurityInfo::SecurityInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    QTimer::singleShot(50, this, SLOT(init()));
}

void SecurityInfo::init()
{
    infoDisplay = new QTextBrowser();
    infoDisplay->installEventFilter( this );
    infoDisplay->setFrameShape( QFrame::NoFrame );
    //infoDisplay->setFocusPolicy( Qt::NoFocus );
    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setSpacing( 0 );
    vb->setMargin( 0 );
    vb->addWidget( infoDisplay );

    QDesktopWidget *desktop = QApplication::desktop();
    double imageScale = ((double)desktop->availableGeometry(desktop->screenNumber(this)).width())/290.0;
    if (imageScale > 1.0)
        imageScale = 1.0;
    int imageSize = (int)(60 * imageScale);

    // Add logo, TODO update with SXE logo, or lids logo
    QString infoString = "<p><img width=\"%1\" height=\"%2\"src=\":image/qpe-logo\"></p>";
    infoString = infoString.arg( imageSize );
    infoString = infoString.arg( imageSize );

    // Add SXE info
    infoString += "<p><center>" +
            tr("Safe Execution Environment:") +
            " " +
            "</p></center>";

#ifndef QT_NO_SXE
    infoString += "<p><center><b>" +
                  tr("Enabled") +
                  "</b></center></p>";
#else
    infoString += "<p><center><font color=\"#ff0000\"><b>" +
                  tr("Disabled") +
                  "</b></font></center></p>";
#endif //QT_NO_SXE

    // Add LIDS info
    bool lidsEnabled = QFile::exists("/proc/sys/lids/locks");
    infoString += "<p><center>" +
            tr("Kernel LIDS support:") +
            " " +
            "</p></center>";

    if (lidsEnabled)
        infoString += "<p><center><b>" +
                tr("Available") +
                "</b></center></p>";
    else
        infoString += "<p><center><font color=\"#ff0000\"><b>" +
                tr("Unavailable") +
                "</b></font></center></p>";


    if (lidsEnabled)
    {
        QProcess lidsconf;
        lidsconf.start ("lidsconf -L");
        if (lidsconf.waitForFinished())
        {
            QStringList output(QString(lidsconf.readAll()).split("\n"));
            if(!output.contains ("Killed"))
            {
                infoString += "<p><center>";
                infoString += tr("No of Security Rules: %1").arg(output.count() - 5);
                infoString += "<p><center>";
            }
        }
    }

    infoDisplay->setHtml( infoString );
}

SecurityInfo::~SecurityInfo()
{
}

bool SecurityInfo::eventFilter( QObject* /*watched*/, QEvent *event )
{
    if ( event->type() == QEvent::KeyPress )
    {
        QScrollBar* sb = infoDisplay->verticalScrollBar();
        int key = ((QKeyEvent*)event)->key();
        if ( key == Qt::Key_Down )
            sb->triggerAction( QAbstractSlider::SliderSingleStepAdd );
        else if ( key == Qt::Key_Up )
            sb->triggerAction( QAbstractSlider::SliderSingleStepSub );
        else
            return false;
        return true;
    }
    return false;
}
