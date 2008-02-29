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

#ifndef QWAITWIDGET_H
#define QWAITWIDGET_H

#include <QDialog>

#include <qtopiaglobal.h>

class QWaitWidgetPrivate;
class QHideEvent;

/**
 * A label that obscures the parent display and puts an image of an wait cursor
 * in the middle of the argument rect, to indicate the display is being built.
 */
class QTOPIA_EXPORT QWaitWidget : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool wasCancelled READ wasCancelled);
    Q_PROPERTY(bool cancelEnabled WRITE setCancelEnabled);
public:
    explicit QWaitWidget( QWidget *parent );

    bool wasCancelled() const;

public slots:
    void show();
    void hide();
    void setText( const QString &str );
    void setColor( const QColor &col );
    void setExpiryTime( int msec );
    void setCancelEnabled(bool enabled);

signals:
    void cancelled();

protected:
    void hideEvent( QHideEvent * );
    void keyPressEvent( QKeyEvent * );

private slots:
    void timeExpired();

private:
    void reset();

private:
    QWaitWidgetPrivate *d;
};

#endif

