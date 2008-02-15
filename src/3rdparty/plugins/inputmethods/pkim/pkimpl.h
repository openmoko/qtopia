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

#include <inputmethodinterface.h>
#include <QIcon>

class PkIM;
class QPixmap;
class QLabel;
class QUuid;
class InputMatcherGuessList;

class PkImpl : public QtopiaInputMethod
{

public:
    PkImpl(QObject *parent = 0);
    virtual ~PkImpl();

#ifndef QT_NO_COMPONENT
//    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
//    Q_REFCOUNT
#endif
    QString name() const;
    QString identifier() const;
    QString version() const;
    QIcon icon() const;

    virtual void reset();
    State state() const;

    virtual QWSInputMethod *inputModifier( );

    virtual QWidget *statusWidget( QWidget *parent );

    virtual int properties() const;

    // hint done by qcop
    virtual void setHint(const QString &, bool);
    bool restrictedToHint() const;


private:
    PkIM *input;
    mutable QIcon icn;
    QLabel *statWid;
    ulong ref;
};

#endif
