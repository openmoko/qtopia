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

#ifndef _e2_LAUNCHER_H_
#define _e2_LAUNCHER_H_

#include "qabstractserverinterface.h"
#include <QList>
#include <QContent>
#include <QPixmap>
#include <QTimer>

class QValueSpaceItem;
class QAbstractBrowserScreen;
class QExportedBackground;
class E2TelephonyBar;
class QAppointmentModel;
class QOccurrenceModel;
class E2Dialer;
class E2ServerInterface : public QAbstractServerInterface
{
Q_OBJECT
public:
    E2ServerInterface(QWidget *parent = 0, Qt::WFlags flags = 0);

protected:
    virtual void resizeEvent(QResizeEvent *e);
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual bool event(QEvent *);

private slots:
    void e2Received(const QString &, const QByteArray &);
    void applicationLaunched(const QString &);
    void wallpaperChanged();
    void operatorChanged();
    void updateAppointment();

private:
    void doAppointmentTimer(bool);
    QTimer appointmentTimer;

    E2TelephonyBar *m_bar;

    QExportedBackground *m_background;
    QList<QContent> m_lastUsedApps;
    QPixmap m_ringProf;
    QAbstractBrowserScreen *m_browser;
    QPixmap m_wallpaper;
    QString operatorName;
    QValueSpaceItem *operatorItem;
    QString occurrenceText;
    QPixmap datebook;

    QOccurrenceModel *m_model;
    QAppointmentModel *m_appointment;
};

#endif // _e2_LAUNCHER_H_

