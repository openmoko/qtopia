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

#ifndef ABSTRACTWIDGETFACTORY_H
#define ABSTRACTWIDGETFACTORY_H

#include <QtDesigner/sdk_global.h>
#include <QtCore/QObject>

QT_BEGIN_HEADER

class QDesignerFormEditorInterface;
class QWidget;
class QLayout;

class QDESIGNER_SDK_EXPORT QDesignerWidgetFactoryInterface: public QObject
{
    Q_OBJECT
public:
    QDesignerWidgetFactoryInterface(QObject *parent = 0);
    virtual ~QDesignerWidgetFactoryInterface();

    virtual QDesignerFormEditorInterface *core() const = 0;

    virtual QWidget* containerOfWidget(QWidget *w) const = 0;
    virtual QWidget* widgetOfContainer(QWidget *w) const = 0;

    virtual QWidget *createWidget(const QString &name, QWidget *parentWidget = 0) const = 0;
    virtual QLayout *createLayout(QWidget *widget, QLayout *layout, int type) const = 0;

    virtual bool isPassiveInteractor(QWidget *widget) = 0;
    virtual void initialize(QObject *object) const = 0;
};

QT_END_HEADER

#endif // ABSTRACTWIDGETFACTORY_H
