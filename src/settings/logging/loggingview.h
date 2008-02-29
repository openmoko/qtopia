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
#ifndef LOGGINGVIEW_H
#define LOGGINGVIEW_H

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
    void showEvent(QShowEvent*);
    bool isAvailable() const;

private slots:
    void settings();
    void readLogFollow();
    void init();

private:
    QProcess *logfollow;
};

#endif // LOGGINGVIEW_H

