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

/*
TRANSLATOR qdesigner_internal::PaletteEditorButton
*/

#include "paletteeditorbutton.h"
#include "paletteeditor.h"

#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

PaletteEditorButton::PaletteEditorButton(QDesignerFormEditorInterface *core, const QPalette &palette,
                QWidget *selectedWidget, QWidget *parent)
    : QToolButton(parent),
      m_palette(palette)
{
    m_core = core;
    m_selectedWidget = selectedWidget;
    setFocusPolicy(Qt::NoFocus);
    setText(tr("Change Palette"));

    connect(this, SIGNAL(clicked()), this, SLOT(showPaletteEditor()));
}

PaletteEditorButton::~PaletteEditorButton()
{
}

void PaletteEditorButton::setPalette(const QPalette &palette)
{
    m_palette = palette;
}

void PaletteEditorButton::showPaletteEditor()
{
    int result;
    QPalette p = QPalette();
    if (m_selectedWidget) {
        if (m_selectedWidget->isWindow())
            p = QApplication::palette(m_selectedWidget);
        else {
            if (m_selectedWidget->parentWidget())
                p = m_selectedWidget->parentWidget()->palette();
        }
    }
    QPalette pal = PaletteEditor::getPalette(m_core, 0, m_palette, p, &result);
    if (result == QDialog::Accepted) {
        m_palette = pal;
        emit changed();
    }
}
