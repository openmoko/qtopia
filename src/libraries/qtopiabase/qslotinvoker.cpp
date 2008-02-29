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

#include <qslotinvoker.h>
#include <qsignalintercepter.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qvarlengtharray.h>

/*!
    \class QSlotInvoker
    \brief The QSlotInvoker class provides an interface for invoking slots with explicit arguments

    The QSlotInvoker class provides an interface to invoke arbitrary slots
    with explicit metacall arguments, without needing to create and emit an
    explicit signal.

    Methods that are marked with Q_INVOKABLE or Q_SCRIPTABLE can also
    be invoked with this class.

    \sa QSignalIntercepter

    \ingroup objectmodel
*/

class QSlotInvokerPrivate
{
public:
    QObject *receiver;
    QByteArray member;
    int memberIndex;
    bool destroyed;
    int returnType;
    int *types;
    int numArgs;

    ~QSlotInvokerPrivate()
    {
        if ( types )
            qFree( types );
    }
};

/*!
    \internal
    Create a slot invoker that can invoke \a member on \a receiver.
    The object will be attached to \a parent, if present.
*/
QSlotInvoker::QSlotInvoker( QObject *receiver, const QByteArray &member,
                            QObject *parent )
    : QObject( parent )
{
    d = new QSlotInvokerPrivate();
    d->receiver = receiver;
    QByteArray name;
    if ( member.size() > 0 && member[0] >= '0' && member[0] <= '9' ) {
        // Strip off the member type code.
        name = member.mid(1);
    } else {
        name = member;
    }
    name = QMetaObject::normalizedSignature( name.constData() );
    d->member = name;
    d->destroyed = false;
    d->returnType = 0;
    d->types = 0;
    d->numArgs = 0;
    if ( receiver && name.size() > 0 ) {
        d->memberIndex
            = receiver->metaObject()->indexOfMethod( name.constData() );
    } else {
        d->memberIndex = -1;
    }
    if ( d->memberIndex != -1 ) {
        QMetaMethod method = receiver->metaObject()->method
                ( d->memberIndex );
        {
            connect( receiver, SIGNAL(destroyed()),
                     this, SLOT(receiverDestroyed()) );
            d->returnType =
                QSignalIntercepter::typeFromName( method.typeName() );
            d->types = QSignalIntercepter::connectionTypes
                ( name, d->numArgs );
            if ( !( d->types ) )
                d->destroyed = true;
        }
    } else {
        d->destroyed = true;
    }
}

/*!
    \internal
    Destroy a slot invoker.
*/
QSlotInvoker::~QSlotInvoker()
{
    delete d;
}

/*!
    \internal
    Determine if the member is present on the object.
*/
bool QSlotInvoker::memberPresent() const
{
    return ! d->destroyed;
}

/*!
    \internal
    Determine if the member can be invoked with \a numArgs arguments.
    That is, the receiver has not been destroyed, the member is present,
    and it requires \a numArgs or less araguments.
*/
bool QSlotInvoker::canInvoke( int numArgs ) const
{
    if ( d->destroyed )
        return false;
    return ( numArgs >= d->numArgs );
}

/*!
    \internal
    Get the object that will receive slot invocations.
*/
QObject *QSlotInvoker::receiver() const
{
    return d->receiver;
}

/*!
    \internal
    Get the member that will receiver slot invocations.
*/
QByteArray QSlotInvoker::member() const
{
    return d->member;
}

/*!
    \internal
    Get the parameter types associated with this member.
*/
int *QSlotInvoker::parameterTypes() const
{
    return d->types;
}

/*!
    \internal
    Get the number of parameter types associated with this member.
*/
int QSlotInvoker::numParameterTypes() const
{
    return d->numArgs;
}

/*!
    \internal
    Invoke the slot represented by this object with the argument
    list \a args.  The slot's return value is returned from
    this method.  If the slot's return type is "void", then a
    QVariant instance of type QVariant::Invalid will be returned.

    If it is possible that the slot may throw an exception,
    it is the responsibility of the caller to catch and
    handle the exception.
*/
QVariant QSlotInvoker::invoke( const QList<QVariant>& args )
{
    int arg;
    QVariant returnValue;

    // Create a default instance of the return type for the result buffer.
    if ( d->returnType != (int)QVariant::Invalid ) {
        returnValue = QVariant( d->returnType, (const void *)0 );
    }

    // Bail out if the receiver object has already disappeared.
    if ( d->destroyed )
        return returnValue;

    // Check that the number of arguments is compatible with the slot.
    int numArgs = args.size();
    if ( numArgs < d->numArgs ) {
        qWarning( "QSlotInvoker::invoke: insufficient arguments for slot" );
        return returnValue;
    } else if ( numArgs > d->numArgs ) {
        // Drop extraneous arguments.
        numArgs = d->numArgs;
    }

    // Construct the raw argument list.
    QVarLengthArray<void *, 32> a( numArgs + 1 );
    if ( d->returnType == (int)QVariant::Invalid )
        a[0] = 0;
    else
        a[0] = returnValue.data();
    for ( arg = 0; arg < numArgs; ++arg ) {
        if ( d->types[arg] == QSignalIntercepter::QVariantId ) {
            a[arg + 1] = (void *)&( args[arg] );
        } else if ( args[arg].userType() != d->types[arg] ) {
            qWarning( "QSlotInvoker::invoke: argument %d has incorrect type",
                      arg );
            return QVariant();
        } else {
            a[arg + 1] = (void *)( args[arg].data() );
        }
    }

    // Invoke the specified slot.
    d->receiver->qt_metacall( QMetaObject::InvokeMetaMethod,
                                     d->memberIndex, a.data() );
    return returnValue;
}

void QSlotInvoker::receiverDestroyed()
{
    d->destroyed = true;
}
