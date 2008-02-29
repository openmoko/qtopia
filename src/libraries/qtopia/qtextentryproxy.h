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
#ifndef __TEXTENTRYPROXY_H__
#define __TEXTENTRYPROXY_H__

#include <qtopiaglobal.h>
#include <QWidget>

class QKeyEvent;
class QInputMethodEvent;

class QTextEntryProxyData;
class QTOPIAPIM_EXPORT QTextEntryProxy : public QWidget
{
    Q_OBJECT
public:
    QTextEntryProxy(QWidget *parent, QWidget *target);
    ~QTextEntryProxy();

    // geom hinting.... focus policy etc.
    QSize sizeHint() const;

    QString text() const;
    void clear();
    void setTarget ( QWidget * target );
    
signals:
    void textChanged(const QString &);
protected:
    void paintEvent(QPaintEvent *);

    // To Be Deprecated
    bool processInputMethodEvent(QInputMethodEvent *);
    bool processKeyPressEvent(QKeyEvent *);

    QVariant inputMethodQuery(Qt::InputMethodQuery) const;

    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void targetDestroyed(QObject *obj);
    
private:
    QTextEntryProxyData *d;
};

#endif//__TEXTENTRYPROXY_H__
