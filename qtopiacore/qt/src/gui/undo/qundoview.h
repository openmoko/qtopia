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

#ifndef QUNDOVIEW_H
#define QUNDOVIEW_H

#include <QtGui/qlistview.h>
#include <QtCore/qstring.h>

#ifndef QT_NO_UNDOVIEW

QT_BEGIN_HEADER

class QUndoViewPrivate;
class QUndoStack;
class QUndoGroup;
class QIcon;

QT_MODULE(Gui)

class Q_GUI_EXPORT QUndoView : public QListView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QUndoView)
    Q_PROPERTY(QString emptyLabel READ emptyLabel WRITE setEmptyLabel)
    Q_PROPERTY(QIcon cleanIcon READ cleanIcon WRITE setCleanIcon)

public:
    explicit QUndoView(QWidget *parent = 0);
    explicit QUndoView(QUndoStack *stack, QWidget *parent = 0);
#ifndef QT_NO_UNDOGROUP
    explicit QUndoView(QUndoGroup *group, QWidget *parent = 0);
#endif
    ~QUndoView();

    QUndoStack *stack() const;
#ifndef QT_NO_UNDOGROUP
    QUndoGroup *group() const;
#endif

    void setEmptyLabel(const QString &label);
    QString emptyLabel() const;

    void setCleanIcon(const QIcon &icon);
    QIcon cleanIcon() const;

public Q_SLOTS:
    void setStack(QUndoStack *stack);
#ifndef QT_NO_UNDOGROUP
    void setGroup(QUndoGroup *group);
#endif

private:
    Q_DISABLE_COPY(QUndoView)
};

QT_END_HEADER

#endif // QT_NO_UNDOVIEW
#endif // QUNDOVIEW_H
