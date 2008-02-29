/****************************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA. All rights reserved.
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

#include <QtGui/QScrollBar>
#include "oubliette.h"
#include "oublietteview.h"

OublietteView::OublietteView()
{
    m_oubliette = new Oubliette;
    setWidget(m_oubliette);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    connect(m_oubliette, SIGNAL(characterMoved(QPoint)),
            this, SLOT(scrollToCharacter(QPoint)));
    setFocusPolicy(Qt::NoFocus);
    m_oubliette->setFocus();
    scrollToCharacter(m_oubliette->visualCursorPos());
}

OublietteView::~OublietteView()
{
}

void OublietteView::scrollToCharacter(const QPoint &pt)
{
    horizontalScrollBar()->setValue(pt.x() - width() / 2);
    verticalScrollBar()->setValue(pt.y() - height() / 2);
}
