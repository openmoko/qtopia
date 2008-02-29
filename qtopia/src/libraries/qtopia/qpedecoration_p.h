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
#ifndef QPE_DECORATION_QWS_H__
#define QPE_DECORATION_QWS_H__


#ifdef QWS
#include <qtopia/qpeglobal.h>
#include <qwsdefaultdecoration_qws.h>
#include <qimage.h>
#include <qdatetime.h>
#include <qguardedptr.h>
#include <qtopia/windowdecorationinterface.h>


#ifndef QT_NO_QWS_QPE_WM_STYLE

class QPEManager;
class QTimer;
#include <qwidget.h>

class QTOPIA_EXPORT QPEDecoration : public QWSDefaultDecoration
{
public:
    QPEDecoration();
    QPEDecoration( const QString &plugin );
    virtual ~QPEDecoration();

    virtual QRegion region(const QWidget *, const QRect &rect, Region);
    virtual void paint(QPainter *, const QWidget *);
    virtual void paintButton(QPainter *, const QWidget *, Region, int state);

    void maximize( QWidget * );
    void minimize( QWidget * );
    virtual QPopupMenu *menu( const QWidget *, const QPoint & );
    virtual void help( QWidget * );

    enum QPERegion { Help=LastRegion+1 };
    void buttonClicked( QPERegion r );
    
protected:
    virtual int getTitleHeight(const QWidget *);
    virtual const char **menuPixmap();
    virtual const char **closePixmap();
    virtual const char **minimizePixmap();
    virtual const char **maximizePixmap();
    virtual const char **normalizePixmap();

private:
    void windowData( const QWidget *w, WindowDecorationInterface::WindowData &wd ) const;

    bool helpExists() const;

protected:
    QImage imageOk;
    QImage imageClose;
    QImage imageHelp;
    QString helpFile;
    bool helpexists;
    QPEManager *qpeManager;
};


#define QTOPIA_DEFINED_QPEDECORATION
#include <qtopia/qtopiawinexport.h>

class QTOPIA_EXPORT QPEManager : public QObject
{
    Q_OBJECT
    friend class QPEDecoration;
public:
    QPEManager( QPEDecoration *d, QObject *parent=0 );

    void updateActive();
    const QWidget *activeWidget() const { return (const QWidget *)active; }
    const QWidget *whatsThisWidget() const { return (const QWidget *)whatsThis; }

protected:
    int pointInQpeRegion( QWidget *w, const QPoint &p );
    virtual bool eventFilter( QObject *, QEvent * );
    void drawButton( QWidget *w, QPEDecoration::QPERegion r, int state );
    void drawTitle( QWidget *w );

protected slots:
    void whatsThisTimeout();

protected:
    QPEDecoration *decoration;
    QGuardedPtr<QWidget> active;
    int helpState;
    QTime pressTime;
    QTimer *wtTimer;
    bool inWhatsThis;
    QGuardedPtr<QWidget> whatsThis;
};


#define QTOPIA_DEFINED_QPEMANAGER
#include <qtopia/qtopiawinexport.h>

#endif // QT_NO_QWS_QPE_WM_STYLE


#endif // QPE_DECORATION_QWS_H__
#endif // QWS
