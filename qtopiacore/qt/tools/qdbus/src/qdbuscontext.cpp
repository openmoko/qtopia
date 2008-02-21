/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.1, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdbusmessage.h"
#include "qdbusconnection.h"
#include "qdbusabstractadaptor.h"

#include "qdbuscontext.h"
#include "qdbuscontext_p.h"

QDBusContextPrivate *QDBusContextPrivate::set(QObject *obj, QDBusContextPrivate *newContext)
{
    // determine if this is an adaptor or not
    if (qobject_cast<QDBusAbstractAdaptor *>(obj))
        obj = obj->parent();

    Q_ASSERT(obj);

    void *ptr = obj->qt_metacast("QDBusContext");
    QDBusContext *q_ptr = reinterpret_cast<QDBusContext *>(ptr);
    if (q_ptr) {
        QDBusContextPrivate *old = q_ptr->d_ptr;
        q_ptr->d_ptr = newContext;
        return old;
    }

    return 0;
}

/*!
    \since 4.3
    \class QDBusContext
    \inmodule QtDBus

    \brief The QDBusContext class allows slots to determine the D-Bus context of the calls.

    When a slot is called in an object due to a signal delivery or due
    to a remote method call, it is sometimes necessary to know the
    context in which that happened. In particular, if the slot
    determines that it wants to send the reply at a later opportunity
    or if it wants to reply with an error, the context is needed.

    The QDBusContext class is an alternative to accessing the context
    that doesn't involve modifying the code generated by the \l
    {QtDBus XML Compiler (qdbusxml2cpp)}.

    QDBusContext is used by subclassing it from the objects being
    exported using QDBusConnection::registerObject(). The following
    example illustrates the usage:

    \code
    class MyObject: public QObject,
                    protected QDBusContext
    {
        Q_OBJECT
        QDBusConnection conn;
        QDBusMessage msg;

    ...

    protected Q_SLOTS:
        void process();
    public Q_SLOTS:
        void methodWithError();
        QString methodWithDelayedReply();
    };

    void MyObject::methodWithError()
    {
        sendErrorReply(QDBusError::NotSupported,
                       "The method call 'methodWithError()' is not supported");
    }

    QString MyObject::methodWithDelayedReply()
    {
        conn = connection();
        msg = message();
        setDelayedReply(true);
        QMetaObject::invokeMethod(this, "process", Qt::QueuedConnection);
        return QString();
    }
    \endcode

    The example illustrates the two typical uses, that of sending
    error replies and that of delayed replies.

    Note: do not subclass QDBusContext and QDBusAbstractAdaptor at the
    same time. QDBusContext should appear in the real object, not the
    adaptor. If it's necessary from the adaptor code to determine the
    context, use a public inheritance and access the functions via
    QObject::parent().
*/

/*!
  Constructs an empty QDBusContext.
 */
QDBusContext::QDBusContext()
    : d_ptr(0)
{
}

/*!
  An empty destructor.
 */
QDBusContext::~QDBusContext()
{
}

/*!
    Returns true if we are processing a D-Bus call. If this function
    returns true, the rest of the functions in this class are
    available.

    Accessing those functions when this function returns false is
    undefined and may lead to crashes.
*/
bool QDBusContext::calledFromDBus() const
{
    return d_ptr;
}

/*!
    Returns the connection from which this call was received.
*/
QDBusConnection QDBusContext::connection() const
{
    return d_ptr->connection;
}

/*!
    Returns the message that generated this call.
*/
const QDBusMessage &QDBusContext::message() const
{
    return d_ptr->message;
}

/*!
    Returns true if this call will have a delayed reply.

    \sa setDelayedReply()
*/
bool QDBusContext::isDelayedReply() const
{
    return message().isDelayedReply();
}

/*!
    Sets whether this call will have a delayed reply or not.

    If \a enable is false, QtDBus will automatically generate a reply
    back to the caller, if needed, as soon as the called slot returns.

    If \a enable is true, QtDBus will not generate automatic
    replies. It will also ignore the return value from the slot and
    any output parameters. Instead, the called object is responsible
    for storing the incoming message and send a reply or error at a
    later time.

    Failing to send a reply will result in an automatic timeout error
    being generated by D-Bus.
*/
void QDBusContext::setDelayedReply(bool enable) const
{
    message().setDelayedReply(enable);
}

/*!
    Sends an error \a name as a reply to the caller. The optional \a
    msg parameter is a human-readable text explaining the failure.

    If an error is sent, the return value and any output parameters
    from the called slot will be ignored by QtDBus.
*/
void QDBusContext::sendErrorReply(const QString &name, const QString &msg) const
{
    setDelayedReply(true);
    connection().send(message().createErrorReply(name, msg));
}

/*!
    \overload
    Sends an error \a type as a reply to the caller. The optional \a
    msg parameter is a human-readable text explaining the failure.

    If an error is sent, the return value and any output parameters
    from the called slot will be ignored by QtDBus.
*/
void QDBusContext::sendErrorReply(QDBusError::ErrorType type, const QString &msg) const
{
    setDelayedReply(true);
    connection().send(message().createErrorReply(type, msg));
}


