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

#ifndef COMBOBOX_TASKMENU_H
#define COMBOBOX_TASKMENU_H

#include <QtGui/QComboBox>
#include <QtCore/QPointer>

#include <qdesigner_taskmenu_p.h>
#include <QtDesigner/default_extensionfactory.h>

class QLineEdit;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class ComboBoxTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    ComboBoxTaskMenu(QComboBox *button, QObject *parent = 0);
    virtual ~ComboBoxTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void editItems();
    void updateSelection();

private:
    QComboBox *m_comboBox;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QPointer<QLineEdit> m_editor;
    mutable QList<QAction*> m_taskActions;
    QAction *m_editItemsAction;
};

class ComboBoxTaskMenuFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    ComboBoxTaskMenuFactory(QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

}  // namespace qdesigner_internal

#endif // COMBOBOX_TASKMENU_H
