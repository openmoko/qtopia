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

#include "extrainfo.h"

/*!
    \class QDesignerExtraInfoExtension
    \brief The QDesignerExtraInfoExtension class provides extra information about a widget in
    Qt Designer.
    \inmodule QtDesigner
    \internal
*/

/*!
    Returns the path to the working directory used by this extension.*/
QString QDesignerExtraInfoExtension::workingDirectory() const
{
    return m_workingDirectory;
}

/*!
    Sets the path to the working directory used by the extension to \a workingDirectory.*/
void QDesignerExtraInfoExtension::setWorkingDirectory(const QString &workingDirectory)
{
    m_workingDirectory = workingDirectory;
}

/*!
    \fn virtual QDesignerExtraInfoExtension::~QDesignerExtraInfoExtension()

    Destroys the extension.
*/

/*!
    \fn virtual QDesignerFormEditorInterface *QDesignerExtraInfoExtension::core() const = 0

    \omit
    ### Description required
    \endomit
*/

/*!
    \fn virtual QWidget *QDesignerExtraInfoExtension::widget() const = 0

    Returns the widget described by this extension.
*/

/*!
    \fn virtual bool QDesignerExtraInfoExtension::saveUiExtraInfo(DomUi *ui) = 0

    Saves the information about the user interface specified by \a ui, and returns true if
    successful; otherwise returns false.
*/

/*!
    \fn virtual bool QDesignerExtraInfoExtension::loadUiExtraInfo(DomUi *ui) = 0

    Loads extra information about the user interface specified by \a ui, and returns true if
    successful; otherwise returns false.
*/

/*!
    \fn virtual bool QDesignerExtraInfoExtension::saveWidgetExtraInfo(DomWidget *widget) = 0

    Saves the information about the specified \a widget, and returns true if successful;
    otherwise returns false.
*/

/*!
    \fn virtual bool QDesignerExtraInfoExtension::loadWidgetExtraInfo(DomWidget *widget) = 0

    Loads extra information about the specified \a widget, and returns true if successful;
    otherwise returns false.
*/
