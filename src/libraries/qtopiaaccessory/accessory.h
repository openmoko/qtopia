/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef ACCESSORY_H
#define ACCESSORY_H

#include <qobject.h>
class QIconSet;
class QString;
class QWidget;
class QPopupMenu;
class AccessoryData;

class Accessory : public QObject
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
