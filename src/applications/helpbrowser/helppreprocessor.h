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
#ifndef HELPPREPROCESSOR_H
#define HELPPREPROCESSOR_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QStack>

class HelpPreProcessor : public QObject
{
    Q_OBJECT
public:
    HelpPreProcessor( const QString &file, int maxrecurs=5 );

    QString text();

private:
    QString parse(const QString& filename);

    QString mFile;
    int levels;

    QString iconSize;

    QStack<QStringList> tests;
    QStack<bool> inverts;
    QMap<QString,QString> replace;
};

#endif
