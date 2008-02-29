/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef COLORSELECTOR_H
#define COLORSELECTOR_H

#include <qpushbutton.h>
#include <qdialog.h>

#include <qtopiaglobal.h>

class QColorSelectorPrivate;

class QTOPIA_EXPORT QColorSelector : public QWidget
{
    Q_OBJECT
public:
    explicit QColorSelector( QWidget *parent=0, Qt::WFlags f=0 );
    ~QColorSelector();

    QColor color() const;
    QSize sizeHint() const;

    void setDefaultColor( const QColor & );
    const QColor &defaultColor() const;

public slots:
    void setColor( const QColor &c );

signals:
    void selected( const QColor &c );

protected:
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void keyPressEvent( QKeyEvent * );
    void showEvent( QShowEvent * );
    QRect rectOfColor( int ) const;

private:
    QColorSelectorPrivate *d;
};

class QColorSelectorDialogPrivate;
class QTOPIA_EXPORT QColorSelectorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QColorSelectorDialog( const QColor &c, QWidget *parent=0, Qt::WFlags f = 0 );
    QColorSelectorDialog( QWidget *parent=0, Qt::WFlags f = 0 );

    QColor color() const;

    void setDefaultColor( const QColor & );
    const QColor &defaultColor() const;

    static QColor getColor( const QColor &c = Qt::white, QWidget *parent = 0 );

public slots:
    void setColor( const QColor &c );

signals:
    void selected( const QColor &c );

protected slots:
    void colorSelected( const QColor &c );

private:
    void init();

private:
    QColorSelectorDialogPrivate *d;
};

class QColorButtonPrivate;

class QTOPIA_EXPORT QColorButton : public QPushButton
{
    Q_OBJECT
public:
    explicit QColorButton( QWidget *parent=0 );
    explicit QColorButton( const QColor &c, QWidget *parent=0 );
    ~QColorButton();

    QColor color() const;

    void setDefaultColor( const QColor & );
    const QColor &defaultColor() const;

public slots:
    void setColor( const QColor &c );

signals:
    void selected( const QColor &c );

private slots:
    void colorSelected( const QColor & );
    void showSelector();

protected:
    void paintEvent( QPaintEvent *e );
    void drawButtonLabel( QPainter * );

private:
    void init();

private:
    QColorButtonPrivate *d;
};

#endif
