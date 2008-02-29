/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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
