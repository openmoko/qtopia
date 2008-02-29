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

#include "abstractobjectinspector.h"

/*!
    \class QDesignerObjectInspectorInterface

    \brief The QDesignerObjectInspectorInterface class allows you to
    change the focus of Qt Designer's object inspector.

    \inmodule QtDesigner

    You can use the QDesignerObjectInspectorInterface to change the
    current form window selection. For example, when implementing a
    custom widget plugin:

    \code
        QDesignerObjectInspectorInterface *objectInspector = 0;
        objectInspector = formEditor->objectInspector();

        QDesignerFormWindowManagerInterface *manager = 0;
        manager = formEditor->formWindowManager();

        objectInspector->setFormWindow(manager->formWindow(0));
    \endcode

    The QDesignerObjectInspectorInterface class is not intended to be
    instantiated directly. You can retrieve an interface to \QD's
    object inspector using the
    QDesignerFormEditorInterface::objectInspector() function.  A
    pointer to \QD's current QDesignerFormEditorInterface object (\c
    formEditor in the example above) is provided by the
    QDesignerCustomWidgetInterface::initialize() function's
    parameter. When implementing a custom widget plugin, you must
    subclass the QDesignerCustomWidgetInterface to expose your plugin
    to \QD.

    The interface provides the core() function that you can use to
    retrieve a pointer to \QD's current QDesignerFormEditorInterface
    object, and the setFormWindow() function that enables you to
    change the current form window selection.

    \sa QDesignerFormEditorInterface, QDesignerFormWindowInterface
*/

/*!
    Constructs an object inspector interface with the given \a parent
    and the specified window \a flags.
*/
QDesignerObjectInspectorInterface::QDesignerObjectInspectorInterface(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
}

/*!
    Destroys the object inspector interface.
*/
QDesignerObjectInspectorInterface::~QDesignerObjectInspectorInterface()
{
}

/*!
    Returns a pointer to \QD's current QDesignerFormEditorInterface
    object.
*/
QDesignerFormEditorInterface *QDesignerObjectInspectorInterface::core() const
{
    return 0;
}

/*!
    \fn void QDesignerObjectInspectorInterface::setFormWindow(QDesignerFormWindowInterface *formWindow)

    Sets the currently selected form window to \a formWindow.
*/
