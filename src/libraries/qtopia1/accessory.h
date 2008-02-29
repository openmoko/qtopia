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

#ifndef ACCESSORY_H
#define ACCESSORY_H

#include <qtopia/qpeglobal.h>
#include <qobject.h>
class QIconSet;
class QString;
class QWidget;
class QPopupMenu;
class AccessoryData;

class QTOPIA_EXPORT Accessory : public QObject
{
    Q_OBJECT

public:
    Accessory(const QIconSet& icon, const QString& text, QWidget* parent);
    Accessory(const QIconSet& icon, const QString& text, QPopupMenu* submenu, QWidget* parent);
    virtual ~Accessory();

    QIconSet iconSet() const;
    QString text() const;

    void setIconSet(const QIconSet& icon);
    void setText(const QString& text);

    QPopupMenu* popup() const;

signals:
    void activated();
    void activated(int);

private slots:
    void maybeActivated(int);

private:
    AccessoryData* d;
};

#endif
