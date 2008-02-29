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

#ifndef QWSEMBEDWIDGET_H
#define QWSEMBEDWIDGET_H

#include <QtGui/qwidget.h>

#ifndef QT_NO_QWSEMBEDWIDGET

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QWSEmbedWidgetPrivate;

class Q_GUI_EXPORT QWSEmbedWidget : public QWidget
{
    Q_OBJECT

public:
    QWSEmbedWidget(WId winId, QWidget *parent = 0);
    ~QWSEmbedWidget();

protected:
    bool eventFilter(QObject *object, QEvent *event);
    void changeEvent(QEvent *event);
    void resizeEvent(QResizeEvent *event);
    void moveEvent(QMoveEvent *event);

private:
    Q_DECLARE_PRIVATE(QWSEmbedWidget)
};

QT_END_HEADER

#endif // QT_NO_QWSEMBEDWIDGET
#endif // QWSEMBEDWIDGET_H
