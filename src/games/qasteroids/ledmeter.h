/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
 * KAsteroids - Copyright (c) Martin R. Jones 1997
 *
 * Part of the KDE project
 */

#ifndef __LEDMETER_H__
#define __LEDMETER_H__

#include <qframe.h>

class KALedMeter : public QFrame
{
    Q_OBJECT

 public:
    KALedMeter(QWidget* parent);

    int maxRawValue() const { return maxRawValue_; }
    void setMaxRawValue(int max);

    int meterLevels() const { return meterLevels_; }
    void setMeterLevels(int count);

    int rawValue () const { return rawValue_; }

 public slots:
    void setValue( int v );

 protected:
    virtual void resizeEvent(QResizeEvent* );
    virtual void paintEvent(QPaintEvent* event);

 private:
    void drawContents();

 protected:
    int maxRawValue_;
    int meterLevels_;
    int currentMeterLevel_;
    int rawValue_;
};

#endif
