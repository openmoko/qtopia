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

#include "abstractformwindowtool.h"

/*!
    \class QDesignerFormWindowToolInterface

    \brief The QDesignerFormWindowToolInterface class provides an
    interface that enables tools to be used on items in a form window.

    \inmodule QtDesigner

    \internal
*/

/*!
*/
QDesignerFormWindowToolInterface::QDesignerFormWindowToolInterface(QObject *parent)
    : QObject(parent)
{
}

/*!
*/
QDesignerFormWindowToolInterface::~QDesignerFormWindowToolInterface()
{
}

/*!
    \fn virtual QDesignerFormEditorInterface *QDesignerFormWindowToolInterface::core() const = 0
*/

/*!
    \fn virtual QDesignerFormWindowInterface *QDesignerFormWindowToolInterface::formWindow() const = 0
*/

/*!
    \fn virtual QWidget *QDesignerFormWindowToolInterface::editor() const = 0
*/

/*!
    \fn virtual QAction *QDesignerFormWindowToolInterface::action() const = 0
*/

/*!
    \fn virtual void QDesignerFormWindowToolInterface::activated() = 0
*/

/*!
    \fn virtual void QDesignerFormWindowToolInterface::deactivated() = 0
*/

/*!
    \fn virtual void QDesignerFormWindowToolInterface::saveToDom(DomUI*, QWidget*) {
*/

/*!
    \fn virtual void QDesignerFormWindowToolInterface::loadFromDom(DomUI*, QWidget*) {
*/

/*!
    \fn virtual bool QDesignerFormWindowToolInterface::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) = 0
*/
