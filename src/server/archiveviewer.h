/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#include <QStack>
#include <QKeyEvent>
#include <QPointer>
#ifdef QTOPIA_PHONE
#include "phone/messagebox.h"
#else
#include <QMessageBox>
#endif

class ArchiveViewer : public LauncherView
{
    Q_OBJECT
public:
    ArchiveViewer( QWidget* parent = 0, Qt::WFlags fl = 0 );

public slots:
    void setDocument( const QString &document );

private slots:
    void executeContent( const QContent &content );
    void showProperties();

protected:
    void hideEvent( QHideEvent *event );
    void showEvent( QShowEvent *event );

#ifdef QTOPIA_KEYPAD_NAVIGATION
    void keyPressEvent( QKeyEvent *event );
#endif

private:
#ifdef QTOPIA_PHONE
    QPointer<QAbstractMessageBox> warningBox;
#else
    QPointer<QMessageBox> warningBox;
#endif
    QAction *propertiesAction;
    QDialog *propDlg;
    QContent propLnk;

    QStack< QContentFilter > filterStack;
};

#endif
