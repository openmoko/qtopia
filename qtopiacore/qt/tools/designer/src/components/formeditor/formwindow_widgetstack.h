/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef FORMWINDOW_WIDGETSTACK_H
#define FORMWINDOW_WIDGETSTACK_H

#include "formeditor_global.h"

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowToolInterface;

class QStackedLayout;

namespace qdesigner_internal {

class QT_FORMEDITOR_EXPORT FormWindowWidgetStack: public QObject
{
    Q_OBJECT
public:
    FormWindowWidgetStack(QObject *parent = 0);
    virtual ~FormWindowWidgetStack();

    QLayout *layout() const;

    int count() const;
    QDesignerFormWindowToolInterface *tool(int index) const;
    QDesignerFormWindowToolInterface *currentTool() const;
    int currentIndex() const;
    int indexOf(QDesignerFormWindowToolInterface *tool) const;

    void setMainContainer(QWidget *w = 0);

signals:
    void currentToolChanged(int index);

public slots:
    void addTool(QDesignerFormWindowToolInterface *tool);
    void setCurrentTool(QDesignerFormWindowToolInterface *tool);
    void setCurrentTool(int index);
    void setSenderAsCurrentTool();

protected:
    QWidget *defaultEditor() const;

private:
    QList<QDesignerFormWindowToolInterface*> m_tools;
    QStackedLayout *m_layout;
    bool m_dummyMainContainer;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMWINDOW_WIDGETSTACK_H
