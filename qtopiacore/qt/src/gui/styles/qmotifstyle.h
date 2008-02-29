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

#ifndef QMOTIFSTYLE_H
#define QMOTIFSTYLE_H

#include <QtGui/qcommonstyle.h>
#include <QtCore/qpointer.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#if !defined(QT_NO_STYLE_MOTIF)

class QPalette;
class QFocusFrame;

class QMotifStylePrivate;
class Q_GUI_EXPORT QMotifStyle : public QCommonStyle
{
    Q_OBJECT
public:
    explicit QMotifStyle(bool useHighlightCols=false);
    virtual ~QMotifStyle();

    void setUseHighlightColors(bool);
    bool useHighlightColors() const;

    void polish(QPalette&);
    void polish(QWidget*);
    void unpolish(QWidget*);
    void polish(QApplication*);
    void unpolish(QApplication*);

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                        const QWidget *w = 0) const;

    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                      const QWidget *w = 0) const;

    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *w = 0) const;

    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                         SubControl sc, const QWidget *widget = 0) const;

    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0,
                     const QWidget *widget = 0) const;

    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
                           const QSize &contentsSize, const QWidget *widget = 0) const;

    QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const;

    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                           const QWidget *widget = 0) const;

    int styleHint(StyleHint hint, const QStyleOption *opt = 0, const QWidget *widget = 0,
                  QStyleHintReturn *returnData = 0) const;

    bool event(QEvent *);
    QPalette standardPalette() const;

protected Q_SLOTS:
    QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0,
                                     const QWidget *widget = 0) const;

protected:
    QPointer<QFocusFrame> focus;
    QMotifStyle(QMotifStylePrivate &dd, bool useHighlightCols = false);
    void timerEvent(QTimerEvent *event);
    bool eventFilter(QObject *o, QEvent *e);

private:
    Q_DECLARE_PRIVATE(QMotifStyle)
    Q_DISABLE_COPY(QMotifStyle)

    bool highlightCols;
};

#endif // QT_NO_STYLE_MOTIF

QT_END_HEADER

#endif // QMOTIFSTYLE_H
