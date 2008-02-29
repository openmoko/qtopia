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

#ifndef GROUPBOX_TASKMENU_H
#define GROUPBOX_TASKMENU_H

#include <QGroupBox>
#include <QPointer>

#include <qdesigner_taskmenu_p.h>
#include <QtDesigner/default_extensionfactory.h>

class QLineEdit;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class GroupBoxTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    GroupBoxTaskMenu(QGroupBox *groupbox, QObject *parent = 0);
    virtual ~GroupBoxTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void editTitle();
    void editIcon();
    void updateText(const QString &text);
    void updateSelection();

private:
    QGroupBox *m_groupbox;
    QPointer<QLineEdit> m_editor;

    QAction *m_editTitleAction;
    QList<QAction*> m_taskActions;
};

class GroupBoxTaskMenuFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    GroupBoxTaskMenuFactory(QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

}  // namespace qdesigner_internal

#endif // GROUPBOX_TASKMENU_H
