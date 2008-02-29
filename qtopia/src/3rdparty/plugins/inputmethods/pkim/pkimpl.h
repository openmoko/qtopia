/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/****************************************************************************
** $Id: .emacs,v 1.3 1998/02/20 15:06:53 agulbra Exp $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef PKIMPL_H
#define PKIMPL_H

#include <qtopia/private/inputmethodinterface_p.h>
class PkIM;
class QPixmap;
class QToolButton;

class PkImpl : public CoopInputMethodInterface
{

public:
    PkImpl();
    virtual ~PkImpl();

#ifndef QT_NO_COMPONENT
    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT
#endif
    virtual QString name();
    virtual QPixmap *icon();

    virtual void resetState();
    State state() const;
    void connectStateChanged(QObject *, const char *);
    void disconnectStateChanged(QObject *, const char *);

    virtual QStringList compatible();

    virtual QWSInputMethod *inputModifier( );
    virtual QWidget *inputWidget( QWidget *, Qt::WFlags ) { return 0; }
    virtual QWSGestureMethod *gestureModifier() { return 0; }

    virtual QWidget *statusWidget( QWidget *parent, Qt::WFlags f);
    virtual QWidget *keyboardWidget( QWidget *parent, Qt::WFlags f);

    virtual void qcopReceive( const QCString &msg, const QByteArray &data );

    virtual int properties();

    virtual void appendGuess(const IMIGuessList &);
    virtual void revertGuess() {}
    virtual void function(const QString &);

    // hint done by qcop
    virtual void setHint(const QString &);
    bool restrictToHint() const;
    void setRestrictToHint(bool);

    virtual void connectAppendGuess(QObject *, const char *) {}
    virtual void disconnectAppendGuess(QObject *, const char *) {}
    virtual void connectRevertGuess(QObject *, const char *);
    virtual void disconnectRevertGuess(QObject *, const char *);
    virtual void connectFunction(QObject *, const char *);
    virtual void disconnectFunction(QObject *, const char *);


private:
    PkIM *input;
    QPixmap *icn;
    QToolButton *statWid;
    ulong ref;
};

#endif
