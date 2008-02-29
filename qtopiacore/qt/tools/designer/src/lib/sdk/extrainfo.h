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

#ifndef EXTRAINFO_H
#define EXTRAINFO_H

#include <QtDesigner/sdk_global.h>
#include <QtDesigner/extension.h>

QT_BEGIN_HEADER

class DomWidget;
class DomUi;
class QWidget;

class QDesignerFormEditorInterface;

class QDESIGNER_SDK_EXPORT QDesignerExtraInfoExtension
{
public:
    virtual ~QDesignerExtraInfoExtension() {}

    virtual QDesignerFormEditorInterface *core() const = 0;
    virtual QWidget *widget() const = 0;

    virtual bool saveUiExtraInfo(DomUi *ui) = 0;
    virtual bool loadUiExtraInfo(DomUi *ui) = 0;

    virtual bool saveWidgetExtraInfo(DomWidget *ui_widget) = 0;
    virtual bool loadWidgetExtraInfo(DomWidget *ui_widget) = 0;

    QString workingDirectory() const;
    void setWorkingDirectory(const QString &workingDirectory);

private:
    QString m_workingDirectory;
};
Q_DECLARE_EXTENSION_INTERFACE(QDesignerExtraInfoExtension, "com.trolltech.Qt.Designer.ExtraInfo")


QT_END_HEADER

#endif // EXTRAINFO_H
