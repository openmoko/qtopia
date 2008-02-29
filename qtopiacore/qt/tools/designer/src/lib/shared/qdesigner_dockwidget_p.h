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

#ifndef QDESIGNER_DOCKWIDGET_H
#define QDESIGNER_DOCKWIDGET_H

#include "shared_global_p.h"
#include <QtGui/QDockWidget>

class QDesignerFormWindowInterface;

class QDESIGNER_SHARED_EXPORT QDesignerDockWidget: public QDockWidget
{
    Q_OBJECT
    Q_PROPERTY(Qt::DockWidgetArea dockWidgetArea READ dockWidgetArea WRITE setDockWidgetArea DESIGNABLE docked STORED false)
    Q_PROPERTY(bool docked READ docked WRITE setDocked DESIGNABLE inMainWindow STORED false)
public:
    QDesignerDockWidget(QWidget *parent = 0);
    virtual ~QDesignerDockWidget();

    bool docked() const;
    void setDocked(bool b);

    Qt::DockWidgetArea dockWidgetArea() const;
    void setDockWidgetArea(Qt::DockWidgetArea dockWidgetArea);

    bool inMainWindow() const;

private:
    QDesignerFormWindowInterface *formWindow() const;
    QMainWindow *findMainWindow() const;
};

#endif // QDESIGNER_DOCKWIDGET_H
