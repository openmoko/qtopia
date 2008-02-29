/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <qfile.h>
// #include <qtl.h>
#include <math.h>
#include <limits.h>
#include <errno.h>
#include <qdatastream.h>
#ifdef Q_WS_QWS
#include <qtopiaapplication.h>
#else
#include <qapplication.h>
#endif
#include "combining.h"
#include "char.h"

#define QIMPEN_MATCH_THRESHOLD      200000

// fortunately there is as yet no collision between old Qt 2.3 key codes, and Qt 4.x key codes.
const QIMPenSpecialKeys qimpen_specialKeys[] = {
    { Qt::Key_Escape,           QT_TRANSLATE_NOOP("Handwriting","[Esc]"), 0x1000 },
    { Qt::Key_Tab,              QT_TRANSLATE_NOOP("Handwriting","[Tab]"), 0x1001 },
    { Qt::Key_Backspace,        QT_TRANSLATE_NOOP("Handwriting","[BackSpace]"), 0x1003 },
    { Qt::Key_Return,           QT_TRANSLATE_NOOP("Handwriting","[Return]"), 0x1004 },
    { QIMPenChar::Caps,         QT_TRANSLATE_NOOP("Handwriting","[Uppercase]"), 0x4001 },
    { QIMPenChar::CapsLock,     QT_TRANSLATE_NOOP("Handwriting","[Caps Lock]"), 0x4003 },
    { QIMPenChar::Shortcut,     QT_TRANSLATE_NOOP("Handwriting","[Shortcut]"), 0x4002 },
    { QIMPenChar::Punctuation,  QT_TRANSLATE_NOOP("Handwriting","[Punctuation]"), 0x4004 },
    { QIMPenChar::Symbol,       QT_TRANSLATE_NOOP("Handwriting","[Symbol]"), 0x4005 },
    { QIMPenChar::NextWord,     QT_TRANSLATE_NOOP("Handwriting","[Next Word]"), 0x4007 },
    { QIMPenChar::WordPopup,    QT_TRANSLATE_NOOP("Handwriting","[Word Menu]"), 0x4008 }, // word popup
    { QIMPenChar::SymbolPopup,  QT_TRANSLATE_NOOP("Handwriting","[Symbol Menu]"), 0x4009 },
    { QIMPenChar::ModePopup,    QT_TRANSLATE_NOOP("Handwriting","[Mode Menu]"), 0x400A },
    { Qt::Key_unknown,          0, 0 } };


/*!
  \class QIMPenChar
  \brief The QIMPenChar class handles a single character.

  Can calculate closeness of match to
  another character.

  \ingroup qtopiahandwriting
*/

QIMPenChar::QIMPenChar()
{
    flags = 0;
}

QIMPenChar::QIMPenChar( const QIMPenChar &chr )
{
    mUnicode = chr.mUnicode;
    mKey = chr.mKey;
    flags = chr.flags;
    QIMPenStroke *s = 0;
    foreach(QIMPenStroke *it, chr.strokes) {
        s = new QIMPenStroke( *it );
        Q_CHECK_PTR( s );
        strokes.append( s );
    }
}

QIMPenChar::~QIMPenChar()
{
    // "autodelete"
    while ( strokes.count() )
        delete strokes.takeLast();
}

QIMPenChar &QIMPenChar::operator=( const QIMPenChar &chr )
{
    strokes.clear();
    mUnicode = chr.mUnicode;
    mKey = chr.mKey;
    flags = chr.flags;
    QIMPenStrokeConstIterator it = chr.strokes.constBegin();
    QIMPenStroke *s = 0;
    while ( it != chr.strokes.constEnd() ) {
        s = new QIMPenStroke( **it );
        Q_CHECK_PTR( s );
        strokes.append( s );
        ++it;
    }

    return *this;
}

QString QIMPenChar::name() const
{
    if (mKey) {
        QString n;
        for ( int i = 0; qimpen_specialKeys[i].code != Qt::Key_unknown; i++ ) {
            if ( qimpen_specialKeys[i].code == mKey ) {
                n = qApp->translate("Handwriting", qimpen_specialKeys[i].name);
                return n;
            }
        }
    }
    return QString(mUnicode);
}

void QIMPenChar::clear()
{
    mUnicode = 0;
    mKey = 0;
    flags = 0;
    strokes.clear();
}

unsigned int QIMPenChar::strokeLength( int s ) const
{
    return strokes.count() > s ? strokes[s]->length() : 0;
    /*
    QIMPenStrokeIterator it( strokes );
    while ( it.current() && s ) {
        ++it;
        --s;
    }

    if ( it.current() )
        return it.current()->length();

    return 0;
    */
}

/*!
  Add a stroke, \a st, to the character
*/
void QIMPenChar::addStroke( QIMPenStroke *st )
{
    QIMPenStroke *stroke = new QIMPenStroke( *st );
    strokes.append( stroke );
}

/*!
  Return an indicator of the closeness of this character to \a pen.
  Lower value is better.
*/
int QIMPenChar::match( QIMPenChar *pen )
{
/*
    if ( strokes.count() > pen->strokes.count() )
        return INT_MAX;
*/
    int err = 0;
    int maxErr = 0;
    int diff = 0;
    QIMPenStrokeIterator it1 = strokes.begin();
    QIMPenStrokeIterator it2 = pen->strokes.begin();
    err = (*it1)->match( *it2 );
    if ( err > maxErr )
        maxErr = err;
    ++it1;
    ++it2;
    /* currently the multi-stroke gravity checks are not
       canvas height dependent, they should be */
    while ( err < 400000
            && it1 != strokes.end()
            && it2 != pen->strokes.end() )
    {
        // the difference between the center of this stroke
        // and the center of the first stroke.
        QPoint p1 = (*it1)->boundingRect().center() -
                    strokes[0]->boundingRect().center();

        // scale to canvas height
        p1 = p1 * 75 / (*it1)->canvasHeight();

        // the difference between the center of this stroke
        // and the center of the first stroke.
        QPoint p2 = (*it2)->boundingRect().center() -
                    pen->strokes[0]->boundingRect().center();

        // scale to canvas height
        p1 = p1 * 75 / (*it1)->canvasHeight();

        int xdiff = qAbs( p1.x() - p2.x() ) - 6;
        int ydiff = qAbs( p1.y() - p2.y() ) - 5;
        if ( xdiff < 0 )
            xdiff = 0;
        if ( ydiff < 0 )
            ydiff = 0;
        if ( xdiff > 10 || ydiff > 10 ) { // not a chance
            return INT_MAX;
        }
        diff += xdiff*xdiff + ydiff*ydiff;
        err = (*it1)->match( *it2 );
        if ( err > maxErr )
            maxErr = err;
        ++it1;
        ++it2;
    }

    maxErr += diff * diff * 6; // magic weighting :)

    return maxErr;
}

/*!
  Return the bounding rect of this character.  It may have sides with
  negative coords since its origin is where the user started drawing
  the character.
*/
QRect QIMPenChar::boundingRect()
{
    QRect br;
    QIMPenStrokeConstIterator st = strokes.constBegin();
    while ( st != strokes.constEnd() ) {
        br |= (*st)->boundingRect();
        st++;
    }

    return br;
}

/*!
  If \a code is in the set of qimpen_specialKeys, sets the key code for the character
  to \a code and clear the unicode value.  Otherwise does nothing.
*/
void QIMPenChar::setKey(uint code)
{
    const QIMPenSpecialKeys *k = qimpen_specialKeys;
    while(k->code != Qt::Key_unknown) {
        if (code == k->code)
            break;
        ++k;
    }

    if (k->code == Qt::Key_unknown)
        return;

    switch(code) {
        default:
            mUnicode = 0;
            break;
        case Qt::Key_Tab:
            mUnicode = 9;
            break;
        case Qt::Key_Return:
            mUnicode = 13;
            break;
        case Qt::Key_Backspace:
            mUnicode = 8;
            break;
        case Qt::Key_Escape:
            mUnicode = 27;
            break;
    }
    mKey = code;
}

/*!
  Sets the unicode for the character to \a code and clears the key value.
*/
void QIMPenChar::setRepCharacter(QChar code)
{
    mUnicode = code;
    uint scan_uni = mUnicode.unicode();

    if ( scan_uni >= 'a' && scan_uni <= 'z' ) {
        mKey = Qt::Key_A + scan_uni - 'a';
    } else if ( scan_uni >= 'A' && scan_uni <= 'Z' ) {
        mKey = Qt::Key_A + scan_uni - 'A';
    } else if ( scan_uni == ' ' ) {
        mKey = Qt::Key_Space;
    } else {
        mKey = Qt::Key_unknown;
    }
}


/*!
  Write the character's data to the stream.
*/
QDataStream &operator<< (QDataStream &s, const QIMPenChar &ws)
{
    /* handle 2.3 legacy of only 16bit keys instead of 28.
       convert both times as needs to work with peoples old files.
       Only needs to handle keys in qimpen_specialKeys */
    uint ch = ws.mUnicode.unicode();
    const QIMPenSpecialKeys *k = qimpen_specialKeys;
    while(k->code != Qt::Key_unknown) {
        if (ws.mKey == k->code) {
            ch = k->q23code << 16;
            break;
        }
        k++;
    }
    s << ch;

    // never write data, its old hat.
    if ( ws.flags & QIMPenChar::Data )
        s << (ws.flags ^ QIMPenChar::Data);
    else
        s << ws.flags;
    s << ws.strokes.count();
    QIMPenStrokeConstIterator it = ws.strokes.constBegin();
    while ( it != ws.strokes.constEnd() ) {
        s << **it;
        ++it;
    }

    return s;
}

/*!
  Read the character's data from the stream.
*/
QDataStream &operator>> (QDataStream &s, QIMPenChar &ws)
{
    /* handle 2.3 legacy of only 16bit keys instead of 28.
       convert both times as needs to work with peoples old files.
       Only needs to handle keys in qimpen_specialKeys */
    uint ch;
    s >> ch;
    ws.setRepCharacter(ch & 0x0000ffff);
    if (ch & 0xffff0000) {
        // is special key
        ch = ch >> 16;
        const QIMPenSpecialKeys *k = qimpen_specialKeys;
        while(k->code != Qt::Key_unknown) {
            if (ch == k->q23code) {
                ws.setKey(k->code);
                break;
            }
            k++;
        }
    }

    s >> ws.flags;
    if ( ws.flags & QIMPenChar::Data ) {
        QString d;
        s >> d;
        // then throw away.
    }
    unsigned size;
    s >> size;
    for ( unsigned i = 0; i < size; i++ ) {
        QIMPenStroke *st = new QIMPenStroke();
        s >> *st;
        ws.strokes.append( st );
    }

    return s;
}

//===========================================================================

bool QIMPenCharMatch::operator>( const QIMPenCharMatch &m ) const
{
    return error > m.error;
}

bool QIMPenCharMatch::operator<( const QIMPenCharMatch &m ) const
{
    return error < m.error;
}

bool QIMPenCharMatch::operator<=( const QIMPenCharMatch &m ) const
{
    return error <= m.error;
}

//===========================================================================

/*!
  \class QIMPenCharSet
  \brief The QIMPenCharSet class maintains a set of related characters.
  \ingroup qtopiahandwriting
*/

QIMPenCharSet::QIMPenCharSet()
{
    desc = qApp->translate("Handwriting","Unnamed","Character set name");
    csTitle = "abc";
    csType = Unknown;
    maxStrokes = 0;
}

/*!
  Construct and load a characters set from file \a fn.
*/
QIMPenCharSet::QIMPenCharSet( const QString &fn )
{
    desc = qApp->translate("Handwriting","Unnamed","Character set name");
    csTitle = "abc"; // No tr;
    csType = Unknown;
    maxStrokes = 0;
    load( fn );
}

QIMPenCharSet::~QIMPenCharSet()
{
    // autodelete
    while ( chars.count() )
        delete chars.takeLast();
}

void QIMPenCharSet::clear()
{
    if (count() > 0) {
        chars.clear();
    }
}

void QIMPenCharSet::setDescription( const QString &d )
{
    if (d != desc) {
        desc = d;
    }
}

void QIMPenCharSet::setTitle( const QString &t )
{
    if (t != csTitle) {
        csTitle = t;
    }
}

void QIMPenCharSet::setType( Type t )
{
    if (t != csType) {
        csType = t;
    }
}

const QString &QIMPenCharSet::filename( ) const
{
    return userFilename;
}

void QIMPenCharSet::setFilename( const QString &fn )
{
    if (fn != userFilename) {
        userFilename = fn;
    }
}

#ifdef Q_WS_QWS
QString QIMPenCharSet::systemPath( ) const
{
    static const QString sysPath(Qtopia::qtopiaDir() + "etc/qimpen/"); // no tr

    return sysPath + userFilename;
}

QString QIMPenCharSet::userPath() const
{
    return Qtopia::applicationFileName("qimpen",userFilename); // no tr
}
#endif

/*!
  Load a character set from file \a fn, in the domain \a d.
*/
bool QIMPenCharSet::load( const QString &fn )
{
    clear();
    if (!fn.isEmpty())
        setFilename( fn );

    bool ok = false;
#ifdef Q_WS_QWS
    for (int isUser = 0; isUser < 2; ++isUser) {
        QString path;

        if (isUser == 1)
            path = userPath();
        else
            path = systemPath();
#else
        QString path = fn;
#endif

        QFile file( path );
        if ( file.open( QIODevice::ReadOnly ) ) {
            QDataStream ds( &file );
            ds.setVersion(QDataStream::Qt_2_1);
            QString version;
            ds >> version;
            ds >> csTitle;
            ds >> desc;
            int major = version.mid( 4, 1 ).toInt();
            int minor = version.mid( 6 ).toInt();
            if ( major >= 1 && minor > 0 ) {
                ds >> (qint8 &)csType;
            } else {
                if ( csTitle == "abc" ) // no tr
                    csType = Lower;
                else if ( csTitle == "ABC" ) // no tr
                    csType = Upper;
                else if ( csTitle == "123" ) // no tr
                    csType = Numeric;
                else if ( fn == "Combining" ) // No tr
                    csType = Combining;
            }
            while ( !ds.atEnd() && file.error() == QFile::NoError ) {
                QIMPenChar *pc = new QIMPenChar;
                ds >> *pc;
#ifdef Q_WS_QWS
                if ( isUser == 1 )
#endif
                    markDeleted( *pc ); // override system
                addChar( pc );
            }
            if ( file.error() == QFile::NoError )
                ok = true;
        }
#ifdef Q_WS_QWS
    }
#endif

    return ok;
}

/*!
  Save this character set, in the domain \a d.
*/
bool QIMPenCharSet::save( ) const
{
    bool ok = false;

    // in 4.0 format, store keys as strings and use Global::stringToKey or equiv.
#ifdef Q_WS_QWS
    QString fn = userPath();
#else
    QString fn = userFilename;
#endif
    QString tmpFn = fn + ".new"; // no tr
    QFile file( tmpFn );
    if ( file.open( QIODevice::WriteOnly|QIODevice::Unbuffered ) ) {
        QDataStream ds( &file );
        ds.setVersion(QDataStream::Qt_2_1);
        ds << QString( "QPT 1.1" ); // no tr
        ds << csTitle;
        ds << desc;
        ds << (qint8)csType;
        QIMPenCharIterator ci = chars.constBegin();
        for ( ; ci != chars.constEnd(); ++ci ) {
            QIMPenChar *pc = *ci;
#ifdef Q_WS_QWS
            // only save user char's, not system char's.
            if ( !pc->testFlag( QIMPenChar::System ) ) {
                ds << *pc;
            }
#else
                ds << *pc;
#endif
            if ( file.error() != QFile::NoError )
                break;
        }
        if ( file.error() == QFile::NoError )
            ok = true;
    }

    if ( ok ) {
        if ( ::rename( tmpFn.toLatin1(), fn.toLatin1() ) < 0 ) {
            qWarning( "problem renaming file %s to %s, errno: %d",
                    (const char *)tmpFn.toLatin1(), (const char *)fn.toLatin1(), errno );
            // remove the tmp file, otherwise, it will just lay around...
            QFile::remove( tmpFn.toLatin1() );
            ok = false;
        }
    }

    return ok;
}

QIMPenChar *QIMPenCharSet::at( int i )
{
    return chars.at(i);
}

void QIMPenCharSet::markDeleted( const QIMPenChar &ch )
{
    QIMPenCharIterator ci = chars.constBegin();
    for ( ; ci != chars.constEnd(); ++ci ) {
        QIMPenChar *pc = *ci;
        if ( pc->key() == ch.key() && pc->repCharacter() == ch.repCharacter()
                && pc->testFlag( QIMPenChar::System ) )
            pc->setFlag( QIMPenChar::Deleted );
    }
}

/*!
  Find the best matches for \a ch in this character set.
*/
QIMPenCharMatchList QIMPenCharSet::match( QIMPenChar *ch )
{
    QIMPenCharMatchList matches;

    QIMPenCharIterator ci = chars.begin();
    // for each character in set.
    for ( ; ci != chars.end(); ++ci ) {
        QIMPenChar *tmplChar = *ci;
        if ( tmplChar->testFlag( QIMPenChar::Deleted ) ) {
            continue;
        }
        int err;
        // if the stroke to match against has equal or less strokes
        if ( ch->penStrokes().count() <= tmplChar->penStrokes().count() ) {
            // check how well it matches,
            err = ch->match( tmplChar );
            // and if it is less than the threshold
            if ( err <= QIMPEN_MATCH_THRESHOLD ) {
                // compare stroke count again
                if (tmplChar->penStrokes().count() != ch->penStrokes().count())
                    err = qMin(err*3, QIMPEN_MATCH_THRESHOLD);
                QIMPenCharMatchIterator it;
                // Correct the error if an existing char match
                // has greater error (e.g. two ways of writing 'a')
                for ( it = matches.begin(); it != matches.end(); ++it ) {
                    if ( it->penChar->repCharacter() == tmplChar->repCharacter() &&
                            it->penChar->key() == tmplChar->key() &&
                         it->penChar->penStrokes().count() == tmplChar->penStrokes().count() ) {
                        if ( it->error > err )
                            it->error = err;
                        break;
                    }
                }
                // if this char isn't already a match
                if ( it == matches.end() ) {
                    // add it as a match
                    QIMPenCharMatch m;
                    m.error = err;
                    m.penChar = tmplChar;
                    matches.append( m );
                }
            }
        }
    }
    // sort and return.
    qStableSort( matches );
    return matches;
}

/*!
  Add a character \a ch to this set.
  QIMPenCharSet will delete this character when it is no longer needed.
*/
void QIMPenCharSet::addChar( QIMPenChar *ch )
{
    if ( (uint)(ch->penStrokes().count()) > maxStrokes )
        maxStrokes = ch->penStrokes().count();
    chars.append( ch );
}

/*!
  Remove a character by reference \a ch from this set.
  QIMPenCharSet will delete this character.
*/
void QIMPenCharSet::removeChar( QIMPenChar *ch )
{
    delete chars.takeAt( chars.indexOf( ch ));
}

/*!
  Move the character \a ch up the list of characters.
*/
void QIMPenCharSet::up( QIMPenChar *ch )
{
    int idx = chars.indexOf( ch );
    if ( idx >= 0 ) {
        if ( idx + 1 < chars.count() )
            chars.swap( idx, idx + 1 );
    }
}

/*!
  Move the character \a ch down the list of characters.
*/
void QIMPenCharSet::down( QIMPenChar *ch )
{
    int idx = chars.indexOf( ch );
    if ( idx > 0 ) {
        chars.swap( idx, idx - 1 );
    }
}

