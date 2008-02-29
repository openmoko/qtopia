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

#ifndef DELAYEDWAITDIALOG_H
#define DELAYEDWAITDIALOG_H

#include <QDialog>
#include <QList>

class Icon;
class QTimer;
class QLabel;

class DelayedWaitDialog : public QDialog
{
    Q_OBJECT
public:
    DelayedWaitDialog( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~DelayedWaitDialog();

    void setText( const QString &str );
    void setDelay( int ms );

protected:
    virtual void showEvent( QShowEvent *se );
    virtual void hideEvent( QHideEvent *he );
    virtual void keyReleaseEvent( QKeyEvent *ke );
    virtual void timerEvent( QTimerEvent *te );

public slots:
    virtual void show();
    virtual void hide();

protected slots:
    void update();

private:
    QList<Icon*> mIconList;
    QLabel *text;
    static const int NUMBEROFICON = 10;
    QTimer *mTimer;
    int mDelay;
    int mTid;
};

#endif //DELAYEDWAITDIALOG_H
