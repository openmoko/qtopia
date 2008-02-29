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

#if defined(QTOPIA_PHONE) && !defined(THEMEDVIEW_INTERFACE_H)
#define THEMEDVIEW_INTERFACE_H

#include <qfactoryinterface.h>
#include <qtopiaglobal.h>

class QPainter;
class QRect;

struct QTOPIA_EXPORT ThemedItemInterface
{
    virtual void resize(int w, int h) = 0;
    virtual void paint(QPainter *p, const QRect &r) = 0;
    virtual ~ThemedItemInterface();
};

#define ThemedItemInterface_iid "com.trolltech.Qtopia.ThemedItemInterface"
Q_DECLARE_INTERFACE(ThemedItemInterface, ThemedItemInterface_iid)

class QTOPIA_EXPORT ThemedItemPlugin : public QObject, public ThemedItemInterface
{
    Q_OBJECT
    Q_INTERFACES(ThemedItemInterface)
public:
    explicit ThemedItemPlugin( QObject* parent = 0 );
    virtual ~ThemedItemPlugin();
};

#endif
