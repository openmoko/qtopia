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

#include "qdateparser_p.h"
#include <qtopialog.h>
#include <QLocale>

//partial from qdatetime.cpp, full wouldn't hurt, cut down for readability
static bool isSpecial(const QChar &c)
{
    switch (c.cell()) {
        case 'd': case 'M': case 'y':
        case '\'': return true;
        default: return false;
    }
}

// from qdatetime.cpp
static int countRepeat(const QString &str, int index)
{
    Q_ASSERT(index >= 0 && index < str.size());
    int count = 1;
    const QChar ch = str.at(index);
    while (index + count < str.size() && str.at(index + count) == ch)
        ++count;
    return count;
}

// from qdatetime.cpp
static QString unquote(const QString &str)
{
    const QLatin1Char quote('\'');
    const QLatin1Char slash('\\');
    const QLatin1Char zero('0');
    QString ret;
    QChar status = zero;
    for (int i=0; i<str.size(); ++i) {
        if (str.at(i) == quote) {
            if (status != quote) {
                status = quote;
            } else if (!ret.isEmpty() && str.at(i - 1) == slash) {
                ret[ret.size() - 1] = quote;
            } else {
                status = zero;
            }
        } else {
            ret += str.at(i);
        }
    }
    return ret;
}

QtopiaDateParser::QtopiaDateParser()
    : mState(QValidator::Invalid), mSection(NoSection), mStart(-1), mEnd(-1), mCursor(-1)
{
    mDefault = QDate::currentDate();
    setFormat(QLocale::system().dateFormat(QLocale::ShortFormat));
}

void QtopiaDateParser::setFormat(const QString &format)
{
    if (format == displayFormat)
        return;

    QString newFormat = format;

    // stolen from QDateTime, trimmed for date only, modified for data that concerns this
    // class
    const QLatin1Char quote('\'');
    const QLatin1Char slash('\\');
    const QLatin1Char zero('0');

    QList<SectionNode> newSectionNodes;
    QStringList newSeparators;
    int i, index = 0;
    int add = 0;
    QChar status = zero;
    int newSections = NoSection;
    for (i = 0; i<newFormat.size(); ++i) {
        if (newFormat.at(i) == quote) {
            ++add;
            if (status != quote) {
                status = quote;
            } else if (newFormat.at(i - 1) != slash) {
                status = zero;
            }
        } else if (i < newFormat.size() && status != quote) {
            const int repeat = qMin(4, countRepeat(newFormat, i));
            if (isSpecial(newFormat.at(i))) {
                const char sect = newFormat.at(i).toLatin1();
                switch (sect) {
                case 'y':
                    if (repeat >= 2) {
                        const bool four = repeat >= 4;
                        const SectionNode sn = { YearSection, i - add, four ? 4 : 2 };
                        newSectionNodes << sn;
                        newSeparators << unquote(newFormat.mid(index, i - index));
                        i += sn.count - 1;
                        index = i + 1;
                        newSections |= YearSection;
                    }
                    break;
                case 'M': {
                    const SectionNode sn = { MonthSection, i - add, repeat };
                    newSectionNodes << sn;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    i += sn.count - 1;
                    index = i + 1;
                    newSections |= MonthSection;
                    break; }
                case 'd': {
                    const SectionNode sn = { DaySection, i - add, repeat };
                    newSectionNodes << sn;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    i += sn.count - 1;
                    index = i + 1;
                    if (repeat == 1 || repeat == 2) 
                        newSections |= DaySection; // only interested in sections we can edit.
                    break; }
                default:
                    break;
                }
            }
        }
    }
    newSeparators << (index < newFormat.size() ? unquote(newFormat.mid(index)) : QString());

    // only store if valid.
    if (newSections == DateSectionMask) {
        displayFormat = newFormat;
        separators = newSeparators;
        sectionNodes = newSectionNodes;
    }
}

QtopiaDateParser::~QtopiaDateParser()
{
}

void QtopiaDateParser::setDefaultDate(const QDate &date)
{
    if (date.isValid())
        mDefault = date;
}

void QtopiaDateParser::setInputText(const QString &text)
{
    // parse text.
    int y, m, d;
    y = m = d = -1;
    QDate bestGuess = mDefault;
    int pos = 0;
    int processed = 0;
    // later handle optional typing of separator text
    mText.clear();
    int sepPos = 0;
    bool parsable = true;
    qLog(Support) << "set inputText - bestGuess" << bestGuess;
    foreach(SectionNode sn, sectionNodes) {
        mText += separators[sepPos++];
        if (pos < text.length() && parsable) {
            if (sn.type == DaySection && sn.count <= 2) {
                int day = parseDay(text, pos, processed);
                if (processed > 0) {
                    qLog(Support) << "parse day" << day << processed;
                    if (day != 0) {
                        d = day;
                        if (bestGuess.daysInMonth() < d) {
                            // at most will have to switch one month, parse day already restricts to 31)
                            // december is 31, so won't switch years.
                            if (m == -1) {
                                bestGuess = bestGuess.addMonths(1);
                            } else if (y == -1 && m == 2 && d <= 29)
                                while(!QDate::isLeapYear(bestGuess.year()))
                                    bestGuess = bestGuess.addYears(1);
                            else
                                d = bestGuess.daysInMonth();
                        }
                        bestGuess.setDate(bestGuess.year(), bestGuess.month(), d);
                        mSection = sn.type;
                        qLog(Support) << "parse day - bestGuess" << bestGuess;
                    }
                    pos += processed;
                    if (pos == text.length()) {
                        // may be intermediate day
                        mStart = mText.length();
                        mText.append(text.mid(pos-processed, processed));
                        mEnd = mText.length();
                        mCursor = mEnd;
                        continue; // else, fall through non-parsed formatting;
                    }
                } else {
                    parsable = false;
                }
            } else if (sn.type == MonthSection) {
                int month = parseMonth(text, pos, processed);
                if (processed > 0) {
                    qLog(Support) << "parse month" << month << processed;
                    if (month != 0) {
                        m = month;
                        // need to make sure month won't invalidate days.
                        int oldday = bestGuess.day();
                        int oldmonth = bestGuess.month();
                        bestGuess.setDate(bestGuess.year(), m, 1);
                        if (bestGuess.daysInMonth() >= oldday) {
                            bestGuess.setDate(bestGuess.year(), m, oldday);
                        } else {
                            if (d == -1) {
                                bestGuess.setDate(bestGuess.year(), m, bestGuess.daysInMonth());
                            } else {
                                // day given first, reject month if have to;
                                // may be able ot shift the year.
                                if (y == -1 && m == 2 && d <= 29) {
                                    while(!QDate::isLeapYear(bestGuess.year()))
                                        bestGuess = bestGuess.addYears(1);
                                } else {
                                    m = oldmonth;
                                }
                                bestGuess.setDate(bestGuess.year(), m, d);
                            }
                        }
                        mSection = sn.type;
                        qLog(Support) << "parse month - bestGuess" << bestGuess;
                    }
                    pos += processed;
                    if (pos == text.length()) {
                        mStart = mText.length();
                        mText.append(text.mid(pos-processed, processed));
                        mEnd = mText.length();
                        mCursor = mEnd;
                        continue;
                    }
                } else {
                    parsable = false;
                }
            } else if (sn.type == YearSection) {
                int year = parseYear(text, pos, processed);
                if (processed > 0) {
                    qLog(Support) << "year before munging" << year;
                    if (year < 100)
                        year = bestGuess.year() - (bestGuess.year()%100) + year;
                    else if (year < 1000)
                        year = bestGuess.year() - (bestGuess.year()%1000) + year;
                    y = year;
                    qLog(Support) << "parse year" << year << processed;
                    // again, need to check month number and leap years.
                    int oldday = bestGuess.day();
                    int oldmonth = bestGuess.month();
                    int oldyear = bestGuess.year();
                    bestGuess.setDate(y, oldmonth, 1);
                    if (bestGuess.daysInMonth() < oldday) {
                        // may have to modify month or day;
                        if (m == -1) {
                            // shift forward a month;
                            // december is 31 so won't switch years.
                            bestGuess = bestGuess.addMonths(1);
                            bestGuess.setDate(y, bestGuess.month(), bestGuess.daysInMonth());
                        } else if (d == -1) {
                            bestGuess.setDate(y, oldmonth, bestGuess.daysInMonth());
                        } else {
                            y = oldyear;
                            bestGuess.setDate(oldyear, oldmonth, oldday);
                        }
                    } else {
                        bestGuess.setDate(y, oldmonth, oldday);
                    }
                    mSection = sn.type;
                    qLog(Support) << "parse year - bestGuess" << bestGuess;
                    pos += processed;
                    if (pos == text.length()) {
                        mStart = mText.length();
                        mText.append(text.mid(pos-processed, processed));
                        mEnd = mText.length();
                        mCursor = mEnd;
                        continue;
                    }
                } else {
                    parsable = false;
                }
            }
        }
        // not parsable.
        switch (sn.type) {
            case DaySection:
                switch(sn.count) {
                    case 1:
                        mText.append(QString::number(bestGuess.day()));
                        break;
                    case 2:
                        mText.append(QString::number(bestGuess.day()).rightJustified(2, '0'));
                        break;
                    case 3:
                        mText.append(QDate::shortDayName(bestGuess.dayOfWeek()));
                        break;
                    default:
                        mText.append(QDate::longDayName(bestGuess.dayOfWeek()));
                        break;

                }
                break;
            case MonthSection:
                switch(sn.count) {
                    case 1:
                        mText.append(QString::number(bestGuess.month()));
                        break;
                    case 2:
                        mText.append(QString::number(bestGuess.month()).rightJustified(2, '0'));
                        break;
                    case 3:
                        mText.append(QDate::shortMonthName(bestGuess.month()));
                        break;
                    default:
                        mText.append(QDate::longMonthName(bestGuess.month()));
                        break;

                }
                break;
            case YearSection:
                mText.append(QString::number(bestGuess.year()));
                break;
            default:
                break;
        }
        mSection = NoSection;
    }
    mText += separators[sepPos];
    mResult = bestGuess;
    if (pos < text.length())
        mState = QValidator::Invalid;
    else if (d == -1 || m == -1 || y == -1)
        mState = QValidator::Intermediate;
    else
        mState = QValidator::Acceptable;
}

int QtopiaDateParser::parseDay(const QString &text, int pos, int &processed)
{
    int result = 0;
    int i = pos;
    int len = text.length();
    if (i < len && text[i].isDigit()) {
        int firstDigit = text[i++].digitValue();
        result = firstDigit;
        // days are one or two digits.
        if (i < len && text[i].isDigit()) {
            result *= 10;
            result += text[i++].digitValue();
            if (result > 31) {
                i--;
                result = firstDigit;
            }
        }
    }
    processed = i-pos;
    return result;
}

int QtopiaDateParser::parseMonth(const QString &text, int pos, int &processed)
{
    // month is either two digits, or the start of a month name.
    int result = 0;
    int i = pos;
    int len = text.length();
    if (i < len && text[i].isDigit()) {
        int firstDigit = text[i++].digitValue();
        result = firstDigit;
        // days are one or two digits.
        if (i < len && text[i].isDigit()) {
            result *= 10;
            result += text[i++].digitValue();
            if (result > 12) {
                i--;
                result = firstDigit;
            }
        }
    } else {
        // start of a month name.
        // since month names are the only text, be greedy.  best match is longest match.
        int bestMonth = 0;
        int bestMonthLength = 0;
        for (int m = 1; m <= 12; m++) {
            // does more tolowers than is needed, but simpler algorithm
            QString mName;
            int mLength, maxLength;

            mLength = 0;
            mName = QDate::longMonthName(m).toLower();
            maxLength = qMin(text.length() - pos - i, mName.length());
            while (mLength < maxLength && text[i+mLength].toLower() == mName[mLength])
                mLength++;
            if (mLength > bestMonthLength) {
                bestMonth = m;
                bestMonthLength = mLength;
            }
            // Short month name might have different letter order.
            mLength = 0;
            mName = QDate::shortMonthName(m).toLower();
            maxLength = qMin(text.length() - pos - i, mName.length());
            while (mLength < maxLength && text[i+mLength].toLower() == mName[mLength])
                mLength++;
            if (mLength > bestMonthLength) {
                bestMonth = m;
                bestMonthLength = mLength;
            }
        }
        i += bestMonthLength;
        result = bestMonth;
    }
    processed = i-pos;
    return result;
}

int QtopiaDateParser::parseYear(const QString &text, int pos, int &processed)
{
    int result = 0;
    int i = pos;
    int len = text.length();
    while (i < len && text[i].isDigit() && i-pos < 4) {
        int digit = text[i++].digitValue();
        result *= 10;
        result += digit;
    }
    processed = i-pos;
    return result;
}
