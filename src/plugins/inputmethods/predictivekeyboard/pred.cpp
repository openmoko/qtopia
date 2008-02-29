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

#include "pred.h"
#include <qtopianamespace.h>
#include <QSet>
#include <math.h>
#include <QFile>
#include <QDebug>

#define QTOPIA_INTERNAL_QDAWG_TRIE
#include "qdawg.h"

#define POST_FREQUENCY
#define LOG_FREQUENCY

// #define PRED_DEBUG

template<class T>
class DawgReduction
{
public:
    DawgReduction();
    ~DawgReduction();

    void reset();
    void addPossibleCharacters(const QByteArray &);
    
    struct Word {
        Word() : frequency(0), isWord(false), node(0), isFresh(false), isValid(true) {}
        virtual ~Word() {}
        Word(const Word &o) : word(o.word), frequency(o.frequency), isWord(o.isWord), node(o.node), isFresh(o.isFresh), isValid(o.isValid) {}

        QString word;
        int frequency;
        bool isWord;
        const QDawg::Node *node;

        bool isFresh;
        bool isValid;
    };

    typedef QList<T *> WordList;

    WordList words() const;

private:
    void updateWords(const QByteArray &characters);
    WordList updateWords(const T *, const QByteArray &characters);

    bool m_isEmpty;
    WordList m_words;

};

template<class T>
DawgReduction<T>::DawgReduction()
: m_isEmpty(true)
{
}

template<class T>
DawgReduction<T>::~DawgReduction()
{
    qDeleteAll(m_words);
}

template<class T>
void DawgReduction<T>::reset()
{
    m_words.clear();
    m_isEmpty = true;
}

template<class T>
void DawgReduction<T>::addPossibleCharacters(const QByteArray &chars)
{
    updateWords(chars);
}

template<class T>
void DawgReduction<T>::updateWords(const QByteArray &characters)
{
    if(!m_isEmpty && m_words.isEmpty())
        return;

    WordList total;

    if(m_words.isEmpty()) {
        T base;
        base.node = Qtopia::fixedDawg().root();

        total = updateWords(&base, characters);
    } else {
        for(int ii = 0; ii < m_words.count(); ++ii) {
            if(m_words.at(ii)->isValid)
                total << updateWords(m_words.at(ii), characters);
        }
    }

    qDeleteAll(m_words);
    m_isEmpty = false;
    m_words = total;
}

template<class T>
typename DawgReduction<T>::WordList DawgReduction<T>::updateWords(const T *w, const QByteArray &characters)
{
    WordList rv;
    for(const QDawg::Node *n = w->node; n; n = n->next()) {
        if(characters.contains(n->letter().toLower().toAscii())) {
            T *nw = new T(*w);
            nw->word.append(n->letter());
            nw->frequency = (n->value() & 0x00003FFF);
            nw->isWord = n->isWord();
            nw->node = n->jump();
            nw->isFresh = false;
            rv << nw;
        }
    }
    return rv;
}

template<class T>
typename DawgReduction<T>::WordList DawgReduction<T>::words() const
{
    return m_words;
}

struct Word {
    qreal frequency;
    QString word;
};

class WeightedList
{
public:
    WeightedList(int max)
        : m_max(max), m_minWeight(-1.0f) {}

    void addWord(qreal weight, const QString &word)
    {
        if(m_minWeight == -1.0f || weight < m_minWeight)
            m_minWeight = weight;

        if(m_words.count() < m_max) {
            m_words << word;
            m_weights << weight;
            return;
        }

        int max = 0;
        for(int ii = 1; ii < m_words.count(); ++ii) {
            if(m_weights.at(ii) > m_weights.at(max))
                max = ii;
        }

        if(m_weights.at(max) > weight) {
            m_weights.removeAt(max);
            m_words.removeAt(max);
            m_words << word;
            m_weights << weight;
        }
    }

    QStringList words() const
    {
        return m_words;
    }

    QList<qreal> weights() const
    {
        return m_weights;
    }

    qreal minWeight() const
    {
        return m_minWeight;
    }

private:
    int m_max;
    qreal m_minWeight;
    QStringList m_words;
    QList<qreal> m_weights;
};

class WPWord : public DawgReduction<WPWord>::Word 
{
public:
    WPWord() : weight(0.0f), distweight(0), correspond(1), maxcorrespond(1) {}
    WPWord(const WPWord &o) : DawgReduction<WPWord>::Word(o), 
                              weight(o.weight),
                              distweight(o.distweight),
                              correspond(o.correspond),
                              maxcorrespond(o.maxcorrespond) {}

    // Total weight
    qreal weight;

    // Distance factor
    int distweight;

    // Shape correspond
    int correspond;
    int maxcorrespond;
};

WordPredict::WordPredict(const Config &config, int max, bool includePrefix)
: m_max(max), reduction(0), m_includePre(includePrefix), m_config(config)
{
    reduction = new DawgReduction<WPWord>();
}

WordPredict::~WordPredict()
{
    delete reduction;
    reduction = 0;
}

void WordPredict::setLetter(char c, const QPoint &p)
{
    m_layout[(int)c] = p;
}

WordPredict::Movement 
WordPredict::movement(const QPoint &p1, const QPoint &p2)
{
    Movement move = None;
    if(p2.x() > (p1.x() + m_config.moveSensitivity)) {
        move = (Movement)(move | Right);
    } else if(p2.x() < (p1.x() - m_config.moveSensitivity)) {
        move = (Movement)(move | Left);
    } 

    if(p2.y() > (p1.y() + m_config.moveSensitivity)) {
        move = (Movement)(move | Down);
    } else if(p2.y() < (p1.y() - m_config.moveSensitivity)) {
        move = (Movement)(move | Up);
    } 

    if(::abs(p2.x() - p1.x()) <= m_config.reallyNoMoveSensitivity && 
       ::abs(p2.y() - p1.y()) <= m_config.reallyNoMoveSensitivity) {
        Q_ASSERT(None == move);
        move = ReallyNone;
    }

    return move;
}

void WordPredict::addLetter(char l)
{
    m_points << m_layout[(int)l];
    m_mPoints << Perfect;
    reduction->addPossibleCharacters(&l);
    m_latestDfp[(int)l] = 0;

    updateWords();
}

void WordPredict::addTouch(const QPoint &p)
{
    Movement move = None;
    if(!m_points.isEmpty()) 
        move = movement(m_points.last(), p);

    m_points << p;
    m_mPoints << move;

    QByteArray reduce;
    for(char *alpha = "abcdefghijklmnopqrstuvwxyz"; *alpha; ++alpha) {
        int dist = distanceForPoint(p, *alpha);
        m_latestDfp[(int)*alpha] = dist;
        if(dist != -1)
            reduce.append(*alpha);
    }
    reduction->addPossibleCharacters(reduce);


    updateWords();
}

void WordPredict::reset()
{
    m_points.clear();
    m_mPoints.clear();
    m_words.clear();
    m_weights.clear();
    m_word = QString();
    reduction->reset();
}

bool operator<(const QPair<QString, qreal> &lhs, const QPair<QString, qreal> &rhs)
{
    return lhs.second < rhs.second;
}

void WordPredict::updateWords()
{
    WeightedList list(m_max);

    int bestWord = -1;
    qreal bestWordWeight = -1.0f;

    DawgReduction<WPWord>::WordList words = reduction->words();

    for(int ii = 0; ii < words.count(); ++ii) {
#ifndef PRED_NO_INCREMENTAL_CALCULATIONS
        qreal weight = incrWeightForWord(words.at(ii));

        if(weight == -1) {
            words.at(ii)->isValid = false;
            continue;
        }

        if(words.at(ii)->isWord)
            list.addWord(weight, words.at(ii)->word);

        if(bestWord == -1 || weight < bestWordWeight) {
            bestWord = ii;
            bestWordWeight = weight;
        }
#else
        Word word;
        word.frequency = words.at(ii)->frequency;
        word.word = words.at(ii)->word;

        qreal weight = weightForWord(word);
        words.at(ii)->weight = weight;

        if(weight == -1) {
            words.at(ii)->isValid = false;
            continue;
        }

        if(words.at(ii)->isWord)
            list.addWord(weight, word.word);
        if(bestWord == -1 || weight < bestWordWeight) {
            bestWord = ii;
            bestWordWeight = weight;
        }
#endif
    }

    if(bestWord != -1)
        m_prefixedWord = words.at(bestWord)->word;
    else
        m_prefixedWord = QString();

    qreal baseWordScore = list.minWeight() * 10.0f;
    QStringList l_words = list.words();
    QList<qreal> l_weights = list.weights();
    QList<QPair<QString, qreal> > finalwords;
    for(int ii = 0; ii < l_words.count(); ++ii) 
        if(l_weights.at(ii) <= baseWordScore) 
            finalwords << qMakePair(l_words.at(ii), l_weights.at(ii));
    qSort(finalwords.begin(), finalwords.end());

    m_words = QStringList();
    m_weights = QList<qreal>();
    for(int ii = 0; ii < finalwords.count(); ++ii) {
        m_words << finalwords.at(ii).first;
        m_weights << finalwords.at(ii).second;
    }

    if(!m_words.isEmpty())
        m_word = m_words.first();
    else
        m_word = QString();

#ifdef PRED_DEBUG
    qWarning() << "Top words are" << l_words;
    qWarning() << "Best word is" << m_word;
    qWarning() << "Best words are" << m_words;
    qWarning() << "Best word weights are" << m_weights;
    qWarning() << "Best word freqs are" << l_freq;
    QString xstr;
    QString ystr;
    for(int ii = 0; ii < m_points.count(); ++ii) {
        if(!ii) {
            xstr += "(" + QString::number(m_points.at(ii).x()) + ") ";
            ystr += "(" + QString::number(m_points.at(ii).y()) + ") ";
        } else {
            xstr += QString::number(m_points.at(ii).x() - m_points.at(ii - 1).x()) + " ";
            ystr += QString::number(m_points.at(ii - 1).y() - m_points.at(ii).y()) + " ";
        }
    }
    qWarning() << "X motion" << xstr;
    qWarning() << "Y motion" << ystr;

    qWarning() << "Movement: " << movementDesc();
#endif
}

QString WordPredict::movementDesc() const
{
    QString out;
    for(int ii = 0; ii < m_mPoints.count(); ++ii) {
        Movement move = m_mPoints.at(ii);
        if(move == None) {
            out += "None";
        } else if(move == ReallyNone) {
            out += "ReallyNone";
        } else {
            if(move & Up)
                out += "Up";
            if(move & Down)
                out += "Down";
            if(move & Left)
                out += "Left";
            if(move & Right)
                out += "Right";
            if(move & Perfect)
                out += "Perfect";
        }

        out += " ";
    }

    return out;
}

#define EXCLUDE_DISTANCE 50
int WordPredict::distanceForPoint(const QPoint &pos, char c)
{
    QPoint center = m_layout[(int)c];
    if(center.isNull())
        return -1;

    int x_dist = pos.x() - center.x();
    x_dist = (int)((qreal)(x_dist) / ((qreal)m_config.excludeDistance / (qreal)EXCLUDE_DISTANCE));
    int y_dist = pos.y() - center.y();
    y_dist = (int)((qreal)(y_dist) / ((qreal)m_config.excludeDistance / (qreal)EXCLUDE_DISTANCE));

    if(::abs(x_dist) > EXCLUDE_DISTANCE ||
       ::abs(y_dist) > EXCLUDE_DISTANCE)
        return -1;

    int distance = x_dist * x_dist + y_dist * y_dist;

    return distance;
}

/* 
   This is EXACTLY the same algorithm as weightForWord, but is evaluated 
   incrementally as a performance optimization.
   */
qreal WordPredict::incrWeightForWord(WPWord *w)
{
    if(!w->isFresh) {
        
        w->isFresh = true;
        if(w->weight == -1)
            return w->weight;

        int len = w->word.length();
        char c = w->word.at(len - 1).toLower().toLatin1();
        QPoint cWordPoint = m_layout[(int)c];

        int distweight = m_latestDfp[(int)c];
        if(distweight == -1) {
            w->weight = -1.0f;
            return w->weight;
        }
        w->distweight += distweight;

        QPoint lastWordPoint;
        if(len > 1)
            lastWordPoint = m_layout[(int)(w->word.at(len - 2).toLower().toLatin1())];

        Movement expected = None;
        if(!lastWordPoint.isNull())
            expected = movement(lastWordPoint, cWordPoint);

        Movement seen = m_mPoints.at(len - 1);
        int correspond = 0;
        int maxcorrespond = 0;
        if(seen != Perfect) {
            if(expected & Up) {
                maxcorrespond++;
                if(seen & Up)
                    correspond++;
                if(seen & Down) {
                    w->weight = -1.0f;
                    return w->weight;
                }
            } else if(seen & Up) {
                maxcorrespond++;
            }

            if(expected & Down) {
                maxcorrespond++;
                if(seen & Down)
                    correspond++;
                if(seen & Up) {
                    w->weight = -1.0f;
                    return w->weight;
                }
            } else if(seen & Down) {
                maxcorrespond++;
            }
            if(expected & Left) {
                maxcorrespond++;
                if(seen & Left)
                    correspond++;
                if(seen & Right) {
                    w->weight = -1.0f;
                    return w->weight;
                }
            } else if(seen & Left) {
                maxcorrespond++;
            }
            if(expected & Right) {
                maxcorrespond++;
                if(seen & Right)
                    correspond++;
                if(seen & Left) {
                    w->weight = -1.0f;
                    return w->weight;
                }
            } else if(seen & Right) {
                maxcorrespond++;
            }
        }

        w->correspond += correspond;
        w->maxcorrespond += maxcorrespond;

        qreal correspondMul = 
            30.0f * ((((qreal)w->maxcorrespond / (qreal)w->correspond) - 1.0f) + 1.0f);

#ifdef LOG_FREQUENCY
        qreal logFrequency = ::log(w->frequency) / ::log(10.0f);
        if(logFrequency < 1.0f)
            logFrequency = 1.0f;
        logFrequency = 1.0/logFrequency;
#endif

        w->weight = (qreal)w->distweight * 
#ifndef POST_FREQUENCY
            (1.0f / w->frequency) *
#endif
            correspondMul
#ifdef LOG_FREQUENCY
            * logFrequency
#endif
            ;
    }

    return w->weight;
}

qreal WordPredict::weightForWord(const Word &word)
{
    Q_ASSERT(word.word.length() == m_points.count());

    int weight = 0;

    QList<Movement> wordMovement;

    QPoint lastWordPoint = QPoint();
    for(int ii = 0; ii < m_points.count(); ++ii) {
        char c = word.word.at(ii).toLower().toLatin1();

        if(lastWordPoint.isNull()) {
            wordMovement << None;
            QPoint cwordpoint = m_layout[(int)c];
            lastWordPoint = cwordpoint;
        } else {
            QPoint cwordpoint = m_layout[(int)c];
            wordMovement << movement(lastWordPoint, cwordpoint);
            lastWordPoint = cwordpoint;
        }

        int cweight = distanceForPoint(m_points.at(ii), c);
        if(cweight == -1)
            return -1;

        weight += cweight;
    }

    int correspond = 1;
    int maxcorrespond = 1;
    for(int ii = 0; ii < wordMovement.count(); ++ii) {
        Movement expected = wordMovement.at(ii);
        Movement seen = m_mPoints.at(ii);
        if(seen == Perfect)
            continue;

        if(expected & Up) {
            maxcorrespond++;
            if(seen & Up)
                correspond++;
            if(seen & Down)
                return -1;
        } else if(seen & Up) {
            maxcorrespond++;
        }

        if(expected & Down) {
            maxcorrespond++;
            if(seen & Down)
                correspond++;
            if(seen & Up)
                return -1;
        } else if(seen & Down) {
            maxcorrespond++;
        }
        if(expected & Left) {
            maxcorrespond++;
            if(seen & Left)
                correspond++;
            if(seen & Right)
                return -1;
        } else if(seen & Left) {
            maxcorrespond++;
        }
        if(expected & Right) {
            maxcorrespond++;
            if(seen & Right)
                correspond++;
            if(seen & Left)
                return -1;
        } else if(seen & Right) {
            maxcorrespond++;
        }

    }
    qreal correspondMul = 
        30.0f * ((((qreal)maxcorrespond / (qreal)correspond) - 1.0f) + 1.0f);

#ifdef LOG_FREQUENCY
    qreal logFrequency = ::log(word.frequency) / ::log(10.0f);
    if(logFrequency < 1.0f)
        logFrequency = 1.0f;
    logFrequency = 1.0/logFrequency;
#endif

    return (qreal)weight * 
#ifndef POST_FREQUENCY
           (1.0f / word.frequency) *
#endif
           correspondMul
#ifdef LOG_FREQUENCY
           * logFrequency
#endif
           
           ;
}

QString WordPredict::prefixedWord() const
{
    return m_prefixedWord;
}

QStringList WordPredict::words() const
{
    return m_words;
}

QList<qreal> WordPredict::ratings() const
{
    return m_weights;
}

QString WordPredict::word() const
{
    return m_word;
}
