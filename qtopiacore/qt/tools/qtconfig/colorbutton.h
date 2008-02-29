/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QAbstractButton>

class ColorButton : public QAbstractButton
{
    Q_OBJECT

public:
    ColorButton(QWidget *);
    ColorButton(const QColor &, QWidget *);

    const QColor &color() const { return col; }

    void setColor(const QColor &);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);

signals:
    void colorChanged(const QColor &);

protected:
    void paintEvent(QPaintEvent *);
    void drawButton(QPainter *);
    void drawButtonLabel(QPainter *);

private slots:
    void changeColor();


private:
    QColor col;
    QPoint presspos;
    bool mousepressed;
};

#endif // COLORBUTTON_H
