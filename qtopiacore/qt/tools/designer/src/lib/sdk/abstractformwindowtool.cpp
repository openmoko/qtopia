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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
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
