/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
class PopupIM;
class PopupIMImpl : public InputMethodInterface
{
public:
    PopupIMImpl();
    ~PopupIMImpl();

#ifndef QT_NO_COMPONENT
    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT
#endif

    QWidget *inputMethod( QWidget *parent, Qt::WFlags f );
    QPixmap *icon();
    QString name();
    void onKeyPress( QObject *receiver, const char *slot );

    // empty because no state kept in keyboard.
    void resetState() {}

private:
    PopupIM *input;
    QPixmap *icn;
    ulong ref;
};
