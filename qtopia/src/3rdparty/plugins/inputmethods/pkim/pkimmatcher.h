/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/****************************************************************************
** $Id: .emacs,v 1.3 1998/02/20 15:06:53 agulbra Exp $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef PKIMMATCHER_H
#define PKIMMATCHER_H

#include <qtopia/qdawg.h>
#include <qtopia/private/inputmethodinterface_p.h>
#include <qvaluestack.h>
#include <qmap.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qvector.h>
#include <qtl.h>

class Config;

QChar convertToUpper(const QChar& letter);

enum PkIMFunc {
    noFunction = 0,
    changeShift,
    changeMode,
    modifyText,
    insertSpace,
    insertSymbol,
    insertText
};

struct PkIMChar {
    QChar id;
    PkIMFunc tapfunc;
    QString taparg;
    PkIMFunc holdfunc; 
    QString holdarg;
    bool showList;
};

class PkIMGuessList : public IMIGuessList
{
public:
    PkIMGuessList() : IMIGuessList(), shift(FALSE) {}
    PkIMGuessList(const IMIGuessList &o) : IMIGuessList(o), shift(FALSE) { }
    PkIMGuessList(const PkIMGuessList &o) : IMIGuessList(), shift(FALSE)
    {
	PkIMGuessList::ConstIterator it;
	for (it = o.begin(); it != o.end(); ++it)
	    append(*it);
	shift = o.shift;
    }

    PkIMGuessList(const QString &s, bool c) : IMIGuessList(), shift(c)
    {
	for (uint i = 0; i < s.length(); ++i) {
	    IMIGuess g;
	    g.c = s[i].unicode();
	    g.length = 1;
	    g.error = 0; // equal with all others in this set.
	    append(g);
	}
    }

    PkIMGuessList &operator=(const IMIGuessList &o) {
	IMIGuessList::operator=(o);
	shift = FALSE;
	return *this;
    }

    PkIMGuessList &operator=(const PkIMGuessList &o)
    {
	PkIMGuessList::ConstIterator it;
	for (it = o.begin(); it != o.end(); ++it)
	    append(*it);
	shift = o.shift;
	return *this;
    }

    bool contains(const QChar &c) const
    {
	PkIMGuessList::ConstIterator it;
	for (it = begin(); it != end(); ++it) {
	    if ((*it).c == c.unicode() || (*it).c == c.lower().unicode())
		return TRUE;
	}
	return FALSE;
    }

    IMIGuess find(const QChar &c) const
    {
	PkIMGuessList::ConstIterator it;
	for (it = begin(); it != end(); ++it) {
	    if ((*it).c == c.unicode() || (*it).c == c.lower().unicode())
		return (*it);
	}
	IMIGuess g;
	g.c = 0xffff;
	g.length = 0;
	g.error = UINT_MAX;
	return g;
    }

    int longest() const {
	int min = 0;
	PkIMGuessList::ConstIterator it;
	for (it = begin(); it != end(); ++it) {
	    if ((*it).length > min)
		min = (*it).length;
	}
	return min;
    }

    bool shift;
};

struct PkIMWordError {
    QString text;
    uint error;
};

class PkIMWordErrorList : public QValueList<PkIMWordError>
{
public:
    PkIMWordErrorList() : QValueList<PkIMWordError>() {}
    ~PkIMWordErrorList() {}
    QStringList asStringList() const {
	QStringList r;
	QValueList<PkIMWordError>::ConstIterator it;
	for (it = QValueList<PkIMWordError>::begin();
		it != QValueList<PkIMWordError>::end(); ++it) {
	    r.append((*it).text);
	}

	return r;
    }

    bool contains(const QString &item) const {
	PkIMWordErrorList::ConstIterator i;
	for ( i=begin(); i!=end(); ++i) {
	    if ((*i).text == item)
		return TRUE;
	}
	return FALSE;
    }

    void insert(const PkIMWordError &item)
    {
	PkIMWordErrorList::Iterator i;
	for ( i=begin(); i!=end(); ++i) {
	    if ((*i).error > item.error) {
		QValueList<PkIMWordError>::insert(i, item);
		return;
	    }
	}
	append(item);
    }

    void remove(const QString &item)
    {
	PkIMWordErrorList::Iterator i;
	for ( i=begin(); i!=end(); ++i) {
	    if ((*i).text == item) {
		QValueList<PkIMWordError>::remove(i);
		return;
	    }
	}
    }

    int merge(const PkIMWordErrorList &o) {
	uint min = UINT_MAX;
	PkIMWordErrorList::ConstIterator i;
	for ( i=o.begin(); i!=o.end(); ++i) {
	    PkIMWordError we = *i;
	    if ( !contains(we.text) ) {
		min = QMIN(min, we.error);
		insert(we);
	    }
	}
	return min;
    }
};

/* what does it take to turn matcher into mode. */
class PkIMMatcher {
public:
    PkIMMatcher( );
    PkIMMatcher( const QString & );

    QString id() const {return mId;}
    bool isValid() const { return !mId.isNull(); }

    bool isNamedMode() const { return mIndex < 0; }
    QString replaces() const;
    bool guessCompatible() const;

    bool lookup() const { return mLookup; }

    QPixmap pixmap(bool ucase=FALSE) const; // depends on name

    // internalized is the concept of word v.s. multi tap.
    void revertLast();

    void reset();

    // hide later?
    QMap<QChar, PkIMChar> map() const;

    QStringList choices() const;
    QStringList choices(bool allowPartial, bool allowPredicted) const;
    QStringList choices(bool allowPartial, bool allowPredicted, const QString &start, const QString &end) const;

    QStringList findAll() const;
    QStringList findAll(const QString &prefix, const QString &suffix) const;

    QString numberWord(const QString &prefix = QString::null, const QString &suffix = QString::null) const;
    QString writtenWord(const QString &prefix = QString::null, const QString &suffix = QString::null) const;

    static QStringList languages();
    static QStringList chosenLanguages();

    int count() const;

    QString at(int) const;
    QString atReverse(int) const;

    bool push(const QChar&, bool = FALSE); // digit

    // reset, then multi push...
    bool match(const QString&); // num string

    // push set
    void pushSet(const QString&, bool = FALSE);
    // reset, then multi push set., makes more sense if QStringList. 
    void matchSet(const QStringList&);
    void matchSet(const QString&);

    void pushGuessSet( const PkIMGuessList &);
   // search for pattern in word, starting at offset in pattern for
   // length in pattern. 
    int match(const QString& word, int offset, const QString& pattern, int length);
    // how is this different to revert?
    PkIMGuessList pop();

    static void readConfig(const Config& cfg, QMap<QChar,PkIMChar>& set);
private:

    int prefixLength(const QString &, int f = 0) const;
    int suffixLength(const QString &, int f = 0) const;
    QStringList searchDict(bool prefixfallback, bool predict, const QString& lang, const QStringList& extradict, int start, int len , uint *merror = 0) const;

    PkIMWordErrorList findWords(const QDawg::Node* node, int set, int maxset, const QString& str, uint error, bool allowprefix, bool predict) const;

    QStringList findAll(int set, int maxset, const QString& str) const;
    void init();

    QString findBest(const QString &prefix, const QString &suffix, bool digit) const;
    QString findBest(int set, int maxset, bool digit, const QString & = QString::null) const;

    QString mId;
    int mIndex;
    int mReplaces;
    bool mLookup;
    QString dict;
    QStringList extradict;
    bool lowerdict;

    QVector<PkIMGuessList> nsets;
    int longestEnd;
    bool qualifiedSets;

    QMap<QChar, PkIMChar> extendedset;

    static QStringList langs;
};

// tasks... 
class PkIMMatcherSet : public QObject
{
    Q_OBJECT
public:
    // also loads up all known modes... or
    // maybe just caches them.
    PkIMMatcherSet(QObject *parent);
    ~PkIMMatcherSet();

    // if named, will repace named mode.
    PkIMMatcher *setCurrentMode(const QString &);
    PkIMMatcher *setHintedMode(const QString &);
    void clearHintedMode();
    PkIMMatcher *namedMode() const { return named; }
    void clearNamedMode();

    PkIMMatcher *currentMode() const;

    void nextMode();
    void toggleHinted();

    QStringList modes();
    QStringList guessCompatibleModes();
    QPixmap pixmapFor(const QString &, bool ucase=FALSE);

signals:
    void modeChanged(PkIMMatcher *);

private:
    void checkDictionaries();
    void populate();

    PkIMMatcher *mode(const QString &);
    // this has...
    QList<PkIMMatcher> mModes;
    PkIMMatcher *named;
    PkIMMatcher *current;
    PkIMMatcher *hintedMode;
};

#endif
