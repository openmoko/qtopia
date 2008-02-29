/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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
#include "q3popupmenu.h"

/*!
    \fn void Q3PopupMenu::setFrameRect(QRect)
    \internal
*/

/*!
    \fn QRect Q3PopupMenu::frameRect() const
    \internal
*/
/*!
    \enum Q3PopupMenu::DummyFrame
    \internal

    \value Box
    \value Sunken
    \value Plain
    \value Raised
    \value MShadow
    \value NoFrame
    \value Panel 
    \value StyledPanel
    \value HLine 
    \value VLine 
    \value GroupBoxPanel
    \value WinPanel 
    \value ToolBarPanel 
    \value MenuBarPanel 
    \value PopupPanel 
    \value LineEditPanel 
    \value TabWidgetPanel 
    \value MShape
*/

/*!
    \fn void Q3PopupMenu::setFrameShadow(DummyFrame)
    \internal
*/

/*!
    \fn DummyFrame Q3PopupMenu::frameShadow() const
    \internal
*/

/*!
    \fn void Q3PopupMenu::setFrameShape(DummyFrame)
    \internal
*/

/*!
    \fn DummyFrame Q3PopupMenu::frameShape() const
    \internal
*/

/*!
    \fn void Q3PopupMenu::setFrameStyle(int)
    \internal
*/

/*!
    \fn int Q3PopupMenu::frameStyle() const
    \internal
*/

/*!
    \fn int Q3PopupMenu::frameWidth() const
    \internal
*/

/*!
    \fn void Q3PopupMenu::setLineWidth(int)
    \internal
*/

/*!
    \fn int Q3PopupMenu::lineWidth() const
    \internal
*/

/*!
    \fn void Q3PopupMenu::setMargin(int margin)
    \since 4.2

    Sets the width of the margin around the contents of the widget to \a margin.
    
    This function uses QWidget::setContentsMargins() to set the margin.
    \sa margin(), QWidget::setContentsMargins()
*/

/*!
    \fn int Q3PopupMenu::margin() const 
    \since 4.2

    Returns the with of the the margin around the contents of the widget.
    
    This function uses QWidget::getContentsMargins() to get the margin.
    \sa setMargin(), QWidget::getContentsMargins()
*/

/*!
    \fn void Q3PopupMenu::setMidLineWidth(int)
    \internal
*/

/*!
    \fn int Q3PopupMenu::midLineWidth() const
    \internal
*/
