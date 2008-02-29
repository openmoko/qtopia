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
