/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <qglobal.h>

#ifdef Q_WS_QWS
#ifndef CALIBRATESETTINGS_H
#define CALIBRATESETTINGS_H

#include <QWSMouseHandler>
#include <QDialog>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QTimer>
#include <QWSServer>
#include <QFile>
#include <QString>
#include <QMessageBox>

#include <stdlib.h>

class QTimer;
class QPoint;
class QPixmap;


class Calibrate : public QDialog
{
    Q_OBJECT
public:
    Calibrate(QWidget* parent=0, Qt::WFlags f=0);
    ~Calibrate();

    void showEvent( QShowEvent *e );
    void hideEvent( QHideEvent *e );

private:
    QPoint fromDevice( const QPoint &p );
    bool sanityCheck();
    void moveCrosshair( QPoint pt );

protected:
    virtual void paintEvent( QPaintEvent * );
    virtual void keyPressEvent( QKeyEvent *);
    virtual void keyReleaseEvent( QKeyEvent *);
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );

private slots:
    void timeout();
    void doGrab();

private:
    void store();
    void reset();
    QPixmap logo;
    QWSPointerCalibrationData goodcd,cd;
    QWSPointerCalibrationData::Location location;
    QPoint crossPos;
    QPoint penPos;
    QPixmap saveUnder;
    QTimer *timer;
    int dx;
    int dy;
    bool showCross;
    bool pressed;
    bool anygood;
};

#endif // CALIBRATESETTINGS_H
#endif // Q_WS_QWS

