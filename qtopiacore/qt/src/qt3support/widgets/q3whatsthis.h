/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3WHATSTHIS_H
#define Q3WHATSTHIS_H

#include <QtGui/qcursor.h>
#include <QtGui/qwhatsthis.h>
#include <QtGui/qwidget.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class QToolButton;

class Q_COMPAT_EXPORT Q3WhatsThis: public QObject
{
    Q_OBJECT
public:
    Q3WhatsThis(QWidget *);
    ~Q3WhatsThis();
    bool eventFilter(QObject *, QEvent *);

    static inline void enterWhatsThisMode() { QWhatsThis::enterWhatsThisMode(); }
    static inline bool inWhatsThisMode() { return QWhatsThis::inWhatsThisMode(); }

    static inline void add(QWidget *w, const QString &s) { w->setWhatsThis(s); }
    static inline void remove(QWidget *w) { w->setWhatsThis(QString()); }
    static QToolButton * whatsThisButton(QWidget * parent);
    static inline void leaveWhatsThisMode(const QString& text = QString(), const QPoint& pos = QCursor::pos(), QWidget* w = 0)
        { QWhatsThis::showText(pos, text, w); }
    static inline void display(const QString& text, const QPoint& pos = QCursor::pos(), QWidget* w = 0)
        { QWhatsThis::showText(pos, text, w); }

    virtual QString text(const QPoint &);
    virtual bool clicked(const QString& href);

};

QT_END_HEADER

#endif // Q3WHATSTHIS_H
