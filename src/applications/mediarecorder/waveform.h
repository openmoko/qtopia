/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <qwidget.h>
#include <qpixmap.h>




class Waveform : public QWidget
{
public:
    Waveform(QWidget *parent = 0, Qt::WFlags fl = 0);
    ~Waveform();

    void changeSettings(int frequency, int channels);
    void reset();
    void newSamples(const short *buf, int len);

protected:

    void paintEvent(QPaintEvent *event);

private:
    void makePixmap();
    void draw();

    int samplesPerPixel;
    int currentValue;
    int numSamples;
    short *window;
    int windowPosn;
    int windowSize;
};


#endif

