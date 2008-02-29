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

#ifndef QTBRUSHWIDGET_H
#define QTBRUSHWIDGET_H

#include <QWidget>

namespace qdesigner_internal {

class QtBrushWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool backgroundTransparent READ backgroundTransparent WRITE setBackgroundTransparent)
public:
    QtBrushWidget(QWidget *parent = 0);
    ~QtBrushWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    int heightForWidth(int w) const;

    void setBackgroundTransparent(bool transparent);
    bool backgroundTransparent() const;

    void setBrush(const QBrush &brush);
    QBrush brush() const;

    void setBackgroundSize(int size);

protected:
    void paintEvent(QPaintEvent *e);

private:
    class QtBrushWidgetPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushWidget)
    Q_DISABLE_COPY(QtBrushWidget)
};

}

#endif
