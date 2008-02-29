/****************************************************************************
**
** Copyright (C) 2005-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
