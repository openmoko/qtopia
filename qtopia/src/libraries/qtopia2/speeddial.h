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

#ifndef SPEEDDIAL_H
#define SPEEDDIAL_H

#include <qtopia/services.h>
#include <qwidget.h>
#include <qlistbox.h>

class SpeedDial;
class SpeedDialListPrivate;

class QTOPIA_EXPORT SpeedDialList : public QListBox {
    Q_OBJECT
public:
    SpeedDialList(QWidget* parent);
    ~SpeedDialList();

    QString currentSpeedDial() const;
    void setCurrentSpeedDial(const QString&);

protected:
    void keyPressEvent(QKeyEvent*);
    void timerEvent(QTimerEvent*);
    void resizeEvent(QResizeEvent*);

signals:
    void speedDialSelected(const QString&);

public slots:
    void reload(const QString& sd);

private slots:
    void select(QListBoxItem*);

private:
    void init(const QString&);
    friend class SpeedDialDialog;
    friend class SpeedDialItem;

    SpeedDialList(const QString& label, const QString& icon, QWidget* parent);
    QString seltext, selicon;

    int sel;
    int sel_tid;
    SpeedDialListPrivate* d;
};


struct QTOPIA_EXPORT SpeedDialRec {
    SpeedDialRec() {}
    SpeedDialRec( ServiceRequest r, QString l, QString ic ) :
	request(r),label(l),icon(ic)
    {
    }

    QPixmap pixmap() const;
    static QPixmap SpeedDialRec::pixmap(const QString&);

    ServiceRequest request;
    QString label;
    QString icon;
};

class QTOPIA_EXPORT SpeedDial
{
public:
    // SpeedDial is very similar to DeviceButtonManager

    static QString addWithDialog(const QString& label, const QString& icon,
	const ServiceRequest& action, QWidget* parent);

    static SpeedDialRec find(const QString& speeddial);
    static void remove(const QString& speeddial);
    static void set(const QString& speeddial, const SpeedDialRec&);
};

#endif
