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

#ifndef ACCESSORY_H
#define ACCESSORY_H

#include <qtopiaglobal.h>
#include <qmenu.h>
class QString;
class QWidget;
class QStartMenuAccessoryData;

class QTOPIA_EXPORT QStartMenuAccessory : public QObject
{
    Q_OBJECT

public:
    QStartMenuAccessory(const QIcon& icon, const QString& text, QWidget* parent);
    QStartMenuAccessory(const QIcon& icon, const QString& text, QMenu* submenu, QWidget* parent);
    virtual ~QStartMenuAccessory();

    QIcon icon() const;
    QString text() const;

    void setIcon(const QIcon& icon);
    void setText(const QString& text);

    QMenu* popup() const;

signals:
    void activated();

private:
    QStartMenuAccessoryData* d;
    friend class StartMenu;
};

#endif
