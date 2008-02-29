/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef TIMEPROGRESSBAR_H
#define TIMEPROGRESSBAR_H

#include <qprogressbar.h>

class TimeProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    TimeProgressBar( QWidget *parent=0, const char *name=0, WFlags fl=0 );
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
