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
#include <inputmethodinterface.h>
#include <qobject.h>
class PopupIM;

/*
   When using multiple inheritence, QObject should always be listed first.

   The implementation does not need to inherit Q_OBJECT if the QWSInputMethod
   calls QWSServer::sendKey itself rather than emitting a signal
*/
class PopupIMExtImpl : public QObject, public ExtInputMethodInterface
{
    Q_OBJECT
public:
    PopupIMExtImpl();
    virtual ~PopupIMExtImpl();

#ifndef QT_NO_COMPONENT
    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT
#endif
    QString name();
    QPixmap *icon();

    void resetState() {}

    QStringList compatible() { return QStringList(); }

    QWSInputMethod *inputMethod( ) { return 0; }

    QWidget *statusWidget( QWidget *, Qt::WFlags ) { return 0; }
    QWidget *keyboardWidget( QWidget *, Qt::WFlags );

    void qcopReceive( const QString &, const QByteArray & ) { }

private slots:
    void sendKeyEvent(ushort, ushort, ushort, bool, bool);

private:
    PopupIM *input;
    QPixmap *icn;
    ulong ref;
};

