/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef TZSELECT_H
#define TZSELECT_H

#include <qtopia/qpeglobal.h>
#include <qhbox.h>
#include <qcombobox.h>
#include <qstringlist.h>

class QToolButton;
class TimeZoneSelector;
class TimeZoneSelectorPrivate;

// a function to load defaults in case there is no file
QTOPIA_EXPORT QStringList timezoneDefaults( void );

class QTOPIA_EXPORT TZCombo : public QComboBox
{
    Q_OBJECT
public:
    TZCombo( QWidget* parent, const char* name = 0 );
    ~TZCombo();

    QString currZone() const;
    void setCurrZone( const QString& id );

protected:
#ifdef QTOPIA_DESKTOP
    friend class TimeZoneSelectorPrivate;
#endif
    friend class TimeZoneSelector;
    void keyPressEvent( QKeyEvent *e );
    void mousePressEvent(QMouseEvent*e);
    void updateZones();

private slots:
    void handleSystemChannel(const QCString&, const QByteArray&);

private:
    QStringList identifiers;
    QStringList extras;
};

class QTOPIA_EXPORT TimeZoneSelector : public QHBox
{
    Q_OBJECT
public:
    TimeZoneSelector( QWidget* parent = 0, const char* name=0 );
    ~TimeZoneSelector();

#ifdef QTOPIA_INTERNAL_TZSELECT_INC_LOCAL
    // badly named.. not to be used outside of qtopia (not forward compatable)
    // basically allows the "No Timezone" or "None" option.
    void setLocalIncluded(bool);
    bool localIncluded() const;
#endif
    QString currentZone() const;
    void setCurrentZone( const QString& id );

signals:
    void signalNewTz( const QString& id );
private slots:
    void slotTzActive( int index );
    void slotExecute( void );

private:
    QToolButton *cmdTz;
#ifdef QTOPIA_DESKTOP
    public:
#endif
    TZCombo *cmbTz;
    TimeZoneSelectorPrivate *d;
};

#endif
