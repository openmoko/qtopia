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
#ifndef MINESWEEP_H
#define MINESWEEP_H

#include <qmainwindow.h>
#include <qdatetime.h>
#include <qpixmap.h>

class MineField;
class QLCDNumber;
class QPushButton;
class QMenu;
class QScrollArea;
class QHBoxLayout;

class MineSweep : public QMainWindow
{
    Q_OBJECT
public:
    MineSweep( QWidget* parent = 0, Qt::WFlags f = 0 );
    ~MineSweep();

public slots:
    void gameOver( bool won );
    void newGame();
    void showPoint(QPoint);

protected slots:
    void setCounter( int );
    void updateTime();

    void beginner();
    void advanced();
    void expert();
    void closeEvent(QCloseEvent *);
    void resizeEvent(QResizeEvent*);

private slots:
    void startPlaying();

protected:
    QScrollArea *scroll;
    QWidget *layoutHolder;
    QHBoxLayout *layout;
    MineField*  field;

private:
    void readConfig();
    void writeConfig() const;

    void newGame(int);
#ifndef QTOPIA_PHONE
    QLCDNumber* guessLCD;
    QLCDNumber* timeLCD;
    QPushButton* newGameButton;

    QDateTime starttime;
    QTimer* timer;

    QPixmap mNewPM, mHappyPM, mDeadPM, mWorriedPM;
#endif
};

#endif // MINESWEEP_H

