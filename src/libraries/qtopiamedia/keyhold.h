/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef KEYHOLD_H
#define KEYHOLD_H

#include <QtCore>
#include <qtopiaglobal.h>

class QTOPIAMEDIA_EXPORT KeyHold : public QObject
{
    Q_OBJECT
public:
    KeyHold( int key, int keyHold, int threshold, QObject* target, QObject* parent = 0 );

    bool eventFilter( QObject* o, QEvent* e );

private slots:
    void generateKeyHoldPress();

private:
    int m_key;
    int m_keyHold;

    int m_threshold;
    QTimer *m_countdown;

    QObject *m_target;
};

#endif // KEYHOLD_H
