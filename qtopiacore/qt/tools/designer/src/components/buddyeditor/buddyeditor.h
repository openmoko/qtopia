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

#ifndef BUDDYEDITOR_H
#define BUDDYEDITOR_H

#include "buddyeditor_global.h"

#include <connectionedit_p.h>
#include <QtCore/QPointer>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QT_BUDDYEDITOR_EXPORT BuddyEditor : public ConnectionEdit
{
    Q_OBJECT

public:
    BuddyEditor(QDesignerFormWindowInterface *form, QWidget *parent);

    QDesignerFormWindowInterface *formWindow() const;
    virtual void setBackground(QWidget *background);
    virtual void deleteSelected();

public slots:
    virtual void updateBackground();
    virtual void widgetRemoved(QWidget *w);

protected:
    virtual QWidget *widgetAt(const QPoint &pos) const;
    virtual Connection *createConnection(QWidget *source, QWidget *destination);
    virtual void endConnection(QWidget *target, const QPoint &pos);

private:
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    bool m_updating;
};

}  // namespace qdesigner_internal

#endif
