/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef _QSOFTMENUBARPROVIDER_H_
#define _QSOFTMENUBARPROVIDER_H_

#include <QObject>
#include <QSharedDataPointer>
#include <QPixmap>

class QWSWindow;
class QSoftMenuBarProviderPrivate;
class QSoftMenuBarProvider : public QObject
{
Q_OBJECT
public:
    QSoftMenuBarProvider(QObject *parent = 0);
    virtual ~QSoftMenuBarProvider();

    class MenuButtonPrivate;
    class MenuButton
    {
    public:
        MenuButton();
        MenuButton(const MenuButton &);
        MenuButton &operator=(const MenuButton &);
        ~MenuButton();

        int index() const;
        int key() const;
        QString text() const;
        QPixmap pixmap() const;
        QString pixmapName() const;

    private:
        friend class QSoftMenuBarProvider;
        QSharedDataPointer<MenuButtonPrivate> d;
    };

    int keyCount() const;
    MenuButton key(int ii) const;
    QList<MenuButton> keys() const;

signals:
    void keyChanged(const QSoftMenuBarProvider::MenuButton &);

private slots:
    void message(const QString &msg, const QByteArray &data);
    void activeChanged(const QString &, const QRect &, WId);

private:
    QSoftMenuBarProviderPrivate *d;
};

#endif // _QSOFTMENUBARPROVIDER_H_
