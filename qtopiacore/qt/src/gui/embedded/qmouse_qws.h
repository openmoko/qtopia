/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMOUSE_QWS_H
#define QMOUSE_QWS_H

#include <QtCore/qobject.h>
#include <QtGui/qpolygon.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QWSMouseHandlerPrivate;
class QScreen;

class Q_GUI_EXPORT QWSPointerCalibrationData
{
public:
    enum Location { TopLeft = 0, BottomLeft = 1, BottomRight = 2, TopRight = 3,
                    Center = 4, LastLocation = Center };
    QPoint devPoints[5];
    QPoint screenPoints[5];
};

class Q_GUI_EXPORT QWSMouseHandler
{
public:
    explicit QWSMouseHandler(const QString &driver = QString(),
                             const QString &device = QString());
    virtual ~QWSMouseHandler();

    virtual void clearCalibration() {}
    virtual void calibrate(const QWSPointerCalibrationData *) {}
    virtual void getCalibration(QWSPointerCalibrationData *) const {}

    virtual void resume() = 0;
    virtual void suspend() = 0;

    void limitToScreen(QPoint &pt);
    void mouseChanged(const QPoint& pos, int bstate, int wheel = 0);
    const QPoint &pos() const { return mousePos; }

    void setScreen(const QScreen *screen);

protected:
    QPoint &mousePos;
    QWSMouseHandlerPrivate *d_ptr;
};


class Q_GUI_EXPORT QWSCalibratedMouseHandler : public QWSMouseHandler
{
public:
    explicit QWSCalibratedMouseHandler(const QString &driver = QString(),
                                       const QString &device = QString());

    virtual void clearCalibration();
    virtual void calibrate(const QWSPointerCalibrationData *);
    virtual void getCalibration(QWSPointerCalibrationData *) const;

protected:
    bool sendFiltered(const QPoint &, int button);
    QPoint transform(const QPoint &);

    void readCalibration();
    void writeCalibration();
    void setFilterSize(int);

private:
    int a, b, c;
    int d, e, f;
    int s;
    QPolygon samples;
    int currSample;
    int numSamples;
};

QT_END_HEADER

#endif // QMOUSE_QWS_H
