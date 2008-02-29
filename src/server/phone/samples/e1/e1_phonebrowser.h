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

#ifndef _e1_PHONEBROWSER_H_
#define _e1_PHONEBROWSER_H_

#include "e1_bar.h"
#include <QStackedWidget>
#include <qcategorymanager.h>
#include "phone/phonebrowser.h"
#include <QPixmap>
#include <QString>
#include <QStringList>
#include <qvaluespace.h>
#include "launcherview.h"

class E1PhoneBrowserTabs;

class E1PhoneBrowser : public LazyContentStack
{
Q_OBJECT
public:
    E1PhoneBrowser(QWidget * = 0, Qt::WFlags = 0);

    void display();

protected:
    virtual QObject* createView(const QString &);
    virtual void resetToView(const QString &);
    virtual void showView(const QString &);
    virtual void raiseView(const QString &, bool reset);
    virtual void keyPressEvent(QKeyEvent *);

private slots:
    void tabChanged(const QString &);
    void message(const QString &, const QByteArray &);
    void toList();
    void toIcon();

private:
    void primeView(const QString &);
    void stopFocus(QObject *);

    QStackedWidget * m_stack;
    E1PhoneBrowserTabs * m_tabs;
    QContentSet appCategories;
    QListView::ViewMode m_mode;

    QMap<QString, QWidget *> m_views;
};


#endif // _e1_PHONEBROWSER_H_

