/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef TIMEPROGRESSBAR_H
#define TIMEPROGRESSBAR_H

#include <qprogressbar.h>

class TimeProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    TimeProgressBar( QWidget *parent=0 );
    ~TimeProgressBar();

    void setRecording();
    void setPlaying();

protected:
    bool setIndicator( QString& progress_str, int progress, int totalSteps );
    bool event( QEvent * );

private:
    int prevValue;
    QPalette origPalette;
    QPalette adjustedPalette;
    bool recording;

    void refreshPalettes();
};

#endif
