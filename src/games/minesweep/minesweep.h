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
#ifndef MINESWEEP_H
#define MINESWEEP_H

#include <QMainWindow>

class MineField;
class QScrollArea;
class QHBoxLayout;
class QEvent;
class ResultIndicator;

class MineSweep : public QMainWindow
{
    Q_OBJECT
public:
    MineSweep( QWidget* parent = 0, Qt::WFlags f = 0 );
    ~MineSweep();

public slots:
    void gameOver( bool won );
    void newGame();
    void showPoint(int x, int y);

protected slots:
    void beginner();
    void advanced();
    void expert();
    void closeEvent(QCloseEvent *);
    void resizeEvent(QResizeEvent*);
    bool eventFilter(QObject *obj, QEvent *event);

protected:
    QScrollArea *scroll;
    MineField* field;
    ResultIndicator *resultIndicator;

private:
    void readConfig();
    void writeConfig() const;
    void newGame(int);
};

#endif // MINESWEEP_H

