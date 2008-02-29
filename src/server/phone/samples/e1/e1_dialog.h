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

#ifndef _e1_DIALOG_H_
#define _e1_DIALOG_H_

#include <QDialog>
#include <QColor>

class QVBoxLayout;
class E1Bar;
class E1Dialog : public QDialog
{
public:
    enum Type {
        Generic,
        Return,
        NewMessage
    };

    E1Dialog( QWidget* parent, E1Dialog::Type t );

    E1Bar *bar() const;
    void setContentsWidget( QWidget* contentsWidget );

protected:
    virtual void showEvent(QShowEvent *);
    void paintEvent( QPaintEvent* e );
    void resizeEvent( QResizeEvent* e );
    void moveEvent( QMoveEvent* e );

    QColor highlightColor() const;

private:
    QVBoxLayout* m_layout;
    QWidget* m_contentsWidget;
    E1Bar* m_bar;
};

#endif // _e1_DIALOG_H_

