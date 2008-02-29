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

#ifndef QPREFIXMATCHER_P_H
#define QPREFIXMATCHER_P_H

#include <qobject.h>
#include <qset.h>

class QPrefixMatcherNode;

class QPrefixMatcher : public QObject
{
    Q_OBJECT
public:
    explicit QPrefixMatcher( QObject *parent );
    ~QPrefixMatcher();

    enum Type
    {
        Unknown,
        OKTerminator,
        Terminator,
        FuzzyTerminator,
        TerminatorOrNotification,
        Ignore,
        Notification,
        NotificationWithPDU,
        CommandEcho
    };

    void add( const QString& prefix, QPrefixMatcher::Type type,
              bool mayBeCommand = false, QObject *target = 0,
              const char *slot = 0 );
    QPrefixMatcher::Type lookup
        ( const QString& value, const QString& command ) const;

private slots:
    void targetDestroyed();

private:
    QSet<QObject *> targets;
    QPrefixMatcherNode *root;
};


#endif /* QPREFIXMATCHER_P_H */
