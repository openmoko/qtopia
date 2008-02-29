/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef BUTTON_EDITOR_DIALOG_H

#include <qdialog.h>
#include <qlist.h>
#include <qstringlist.h>
#include <qtopia/services/services.h>

class DeviceButton;
class QComboBox;
class AppLnk;

class ButtonEditorDialog : public QDialog {
public:
    ButtonEditorDialog();
    virtual ~ButtonEditorDialog();
protected:
    virtual void accept();
    virtual void done(int r);
private:
    void populate(QComboBox* comboBox, const ServiceRequest& current);
    ServiceRequest ButtonEditorDialog::actionFor(int cur) const;
    void initAppList();
    void buildGUI();

    QList<AppLnk> m_AppList;
    QStringList m_Actions;
    QList<QComboBox> m_PressComboBoxes;
    QList<QComboBox> m_HoldComboBoxes;
};

#endif
