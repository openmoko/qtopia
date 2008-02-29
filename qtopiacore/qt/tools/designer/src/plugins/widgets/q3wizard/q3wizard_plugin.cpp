/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3wizard_plugin.h"
#include "q3wizard_container.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <QtGui/QPushButton>
#include <Qt3Support/Q3Wizard>

Q3WizardPlugin::Q3WizardPlugin(QObject *parent)
    : QObject(parent), m_initialized(false)
{}

QString Q3WizardPlugin::name() const
{ return QLatin1String("Q3Wizard"); }

QString Q3WizardPlugin::group() const
{ return QLatin1String("[invisible]"); }

QString Q3WizardPlugin::toolTip() const
{ return QString(); }

QString Q3WizardPlugin::whatsThis() const
{ return QString(); }

QString Q3WizardPlugin::includeFile() const
{ return QLatin1String("q3wizard.h"); }

QIcon Q3WizardPlugin::icon() const
{ return QIcon(); }

bool Q3WizardPlugin::isContainer() const
{ return true; }

QWidget *Q3WizardPlugin::createWidget(QWidget *parent)
{
    Q3Wizard *wizard = new Q3Wizard(parent);
    wizard->backButton()->setObjectName(QLatin1String("__qt__passive_") + wizard->backButton()->objectName());
    wizard->nextButton()->setObjectName(QLatin1String("__qt__passive_") + wizard->nextButton()->objectName());
    return wizard;
}

bool Q3WizardPlugin::isInitialized() const
{ return m_initialized; }

void Q3WizardPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);

    if (m_initialized)
        return;

    m_initialized = true;
    QExtensionManager *mgr = core->extensionManager();
    mgr->registerExtensions(new Q3WizardContainerFactory(mgr), Q_TYPEID(QDesignerContainerExtension));
}

QString Q3WizardPlugin::codeTemplate() const
{ return QString(); }

QString Q3WizardPlugin::domXml() const
{
    return QLatin1String("\
    <widget class=\"Q3Wizard\" name=\"wizard\">\
        <property name=\"geometry\">\
            <rect>\
                <x>0</x>\
                <y>0</y>\
                <width>100</width>\
                <height>80</height>\
            </rect>\
        </property>\
        <widget class=\"QWidget\" />\
        <widget class=\"QWidget\" />\
    </widget>\
    ");
}

