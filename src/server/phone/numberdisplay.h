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

#ifndef NUMBERDISPLAY_H
#define NUMBERDISPLAY_H

#include <QWidget>
#include <QList>
#include <QString>

class QTimer;
class QAction;
class PhoneMessageBox;
class NumberDisplayMultiTap;

class NumberDisplay : public QWidget
{
    Q_OBJECT
public:
    NumberDisplay( QWidget *parent );
    void appendNumber( const QString &numbers, bool speedDial = true );
    void setNumber( const QString &number );
    QString number() const;

public slots:
    void backspace();

    QSize sizeHint() const;
protected slots:
    void addPhoneNumberToContact();
    void enableAction( const QString &n );
    void sendMessage();
    void composeTap(const QChar &);
    void completeTap(const QChar &);

protected:
    void keyPressEvent( QKeyEvent *e );

    void checkForStartTap();

    void setText( const QString &txt );
    QString text() const;

    void paintEvent( QPaintEvent *e );
    void keyReleaseEvent( QKeyEvent *e );
    void timerEvent( QTimerEvent *e );

signals:
    void numberChanged(const QString&);
    void numberSelected(const QString &);
    void hangupActivated();
    void speedDialed(const QString&);

private:

    int mLargestCharWidth;
    QList<int> mFontSizes;
    QString mNumber;

    QAction *mNewAC;
    QAction *mSendMessage;
    int tid_speeddial;
    PhoneMessageBox *addContactMsg;
    NumberDisplayMultiTap *tap;
    QChar composeKey;
    bool delayEmitNumberChanged;
};

#endif
