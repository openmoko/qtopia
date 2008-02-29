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
#ifndef LOGGINGVIEW_H
#define LOGGINGVIEW_H

#include <qtopialog.h>
#include <QTextEdit>
#include <QBasicTimer>

class QProcess;

class LoggingView : public QTextEdit
{
    Q_OBJECT

public:
    LoggingView(QWidget* parent = 0, Qt::WFlags = 0);
    ~LoggingView();

    void resizeEvent(QResizeEvent*);
    bool isAvailable() const;

protected slots:
    void settings();
    void readLogFollow();

private:
    QProcess *logfollow;
};

#endif // LOGGINGVIEW_H

