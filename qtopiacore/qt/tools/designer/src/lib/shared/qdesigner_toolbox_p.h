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

#ifndef QDESIGNER_TOOLBOX_H
#define QDESIGNER_TOOLBOX_H

#include "shared_global_p.h"

#include <QToolBox>

class QAction;

class QDESIGNER_SHARED_EXPORT QDesignerToolBox : public QToolBox
{
    Q_OBJECT
    Q_PROPERTY(QString currentItemText READ currentItemText WRITE setCurrentItemText STORED false DESIGNABLE true)
    Q_PROPERTY(QString currentItemName READ currentItemName WRITE setCurrentItemName STORED false DESIGNABLE true)
    Q_PROPERTY(QIcon currentItemIcon READ currentItemIcon WRITE setCurrentItemIcon STORED false DESIGNABLE true)
    Q_PROPERTY(QString currentItemToolTip READ currentItemToolTip WRITE setCurrentItemToolTip STORED false DESIGNABLE true)
public:
    QDesignerToolBox(QWidget *parent = 0);

    QString currentItemText() const;
    void setCurrentItemText(const QString &itemText);

    QString currentItemName() const;
    void setCurrentItemName(const QString &itemName);

    QIcon currentItemIcon() const;
    void setCurrentItemIcon(const QIcon &itemIcon);

    QString currentItemToolTip() const;
    void setCurrentItemToolTip(const QString &itemToolTip);

    QPalette::ColorRole currentItemBackgroundRole() const;
    void setCurrentItemBackgroundRole(QPalette::ColorRole role);

    inline QAction *actionDeletePage() const
    { return m_actionDeletePage; }

    inline QAction *actionInsertPage() const
    { return m_actionInsertPage; }

    inline QAction *actionInsertPageAfter() const
    { return m_actionInsertPageAfter; }

    inline QAction *actionChangePageOrder() const
    { return m_actionChangePageOrder; }

private slots:
    void removeCurrentPage();
    void addPage();
    void addPageAfter();
    void changeOrder();
    void slotCurrentChanged(int index);

protected:
    void itemInserted(int index);

private:
    QAction *m_actionDeletePage;
    QAction *m_actionInsertPage;
    QAction *m_actionInsertPageAfter;
    QAction *m_actionChangePageOrder;
};

#endif // QDESIGNER_TOOLBOX_H
