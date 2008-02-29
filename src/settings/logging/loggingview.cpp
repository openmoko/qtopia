/****************************************************************************
**
** Copyright (C) 2006-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#include <QTextCursor>
#include <QScrollBar>
#include <QTimer>

LoggingView::LoggingView( QWidget* parent, Qt::WFlags /*fl*/ )
:   QTextEdit( parent /*,fl*/ )
{
    setWindowTitle(tr("Log"));
    setReadOnly(true);

    QMenu* menu = QSoftMenuBar::menuFor( this );

    menu->addAction( QIcon(":icon/categories"), tr("Categories..."), this, SLOT(settings()) );
    menu->addAction( QIcon(":icon/trash"), tr("Clear"), this, SLOT(clear()) );

    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);

    logfollow = 0;
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
    QScrollBar *vscroll = verticalScrollBar();
    QScrollBar *hscroll = horizontalScrollBar();
    bool scrollToEnd = ( vscroll->value() == vscroll->maximum() );

    // save the user's scroll position
    int vscrollpos = vscroll->value();
    int hscrollpos = hscroll->value();

    QTextEdit::resizeEvent(r);

    if ( scrollToEnd )
        // Scroll to the bottom
        vscroll->setValue( vscroll->maximum() );
    else
        // Scroll to the previous value
        vscroll->setValue( vscrollpos );
    // Don't interfere with the user's horizontal scrolling
    hscroll->setValue( hscrollpos );
}

void LoggingView::readLogFollow()
{
    // Determine if the insertion point (at text end) is already visible.
    // If it is, then we scroll automatically.  Otherwise we don't
    // scroll because the user may be doing manual scrolling.
    QScrollBar *vscroll = verticalScrollBar();
    QScrollBar *hscroll = horizontalScrollBar();
    bool scrollToEnd = ( vscroll->value() == vscroll->maximum() );

    // Save the user's cursor and scroll position.
    QTextCursor userCursor = textCursor();
    int vscrollpos = vscroll->value();
    int hscrollpos = hscroll->value();

    // Ensure the cursor is at the end (so insertPlainText works correctly)
    QTextCursor end = textCursor();
    // What a disgusting way to find the end of the document!
    for ( int i = 0; !end.atEnd(); i++ )
        end.setPosition( i );
    setTextCursor( end );

    // Add the text to the view.
    QByteArray more = logfollow->readAllStandardOutput();
    insertPlainText(more);

    // Restore the user's cursor
    setTextCursor( userCursor );

    if ( scrollToEnd ) {
        // Scroll to the bottom
        vscroll->setValue( vscroll->maximum() );
    } else {
        // Scroll to the previous value
        vscroll->setValue( vscrollpos );
    }
    // Don't interfere with the user's horizontal scrolling
    hscroll->setValue( hscrollpos );
}

void LoggingView::settings()
{
    LoggingEdit d(this);
    QtopiaApplication::execDialog(&d);
}

void LoggingView::showEvent( QShowEvent * /*e*/ )
{
    if ( !logfollow ) {
        // Get _something_ on the screen quickly... we can fetch the real log later.
        insertPlainText(tr("Reading Log..."));
        QTimer::singleShot( 0, this, SLOT(init()) );
    }
}

void LoggingView::init()
{
    clear();

    logfollow = new QProcess(this);
    connect(logfollow,SIGNAL(readyReadStandardOutput()),this,SLOT(readLogFollow()));
    logfollow->start("logread -f");

    QProcess loginitread;
    loginitread.start("logread");
    loginitread.waitForFinished(8000);

    if ( loginitread.error() == QProcess::FailedToStart ) {
        delete logfollow;
        logfollow = 0;
        setHtml(tr("<h3>Not available</h3>"
            "<p>This tool requires the <tt>logread</tt> program and the corresponding <tt>syslogd</tt>. "
            "<p>You may still adjust logging categories from Options <img src=:icon/options>.")
        );
    } else {
        // don't wrap this output, it looks weird
        setWordWrapMode(QTextOption::ManualWrap);
        insertPlainText(loginitread.readAllStandardOutput());
        // Scroll to the bottom
        QScrollBar *vscroll = verticalScrollBar();
        vscroll->setValue( vscroll->maximum() );
    }
}

