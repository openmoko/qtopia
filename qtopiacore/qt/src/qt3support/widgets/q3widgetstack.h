/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3WIDGETSTACK_H
#define Q3WIDGETSTACK_H

#include <Qt3Support/q3frame.h>
#include <Qt3Support/q3intdict.h>
#include <Qt3Support/q3ptrdict.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q3WidgetStackPrivate;


class Q_COMPAT_EXPORT Q3WidgetStack: public Q3Frame
{
    Q_OBJECT
public:
    Q3WidgetStack(QWidget* parent, const char* name=0, Qt::WindowFlags f=0);

    ~Q3WidgetStack();

    int addWidget(QWidget *, int = -1);
    void removeWidget(QWidget *);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void setVisible(bool visible);

    QWidget * widget(int) const;
    int id(QWidget *) const;

    QWidget * visibleWidget() const;

    void setFrameRect(const QRect &);

Q_SIGNALS:
    void aboutToShow(int);
    void aboutToShow(QWidget *);

public Q_SLOTS:
    void raiseWidget(int);
    void raiseWidget(QWidget *);

protected:
    void frameChanged();
    void resizeEvent(QResizeEvent *);
    bool event(QEvent* e);

    virtual void setChildGeometries();
    void childEvent(QChildEvent *);

private:
    void init();

    Q3WidgetStackPrivate * d;
    Q3IntDict<QWidget> * dict;
    Q3PtrDict<QWidget> * focusWidgets;
    QWidget * topWidget;
    QWidget * invisible;

    Q_DISABLE_COPY(Q3WidgetStack)
};

QT_END_HEADER

#endif // Q3WIDGETSTACK_H
