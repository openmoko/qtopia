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

#ifndef PREVIEWWIDGETBASE_H
#define PREVIEWWIDGETBASE_H

#include "ui_previewwidgetbase.h"
#include <QVariant>

class PreviewWidgetBase : public QWidget, public Ui::PreviewWidgetBase
{
    Q_OBJECT

public:
    PreviewWidgetBase(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0);
    ~PreviewWidgetBase();

protected slots:
    virtual void languageChange();

    virtual void init();
    virtual void destroy();

};

#endif // PREVIEWWIDGETBASE_H
