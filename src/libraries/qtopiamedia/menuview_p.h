/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MENUVIEW_H
#define MENUVIEW_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/menumodel_p.h>

#include <QtGui>

class MenuViewStyle;

class QTOPIAMEDIA_EXPORT MenuView : public QListView
{
    Q_OBJECT
public:
    explicit MenuView( QWidget* parent = 0 );
    ~MenuView();

signals:
    void selected( const QModelIndex& index );
    void held( const QModelIndex& index );

private slots:
    void emitHeld();

protected:
    void keyPressEvent( QKeyEvent* e );

    void mousePressEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );

private:
    MenuViewStyle *m_style;

    QTimer *m_holdtimer;
    QMouseEvent m_eventcache;
};

class QTOPIAMEDIA_EXPORT MenuStack
{
public:
    explicit MenuStack( MenuView* view )
        : m_view( view )
    { }

    MenuModel* top() const { return m_stack.top(); }

    void push( MenuModel* model );
    MenuModel* pop();

private:
    MenuView* m_view;
    QStack<MenuModel*> m_stack;
};

#endif // MENUVIEW_H
