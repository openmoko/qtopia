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

#ifndef QDESIGNER_TABWIDGET_H
#define QDESIGNER_TABWIDGET_H

#include "shared_global_p.h"

#include <QtGui/QTabWidget>

class QDesignerFormWindowInterface;

class QDESIGNER_SHARED_EXPORT QDesignerTabWidget : public QTabWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentTabName READ currentTabName WRITE setCurrentTabName STORED false DESIGNABLE true)
    Q_PROPERTY(QString currentTabText READ currentTabText WRITE setCurrentTabText STORED false DESIGNABLE true)
    Q_PROPERTY(QString currentTabToolTip READ currentTabToolTip WRITE setCurrentTabToolTip STORED false DESIGNABLE true)
    Q_PROPERTY(QIcon currentTabIcon READ currentTabIcon WRITE setCurrentTabIcon STORED false DESIGNABLE true)

public:
    QDesignerTabWidget(QWidget *parent = 0);
    ~QDesignerTabWidget();

    QString currentTabName() const;
    void setCurrentTabName(const QString &tabName);

    QString currentTabText() const;
    void setCurrentTabText(const QString &tabText);

    QString currentTabToolTip() const;
    void setCurrentTabToolTip(const QString &tabToolTip);

    QIcon currentTabIcon() const;
    void setCurrentTabIcon(const QIcon &tabIcon);

    inline QAction *actionDeletePage() const
    { return m_actionDeletePage; }

    inline QAction *actionInsertPage() const
    { return m_actionInsertPage; }

    inline QAction *actionInsertPageAfter() const
    { return m_actionInsertPageAfter; }

    bool eventFilter(QObject *o, QEvent *e);

    QDesignerFormWindowInterface *formWindow() const;

private slots:
    void removeCurrentPage();
    void addPage();
    void addPageAfter();
    void slotCurrentChanged(int index);

protected:
    bool canMove(QMouseEvent *e) const;
    virtual void tabInserted(int index);
    virtual void tabRemoved(int index);

private:
    QPoint pressPoint;
    QWidget *dropIndicator;
    int dragIndex;
    QWidget *dragPage;
    QString dragLabel;
    QIcon dragIcon;
    bool mousePressed;
    QAction *m_actionDeletePage;
    QAction *m_actionInsertPage;
    QAction *m_actionInsertPageAfter;
};

#endif // QDESIGNER_TABWIDGET_H
