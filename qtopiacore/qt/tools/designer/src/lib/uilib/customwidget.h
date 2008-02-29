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

#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <QtDesigner/extension.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QIcon>

QT_BEGIN_HEADER

class QWidget;
class QDesignerFormEditorInterface;

class QDesignerCustomWidgetInterface
{
public:
    virtual ~QDesignerCustomWidgetInterface() {}

    virtual QString name() const = 0;
    virtual QString group() const = 0;
    virtual QString toolTip() const = 0;
    virtual QString whatsThis() const = 0;
    virtual QString includeFile() const = 0;
    virtual QIcon icon() const = 0;

    virtual bool isContainer() const = 0;

    virtual QWidget *createWidget(QWidget *parent) = 0;

    virtual bool isInitialized() const { return false; }
    virtual void initialize(QDesignerFormEditorInterface *core) { Q_UNUSED(core); }

    virtual QString domXml() const
    {
        return QString::fromUtf8("<widget class=\"%1\" name=\"%2\"/>")
            .arg(name()).arg(name().toLower());
    }

    virtual QString codeTemplate() const { return QString(); }
};
Q_DECLARE_INTERFACE(QDesignerCustomWidgetInterface, "com.trolltech.Qt.Designer.CustomWidget")


class QDesignerCustomWidgetCollectionInterface
{
public:
    virtual ~QDesignerCustomWidgetCollectionInterface() {}

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const = 0;
};
Q_DECLARE_INTERFACE(QDesignerCustomWidgetCollectionInterface, "com.trolltech.Qt.Designer.CustomWidgetCollection")

QT_END_HEADER

#endif // CUSTOMWIDGET_H
