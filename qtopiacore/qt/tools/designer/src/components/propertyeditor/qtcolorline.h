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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTCOLORLINE_H
#define QTCOLORLINE_H

#include <QWidget>

namespace qdesigner_internal {

class QtColorLine : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(int indicatorSpace READ indicatorSpace WRITE setIndicatorSpace)
    Q_PROPERTY(int indicatorSize READ indicatorSize WRITE setIndicatorSize)
    Q_PROPERTY(bool flip READ flip WRITE setFlip)
    Q_PROPERTY(bool backgroundTransparent READ backgroundTransparent WRITE setBackgroundTransparent)
    Q_PROPERTY(ColorComponent colorComponent READ colorComponent WRITE setColorComponent)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_ENUMS(ColorComponent)
public:

    enum ColorComponent {
        Red,
        Green,
        Blue,
        Hue,
        Saturation,
        Value,
        Alpha
    };

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    QtColorLine(QWidget *parent = 0);
    ~QtColorLine();

    QColor color() const;

    void setIndicatorSize(int size);
    int indicatorSize() const;

    void setIndicatorSpace(int space);
    int indicatorSpace() const;

    void setFlip(bool flip);
    bool flip() const;

    void setBackgroundTransparent(bool transparent);
    bool backgroundTransparent() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void setColorComponent(ColorComponent component);
    ColorComponent colorComponent() const;

public slots:

    void setColor(const QColor &color);

signals:

    void colorChanged(const QColor &color);

protected:

    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:

    class QtColorLinePrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtColorLine)
    Q_DISABLE_COPY(QtColorLine)
};

}

#endif
