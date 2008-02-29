/****************************************************************************
**
** Copyright (C) 2006-2007 TROLLTECH ASA. All rights reserved.
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
#include "loggingview.h"
#include "loggingedit.h"

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

#include <QMenu>
#include <QProcess>


LoggingView::LoggingView( QWidget* parent, Qt::WFlags /*fl*/ )
:   QTextEdit( parent /*,fl*/ )
{
    setWindowTitle(tr("Log"));
    setReadOnly(true);

#ifdef QTOPIA_PHONE
    QMenu* menu = QSoftMenuBar::menuFor( this );

    QAction *a;

    a = new QAction(QIcon(":icon/categories"),tr("Categories..."),this);
    connect( a, SIGNAL(triggered()), this, SLOT(settings()) );
    menu->addAction( a );

    a = new QAction(QIcon(":icon/trash"),tr("Clear"),this);
    connect( a, SIGNAL(triggered()), this, SLOT(clear()) );
    menu->addAction( a );

    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
#endif

    logfollow = new QProcess(this);
    connect(logfollow,SIGNAL(readyReadStandardOutput()),this,SLOT(readLogFollow()));
    logfollow->start("logread -f");

    QProcess loginitread;
    loginitread.start("logread");
    loginitread.waitForFinished(8000);
    insertPlainText(loginitread.readAllStandardOutput());

    if ( loginitread.error() == QProcess::FailedToStart ) {
        delete logfollow;
        logfollow = 0;
        setText(tr("<h2>Not available</h2>"
            "<p>This tool requires the <tt>logread</tt> program and the corresponding <tt>syslogd</tt>. "
            "<p>You may still adjust logging categories from <img src=:icon/options>.")
        );
    } else {
        setWordWrapMode(QTextOption::ManualWrap);
    }
}

LoggingView::~LoggingView()
{
}

bool LoggingView::isAvailable() const
{
    return logfollow && logfollow->state() != QProcess::NotRunning;
}

void LoggingView::resizeEvent(QResizeEvent* r)
{
    QTextEdit::resizeEvent(r);
    ensureCursorVisible();
}

void LoggingView::readLogFollow()
{
    // Determine if the cursor (at text end) is already visible.
    // If it is, then we scroll automatically.  Otherwise we don't
    // scroll because the user may be doing manual scrolling.
    QRect rect = cursorRect();
    bool wasVisible = rect.intersects(frameRect());

    // Add the text to the view.
    QByteArray more = logfollow->readAllStandardOutput();
    append(more);

    // Scroll to the end if necessary.
    if ( wasVisible )
        ensureCursorVisible();
}

void LoggingView::settings()
{
    LoggingEdit d(this);
    QtopiaApplication::execDialog(&d);
}

