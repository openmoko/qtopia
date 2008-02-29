/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QPROGRESSBAR_H
#define QPROGRESSBAR_H

#include <QtGui/qframe.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_PROGRESSBAR

class QProgressBarPrivate;

class Q_GUI_EXPORT QProgressBar : public QWidget
{
    Q_OBJECT
    Q_ENUMS(Direction)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool textVisible READ isTextVisible WRITE setTextVisible)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance)
    Q_PROPERTY(Direction textDirection READ textDirection WRITE setTextDirection)
    Q_PROPERTY(QString format READ format WRITE setFormat)

public:
    enum Direction { TopToBottom, BottomToTop };

    explicit QProgressBar(QWidget *parent = 0);

    int minimum() const;
    int maximum() const;

    void setRange(int minimum, int maximum);
    int value() const;

    virtual QString text() const;
    void setTextVisible(bool visible);
    bool isTextVisible() const;

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    Qt::Orientation orientation() const;

    void setInvertedAppearance(bool invert);
    bool invertedAppearance();
    void setTextDirection(QProgressBar::Direction textDirection);
    QProgressBar::Direction textDirection();

    void setFormat(const QString &format);
    QString format() const;

public Q_SLOTS:
    void reset();
    void setMinimum(int minimum);
    void setMaximum(int maximum);
    void setValue(int value);
    void setOrientation(Qt::Orientation);

Q_SIGNALS:
    void valueChanged(int value);

protected:
    bool event(QEvent *e);
    void paintEvent(QPaintEvent *);

private:
    Q_DECLARE_PRIVATE(QProgressBar)
    Q_DISABLE_COPY(QProgressBar)
};

#endif // QT_NO_PROGRESSBAR

QT_END_HEADER

#endif // QPROGRESSBAR_H
