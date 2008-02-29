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

#ifndef QDESIGNER_STACKEDBOX_H
#define QDESIGNER_STACKEDBOX_H

#include "shared_global_p.h"

#include <QtGui/QStackedWidget>
#include <QtCore/QList>

class QAction;
class QToolButton;

class QDESIGNER_SHARED_EXPORT QDesignerStackedWidget : public QStackedWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentPageName READ currentPageName WRITE setCurrentPageName STORED false DESIGNABLE true)
public:
    QDesignerStackedWidget(QWidget *parent);

    inline QAction *actionPreviousPage() const
    { return m_actionPreviousPage; }

    inline QAction *actionNextPage() const
    { return m_actionNextPage; }

    inline QAction *actionDeletePage() const
    { return m_actionDeletePage; }

    inline QAction *actionInsertPage() const
    { return m_actionInsertPage; }

    inline QAction *actionInsertPageAfter() const
    { return m_actionInsertPageAfter; }

    inline QAction *actionChangePageOrder() const
    { return m_actionChangePageOrder; }

    QString currentPageName() const;
    void setCurrentPageName(const QString &pageName);

public slots:
    void updateButtons();

protected:
    virtual void childEvent(QChildEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void showEvent(QShowEvent *e);
    virtual bool event(QEvent *e);

private slots:
    void prevPage();
    void nextPage();
    void removeCurrentPage();
    void addPage();
    void addPageAfter();
    void changeOrder();
    void slotCurrentChanged(int index);

private:
    QToolButton *prev, *next;
    QAction *m_actionPreviousPage;
    QAction *m_actionNextPage;
    QAction *m_actionDeletePage;
    QAction *m_actionInsertPage;
    QAction *m_actionInsertPageAfter;
    QAction *m_actionChangePageOrder;
};

#endif // QDESIGNER_STACKEDBOX_H
