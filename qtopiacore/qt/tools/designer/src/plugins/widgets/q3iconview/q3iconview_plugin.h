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

#ifndef Q3ICONVIEW_PLUGIN_H
#define Q3ICONVIEW_PLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>

class Q3IconViewPlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    Q3IconViewPlugin(QObject *parent = 0);

    virtual QString name() const;
    virtual QString group() const;
    virtual QString toolTip() const;
    virtual QString whatsThis() const;
    virtual QString includeFile() const;
    virtual QIcon icon() const;
    virtual bool isContainer() const;
    virtual QWidget *createWidget(QWidget *parent);
    virtual bool isInitialized() const;
    virtual void initialize(QDesignerFormEditorInterface *core);
    virtual QString codeTemplate() const;
    virtual QString domXml() const;

private:
    bool m_initialized;
};

#endif // Q3ICONVIEW_PLUGIN_H
