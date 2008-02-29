/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BUTTON_TASKMENU_H
#define BUTTON_TASKMENU_H

#include <QAbstractButton>
#include <QPointer>

#include <qdesigner_taskmenu_p.h>
#include <QtDesigner/default_extensionfactory.h>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {
class InPlaceEditor;

class ButtonTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    ButtonTaskMenu(QAbstractButton *button, QObject *parent = 0);
    virtual ~ButtonTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void editText();
    void editIcon();
    void updateText(const QString &text);
    void updateSelection();

private:
    QAbstractButton *m_button;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QPointer<InPlaceEditor> m_editor;
    mutable QList<QAction*> m_taskActions;
    QAction *m_preferredEditAction;
};

class ButtonTaskMenuFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    ButtonTaskMenuFactory(QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

}  // namespace qdesigner_internal

#endif // BUTTON_TASKMENU_H
