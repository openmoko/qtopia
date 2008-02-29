/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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
