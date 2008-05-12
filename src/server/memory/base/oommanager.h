// -*-C++-*-
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

#ifndef _OOMMANAGER_H_
#define _OOMMANAGER_H_

#include <QObject>
#include <QMap>

class OomPrivate;

class OomManager : public QObject
{
    Q_OBJECT

  public:
    enum Importance { Expendable, Important, Critical };

    OomManager();
    ~OomManager();

    void insert(const QString& app, int pid);
    void remove(const QString& app);

    const QMap<QString,int>& expendableProcs() const;
    const QMap<QString,int>& importantProcs() const;
    bool hasExpendableProcs() const;
    bool hasImportantProcs() const;
    bool isExpendable(const QString& app) const;
    bool isImportant(const QString& app) const;

    QString procWithBiggestScore(Importance t) const;
    void printOomValues(bool score);

  private:
    OomPrivate* d;
};

#endif
