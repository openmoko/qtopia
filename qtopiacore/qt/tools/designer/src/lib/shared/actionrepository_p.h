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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef ACTIONREPOSITORY_H
#define ACTIONREPOSITORY_H

#include "shared_global_p.h"
#include <QtCore/QMimeData>
#include <QtGui/QListWidget>

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT ActionRepository: public QListWidget
{
    Q_OBJECT
public:
    enum
    {
        ActionRole = Qt::UserRole + 1000
    };

public:
    ActionRepository(QWidget *parent = 0);
    virtual ~ActionRepository();

signals:
    void contextMenuRequested(QContextMenuEvent *event, QListWidgetItem *item);

public slots:
    void filter(const QString &text);

protected:
    virtual void startDrag(Qt::DropActions supportedActions);
    virtual QMimeData *mimeData(const QList<QListWidgetItem*> items) const;
    virtual void focusInEvent(QFocusEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);
};

class QDESIGNER_SHARED_EXPORT ActionRepositoryMimeData: public QMimeData
{
    Q_OBJECT
public:
    ActionRepositoryMimeData() {}
    virtual ~ActionRepositoryMimeData() {}

    QList<QAction*> items;

    virtual QStringList formats() const { return QStringList() << "action-repository/actions"; }
};

} // namespace qdesigner_internal

#endif // ACTIONREPOSITORY_H
