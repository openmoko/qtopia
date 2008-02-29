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

#ifndef _DATEPARSER_H_
#define _DATEPARSER_H_

#include <QString>
#include <QDate>
#include <QList>
#include <QStringList>
#include <QValidator>

class QtopiaDateParser
{
public:
    QtopiaDateParser();
    ~QtopiaDateParser();

    enum Section {
        NoSection = 0x0000,
        Internal = 0x8000,
        DaySection = 0x0100,
        MonthSection = 0x0200,
        YearSection = 0x0400,
        DateSectionMask = (DaySection|MonthSection|YearSection),
        FirstSection = 0x1000|Internal,
        LastSection = 0x2000|Internal
    }; // duplicated from qdatetimeedit.h

    void setFormat(const QString &);
    QString format() const { return displayFormat; }

    void setDefaultDate(const QDate &);

    void setInputText(const QString &);

    QDate date() const { return mResult; }
    QValidator::State state() const { return mState; }

    QString text() const { return mText; }  // may not be valid date

    Section highlightedSection() const { return mSection; }
    int highlightStart() const { return mStart; }
    int highlightEnd() const { return mEnd; }
    int cursorPosition() const { return mCursor; }

private:
    static int parseDay(const QString &, int pos, int &processed);
    static int parseMonth(const QString &, int pos, int &processed);
    static int parseYear(const QString &, int pos, int &processed);

    struct SectionNode {
        Section type;
        mutable int pos;
        int count;
    };
    QList<SectionNode> sectionNodes;
    QString displayFormat;
    QStringList separators;
    QDate mDefault;
    QDate mResult;

    QString mText;
    QValidator::State mState;
    Section mSection;
    int mStart;
    int mEnd;
    int mCursor;
};

#endif//_DATEPARSER_H_
