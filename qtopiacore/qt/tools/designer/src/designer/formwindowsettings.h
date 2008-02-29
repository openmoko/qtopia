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

#ifndef FORMWINDOWSETTINGS_H
#define FORMWINDOWSETTINGS_H

#include "ui_formwindowsettings.h"

class QDesignerFormWindowInterface;

class FormWindowSettings: public QDialog
{
    Q_OBJECT
public:
    FormWindowSettings(QDesignerFormWindowInterface *formWindow);
    virtual ~FormWindowSettings();

    QDesignerFormWindowInterface *formWindow() const;

    virtual void accept();

private:
    Ui::FormWindowSettings ui;
    QDesignerFormWindowInterface *m_formWindow;
};

#endif // FORMWINDOWSETTINGS_H

