/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT3TO4_H
#define QT3TO4_H

#include <QString>
#include <QList>
#include <QPair>

class Porting
{
public:
    typedef QPair<QString, QString> Rule;
    typedef QList<Rule> RuleList;

public:
    Porting();

    inline RuleList renamedHeaders() const
    { return m_renamedHeaders; }

    inline RuleList renamedClasses() const
    { return m_renamedClasses; }

    QString renameHeader(const QString &headerName) const;
    QString renameClass(const QString &className) const;
    QString renameEnumerator(const QString &enumName) const;

protected:
    static void readXML(RuleList *renamedHeaders, RuleList *renamedClasses, RuleList *renamedEnums);
    static int findRule(const RuleList &rules, const QString &q3);

private:
    RuleList m_renamedHeaders;
    RuleList m_renamedClasses;
    RuleList m_renamedEnums;
};

#endif // QT3TO4_H
