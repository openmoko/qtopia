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

#ifndef COMPOSEIMPL_H
#define COMPOSEIMPL_H

#include <qpe/inputmethodinterface.h>
class ComposeIM;
class QPixmap;

class ComposeImpl : public ExtInputMethodInterface
{

public:
    ComposeImpl();
    virtual ~ComposeImpl();

#ifndef QT_NO_COMPONENT
    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT
#endif
    virtual QString name();
    virtual QPixmap *icon();

    virtual void resetState();

    virtual QStringList compatible();

    virtual QWSInputMethod *inputMethod( );

    virtual QWidget *statusWidget( QWidget *parent, Qt::WFlags f);
    virtual QWidget *keyboardWidget( QWidget *parent, Qt::WFlags f);

    virtual void qcopReceive( const QCString &msg, const QByteArray &data );

private:
    ComposeIM *input;
    QPixmap *icn;
    QWidget *statWid;
    ulong ref;
};

#endif
