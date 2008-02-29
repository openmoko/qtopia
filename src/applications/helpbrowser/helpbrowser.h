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

#ifndef HELPBROWSER_H
#define HELPBROWSER_H

#include <qtopia/qsoftmenubar.h>
#include <qtopiaabstractservice.h>

#include <QMainWindow>
#include <QTextBrowser>
#include <QStack>
#include <QUrl>
#include "navigationbar_p.h"

class QAction;
class QLabel;

class MagicTextBrowser : public QTextBrowser {
    Q_OBJECT
public:
    MagicTextBrowser( QWidget* parent );

    virtual QVariant loadResource (int type, const QUrl &name);

private:
    // Generate help page links
    QString generate( const QString& );
};

class HelpBrowser : public QMainWindow
{
    Q_OBJECT
public:
    HelpBrowser( QWidget* parent=0, Qt::WFlags f=0 );

    virtual ~HelpBrowser();

    bool eventFilter( QObject*, QEvent* );

public slots:
    void setDocument( const QString &doc );

private slots:
    void goHome();

    void textChanged(QUrl);

protected:
    void closeEvent( QCloseEvent* );

private:
    void init();

    MagicTextBrowser *browser;
    QAction *backAction, *forwardAction;
#ifdef DEBUG
    QLabel *location;
#endif
    NavigationBar *navigationBar;

    QMenu *contextMenu;
};

class HelpService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class HelpBrowser;
private:
    HelpService( HelpBrowser *parent )
        : QtopiaAbstractService( "Help", parent )
        { this->parent = parent; publishAll(); }

public:
    ~HelpService();

public slots:
    void setDocument( const QString& doc );

private:
    HelpBrowser *parent;
};

#endif // HELPBROWSER_H
