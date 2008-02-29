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
#include "pkimmatcher.h"
#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>
#include <qtopia/config.h>
#include <qpixmapcache.h>
#include <qpainter.h>
#include <qaccel.h>
#include <qdir.h>


// could later move all this info into conf file?
struct MatcherConfig {
    char *name;
    char *buttonDiff;
    bool lookup;
    bool guessCompatible;
    bool hasCase;
    bool extendTextButtons;
};

// only means 'no 0..9 + lookup mode.
// really means.. no 'textbuttons' if in lookup mode.
//#define NO_DICT_MODE

// special characters are not properly converted by QChar::upper()
// of course still misses a lot.
QChar convertToUpper(const QChar& letter)
{
    const short offset = 32; //offset between upper and lower case letter
    if ( letter >= QChar(0x00e0) && letter <= QChar(0x00fe)) // special chars range
	return QChar(letter.unicode() - offset);
    else if ( letter == QChar(0x00ff) )
	return QChar(0x0179);
    return letter.upper();
}

MatcherConfig validNames[] =
{
    { "dict", 0, TRUE, TRUE, TRUE, FALSE },
    { "abc", "LocaleTextButtons", FALSE, TRUE, TRUE, TRUE },
    { "ext", 0, FALSE, FALSE, TRUE, FALSE },
    { "phone", "PhoneTextButtons", FALSE, FALSE, FALSE, FALSE },
    { 0, 0, FALSE, FALSE, FALSE, FALSE }
};

static const int null_index = -1;
static const int dict_index = 0;
static const int abc_index = 1;
static const int ext_index = 2;
static const int phone_index = 3;

struct MatcherAlias {
    char *name;
    char *lookup;
};

MatcherAlias aliasLookup[] =
{
    { "extended", "ext" },
    { "words", "dict" },
    { "text", "abc" },
    { 0, 0 }
};

PkIMMatcher::PkIMMatcher() : mIndex(null_index), mReplaces(null_index), mLookup(TRUE), lowerdict(FALSE)
{
    mId = "abc";
    mIndex = abc_index;
    init();
}

PkIMMatcher::PkIMMatcher(const QString &n) : mIndex(null_index), mReplaces(null_index), mLookup(TRUE), lowerdict(FALSE)
{
    int index = 0;
    QString tmpName = n;
    while(aliasLookup[index].name != 0) {
	if (tmpName == aliasLookup[index].name) {
	    tmpName = aliasLookup[index].lookup;
	    break;
	}
	index++;
    }
    index = 0;
    while(validNames[index].name != 0) {
	if (tmpName == validNames[index].name) {
	    mId = tmpName;
	    mIndex = index;
	    break;
	}
	index++;
    }

    QStringList cl = chosenLanguages();
    // named dictionary.  mIndex is still dict, but name isn't
    if (tmpName.left(5) == "dict-" && cl.grep(mId.mid(5)).count() > 0) {
	mId = tmpName;
	mIndex = dict_index;
    }

    if (mId.isNull())
	mId = tmpName;
    // init will set mId to null if there is no valid mode of that name
    init();
}

QString PkIMMatcher::replaces() const
{
    return validNames[mReplaces].name;
}

QPixmap PkIMMatcher::pixmap(bool shift) const
{
    QString key;
    QString ul;
    if (mIndex != null_index) {
	if (validNames[mIndex].hasCase)
	    ul = shift ? "-uc" : "-lc";
    } else {
	if (validNames[mReplaces].hasCase)
	    ul = shift ? "-uc" : "-lc";
    }

    QPixmap pm;

    // should be icons for named modes, both lc and uc
    if (isNamedMode()) {
	key = "named_"+mId;
    } else {
        int index = mId.find('-');
        QString modifiedId = mId;
        if (index >= 0) 
            modifiedId = "i18n/" + mId.mid(index+1) + '/' + mId.left(index);
 	key = modifiedId;
    }

    if ( !key.isEmpty() )
	pm = Resource::loadPixmap("pkim/"+key+ul);
    //mark the pictures that need translations 
    /*Resource::loadPixmap("dict-lc")*/ QT_TRANSLATE_NOOP("PkIMMatcher", "dict-lc");
    /*Resource::loadPixmap("dict-uc")*/ QT_TRANSLATE_NOOP("PkIMMatcher", "dict-uc");

    if ( pm.isNull() && mIndex == dict_index) {
	// was a dictionary but no pixmap found, so make one.
	key = "qpe_"+key;
	if ( !QPixmapCache::find(key,pm) ) {
	    pm = Resource::loadPixmap("pkim/dict-lang");
	    QPainter p(&pm);
	    QString l = dict.lower().left(2);
	    if ( shift )
		l[0] = l[0].upper();
	    p.drawText(pm.rect(),Qt::AlignCenter,l);
	    QPixmapCache::insert(key,pm);
	}
    }
    if ( pm.isNull()) {
	// try from base type?
	if (mReplaces != null_index) {
	    key = validNames[mReplaces].name;
	    if (!key.isEmpty())
		pm = Resource::loadPixmap("pkim/"+key+ul);
	}
    }
    return pm;
}

QStringList PkIMMatcher::findAll() const
{
    return findAll(0, nsets.size(), QString::null );
}

QStringList PkIMMatcher::findAll(const QString &prefix, const QString &suffix) const
{
    int pl = prefixLength(prefix);
    int sl = suffixLength(suffix); 
    QStringList ch;
    if (pl != -1 && sl != -1 && (pl + sl < (int)nsets.size())) {
	ch = findAll(pl, nsets.size()-sl, QString::null);
    }
    QStringList::Iterator it;
    for (it = ch.begin(); it != ch.end(); ++it) {
	*it = prefix + *it + suffix;
    }
    return ch;
}

QStringList PkIMMatcher::choices() const
{
    return choices(TRUE, FALSE, QString::null, QString::null);
}

QStringList PkIMMatcher::choices(bool allowpartial, bool allowpredicted) const
{
    return choices(allowpartial, allowpredicted, QString::null, QString::null);
}


int PkIMMatcher::prefixLength(const QString &p, int from) const
{
    if (from >= (int)nsets.size())
	return -1;
    if (p.isEmpty())
	return from;

    QChar c = p[0];

    PkIMGuessList *gl = nsets[from];
    if (gl->contains(c)) {
	IMIGuess guess = gl->find(c);
	//ASSERT(guess.length != 0);
	return prefixLength(p.mid(1), from + guess.length);
    } else {
	return -1;
    }
}

int PkIMMatcher::suffixLength(const QString &p, int from) const
{
    if (from >= (int)nsets.size())
	return -1;
    if (p.isEmpty())
	return from;

    QChar c = p[p.length()-1];

    int i;
    for (i = (int)nsets.size()-1; i >= 0; --i) {
	PkIMGuessList *g = nsets[i];
	PkIMGuessList::ConstIterator it = g->begin();
	while(it != g->end()) {
	    IMIGuess guess = *it;
	    if ((guess.length + i == (int)nsets.size() - from  )
		    && QChar(guess.c) == c && guess.length != 0)
		return suffixLength(p.left(p.length()-1),
			from + guess.length);
	    ++it;
	}
    }
    return -1;
}

QStringList PkIMMatcher::choices(bool allowpartial, bool allowpredicted, const QString &prefix, const QString &suffix) const
{
    if (mLookup) {
	QStringList ch;
	uint minError;
	int pl = prefixLength(prefix);
	int sl = suffixLength(suffix); 
	if (pl == -1 || sl == -1 || (pl + sl >= (int)nsets.size()))
	    return QStringList();
	ch = searchDict(allowpartial, allowpredicted, dict, extradict,
		pl, nsets.size()-sl, &minError);
	if ( lowerdict ) {
	    for (QStringList::Iterator i=ch.begin(); i!=ch.end(); ++i)
		*i = (*i).lower();
	}
	QStringList::Iterator it;
	for (it = ch.begin(); it != ch.end(); ++it) {
	    *it = prefix + *it + suffix;
	}
	return ch;
    } else {
	return findAll(nsets.size()-longestEnd, nsets.size(), QString::null );
    }
}

QString PkIMMatcher::numberWord(const QString &prefix, const QString &suffix) const
{
    return findBest(prefix, suffix, TRUE);
}

QString PkIMMatcher::writtenWord(const QString &prefix, const QString &suffix) const
{
    return findBest(prefix, suffix, FALSE);
}

QString PkIMMatcher::findBest(const QString &prefix, const QString &suffix, bool isDigit) const
{
    int pl = prefixLength(prefix);
    int sl = suffixLength(suffix); 
    if (pl == -1 || sl == -1 || (pl + sl >= (int)nsets.size()))
	return QString::null;
    QString nword = findBest(pl, nsets.size()-sl, isDigit);
    if (nword.isEmpty())
	return QString::null;
    else
	return prefix+nword+suffix;
}

QString PkIMMatcher::findBest(int set, int maxset, bool isDigit, const QString &str) const
{
    // only return if error values given.
    if (!qualifiedSets)
	return QString::null;
    if (set == maxset)
	return str;
    else if (set >maxset)
	return QString::null;

    // add word as spelt.
    PkIMGuessList *gl = nsets[set];
    PkIMGuessList::Iterator it = gl->begin();
    QValueList<int> avoidLength;
    avoidLength.append(0);
    while(it != gl->end()) {
	IMIGuess guess = *it;
	QChar ch(guess.c);
	if (gl->shift)
	    ch = convertToUpper(ch);
	if (!avoidLength.contains(guess.length) && ch.isDigit() == isDigit) {
	    QString r = findBest(set+guess.length, maxset, isDigit, str+ch);
	    if (!r.isEmpty())
		return r;
	    avoidLength.append(guess.length);
	}
	++it;
    }
    return QString::null;
}

static PkIMFunc funcNameToEnum(const QString &s, QString& arg, bool* showlist)
{
    if (s[0] == '\'' || s[0] == '"' ) {
	arg = s.mid(1);
	if ( showlist )
	    *showlist = s[0] == '"';
	return insertText;
    }
    if (s == "shift")
	return changeShift;
    if (s == "mode")
	return changeMode;
    if (s == "modify")
	return modifyText;
    if (s == "space")
	return insertSpace;
    if (s == "symbol")
	return insertSymbol;
    return noFunction;
}


QStringList PkIMMatcher::searchDict(bool prefixfallback,
	bool predict, const QString& language,
	const QStringList& extradict, int start, int end, uint *me) const
{
    // Search dictionaries. Return list of words matching
    // this->sets, with the most likely word first. If no
    // words match, prefixes of the most likely words are
    // returned.

    PkIMWordErrorList r;
    QString lang = language;
    QString lang2;
    if ( lang.isNull() )
	lang = chosenLanguages()[0];
    if ( lang[2] == '_' ) {
	// Non country-specific words
	QString l2 = lang.left(2);
	if ( languages().contains(l2) )
	    lang2 = "-" + l2;
    }
    lang = "-" + lang;

    uint minError = UINT_MAX;
    for (int prefix=0; r.isEmpty() && prefix<=1; ++prefix) {
	PkIMWordErrorList preferred, added, extra;
	PkIMWordErrorList common, fixed, common2, fixed2;

	if(!qualifiedSets)
	    preferred = findWords(Global::dawg("preferred").root(),start,end,
		    "", 0, prefix, predict);
	added = findWords(Global::addedDawg().root(),start,end,
		    "",0, prefix, predict);
	if(!qualifiedSets)
	    common = findWords(Global::dawg("_common"+lang).root(),start,end,
		    "",0, prefix, predict);
	fixed = findWords(Global::dawg("_words"+lang).root(),start,end,
		    "",0, prefix, predict);

	if ( !lang2.isEmpty() ) {
	    common2 = findWords(Global::dawg("_common"+lang2).root(),
		    start,end,"",0, prefix, predict);
	    fixed2 = findWords(Global::dawg("_words"+lang2).root(),
		    start,end,"",0, prefix, predict);
	}

	uint nerror;
	for (QStringList::ConstIterator xdit=extradict.begin(); xdit!=extradict.end(); ++xdit) {
	    extra = findWords(Global::dawg(*xdit).root(),start,end,"",0, prefix, predict);
	    nerror = r.merge(extra);
	    if (nerror < minError)
		minError = nerror;
	}

	nerror = r.merge(preferred);
	if (nerror < minError)
	    minError = nerror;
	nerror = r.merge(added);
	if (nerror < minError)
	    minError = nerror;
	nerror = r.merge(common);
	if (nerror < minError)
	    minError = nerror;
	nerror = r.merge(fixed);
	if (nerror < minError)
	    minError = nerror;
	nerror = r.merge(common2);
	if (nerror < minError)
	    minError = nerror;
	nerror = r.merge(fixed2);
	if (nerror < minError)
	    minError = nerror;

	if ( !prefix ) {
	    QStringList deleted = findWords(Global::dawg("deleted").root(),start,end,"",0, prefix, predict).asStringList();
	    for ( QStringList::ConstIterator i=deleted.begin(); i!=deleted.end(); ++i)
		r.remove(*i);
	}
	if ( !prefixfallback )
	    break;
    }

    if (me)
	*me = minError;
    // if error larger than X add extra word? not here.
    return r.asStringList();
}

QStringList PkIMMatcher::findAll(int set, int maxset, const QString& str) const
{
    if ( set == maxset ) {
	// fits in asked for set
	return QStringList(str);
    } else if (set > maxset ) {
	// does not fit in asked for set
	return QStringList();
    }

    QStringList r;
    PkIMGuessList *g = nsets[set];
    PkIMGuessList::Iterator it = g->begin();
    while(it != g->end()) {
	IMIGuess guess = *it;
	QChar c(guess.c);
	if (g->shift)
	    c = convertToUpper(c);
	r += findAll(set+guess.length, maxset, str+c);
	++it;
    }
    return r;

}

PkIMWordErrorList PkIMMatcher::findWords(const QDawg::Node* node,
	int set, int maxset,
	const QString& str, uint error, bool allowprefix, bool predict) const
{
    if ( !node || (set >= maxset && !predict) )
	return PkIMWordErrorList();

    PkIMWordErrorList r;
    PkIMGuessList *g = 0;
    if ( set < maxset ) {
	g = nsets[set];
	// no letters to follow, don't try and make word, invalid path.
	if (g->count() == 0) {
	    return PkIMWordErrorList();
	}
    }
    PkIMWordError any;
    while (node) {
	QChar ch = node->letter();
	IMIGuess guess;
	if (g && g->contains(ch)) {
	    guess = g->find(ch);
	} else {
	    guess.length = 0;
	    guess.c = 0xffff;
	    guess.error = UINT_MAX;
	}
	if ( (predict && (g == 0 || g->isEmpty())) || guess.length != 0) {
	    if (g && g->shift)
		ch = convertToUpper(ch);
	    if ( set >= maxset-(guess.length > 0 ? guess.length : 1) ) {
		PkIMWordError we;
		we.text = str+ch;
		we.error = error;
		if (guess.error != UINT_MAX)
		    we.error += guess.error;
		if ( node->isWord() )
		    r.append(we);
		else
		    any = we;
	    }
	    // is >> 10 so can up 2^10 = 1024 char words.
	    if (guess.length) {
		r += findWords(node->jump(),
			set+guess.length, maxset,
			str+ch, error+(guess.error >> 10), 
			allowprefix, predict);
	    } else if (predict) {
		r += findWords(node->jump(),
			set+1, maxset,
			str+ch, error,
			allowprefix, predict);
	    }
	}
	node = node->next();
    }
    if ( allowprefix && r.isEmpty() && !any.text.isEmpty() ) {
	r.append(any);
    }
    return r;
}

void PkIMMatcher::reset()
{
    nsets.resize(0);
    longestEnd = 0;
    qualifiedSets = FALSE;
}

void PkIMMatcher::matchSet(const QString& s)
{
    reset();
    for (int i=0; i<(int)s.length(); i++)
	pushSet(QString(s[i]));
}

void PkIMMatcher::matchSet(const QStringList& s)
{
    reset();
    for (int i=0; i<(int)s.count(); i++)
	pushSet(QString(s[i]));
}

// reset then do input...
bool PkIMMatcher::match(const QString& s)
{
    reset();
    for (int i=0; i<(int)s.length(); i++) {
	if ( !push(s[i]) ) {
	    reset();
	    return FALSE;
	}
    }
    return TRUE;
}

bool PkIMMatcher::push(const QChar& ch, bool shift)
{
    if (extendedset.contains(ch)) {
	PkIMChar item = extendedset[ch];
	if (item.tapfunc == insertText) {
	    pushSet(item.taparg, shift);
	    return TRUE;
	}
    }
    return FALSE;
}

void PkIMMatcher::pushSet(const QString& s, bool shift)
{
    nsets.resize(nsets.size()+1);
    nsets.insert(nsets.size()-1, new PkIMGuessList(s, shift));
    longestEnd = 1;
} 

void PkIMMatcher::pushGuessSet( const PkIMGuessList &nl)
{
    longestEnd = 0;
    nsets.resize(nsets.size()+1);

    PkIMGuessList *set = new PkIMGuessList();
    set->shift = nl.shift;
    nsets.insert(nsets.size()-1, set);

    PkIMGuessList::ConstIterator it = nl.begin();
    while(it != nl.end()) {
	int l = (*it).length;
	int index = nsets.size()-l;
	if (index < 0 || index >=(int)nsets.size()) {
	    // drop, wouldn't fit into word list
	    ++it;
	    continue;
	}
	if (l > longestEnd)
	    longestEnd = l;
	if ((*it).error != 0)
	    qualifiedSets = TRUE;

	PkIMGuessList *gl = nsets[nsets.size()-l];

	// need to insert in order of error
	PkIMGuessList::Iterator git;
	for (git = gl->begin(); git != gl->end(); ++git) {
	    if ((*git).error > (*it).error) {
		gl->insert(git, *it);
		break;
	    }
	}
	if (git == gl->end()) {
	    // end of list
	    gl->append(*it);
	}
	++it;
    }
}

PkIMGuessList PkIMMatcher::pop()
{
    PkIMGuessList result;
    if (nsets.size() > 0) {
	longestEnd = 0;
	for (uint i = 0; i < nsets.size(); i++) {
	    PkIMGuessList *gl = nsets[i];
	    PkIMGuessList::Iterator it = gl->begin();
	    while(it != gl->end()) {
		if ((*it).length + i == nsets.size()) {
		    result.append(*it);
		    it = gl->remove(it);
		} else {
		    ++it;
		}
		if (i+1 == nsets.size() && (*it).length > longestEnd) {
		    longestEnd = (*it).length;
		}
	    }
	}
	nsets.resize(nsets.size()-1);
    }
    if (nsets.size() == 0)
	qualifiedSets = FALSE;
    return result;
}

int PkIMMatcher::count() const
{
    return nsets.size();
}

QString PkIMMatcher::at(int ind) const
{
    QString result;
    PkIMGuessList *gl = nsets[ind];
    PkIMGuessList::Iterator it = gl->begin();
    while(it != gl->end()) {
	result += (*it).c;
	++it;
    }
    return result;
}

QString PkIMMatcher::atReverse(int ind) const
{
    QString result;
    ind = nsets.size()-ind;
    for (int i = 0; i < (int)nsets.size(); i++) {
	PkIMGuessList *gl = nsets[i];
	PkIMGuessList::Iterator it = gl->begin();
	while(it != gl->end()) {
	    if ((*it).length + i == ind) {
		result += (*it).c;
	    }
	    ++it;
	}
    }
    return result;
}


QMap<QChar, PkIMChar> PkIMMatcher::map() const
{
    return extendedset;
}

void PkIMMatcher::readConfig(const Config& cfg, QMap<QChar,PkIMChar>& set)
{
    QString buttons = cfg.readEntry("Buttons"); // No tr

    for (int i = 0; i < (int)buttons.length(); i++) {
	QChar ch = buttons[i];

	QString tapfunc("Tap"); // No tr
	QString holdfunc("Hold"); // No tr
	tapfunc += ch;
	holdfunc += ch;

	PkIMChar c = *set.find(ch); // lokup existing setting
	c.id = ch; // (if not found)

	QString fn;
	fn = cfg.readEntry(tapfunc);
	if ( !fn.isNull() )
	    c.tapfunc = funcNameToEnum(fn,c.taparg,&c.showList);
	fn = cfg.readEntry(holdfunc);
	if ( !fn.isNull() )
	    c.holdfunc = funcNameToEnum(fn,c.holdarg,0); // no showlist for Hold

	set.replace(c.id, c);
    }
}

/*
   Read the config of button to behaviour map
   */
void PkIMMatcher::init()
{
    nsets.setAutoDelete(TRUE);
    longestEnd = 0;
    qualifiedSets = 0;
    if (isNamedMode()) {
	Config cfg(QPEApplication::qpeDir()+"etc/im/pkim/named_"+mId+".conf", Config::File);
	if ( cfg.isValid() ) {
	    cfg.setGroup("Buttons");
	    extradict = QStringList::split(' ',cfg.readEntry("ExtraDict"));
	    QString d = cfg.readEntry("Dict");
	    if ( !d.isEmpty() )
		dict = d;
	    else
		dict = chosenLanguages()[0];
	    QString m = cfg.readEntry("Mode");
	    if ( m.isEmpty() ) {
		mReplaces = abc_index; // "abc"
	    } else switch (m[0].latin1()) {
		default:
		    mReplaces=abc_index; // "abc"
		    break;
		case 'D':
		    mReplaces=dict_index; // "dict"
		    break;
		case 'P':
		    mReplaces=phone_index; // Phone;
		    break;
	    }
	    QString cs = cfg.readEntry("Case");
	    if ( !cs.isEmpty() ) {
		if (cs[0] == 'l')
		    lowerdict = TRUE;
	    }

	    QString base = cfg.readEntry("Base");
#ifdef NO_DICT_MODE
	    if (!validNames[mReplaces].lookup || base != "TextButtons") { // no tr
#endif
		if (!base.isNull()) {
		    Config globalcfg(Global::defaultButtonsFile(), Config::File); // No tr
		    globalcfg.setGroup(base);
		    readConfig(globalcfg,extendedset);
		}
		// overrides?
		readConfig(cfg,extendedset);
#ifdef NO_DICT_MODE
	    }
#endif
	} else {
	    mId = QString::null; // make invalid.
	}
    } else {
	if (mId.left(5) == "dict-") {
	    dict = mId.mid(5);
	} else {
	    dict = chosenLanguages()[0];
	}

	mLookup = validNames[mIndex].lookup;

	Config globalcfg(Global::defaultButtonsFile(), Config::File); // No tr
#ifdef NO_DICT_MODE
	if (!validNames[mIndex].lookup) {
#endif
	    if (validNames[mIndex].buttonDiff == 0 || validNames[mIndex].extendTextButtons) {
		globalcfg.setGroup("TextButtons"); // No tr
		readConfig(globalcfg,extendedset);
	    }
	    if (validNames[mIndex].buttonDiff) {
		globalcfg.setGroup(validNames[mIndex].buttonDiff);
		readConfig(globalcfg,extendedset);
	    }
#ifdef NO_DICT_MODE
	}
#endif
    }
}

/*
   Matches phone keypad input \a needle to string \a haystack, starting
   at \a offset, for n characters,
   returning how many matched consecutively.

   // this is more of a compare
*/
int PkIMMatcher::match(const QString& haystack, int offset, const QString& needle, int n)
{
    for (int i=0; i<n; i++) 
    {
	QChar ch = haystack[i+offset].lower();
	QChar pk = needle[i];
	if( pk < '0' || pk > '9' ) {
	    if ( ch != pk )
		return i;
	} else {
	    if (extendedset.contains(pk)) {
		PkIMChar item = extendedset[pk];
		if (item.tapfunc != insertText || !item.taparg.contains(ch))
		    return i;
	    } else
		return i;
	}
    }

    return n; // match!
}


QStringList PkIMMatcher::langs;

QStringList PkIMMatcher::languages()
{
    if ( langs.isEmpty() ) {
        QString basename = QPEApplication::qpeDir() + "/etc/dict/";
        QDir dir(basename);
        QStringList dftLangs = dir.entryList(QDir::Dirs);
        for (QStringList::ConstIterator it = dftLangs.begin(); it != dftLangs.end(); ++it){
            QString lang = *it;
            if (QFile::exists(basename+lang+"/words.dawg")) 
                langs.append(lang);
        }
    }
    return langs;
}

QStringList PkIMMatcher::chosenLanguages()
{
    // Rely on Config's caching, so we just re-read every time
    // rather than needing a change message.
    Config cfg("locale");
    cfg.setGroup("Language");
    QStringList r = cfg.readListEntry("InputLanguages",' ');
    if ( r.isEmpty() )
	r.append(cfg.readEntry("Language","en_US"));
    return r;
}

bool PkIMMatcher::guessCompatible() const
{
    if (mIndex != null_index)
	return validNames[mIndex].guessCompatible;
    else
	return validNames[mReplaces].guessCompatible;
}

/*******************
  * PkIMMatcherSet
  ********************/

PkIMMatcherSet::PkIMMatcherSet(QObject *parent)
    : QObject(parent), named(0), current(0), hintedMode(0)
{
    mModes.setAutoDelete(TRUE);
    populate();
}

void PkIMMatcherSet::populate()
{
    int index = 0;
    while(validNames[index].name != 0) {
	mModes.append(new PkIMMatcher(validNames[index].name));
	index++;
    }

    // load up dictionaries.
    QStringList cl = PkIMMatcher::chosenLanguages();
    QStringList::Iterator it = cl.begin();
    ++it; // skip the first.
    while(it != cl.end()) {
	mModes.append(new PkIMMatcher("dict-"+(*it)));
	++it;
    }

    current = mModes.getFirst();
}

PkIMMatcherSet::~PkIMMatcherSet()
{
    if (named)
	delete named;
    named = 0; // pointless, but meaningful
    current = 0;
}

void PkIMMatcherSet::checkDictionaries()
{
    // load up dictionaries.
    QStringList cl = PkIMMatcher::chosenLanguages();
    QStringList::Iterator it;
    // first remove ones from mMode that are not chosen.
	// 
    QListIterator<PkIMMatcher> mit(mModes);
    while ( mit.current()) {
	if (mit.current()->id().left(5) == "dict-") {
	    it = cl.begin();
	    ++it; // skip the first.
	    bool keep = FALSE;
	    while(it != cl.end()) {
		if (mit.current()->id() == "dict-"+(*it)) {
		    // remove from list so not added later.
		    cl.remove(it);
		    keep = TRUE;
		    break;
		}
		++it;
	    }
	    if (keep) {
		++mit;
	    } else {
		if (current == mit.current())
		    current = 0;
		if (hintedMode == mit.current())
		    hintedMode = 0;
		if (named == mit.current())
		    named = 0;
		mModes.remove(mit.current());
	    }
	} else {
	    ++mit;
	}
    }

    // then put the ones chosen back in. Only ones not already in
    // list are in cl, others removed above.
    it = cl.begin();
    ++it; // skip the first.
    while(it != cl.end()) {
	mModes.append(new PkIMMatcher("dict-"+(*it)));
	++it;
    }
    if (!current) {
	if (hintedMode)
	    current = hintedMode;
	else
	    current = mModes.getFirst();
	emit modeChanged(current);
    }
}

void PkIMMatcherSet::clearHintedMode()
{
    hintedMode = 0;
}

PkIMMatcher *PkIMMatcherSet::setHintedMode(const QString &name)
{
    hintedMode = setCurrentMode(name);
    return hintedMode;
}

PkIMMatcher *PkIMMatcherSet::setCurrentMode(const QString &name)
{
    PkIMMatcher *target = mode(name);
    if (!target) {
	target = new PkIMMatcher(name);
	if (target->isValid()) {
	    if (target->isNamedMode()) {
		if (named)
		    delete named;
		named = target;
	    } else {
		mModes.append(target);
	    }
	} else {
	    delete target;
	    target = 0;
	}
    }

    if (target) {
	current = target;
	emit modeChanged(current);
	return target;
    } else {
	return 0;
    }
}

void PkIMMatcherSet::clearNamedMode()
{
    if (named) {
	if (hintedMode == named)
	    hintedMode = 0;
	if (current == named) {
	    current = mModes.getFirst();
	    emit modeChanged(current);
	}
	delete named;
	named = 0;
    }
}

PkIMMatcher *PkIMMatcherSet::currentMode() const
{
    // ASSERT current != 0.  current should be set to non-0
    // in constructor, and only set to non-0 thereafter.
    return current;
}

/* if not first, sets to first.  otherwise, toggles

   There is a slight assumption in this code that if there
   is a named and a hintedMode, that they are equal.
   */
void PkIMMatcherSet::toggleHinted()
{
    // if there is a hinted mode.. then that
    // else if there is a first mode, then that,
    // if already 'first' then next.
    PkIMMatcher *target = 0;
    if (hintedMode)
	target = hintedMode;
    else
	target = mModes.getFirst();

    if (current != target) {
	current = target;
	emit modeChanged(current);
    } else {
	nextMode();
    }
}

/* order is
   hinted
   as loaded in mModes (without hinted)
    but with named mode replacing one of them
*/
void PkIMMatcherSet::nextMode()
{
    checkDictionaries();
    if (hintedMode && current == hintedMode) {
	current = mModes.first();
	// skip any mode named replaces.
	if (current == hintedMode || (named && named->replaces() == current->id()))
	    current = mModes.next();
    } else {
	// if current in mModes, then next in mModes,
	// if current !in mModes, first mModes
	QListIterator<PkIMMatcher> it(mModes);
	PkIMMatcher *target = 0;
	for ( ; it.current(); ++it ) {
	    if (it.current() == current) {
		++it;
		if (it.current()) { 
		    target = it.current();
		    // skip any mode named replaces.
		    if (named && named->replaces() == target->id()) {
			++it;
			if (it.current())
			    target = it.current();
			else
			    target = hintedMode;
		    }
		} else {
		    target = hintedMode;//it.toFirst();
		}
		break;
	    }
	}
	if (!target) {
	    current = mModes.getFirst();
	} else {
	    current = target;
	}
    }
    emit modeChanged(current);
}

QStringList PkIMMatcherSet::guessCompatibleModes()
{
    checkDictionaries();
    QStringList res;
    QListIterator<PkIMMatcher> it(mModes);
    for ( ; it.current(); ++it ) {
	PkIMMatcher *match = it.current();
	if (match->guessCompatible()) {
	    if (named && named->replaces() == match->id())
		res += named->id();
	    else
		res += match->id();
	}
    }
    return res;
}

QStringList PkIMMatcherSet::modes()
{
    checkDictionaries();
    QStringList res;
    QListIterator<PkIMMatcher> it(mModes);
    for ( ; it.current(); ++it ) {
	PkIMMatcher *match = it.current();
	if (named && named->replaces() == match->id())
	    res += named->id();
	else
	    res += match->id();
    }
    return res;
}


QPixmap PkIMMatcherSet::pixmapFor(const QString &name, bool ucase)
{
    if (mode(name))
	return mode(name)->pixmap(ucase);
    return QPixmap();
}

PkIMMatcher *PkIMMatcherSet::mode(const QString &name)
{
    checkDictionaries();
    // first see if name has an alias.
    QString tmpName = name;
    uint index=0;
    while(aliasLookup[index].name != 0) {
	if (tmpName == aliasLookup[index].name) {
	    tmpName = aliasLookup[index].lookup;
	    break;
	}
	index++;
    }

    // ?? yeah.. this is what I mean.  
    if (named && (named->id() == tmpName || named->replaces() == tmpName))
	return named;

    QListIterator<PkIMMatcher> it(mModes);
    for ( ; it.current(); ++it ) {
	PkIMMatcher *match = it.current();
	if (match->id() == tmpName)
	    return match;
    }
    return 0;
}
