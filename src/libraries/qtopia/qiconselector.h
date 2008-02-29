/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef HAVE_ICONSELECT_H
#define HAVE_ICONSELECT_H

#include <qtopiaglobal.h>
#include <QToolButton>

class QIconSelectorPrivate;
class QTOPIA_EXPORT QIconSelector : public QToolButton
{
    Q_OBJECT
public:
    explicit QIconSelector( QWidget *parent = 0 );
    explicit QIconSelector( const QIcon &icn, QWidget *parent = 0 );
    ~QIconSelector();

    uint count();
    void insertItem( const QIcon &icn, const QString &text = QString() );
    void removeIndex( int index );
    void clear();
    int currentIndex() const;

    QIcon icon() const;
    void setIcon( const QIcon &icn );
    QSize sizeHint() const;

signals:
    void activated(int);

public slots:
    void setCurrentIndex( int index );

protected slots:
    void popup();
    void popdown();

protected:
    void keyPressEvent( QKeyEvent *e );
    void itemSelected( int index );
    virtual bool eventFilter( QObject *obj, QEvent *e );

private:
    void init();
    QIconSelectorPrivate *d;
};

#endif
