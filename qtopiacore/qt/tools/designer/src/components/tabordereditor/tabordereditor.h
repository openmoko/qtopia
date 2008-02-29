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

#ifndef TABORDEREDITOR_H
#define TABORDEREDITOR_H

#include "tabordereditor_global.h"

#include <QtCore/QPointer>
#include <QtGui/QPixmap>
#include <QtGui/QWidget>
#include <QtGui/QRegion>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>

class QUndoStack;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QT_TABORDEREDITOR_EXPORT TabOrderEditor : public QWidget
{
    Q_OBJECT

public:
    TabOrderEditor(QDesignerFormWindowInterface *form, QWidget *parent);

    QDesignerFormWindowInterface *formWindow() const;

public slots:
    void setBackground(QWidget *background);
    void updateBackground();
    void widgetRemoved(QWidget*);
    void initTabOrder();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void showEvent(QShowEvent *e);

private:
    QRect indicatorRect(int index) const;
    int widgetIndexAt(const QPoint &pos) const;
    bool skipWidget(QWidget *w) const;

    QPointer<QDesignerFormWindowInterface> m_form_window;

    QList<QWidget*> m_tab_order_list;

    QWidget *m_bg_widget;
    QUndoStack *m_undo_stack;
    QRegion m_indicator_region;

    QFontMetrics m_font_metrics;
    int m_current_index;
};

}  // namespace qdesigner_internal

#endif
