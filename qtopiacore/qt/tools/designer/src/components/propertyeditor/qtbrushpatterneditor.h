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

#ifndef QTBRUSHPATTERNEDITOR_H
#define QTBRUSHPATTERNEDITOR_H

#include <QWidget>

namespace qdesigner_internal {

class QtBrushPatternEditor : public QWidget
{
    Q_OBJECT
public:
    QtBrushPatternEditor(QWidget *parent = 0);
    ~QtBrushPatternEditor();

    void setBrush(const QBrush &brush);
    QBrush brush() const;

private:
    class QtBrushPatternEditorPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushPatternEditor)
    Q_DISABLE_COPY(QtBrushPatternEditor)
    Q_PRIVATE_SLOT(d_func(), void slotHsvClicked())
    Q_PRIVATE_SLOT(d_func(), void slotRgbClicked())
    Q_PRIVATE_SLOT(d_func(), void slotPatternChanged(int pattern))
    Q_PRIVATE_SLOT(d_func(), void slotChangeColor(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeHue(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeSaturation(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeValue(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeAlpha(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeHue(int color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeSaturation(int color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeValue(int color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeAlpha(int color))
};

}

#endif
