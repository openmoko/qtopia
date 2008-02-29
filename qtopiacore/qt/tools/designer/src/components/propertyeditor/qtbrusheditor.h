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

#ifndef QTBRUSHEDITOR_H
#define QTBRUSHEDITOR_H

#include <QWidget>

class QDesignerBrushManagerInterface;

namespace qdesigner_internal {

class QtBrushEditor : public QWidget
{
    Q_OBJECT
public:
    QtBrushEditor(QWidget *parent = 0);
    ~QtBrushEditor();

    void setBrush(const QBrush &brush);
    QBrush brush() const;

    void setBrushManager(QDesignerBrushManagerInterface *manager);

protected:
    void contextMenuEvent(QContextMenuEvent *e);
signals:
    void textureChooserActivated(QWidget *parent, const QBrush &initialBrush);
private:
    class QtBrushEditorPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushEditor)
    Q_DISABLE_COPY(QtBrushEditor)
    Q_PRIVATE_SLOT(d_func(), void slotPatternChooserClicked())
    Q_PRIVATE_SLOT(d_func(), void slotTextureChooserClicked())
    Q_PRIVATE_SLOT(d_func(), void slotGradientChooserClicked())
    Q_PRIVATE_SLOT(d_func(), void slotChooserClicked())
    Q_PRIVATE_SLOT(d_func(), void slotApplyClicked())
    Q_PRIVATE_SLOT(d_func(), void slotAddToCustomClicked())
    Q_PRIVATE_SLOT(d_func(), void slotRemoveClicked())
    Q_PRIVATE_SLOT(d_func(), void slotItemActivated(QListWidgetItem *))
    Q_PRIVATE_SLOT(d_func(), void slotCurrentItemChanged(QListWidgetItem *))
    Q_PRIVATE_SLOT(d_func(), void slotItemRenamed(QListWidgetItem *))
    Q_PRIVATE_SLOT(d_func(), void slotBrushAdded(const QString &, const QBrush &))
    Q_PRIVATE_SLOT(d_func(), void slotBrushRemoved(const QString &))
    Q_PRIVATE_SLOT(d_func(), void slotCurrentBrushChanged(const QString &, const QBrush &))
};

}

#endif
