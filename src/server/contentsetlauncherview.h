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

#ifndef CONTENTSETLAUNCHERVIEW_H
#define CONTENTSETLAUNCHERVIEW_H

#include "launcherview.h"
#include <qtopiaabstractservice.h>

class QAction;

class ContentSetLauncherView : public LauncherView
{
    Q_OBJECT
public:
    ContentSetLauncherView( QWidget* parent = 0, Qt::WFlags fl = 0 );

private slots:
    void showContentSet();

    void showProperties();

private:
    QAction *propertiesAction;
    QDialog *propDlg;
    QContent propLnk;
};

class ContentSetViewService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    virtual ~ContentSetViewService();

    static ContentSetViewService *instance();

    QContentSet contentSet() const;
    QString title() const;

public slots:
    void showContentSet( const QContentSet &set );
    void showContentSet( const QString &title, const QContentSet &set );

signals:
    void showContentSet();

private:
    ContentSetViewService();

    QContentSet m_contentSet;
    QString m_title;
};

#endif
