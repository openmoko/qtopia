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

