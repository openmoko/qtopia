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

#include "qdesigner_integration_p.h"
#include "qdesigner_command_p.h"

// sdk
#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionManager>

#include <QtCore/QVariant>

#include <QtCore/qdebug.h>

namespace qdesigner_internal {

QDesignerIntegration::QDesignerIntegration(QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent),
      m_core(core)
{
    initialize();
}

QDesignerIntegration::~QDesignerIntegration()
{
}

void QDesignerIntegration::initialize()
{
    //
    // integrate the `Form Editor component'
    //
    connect(core()->propertyEditor(), SIGNAL(propertyChanged(QString,QVariant)),
            this, SLOT(updateProperty(QString,QVariant)));

    connect(core()->formWindowManager(), SIGNAL(formWindowAdded(QDesignerFormWindowInterface*)),
            this, SLOT(setupFormWindow(QDesignerFormWindowInterface*)));

    connect(core()->formWindowManager(), SIGNAL(activeFormWindowChanged(QDesignerFormWindowInterface*)),
            this, SLOT(updateActiveFormWindow(QDesignerFormWindowInterface*)));
}

void QDesignerIntegration::updateProperty(const QString &name, const QVariant &value)
{
    Q_ASSERT(core()->propertyEditor() != 0);
    Q_ASSERT(core()->propertyEditor()->object() != 0);

    if (QDesignerFormWindowInterface *formWindow = core()->formWindowManager()->activeFormWindow()) {
        QObject *object = core()->propertyEditor()->object();
        QWidget *widget = qobject_cast<QWidget*>(object);

        QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), object);
        Q_ASSERT(sheet != 0);

        int propertyIndex = sheet->indexOf(name);
        QDesignerFormWindowCursorInterface *cursor = formWindow->cursor();

        if (widget && cursor->isWidgetSelected(widget)) {
            if (cursor->isWidgetSelected(formWindow->mainContainer())) {
                if (name == QLatin1String("windowTitle")) {
                    QString filename = formWindow->fileName().isEmpty()
                            ? QString::fromUtf8("Untitled")
                            : formWindow->fileName();

                    formWindow->setWindowTitle(QString::fromUtf8("%1 - (%2)")
                                            .arg(value.toString())
                                            .arg(filename));

                } else if (name == QLatin1String("geometry")) {
                    if (QWidget *container = containerWindow(formWindow)) {
                        SetFormPropertyCommand *cmd = new SetFormPropertyCommand(formWindow);
                        cmd->init(object, name, value);
                        cmd->setOldValue(container->geometry());
                        formWindow->commandHistory()->push(cmd);

                        emit propertyChanged(formWindow, name, value);
                    }

                    return;
                }
            }

            cursor->setProperty(name, value);
        } else if (propertyIndex != -1) {
            SetPropertyCommand *cmd = new SetPropertyCommand(formWindow);
            cmd->init(object, name, value);
            formWindow->commandHistory()->push(cmd);
        }

        if (name == QLatin1String("objectName") && core()->objectInspector()) {
            core()->objectInspector()->setFormWindow(formWindow);
        }

        emit propertyChanged(formWindow, name, value);

        core()->propertyEditor()->setPropertyValue(name, sheet->property(propertyIndex));
    }
}

void QDesignerIntegration::updateActiveFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_UNUSED(formWindow);
    updateSelection();
}

void QDesignerIntegration::setupFormWindow(QDesignerFormWindowInterface *formWindow)
{
    connect(formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
    connect(formWindow, SIGNAL(activated(QWidget*)), this, SLOT(activateWidget(QWidget*)));
}

void QDesignerIntegration::updateGeometry()
{
}

void QDesignerIntegration::updateSelection()
{
    QDesignerFormWindowInterface *formWindow = core()->formWindowManager()->activeFormWindow();
    QWidget *selection = 0;

    if (formWindow)
        selection = formWindow->cursor()->selectedWidget(0);

    if (QDesignerActionEditorInterface *actionEditor = core()->actionEditor())
        actionEditor->setFormWindow(formWindow);

    if (QDesignerPropertyEditorInterface *propertyEditor = core()->propertyEditor()) {
        propertyEditor->setObject(selection);
        propertyEditor->setEnabled(formWindow && formWindow->cursor()->selectedWidgetCount() == 1);
    }
    if (QDesignerObjectInspectorInterface *objectInspector = core()->objectInspector())
        objectInspector->setFormWindow(formWindow);

}

void QDesignerIntegration::activateWidget(QWidget *widget)
{
    Q_UNUSED(widget);
}

QWidget *QDesignerIntegration::containerWindow(QWidget *widget)
{
    while (widget) {
        if (widget->isWindow())
            break;
        if (widget->parentWidget() && !qstrcmp(widget->parentWidget()->metaObject()->className(), "QWorkspaceChild"))
            break;

        widget = widget->parentWidget();
    }

    return widget;
}


} // namespace qdesigner_internal
