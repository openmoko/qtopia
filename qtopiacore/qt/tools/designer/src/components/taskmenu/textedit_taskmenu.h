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

#ifndef TEXTEDIT_TASKMENU_H
#define TEXTEDIT_TASKMENU_H

#include <QtGui/QTextEdit>
#include <QtCore/QPointer>

#include <qdesigner_taskmenu_p.h>
#include <QtDesigner/default_extensionfactory.h>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class TextEditTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    TextEditTaskMenu(QTextEdit *button, QObject *parent = 0);
    virtual ~TextEditTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void editText();
    void editIcon();
    void updateText(const QString &text);

private:
    QTextEdit *m_textEdit;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    mutable QList<QAction*> m_taskActions;
    QAction *m_editTextAction;
};

class TextEditTaskMenuFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    TextEditTaskMenuFactory(QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

}  // namespace qdesigner_internal

#endif // TEXTEDIT_TASKMENU_H
