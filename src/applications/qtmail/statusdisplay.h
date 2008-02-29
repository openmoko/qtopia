/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef STATUSDISPLAY_H
#define STATUSDISPLAY_H

#include <QProgressBar>


// A QProgressBar and status label combined. No percentage is shown, as
// that's represented by the bar alone.
//
class StatusProgressBar : public QProgressBar 
{
    Q_OBJECT

public:
    StatusProgressBar( QWidget* parent = 0 );
    virtual ~StatusProgressBar();

    QSize sizeHint() const;

    void setText(const QString& s);

    QString text() const;

private:
    QString txt;
    mutable bool txtchanged;
};

// Implements some policy for the display of status and progress
class StatusDisplay : public StatusProgressBar
{
    Q_OBJECT

public:
    StatusDisplay(QWidget* parent = 0);

public slots:
    void showStatus(bool visible);
    void displayStatus(const QString& txt);
    void displayProgress(uint value, uint range);
    void clearStatus();

private:
    bool suppressed;
};

#endif // STATUSDISPLAY_H

