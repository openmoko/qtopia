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

#include "abstractformeditor.h"

#include <QtDesigner/QtDesigner>

/*!
    \class QDesignerFormEditorInterface

    \brief The QDesignerFormEditorInterface class allows you to access
    Qt Designer's various components.

    \inmodule QtDesigner

    \QD's current QDesignerFormEditorInterface object holds
    information about all \QD's components: The action editor, the
    object inspector, the property editor, the widget box, and the
    extension and form window managers. QDesignerFormEditorInterface
    contains a collection of functions that provides interfaces to all
    these components. They are typically used to query (and
    manipulate) the respective component. For example:

    \code
        QDesignerObjectInspectorInterface *objectInspector = 0;
        objectInspector = formEditor->objectInspector();

        QDesignerFormWindowManagerInterface *manager = 0;
        manager = formEditor->formWindowManager();

        objectInspector->setFormWindow(manager->formWindow(0));
    \endcode

    QDesignerFormEditorInterface is not intended to be instantiated
    directly. A pointer to \QD's current QDesignerFormEditorInterface
    object (\c formEditor in the example above) is provided by the
    QDesignerCustomWidgetInterface::initialize() function's
    parameter. When implementing a custom widget plugin, you must
    subclass the QDesignerCustomWidgetInterface to expose your plugin
    to \QD.

    QDesignerFormEditorInterface also provides functions that can set
    the action editor, property editor, object inspector and widget
    box. These are only useful if you want to provide your own custom
    components.

    Finally, QDesignerFormEditorInterface provides the topLevel()
    function that returns \QD's top-level widget.

    \sa QDesignerCustomWidgetInterface
*/

/*!
    Constructs a QDesignerFormEditorInterface object with the given \a
    parent.
*/
QDesignerFormEditorInterface::QDesignerFormEditorInterface(QObject *parent)
    : QObject(parent),
      m_pluginManager(0)
{
}

/*!
    Destroys the QDesignerFormEditorInterface object.
*/
QDesignerFormEditorInterface::~QDesignerFormEditorInterface()
{
}

/*!
    Returns an interface to \QD's widget box.

    \sa setWidgetBox()
*/
QDesignerWidgetBoxInterface *QDesignerFormEditorInterface::widgetBox() const
{ return m_widgetBox; }

/*!
    Sets \QD's widget box to be the specified \a widgetBox.

    \sa widgetBox()
*/
void QDesignerFormEditorInterface::setWidgetBox(QDesignerWidgetBoxInterface *widgetBox)
{ m_widgetBox = widgetBox; }

/*!
    Returns an interface to \QD's property editor.

    \sa setPropertyEditor()
*/
QDesignerPropertyEditorInterface *QDesignerFormEditorInterface::propertyEditor() const
{ return m_propertyEditor; }

/*!
    Sets \QD's property editor to be the specified \a propertyEditor.

    \sa propertyEditor()
*/
void QDesignerFormEditorInterface::setPropertyEditor(QDesignerPropertyEditorInterface *propertyEditor)
{ m_propertyEditor = propertyEditor; }

/*!
    Returns an interface to \QD's action editor.

    \sa setActionEditor()
*/
QDesignerActionEditorInterface *QDesignerFormEditorInterface::actionEditor() const
{ return m_actionEditor; }

/*!
    Sets \QD's action editor to be the specified \a actionEditor.

    \sa actionEditor()
*/
void QDesignerFormEditorInterface::setActionEditor(QDesignerActionEditorInterface *actionEditor)
{ m_actionEditor = actionEditor; }

/*!
    Returns \QD's top-level widget.
*/
QWidget *QDesignerFormEditorInterface::topLevel() const
{ return m_topLevel; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setTopLevel(QWidget *topLevel)
{ m_topLevel = topLevel; }

/*!
    Returns an interface to \QD's form window manager.
*/
QDesignerFormWindowManagerInterface *QDesignerFormEditorInterface::formWindowManager() const
{ return m_formWindowManager; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setFormManager(QDesignerFormWindowManagerInterface *formWindowManager)
{ m_formWindowManager = formWindowManager; }

/*!
    Returns an interface to \QD's extension manager.
*/
QExtensionManager *QDesignerFormEditorInterface::extensionManager() const
{ return m_extensionManager; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setExtensionManager(QExtensionManager *extensionManager)
{ m_extensionManager = extensionManager; }

/*!
    \internal

    Returns an interface to the meta database used by the form editor.
*/
QDesignerMetaDataBaseInterface *QDesignerFormEditorInterface::metaDataBase() const
{ return m_metaDataBase; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setMetaDataBase(QDesignerMetaDataBaseInterface *metaDataBase)
{ m_metaDataBase = metaDataBase; }

/*!
    \internal

    Returns an interface to the widget database used by the form editor.
*/
QDesignerWidgetDataBaseInterface *QDesignerFormEditorInterface::widgetDataBase() const
{ return m_widgetDataBase; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setWidgetDataBase(QDesignerWidgetDataBaseInterface *widgetDataBase)
{ m_widgetDataBase = widgetDataBase; }

/*!
    \internal

    Returns an interface to the widget factory used by the form editor
    to create widgets for the form.
*/
QDesignerWidgetFactoryInterface *QDesignerFormEditorInterface::widgetFactory() const
{ return m_widgetFactory; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setWidgetFactory(QDesignerWidgetFactoryInterface *widgetFactory)
{ m_widgetFactory = widgetFactory; }

/*!
    Returns an interface to \QD's object inspector.
*/
QDesignerObjectInspectorInterface *QDesignerFormEditorInterface::objectInspector() const
{ return m_objectInspector; }

/*!
    Sets \QD's object inspector to be the specified \a
    objectInspector.

    \sa objectInspector()
*/
void QDesignerFormEditorInterface::setObjectInspector(QDesignerObjectInspectorInterface *objectInspector)
{ m_objectInspector = objectInspector; }

/*!
    \internal

    Returns an interface to the brush manager used by the palette editor.
*/
QDesignerBrushManagerInterface *QDesignerFormEditorInterface::brushManager() const
{ return m_brushManager; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setBrushManager(QDesignerBrushManagerInterface *brushManager)
{ m_brushManager = brushManager; }

/*!
    \internal

    Returns an interface to the icon cache used by the form editor to
    manage icons.
*/
QDesignerIconCacheInterface *QDesignerFormEditorInterface::iconCache() const
{ return m_iconCache; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setIconCache(QDesignerIconCacheInterface *cache)
{ m_iconCache = cache; }

/*!
    \internal

    Returns the plugin manager used by the form editor.
*/
QDesignerPluginManager *QDesignerFormEditorInterface::pluginManager() const
{ return m_pluginManager; }

/*!
    \internal

    Sets the plugin manager used by the form editor to the specified
    \a pluginManager.
*/
void QDesignerFormEditorInterface::setPluginManager(QDesignerPluginManager *pluginManager)
{ m_pluginManager = pluginManager; }

/*!
    \internal

    Returns the path to the resources used by the form editor.
*/
QString QDesignerFormEditorInterface::resourceLocation() const
{
#ifdef Q_WS_MAC
    return QLatin1String(":/trolltech/formeditor/images/mac");
#else
    return QLatin1String(":/trolltech/formeditor/images/win");
#endif
}
