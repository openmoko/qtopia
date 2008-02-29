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

#ifndef PALETTEEDITORBUTTON_H
#define PALETTEEDITORBUTTON_H

#include "propertyeditor_global.h"

#include <QtGui/QPalette>
#include <QtGui/QToolButton>

#include "abstractformeditor.h"

namespace qdesigner_internal {

class QT_PROPERTYEDITOR_EXPORT PaletteEditorButton: public QToolButton
{
    Q_OBJECT
public:
    PaletteEditorButton(QDesignerFormEditorInterface *core, const QPalette &palette,
                QWidget *selectedWidget, QWidget *parent = 0);
    virtual ~PaletteEditorButton();

    inline QPalette palette() const
    { return m_palette; }

signals:
    void changed();

public slots:
    void setPalette(const QPalette &palette);

private slots:
    void showPaletteEditor();

private:
    QPalette m_palette;
    QWidget *m_selectedWidget;
    QDesignerFormEditorInterface *m_core;
};

}  // namespace qdesigner_internal

#endif // PALETTEEDITORBUTTON_H
