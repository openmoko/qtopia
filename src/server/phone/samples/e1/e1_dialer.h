/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef e1_DIALER_H
#define e1_DIALER_H

#include <QWidget>
#include <QPixmap>
#include <QColor>
#include <QString>

class E1Button;
class E1Bar;
class QResizeEvent;
class QLineEdit;
class QMoveEvent;
class E1Dialer : public QWidget
{
    Q_OBJECT
public:

    E1Dialer( E1Button *, QWidget* parent = 0, Qt::WFlags f = 0 );

    void setActive();

    void setNumber(const QString &);
signals:
    void sendNumber(const QString &);
    void toCallScreen();

protected slots:
    void historyClicked();
    void eraseClicked();
    void activeCallCount(int);
    void selectCallHistory();
    void buttonClicked( const QString& txt );
    void addPlus();
    void addPause();
    void addWait();
    void numberChanged(const QString&);
    void textButtonClicked();

protected:
    void resizeEvent( QResizeEvent* );

private:
    QLineEdit* m_input;
    E1Button* m_textButton;
    E1Bar* m_bar;
    E1Bar* m_callscreenBar;
    int m_activeCallCount;
};

// declare E1DialerButton
class E1DialerButton : public QWidget
{
    Q_OBJECT
public:
    enum State
    {
        Up,
        Down
    };

    E1DialerButton( QWidget* parent, Qt::WindowFlags f = 0 );

    void setBackgroundEnabled( bool e );
    void setText( const QString& txt );
    void setFgPixmap( const QPixmap& );
    //void setColor( const E1DialerButton::State& st, const QColor& color );

signals:
    void clicked( const QString& ch );

protected:
    void paintEvent( QPaintEvent* e );
    void mousePressEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );
    void resizeEvent( QResizeEvent* e );
    void moveEvent( QMoveEvent* e );

private:
    void generateBgPixmap( const E1DialerButton::State& st );

    E1DialerButton::State m_state;
    QPixmap m_fgPixmap;
    QImage m_bgImage;
    QString m_text;
    QPixmap m_bgForState[2];
    QColor m_colorForState[2];
    bool m_backgroundEnabled;
};


#endif
