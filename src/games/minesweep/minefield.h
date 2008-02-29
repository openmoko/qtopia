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
#ifndef MINEFIELD_H
#define MINEFIELD_H

#include <qframe.h>

class Mine;
class QSettings;

class MineField : public QFrame
{
    Q_OBJECT
public:
    MineField( QWidget* parent = 0 );
    ~MineField();

    enum State { Waiting, Playing, GameOver };

    State state() const { return stat; }

    void readConfig(QSettings&);
    void writeConfig(QSettings&) const;

    int level() const { return lev; }

    void setAvailableRect( const QRect & );

    QSize sizeHint() const;
    QSize minimumSize() const;

public slots:
    void setup( int level );

    void showMines();

signals:
    void gameOver( bool won );
    void gameStarted();
    void mineCount( int );
    void newGameSelected();
    void currentPointChanged(QPoint);
    void debugGeometries();
protected:

    void mousePressEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent* );
    void keyReleaseEvent( QKeyEvent* );
    void paintEvent(QPaintEvent* );

    int getHint( int row, int col );
    void setHint( int r, int c );
    void updateMine( int row, int col );
    void paletteChange( const QPalette & );
    void updateCell( int r, int c );
    bool onBoard( int r, int c ) const { return r >= 0 && r < numRows && c >= 0 && c < numCols; }
    Mine *mine( int row, int col ) { return onBoard(row, col ) ? mines[row+numCols*col] : 0; }
    const Mine *mine( int row, int col ) const { return onBoard(row, col ) ? mines[row+numCols*col] : 0; }

protected slots:
    void cellPressed( int row, int col );
    void cellClicked( int row, int col );
    void held();

private:

    int findCellSize();
    void setCellSize( int );

#ifdef QTOPIA_PHONE
    bool mAlreadyHeld; // true if we've already taken action for holding down on the current key press.
    // TODO: Investigate troubles with select button for pressing vs holding sometimes
#endif
    State stat;
    void setState( State st );
    void placeMines();
    enum FlagAction { NoAction, FlagOn, FlagNext };
    FlagAction flagAction;
    bool ignoreClick;
    int currRow;
    int currCol;
    int numRows, numCols;
    bool pressed;

    int minecount;
    int mineguess;
    int nonminecount;
    int lev;
    QRect availableRect;
    int cellSize;
    QTimer *holdTimer;
    Mine **mines;

    int topMargin;
    int leftMargin;

    int sizeHintX;
    int sizeHintY;

};

#endif // MINEFIELD_H
