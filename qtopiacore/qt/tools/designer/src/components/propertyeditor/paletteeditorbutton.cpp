/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.1, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
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
