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

#ifndef _PRED_H_
#define _PRED_H_

#include <QList>
#include <QHash>
#include <QPoint>
#include <QString>
#include <QStringList>

class Word;
class WPWord;
template<class T> class DawgReduction;
class WordPredict
{
public:
    struct Config 
    {
        int reallyNoMoveSensitivity;
        int moveSensitivity;
        int excludeDistance;
    };

    WordPredict(const Config &, int max, bool includePrefix = false);
    ~WordPredict();

    void setLetter(char, const QPoint &);

    void addLetter(char);
    void addTouch(const QPoint &);

    QString prefixedWord() const;
    QStringList words() const;
    QList<qreal> ratings() const;
    QString word() const;

    void reset();
    QList<QPoint> points() const { return m_points; }

    QString movementDesc() const;
private:
    int distanceForPoint(const QPoint &pos, char c);
    qreal weightForWord(const Word &word);
    qreal incrWeightForWord(WPWord *w);

    void updateWords();
    QStringList m_words;
    QList<qreal> m_weights;
    QString m_word;

    enum Movement { None = 0x0000,
                    ReallyNone = 0x0010,
                    Up = 0x0001,
                    Down = 0x0002,
                    Left = 0x0004,
                    Right = 0x0008,
                    Perfect = 0x0010 };
    Movement movement(const QPoint &, const QPoint &);
    
    QList<Movement> m_mPoints;
    QList<QPoint> m_points;
    QPoint m_layout[256];
    int m_latestDfp[256];
    int m_max;
    QString m_prefixedWord;

    DawgReduction<WPWord> *reduction;

    bool m_includePre;
    Config m_config;
};

#endif // _PRED_H_
