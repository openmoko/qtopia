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
#ifndef QPE_DECORATION_QWS_H__
#define QPE_DECORATION_QWS_H__

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qconfig.h>
#include <qtopiaglobal.h>
#include <qdecorationdefault_qws.h>
#include <qimage.h>
#include <qdatetime.h>
#include <qpointer.h>
#include <qwindowdecorationinterface.h>

#ifndef QT_NO_QWS_QPE_WM_STYLE

class QTimer;
#include <qwidget.h>

class QTOPIA_EXPORT QtopiaDecoration : public QDecorationDefault
{
public:
    QtopiaDecoration();
    explicit QtopiaDecoration( const QString &plugin );
    virtual ~QtopiaDecoration();

    virtual QRegion region(const QWidget *, const QRect &rect, int region);
    virtual int regionAt(const QWidget *w, const QPoint &point);
    virtual bool paint(QPainter *, const QWidget *, int region, DecorationState state);
    
    using QDecorationDefault::paintButton; // Don't hide these symbols!
    virtual void paintButton(QPainter *, const QWidget *, int region, int state);

    virtual void regionClicked(QWidget *widget, int region);
    virtual void buildSysMenu( QWidget *, QMenu *menu );

protected:
    void help(QWidget *);
    virtual int getTitleHeight(const QWidget *);

private:
    void windowData( const QWidget *w, QWindowDecorationInterface::WindowData &wd ) const;

    bool helpExists() const;

protected:
    QImage imageOk;
    QImage imageClose;
    QImage imageHelp;
    QString helpFile;
    bool helpexists;
    QRect desktopRect;
};

#endif // QT_NO_QWS_QPE_WM_STYLE

#endif // QPE_DECORATION_QWS_H__
