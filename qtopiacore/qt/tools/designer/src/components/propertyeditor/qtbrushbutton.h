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

#ifndef QTBRUSHBUTTON_H
#define QTBRUSHBUTTON_H

#include <QToolButton>

class QDesignerBrushManagerInterface;

namespace qdesigner_internal {

class QtBrushButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(bool backgroundTransparent READ backgroundTransparent WRITE setBackgroundTransparent)
public:
    QtBrushButton(QWidget *parent = 0);
    ~QtBrushButton();

    void setBackgroundTransparent(bool transparent);
    bool backgroundTransparent() const;

    void setBrush(const QBrush &brush);
    QBrush brush() const;

    void setBrushManager(QDesignerBrushManagerInterface *manager);

    void setTexture(const QPixmap &texture);
signals:
    void brushChanged(const QBrush &brush);
    void textureChooserActivated(QWidget *parent, const QBrush &initialBrush);
protected:
    void paintEvent(QPaintEvent *e);

private:
    class QtBrushButtonPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushButton)
    Q_DISABLE_COPY(QtBrushButton)
    Q_PRIVATE_SLOT(d_func(), void slotEditBrush())
};

}

#endif
