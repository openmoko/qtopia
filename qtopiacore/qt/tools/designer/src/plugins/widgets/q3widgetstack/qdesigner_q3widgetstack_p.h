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

#ifndef QDESIGNER_Q3WIDGETSTACK_P_H
#define QDESIGNER_Q3WIDGETSTACK_P_H

#include <Qt3Support/Q3WidgetStack>

class QDesignerFormWindowInterface;
class QDesignerContainerExtension;
class QToolButton;
class QChildEvent;
class QResizeEvent;
class QShowEvent;
class QEvent;
class QWidget;

class QDesignerQ3WidgetStack : public Q3WidgetStack
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex STORED false DESIGNABLE true)
    Q_PROPERTY(QString currentPageName READ currentPageName WRITE setCurrentPageName STORED false DESIGNABLE true)
public:
    QDesignerQ3WidgetStack(QWidget *parent = 0);
    int currentIndex();
    QString currentPageName();

public slots:
    void updateButtons();
    void setCurrentIndex(int index);
    void setCurrentPageName(const QString &pageName);

private slots:
    void prevPage();
    void nextPage();
    void slotCurrentChanged(int index);

signals:
    void currentChanged(int index);

protected:
    virtual void childEvent(QChildEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void showEvent(QShowEvent *e);
    virtual bool event(QEvent *e);

private:
    QDesignerFormWindowInterface *formWindow();
    QDesignerContainerExtension *container();
    int count();
    QWidget *widget(int index);
    QToolButton *prev, *next;
};

#endif // !QDESIGNER_Q3WIDGETSTACK_P_H
