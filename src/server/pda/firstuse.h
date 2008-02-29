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

#ifndef FIRSTUSE_H
#define FIRSTUSE_H

#include <qdialog.h>
#include <qpixmap.h>
#include <qfont.h>

class InputMethods;
class QPushButton;
class QLabel;

class FirstUse : public QDialog
{
    Q_OBJECT
public:
    FirstUse(QWidget* parent=0, Qt::WFlags=0);
    ~FirstUse();

    bool restartNeeded() const { return needRestart; }
    void reloadLanguages();

private slots:
    void calcMaxWindowRect();
    void nextDialog();
    void previousDialog();
    void switchDialog();

protected:
    void paintEvent( QPaintEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void keyPressEvent( QKeyEvent *e );

private:
    void loadPixmaps();
    void drawText(QPainter &p, const QString &text);
    int findNextDialog(bool forwards);
    void updateButtons();

private:
    QPixmap splash;
    QPixmap buttons;
    int currDlgIdx;
    QDialog *currDlg;
    InputMethods *inputMethods;
    QPushButton *back;
    QPushButton *next;
    int controlHeight;
    QString lang;
    bool needCalibrate;
    QWidget *taskBar;
    QLabel *titleBar;
    bool needRestart;
    QFont defaultFont;
};

#endif

