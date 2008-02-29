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

#ifndef QPEMENUBAR_H
#define QPEMENUBAR_H

#include <qtopia/qpeglobal.h>
#include <qmenubar.h>
#include <qguardedptr.h>
#include <qvaluelist.h>
#include <qwidget.h>

#include <qtopia/qtopiawinexport.h> 

typedef QGuardedPtr<QWidget> GuardedWidget;

class QTOPIA_EXPORT QPEMenuToolFocusManager : public QObject
{
    Q_OBJECT
public:
    QPEMenuToolFocusManager();

    void addWidget( QWidget *w );
    void removeWidget( QWidget *w );
    void setActive( bool a );
    bool isActive() const;
    void moveFocus( bool next );

    static QPEMenuToolFocusManager *manager();

protected:
    void setFocus( QWidget *w, bool next=TRUE );
    bool eventFilter( QObject *object, QEvent *event );

private slots:
    void deactivate();

private:
    friend class QPEApplication;
    static void initialize();
    QValueList<GuardedWidget> list;
    GuardedWidget inFocus;
    GuardedWidget oldFocus;
    static QPEMenuToolFocusManager *me;
};

#define QTOPIA_DEFINED_QPEMENUTOOLFOCUSMANAGER
#include <qtopia/qtopiawinexport.h>


class QTOPIA_EXPORT QPEMenuBar : public QMenuBar
{
    Q_OBJECT
public:
    QPEMenuBar( QWidget *parent=0, const char* name=0 );
    ~QPEMenuBar();

protected:
    virtual void keyPressEvent( QKeyEvent *e );
};

#define QTOPIA_DEFINED_QPEMENUBAR
#include <qtopia/qtopiawinexport.h>

#endif

