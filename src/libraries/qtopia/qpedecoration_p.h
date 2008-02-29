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
#ifndef QPE_DECORATION_QWS_H__
#define QPE_DECORATION_QWS_H__

#include <qconfig.h>
#ifdef Q_WS_QWS
#include <qtopiaglobal.h>
#include <qdecorationdefault_qws.h>
#include <qimage.h>
#include <qdatetime.h>
#include <qpointer.h>
#include <qwindowdecorationinterface.h>


#ifndef QT_NO_QWS_QPE_WM_STYLE

class QPEManager;
class QTimer;
#include <qwidget.h>

class QTOPIA_EXPORT QtopiaDecoration : public QDecorationDefault
{
public:
    QtopiaDecoration();
    explicit QtopiaDecoration( const QString &plugin );
    virtual ~QtopiaDecoration();

    virtual QRegion region(const QWidget *, const QRect &rect, int region);
    virtual bool paint(QPainter *, const QWidget *, int region, DecorationState state);
    virtual void paintButton(QPainter *, const QWidget *, int region, int state);

    virtual void regionClicked(QWidget *widget, int region);
    virtual void buildSysMenu( QWidget *, QMenu *menu );

protected:
    void help(QWidget *);
    virtual int getTitleHeight(const QWidget *);
    virtual const char **menuPixmap();
    virtual const char **closePixmap();
    virtual const char **minimizePixmap();
    virtual const char **maximizePixmap();
    virtual const char **normalizePixmap();

private:
    void windowData( const QWidget *w, QWindowDecorationInterface::WindowData &wd ) const;

    bool helpExists() const;

protected:
    QImage imageOk;
    QImage imageClose;
    QImage imageHelp;
    QString helpFile;
    bool helpexists;
    QPEManager *qpeManager;
    QRect desktopRect;
};

#ifdef QTOPIA4_TODO

#define QTOPIA_DEFINED_QPEDECORATION

class QTOPIA_EXPORT QPEManager : public QObject
{
    Q_OBJECT
    friend class QtopiaDecoration;
public:
    explicit QPEManager( QtopiaDecoration *d, QObject *parent=0 );

    void updateActive();
    const QWidget *activeWidget() const { return (const QWidget *)active; }
    const QWidget *whatsThisWidget() const { return (const QWidget *)whatsThis; }

protected:
    int pointInQpeRegion( QWidget *w, const QPoint &p );
    virtual bool eventFilter( QObject *, QEvent * );
    void drawButton( QWidget *w, QtopiaDecoration::QPERegion r, int state );
    void drawTitle( QWidget *w );

protected slots:
    void whatsThisTimeout();

protected:
    QtopiaDecoration *decoration;
    QPointer<QWidget> active;
    int helpState;
    QTime pressTime;
    QTimer *wtTimer;
    bool inWhatsThis;
    QPointer<QWidget> whatsThis;
};


#define QTOPIA_DEFINED_QPEMANAGER

#endif

#endif // QT_NO_QWS_QPE_WM_STYLE
#endif // Q_WS_QWS

#endif // QPE_DECORATION_QWS_H__
