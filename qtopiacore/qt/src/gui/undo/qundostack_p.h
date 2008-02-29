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

#ifndef QUNDOSTACK_P_H
#define QUNDOSTACK_P_H

#include <private/qobject_p.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtGui/qaction.h>

#include "qundostack.h"
class QUndoCommand;
class QUndoGroup;

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

class QUndoCommandPrivate
{
public:
    QUndoCommandPrivate() : id(-1) {}
    QList<QUndoCommand*> child_list;
    QString text;
    int id;
};

#ifndef QT_NO_UNDOSTACK

class QUndoStackPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QUndoStack)
public:
    QUndoStackPrivate() : index(0), clean_index(0), group(0) {}

    QList<QUndoCommand*> command_list;
    QList<QUndoCommand*> macro_stack;
    int index;
    int clean_index;
    QUndoGroup *group;

    void setIndex(int idx, bool clean);
};

#ifndef QT_NO_ACTION
class QUndoAction : public QAction
{
    Q_OBJECT
public:
    QUndoAction(const QString &prefix, QObject *parent = 0);
public Q_SLOTS:
    void setPrefixedText(const QString &text);
private:
    QString m_prefix;
};
#endif // QT_NO_ACTION

#endif // QT_NO_UNDOSTACK
#endif // QUNDOSTACK_P_H
