/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef QTIMEZONEWIDGET_H
#define QTIMEZONEWIDGET_H

#include <qtopiaglobal.h>
#include <QWidget>

class QTimeZoneSelectorPrivate;

class QTOPIA_EXPORT QTimeZoneSelector : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentZone READ currentZone WRITE setCurrentZone)
    Q_PROPERTY(bool allowNoZone READ allowNoZone WRITE setAllowNoZone)
public:
    explicit QTimeZoneSelector( QWidget* parent = 0 );
    ~QTimeZoneSelector();

    QString currentZone() const;
    void setCurrentZone( const QString& id );
    
    void setAllowNoZone(bool);
    bool allowNoZone() const;

signals:
    void zoneChanged( const QString& id );

private slots:
    void tzActivated( int index );

private:
    QTimeZoneSelectorPrivate *d;
};

#endif
