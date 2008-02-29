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

#ifndef QDESIGNER_FORMWINDOW_H
#define QDESIGNER_FORMWINDOW_H

#include <QtCore/QPointer>
#include <QtGui/QMainWindow>

class QDesignerWorkbench;
class QDesignerFormWindowInterface;

class QDesignerFormWindow: public QMainWindow
{
    Q_OBJECT
public:
    QDesignerFormWindow(QDesignerFormWindowInterface *formWindow, QDesignerWorkbench *workbench,
                        QWidget *parent = 0, Qt::WindowFlags flags = 0);

    virtual ~QDesignerFormWindow();

    QAction *action() const;
    QDesignerWorkbench *workbench() const;
    QDesignerFormWindowInterface *editor() const;

    virtual QRect geometryHint() const;

public slots:
    void updateWindowTitle(const QString &fileName);
    void updateChanged();
    void geometryChanged();

protected:
    virtual void changeEvent(QEvent *e);
    virtual void closeEvent(QCloseEvent *ev);
    virtual void resizeEvent(QResizeEvent* rev);

private:
    QDesignerFormWindowInterface *m_editor;
    QPointer<QDesignerWorkbench> m_workbench;
    QAction *m_action;
    bool initialized;
};

#endif // QDESIGNER_FORMWINDOW_H
