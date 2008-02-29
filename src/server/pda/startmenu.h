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

#ifndef __START_MENU_H__
#define __START_MENU_H__

#include <qstring.h>
#include <qlist.h>
#include <qmap.h>
#include <qlabel.h>
#include <qmenu.h>
#include <qtopia/private/qstartmenuaccessory_p.h>

class StartMenuAction;

class StartMenu : public QLabel, public IStartMenuAccessory {
    Q_OBJECT
    Q_INTERFACES(IStartMenuAccessory)

public:
    ~StartMenu();

    void refreshMenu();

    static StartMenu * create(QWidget *);

signals:
    // Emitted whenever the user selects a "tab" from
    // the Q menu
    void tabSelected(const QString&);

public slots:
    void launch( );
    void createMenu( );

protected slots:
    void itemSelected( QAction * );

protected:
    virtual void mousePressEvent( QMouseEvent * );

    virtual QAction * add(QStartMenuAccessory *, const QIcon&, const QString&);
    virtual QAction * add(const QIcon&, const QString&, QMenu *);
    virtual void rem(QAction *);

private:
    StartMenu( QWidget * );
    bool loadMenu( QMenu *menu );
    void doSep();

private:
    QMenu *launchMenu;

    bool useWidePopupMenu;
    QString popupMenuSidePixmap;

    bool startButtonIsFlat;
    QString startButtonPixmap;

    QList<QAction *> items;
};

#endif // __START_MENU_H__
