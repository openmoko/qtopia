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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//


#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include "shared_global_p.h"
#include "pluginmanager_p.h"

#include <QtDesigner/QDesignerWidgetFactoryInterface>

#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtCore/QPointer>

class QObject;
class QWidget;
class QLayout;
class QDesignerFormEditorInterface;
class QDesignerCustomWidgetInterface;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT WidgetFactory: public QDesignerWidgetFactoryInterface
{
    Q_OBJECT
public:
    WidgetFactory(QDesignerFormEditorInterface *core, QObject *parent = 0);
    ~WidgetFactory();

    virtual QWidget* containerOfWidget(QWidget *widget) const;
    virtual QWidget* widgetOfContainer(QWidget *widget) const;

    virtual QWidget *createWidget(const QString &className, QWidget *parentWidget) const;
    virtual QLayout *createLayout(QWidget *widget, QLayout *layout, int type) const;

    virtual bool isPassiveInteractor(QWidget *widget);
    virtual void initialize(QObject *object) const;

    virtual QDesignerFormEditorInterface *core() const;

    static const char* classNameOf(QObject* o);

    QDesignerFormWindowInterface *currentFormWindow(QDesignerFormWindowInterface *fw);

public slots:
    void loadPlugins();

private:
    QDesignerFormEditorInterface *m_core;
    QMap<QString, QDesignerCustomWidgetInterface*> m_customFactory;
    QDesignerFormWindowInterface *m_formWindow;

    static QPointer<QWidget> *m_lastPassiveInteractor;
    static bool m_lastWasAPassiveInteractor;
};

} // namespace qdesigner_internal

#endif // WIDGETFACTORY_H
