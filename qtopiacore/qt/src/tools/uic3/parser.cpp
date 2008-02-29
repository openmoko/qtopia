/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "parser.h"
#include <QObject>
#include <QStringList>

QString Parser::cleanArgs(const QString &func)
{
    QString slot(func);
    int begin = slot.indexOf(QLatin1String("(")) + 1;
    QString args = slot.mid(begin);
    args = args.left(args.indexOf(QLatin1String(")")));
    QStringList lst = args.split(QLatin1Char(','));
    QString res = slot.left(begin);
    for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it) {
        if (it != lst.begin())
            res += QLatin1String(",");
        QString arg = *it;
        int pos = 0;
        if ((pos = arg.indexOf(QLatin1Char('&'))) != -1) {
            arg = arg.left(pos + 1);
        } else if ((pos = arg.indexOf(QLatin1Char('*'))) != -1) {
            arg = arg.left(pos + 1);
        } else {
            arg = arg.simplified();
            if ((pos = arg.indexOf(QLatin1Char(':'))) != -1)
                arg = arg.left(pos).simplified() + QLatin1String(":") + arg.mid(pos + 1).simplified();
            QStringList l = arg.split(QLatin1Char(' '));
            if (l.count() == 2) {
                if (l[0] != QLatin1String("const")
                        && l[0] != QLatin1String("unsigned")
                        && l[0] != QLatin1String("var"))
                    arg = l[0];
            } else if (l.count() == 3) {
                arg = l[0] + QLatin1String(" ") + l[1];
            }
        }
        res += arg;
    }
    res += QLatin1String(")");
    return res;
}
