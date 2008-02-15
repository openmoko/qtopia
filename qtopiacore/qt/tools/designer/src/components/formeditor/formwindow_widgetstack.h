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

#ifndef FORMWINDOW_WIDGETSTACK_H
#define FORMWINDOW_WIDGETSTACK_H

#include "formeditor_global.h"

#include <QtGui/QWidget>

class QDesignerFormWindowToolInterface;

namespace qdesigner_internal {

class QT_FORMEDITOR_EXPORT FormWindowWidgetStack: public QWidget
{
    Q_OBJECT
public:
    FormWindowWidgetStack(QWidget *parent = 0);
    virtual ~FormWindowWidgetStack();

    int count() const;
    QDesignerFormWindowToolInterface *tool(int index) const;
    QDesignerFormWindowToolInterface *currentTool() const;
    int currentIndex() const;
    int indexOf(QDesignerFormWindowToolInterface *tool) const;

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

signals:
    void currentToolChanged(int index);

public slots:
    void addTool(QDesignerFormWindowToolInterface *tool);
    void setCurrentTool(QDesignerFormWindowToolInterface *tool);
    void setCurrentTool(int index);
    void setSenderAsCurrentTool();

protected:
    virtual void resizeEvent(QResizeEvent *event);

    QWidget *defaultEditor() const;

private:
    QList<QDesignerFormWindowToolInterface*> m_tools;
    int m_current_index;
};

}  // namespace qdesigner_internal

#endif // FORMWINDOW_WIDGETSTACK_H
