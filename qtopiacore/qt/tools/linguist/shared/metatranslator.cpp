/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "metatranslator.h"
#include "translator.h"

#include <QApplication>
#include <QByteArray>
#include <QFile>
#include <QMessageBox>
#include <QTextCodec>
#include <QTextStream>
#include <QtXml>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

static bool encodingIsUtf8( const QXmlAttributes& atts )
{
    for ( int i = 0; i < atts.length(); i++ ) {
        // utf8="true" is a pre-3.0 syntax
        if ( atts.qName(i) == QString("utf8") ) {
            return ( atts.value(i) == QString("true") );
        } else if ( atts.qName(i) == QString("encoding") ) {
            return ( atts.value(i) == QString("UTF-8") );
        }
    }
    return false;
}

class TsHandler : public QXmlDefaultHandler
{
public:
    TsHandler( MetaTranslator *translator )
        : tor( translator ), type( MetaTranslatorMessage::Finished ),
          inMessage( false ), ferrorCount( 0 ), contextIsUtf8( false ),
          messageIsUtf8( false ), m_isPlural(false) { }

    virtual bool startElement( const QString& namespaceURI,
                               const QString& localName, const QString& qName,
                               const QXmlAttributes& atts );
    virtual bool endElement( const QString& namespaceURI,
                             const QString& localName, const QString& qName );
    virtual bool characters( const QString& ch );
    virtual bool fatalError( const QXmlParseException& exception );

    QString language() const { return m_language; }
private:
    MetaTranslator *tor;
    MetaTranslatorMessage::Type type;
    bool inMessage;
    QString m_language;
    QString context;
    QString source;
    QString comment;
    QStringList translations;
    QString m_fileName;
    int     m_lineNumber;

    QString accum;
    int ferrorCount;
    bool contextIsUtf8;
    bool messageIsUtf8;
    bool m_isPlural;
};

bool TsHandler::startElement( const QString& /* namespaceURI */,
                              const QString& /* localName */,
                              const QString& qName,
                              const QXmlAttributes& atts )
{
    if ( qName == QString("byte") ) {
        for ( int i = 0; i < atts.length(); i++ ) {
            if ( atts.qName(i) == QString("value") ) {
                QString value = atts.value( i );
                int base = 10;
                if ( value.startsWith("x") ) {
                    base = 16;
                    value = value.mid( 1 );
                }
                int n = value.toUInt( 0, base );
                if ( n != 0 )
                    accum += QChar( n );
            }
        }
    } else {
        if ( qName == QString("TS") ) {
            m_language = atts.value(QLatin1String("language"));
        } else if ( qName == QString("context") ) {
            context.clear();
            source.clear();
            comment.clear();
            translations.clear();
            contextIsUtf8 = encodingIsUtf8( atts );
        } else if ( qName == QString("message") ) {
            inMessage = true;
            type = MetaTranslatorMessage::Finished;
            source.clear();
            comment.clear();
            translations.clear();
            messageIsUtf8 = encodingIsUtf8( atts );
            m_isPlural = atts.value(QLatin1String("numerus")).compare(QLatin1String("yes")) == 0;
        } else if (qName == QString("location") && inMessage) {
            bool bOK;
            int lineNo = atts.value(QString("line")).toInt(&bOK);
            if (!bOK) lineNo = -1;
            m_fileName = atts.value(QString("filename"));
            m_lineNumber = lineNo;
        } else if ( qName == QString("translation") ) {
            for ( int i = 0; i < atts.length(); i++ ) {
                if ( atts.qName(i) == QString("type") ) {
                    if ( atts.value(i) == QString("unfinished") )
                        type = MetaTranslatorMessage::Unfinished;
                    else if ( atts.value(i) == QString("obsolete") )
                        type = MetaTranslatorMessage::Obsolete;
                    else
                        type = MetaTranslatorMessage::Finished;
                }
            }
        }
        accum.clear();
    }
    return true;
}

bool TsHandler::endElement( const QString& /* namespaceURI */,
                            const QString& /* localName */,
                            const QString& qName )
{
    if ( qName == QString("codec") || qName == QString("defaultcodec") ) {
        // "codec" is a pre-3.0 syntax
        tor->setCodec( accum.toLatin1() );
    } else if ( qName == QString("name") ) {
        context = accum;
    } else if ( qName == QString("source") ) {
        source = accum;
    } else if ( qName == QString("comment") ) {
        if ( inMessage ) {
            comment = accum;
        } else {
            if ( contextIsUtf8 )
                tor->insert( MetaTranslatorMessage(context.toUtf8(),
                             ContextComment, accum.toUtf8(), QString(), 0,
                             QStringList(), true,
                             MetaTranslatorMessage::Unfinished) );
            else
                tor->insert( MetaTranslatorMessage(context.toAscii(),
                             ContextComment, accum.toAscii(), QString(), 0,
                             QStringList(), false,
                             MetaTranslatorMessage::Unfinished) );
        }
    } else if ( qName == QString("numerusform") ) {
        translations.append(accum);
        m_isPlural = true;
    } else if ( qName == QString("translation") ) {
        if (translations.isEmpty())
            translations.append(accum);
    } else if ( qName == QString("message") ) {
        if ( messageIsUtf8 )
            tor->insert( MetaTranslatorMessage(context.toUtf8(), source.toUtf8(),
                                            comment.toUtf8(), m_fileName, m_lineNumber, 
                                            translations, true, type, m_isPlural) );
        else
            tor->insert( MetaTranslatorMessage(context.toAscii(), source.toAscii(),
                                            comment.toAscii(), m_fileName, m_lineNumber, 
                                            translations, false, type, m_isPlural) );
        inMessage = false;
    }
    return true;
}

bool TsHandler::characters( const QString& ch )
{
    QString t = ch;
    t.replace( "\r", "" );
    accum += t;
    return true;
}

bool TsHandler::fatalError( const QXmlParseException& exception )
{
    if ( ferrorCount++ == 0 ) {
        QString msg;
        msg.sprintf( "Parse error at line %d, column %d (%s).",
                     exception.lineNumber(), exception.columnNumber(),
                     exception.message().toLatin1().data() );
        if ( qApp == 0 )
            fprintf( stderr, "XML error: %s\n", msg.toLatin1().data() );
        else
            QMessageBox::information(0,
                                      QObject::tr("Qt Linguist"), msg );
    }
    return false;
}

static QString numericEntity( int ch )
{
    return QString( ch <= 0x20 ? "<byte value=\"x%1\"/>" : "&#x%1;" )
           .arg( ch, 0, 16 );
}

static QString protect( const QByteArray& str )
{
    QString result;
    int len = (int) str.length();
    for ( int k = 0; k < len; k++ ) {
        switch( str[k] ) {
        case '\"':
            result += QString( "&quot;" );
            break;
        case '&':
            result += QString( "&amp;" );
            break;
        case '>':
            result += QString( "&gt;" );
            break;
        case '<':
            result += QString( "&lt;" );
            break;
        case '\'':
            result += QString( "&apos;" );
            break;
        default:
            if ( (uchar) str[k] < 0x20 && str[k] != '\n' )
                result += numericEntity( (uchar) str[k] );
            else
                result += str[k];
        }
    }
    return result;
}

static QString evilBytes( const QByteArray& str, bool utf8 )
{
    if ( utf8 ) {
        return protect( str );
    } else {
        QString result;
        QByteArray t = protect( str ).toLatin1();
        int len = (int) t.length();
        for ( int k = 0; k < len; k++ ) {
            if ( (uchar) t[k] >= 0x7f )
                result += numericEntity( (uchar) t[k] );
            else
                result += QChar( t[k] );
        }
        return result;
    }
}

MetaTranslatorMessage::MetaTranslatorMessage()
    : utfeight( false ), ty( Unfinished ), m_plural(false)
{
}

MetaTranslatorMessage::MetaTranslatorMessage( const char *context,
                                              const char *sourceText,
                                              const char *comment,
                                              const QString &fileName,
                                              int lineNumber,
                                              const QStringList& translations,
                                              bool utf8, Type type, bool plural )
    : TranslatorMessage( context, sourceText, comment, fileName, lineNumber, translations ),
      utfeight( false ), ty( type ), m_plural(plural)
{
    /*
      Don't use UTF-8 if it makes no difference. UTF-8 should be
      reserved for the real problematic case: non-ASCII (possibly
      non-Latin1) characters in .ui files.
    */
    if ( utf8 ) {
        if ( sourceText != 0 ) {
            int i = 0;
            while ( sourceText[i] != '\0' ) {
                if ( (uchar) sourceText[i] >= 0x80 ) {
                    utfeight = true;
                    break;
                }
                i++;
            }
        }
        if ( !utfeight && comment != 0 ) {
            int i = 0;
            while ( comment[i] != '\0' ) {
                if ( (uchar) comment[i] >= 0x80 ) {
                    utfeight = true;
                    break;
                }
                i++;
            }
        }

        if ( !utfeight && context != 0 ) {
            int i = 0;
            while ( context[i] != '\0' ) {
                if ( (uchar) context[i] >= 0x80 ) {
                    utfeight = true;
                    break;
                }
                i++;
            }
        }
    }
}


MetaTranslatorMessage::MetaTranslatorMessage( const MetaTranslatorMessage& m )
    : TranslatorMessage( m ), utfeight( m.utfeight ), ty( m.ty ), m_plural(m.m_plural)
{
}

MetaTranslatorMessage& MetaTranslatorMessage::operator=(
        const MetaTranslatorMessage& m )
{
    TranslatorMessage::operator=( m );
    utfeight = m.utfeight;
    ty = m.ty;
    m_plural = m.m_plural;
    return *this;
}

bool MetaTranslatorMessage::operator==( const MetaTranslatorMessage& m ) const
{
    return qstrcmp( context(), m.context() ) == 0 &&
           qstrcmp( sourceText(), m.sourceText() ) == 0 &&
           qstrcmp( comment(), m.comment() ) == 0;
}

bool MetaTranslatorMessage::operator<( const MetaTranslatorMessage& m ) const
{
    int delta = qstrcmp( context(), m.context() );
    if ( delta == 0 )
        delta = qstrcmp( sourceText(), m.sourceText() );
    if ( delta == 0 )
        delta = qstrcmp( comment(), m.comment() );
    return delta < 0;
}

MetaTranslator::MetaTranslator()
{
    clear();
}

MetaTranslator::MetaTranslator( const MetaTranslator& tor )
    : mm( tor.mm ), codecName( tor.codecName ), codec( tor.codec )
{
}

MetaTranslator& MetaTranslator::operator=( const MetaTranslator& tor )
{
    mm = tor.mm;
    codecName = tor.codecName;
    codec = tor.codec;
    return *this;
}

void MetaTranslator::clear()
{
    mm.clear();
    codecName = "ISO-8859-1";
    codec = 0;
}

bool MetaTranslator::load( const QString& filename )
{
    QFile f( filename );
    if ( !f.open(QIODevice::ReadOnly) )
        return false;

    QXmlInputSource in( &f );
    QXmlSimpleReader reader;
    reader.setFeature( "http://xml.org/sax/features/namespaces", false );
    reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", true );
    TsHandler *hand = new TsHandler( this );
    reader.setContentHandler( static_cast<QXmlDefaultHandler*>(hand) );
    reader.setErrorHandler( static_cast<QXmlDefaultHandler*>(hand) );

    bool ok = reader.parse( in );
    reader.setContentHandler( 0 );
    reader.setErrorHandler( 0 );

    m_language = hand->language();
    makeFileNamesAbsolute(QFileInfo(filename).absoluteDir());

    delete hand;
    f.close();
    return ok;
}

bool MetaTranslator::save( const QString& filename) const
{
    QFile f( filename );
    if ( !f.open(QIODevice::WriteOnly) )
        return false;

    QTextStream t( &f );
    t.setCodec( QTextCodec::codecForName("ISO-8859-1") );

    //### The xml prolog allows processors to easily detect the correct encoding
    t << "<?xml version=\"1.0\"";
    t << " encoding=\"utf-8\"";
    t << "?>\n<!DOCTYPE TS><TS version=\"1.1\"";
    if (!languageCode().isEmpty() && languageCode() != QLatin1String("C"))
        t << " language=\"" << languageCode() << "\"";

    t << ">\n";
    if ( codecName != "ISO-8859-1" )
        t << "<defaultcodec>" << codecName << "</defaultcodec>\n";
    TMM::ConstIterator m = mm.begin();
    while ( m != mm.end() ) {
        TMMInv inv;
        TMMInv::Iterator i;
        bool contextIsUtf8 = m.key().utf8();
        QByteArray context = m.key().context();
        QByteArray comment = "";

        do {
            if (QByteArray(m.key().sourceText()) == ContextComment) {
                if ( m.key().type() != MetaTranslatorMessage::Obsolete ) {
                    contextIsUtf8 = m.key().utf8();
                    comment = QByteArray(m.key().comment());
                }
            } else {
                inv.insert( *m, m.key() );
            }
        } while ( ++m != mm.end() && QByteArray(m.key().context()) == context );

        t << "<context";
        if ( contextIsUtf8 )
            t << " encoding=\"UTF-8\"";
        t << ">\n";
        t << "    <name>" << evilBytes( context, contextIsUtf8 )
          << "</name>\n";
        if ( !comment.isEmpty() )
            t << "    <comment>" << evilBytes( comment, contextIsUtf8 )
              << "</comment>\n";

        for ( i = inv.begin(); i != inv.end(); ++i ) {
            MetaTranslatorMessage msg = *i;
            // no need for such noise
            if ( msg.type() == MetaTranslatorMessage::Obsolete && msg.translation().isEmpty() ) {
                continue;
            }

            t << "    <message";
            if ( msg.utf8() )
                t << " encoding=\"UTF-8\"";
            if ( msg.isPlural() )
                t << " numerus=\"yes\"";
            t << ">\n";
            if (!msg.fileName().isEmpty() && msg.lineNumber() >= 0) {
                QDir tsPath = QFileInfo(filename).absoluteDir();
                QString fn = tsPath.relativeFilePath(msg.fileName()).replace('\\','/');
                t << "        <location filename=\"" << fn << "\" line=\"" << msg.lineNumber() << "\"/>\n";
            }
            t  << "        <source>" << evilBytes( (*i).sourceText(), (*i).utf8() )
              << "</source>\n";
            if ( !QByteArray((*i).comment()).isEmpty() )
                t << "        <comment>" << evilBytes( (*i).comment(), (*i).utf8() )
                  << "</comment>\n";
            t << "        <translation";
            if ( (*i).type() == MetaTranslatorMessage::Unfinished )
                t << " type=\"unfinished\"";
            else if ( (*i).type() == MetaTranslatorMessage::Obsolete )
                t << " type=\"obsolete\"";
            t << ">";

            if (msg.isPlural()) {
                t << "\n";
                QLocale::Language l;
                QLocale::Country c;
                languageAndCountry(m_language, &l, &c);
                QStringList translns = normalizedTranslations(*i, l, c);
                for (int j = 0; j < qMax(1, translns.count()); ++j)
                    t << "            <numerusform>" << protect( translns.value(j).toUtf8() ) << "</numerusform>\n";
                t << "        ";
            } else {
                t << protect( (*i).translation().toUtf8() );
            }

            t << "</translation>\n";
            t << "    </message>\n";
        }
        t << "</context>\n";
    }
    t << "</TS>\n";
    f.close();
    return true;
}

bool MetaTranslator::release( const QString& filename, bool verbose,
                              bool ignoreUnfinished,
                              Translator::SaveMode mode ) const
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        bool ok = release(&file, verbose, ignoreUnfinished, mode);
        file.close();
        return ok;
    }
    return false;
}

void MetaTranslator::languageAndCountry(const QString &languageCode, QLocale::Language *lang, QLocale::Country *country)
{
    QLocale locale(languageCode);
    if (lang) 
        *lang = locale.language();

    if (country) {
        if (languageCode.indexOf(QLatin1Char('_')) != -1) {
            *country = locale.country();
        } else {
            *country = QLocale::AnyCountry;
        }
    }
}

bool MetaTranslator::release( QIODevice *iod, bool verbose /*= false*/,
              bool ignoreUnfinished /*= false*/,
              Translator::SaveMode mode /*= Translator::Stripped */) const
{
    Translator tor( 0 );
    QLocale::Language l;
    QLocale::Country c;
    languageAndCountry(m_language, &l, &c);
    QByteArray rules;
    if (getNumerusInfo(l, c, &rules, 0)) {
        tor.setNumerusRules(rules);
    }

    int finished = 0;
    int unfinished = 0;
    int untranslated = 0;
    TMM::ConstIterator m;

    for ( m = mm.begin(); m != mm.end(); ++m ) {
        MetaTranslatorMessage::Type typ = m.key().type();
        if ( typ != MetaTranslatorMessage::Obsolete ) {
            if ( typ == MetaTranslatorMessage::Unfinished ) {
                if (m.key().translation().isEmpty()) {
                    untranslated++;
                } else {
                    unfinished++;
                }
            } else {
                finished++;
            }
            QByteArray context = m.key().context();
            QByteArray sourceText = m.key().sourceText();
            QByteArray comment = m.key().comment();
            QStringList translations = m.key().translations();

            if ( !ignoreUnfinished
                || typ != MetaTranslatorMessage::Unfinished ) {
                /*
                  Drop the comment in (context, sourceText, comment),
                  unless the context is empty,
                  unless (context, sourceText, "") already exists or
                  unless we already dropped the comment of (context,
                  sourceText, comment0).
                */
                if ( comment.isEmpty()
                     || context.isEmpty()
                     || contains(context, sourceText, "")
                     || !tor.findMessage(context, sourceText, "").translation()
                            .isNull() ) {
                    tor.insert( m.key() );
                } else {
                    tor.insert( TranslatorMessage(context, sourceText, "",
                                                   QString(), -1, translations) );    //filename and lineNumbers will be ignored from now.
                }
            }
        }
    }

    bool saved = tor.save( iod, mode );
    if ( saved && verbose ) {
        int generatedCount = finished + unfinished;
        fprintf( stderr,
            "    Generated %d translation%s (%d finished and %d unfinished)\n",
            generatedCount, generatedCount == 1 ? "" : "s", finished, unfinished);
        if (untranslated)
            fprintf( stderr, "    Ignored %d untranslated source text%s\n", 
                untranslated, untranslated == 1 ? "" : "s");
    }
    return saved;
}

QString MetaTranslator::languageCode() const
{
    return m_language;
}

void MetaTranslator::setLanguageCode(const QString &languageCode)
{
    m_language = languageCode;
}

bool MetaTranslator::contains( const char *context, const char *sourceText,
                               const char *comment ) const
{
    return mm.contains(MetaTranslatorMessage(context, sourceText, comment, QString(), 0));
}

MetaTranslatorMessage MetaTranslator::find( const char *context, const char *sourceText,
                   const char *comment ) const
{
    QList<MetaTranslatorMessage> all = messages();
    QList<MetaTranslatorMessage>::const_iterator i1;
    int delta = -1;
    for (i1 = all.constBegin(); i1 != all.constEnd(); ++i1) {
        MetaTranslatorMessage m = *i1;
        delta = qstrcmp(m.context(), context);
        if (delta == 0) {
            delta = qstrcmp(m.comment(), comment);
            if (delta == 0) {
                delta = QString::compare(m.sourceText(), sourceText);
                if (delta == 0) return (*i1);
            }
        }
    }
    return MetaTranslatorMessage();    
}

MetaTranslatorMessage MetaTranslator::find(const char *context, const char *comment, 
                                const QString &fileName, int lineNumber) const
{
    QList<MetaTranslatorMessage> all = messages();
    QList<MetaTranslatorMessage>::const_iterator i1;
    int delta = -1;
    if (lineNumber >= 0 && !fileName.isEmpty()) {
        for (i1 = all.constBegin(); i1 != all.constEnd(); ++i1) {
            MetaTranslatorMessage m = *i1;
            delta = qstrcmp(m.context(), context);
            if (delta == 0) {
                delta = qstrcmp(m.comment(), comment);
                if (delta == 0) {
                    delta = QString::compare(m.fileName(), fileName);
                    if (delta == 0) {
                        delta = m.lineNumber() - lineNumber;
                        if (delta == 0) return (*i1);
                    }
                }
            }
        }
    }
    return MetaTranslatorMessage();
}

void MetaTranslator::insert( const MetaTranslatorMessage& m )
{
    int pos = mm.count();
    if (mm.contains(m)) {
        pos = mm.value(m);

        mm.remove(m);
    }
    mm.insert(m, pos);
}

void MetaTranslator::stripObsoleteMessages()
{
    TMM newmm;
    TMM::Iterator m = mm.begin();
    while ( m != mm.end() ) {
        if ( m.key().type() != MetaTranslatorMessage::Obsolete )
            newmm.insert( m.key(), *m );
        ++m;
    }
    mm = newmm;
}

void MetaTranslator::stripEmptyContexts()
{
    TMM newmm;

    TMM::Iterator m = mm.begin();
    while ( m != mm.end() ) {
        if ( QByteArray(m.key().sourceText()) == ContextComment ) {
            TMM::Iterator n = m;
            ++n;
            // the context comment is followed by other messages
            if ( n != newmm.end() &&
                 qstrcmp(m.key().context(), n.key().context()) == 0 )
                newmm.insert( m.key(), *m );
        } else {
            newmm.insert( m.key(), *m );
        }
        ++m;
    }
    mm = newmm;
}

void MetaTranslator::makeFileNamesAbsolute(const QDir &oldPath)
{
    TMM newmm;
    for (TMM::iterator m = mm.begin(); m != mm.end(); ++m) {
        MetaTranslatorMessage msg = m.key();
        QString fileName = m.key().fileName();
        QFileInfo fi (fileName);
        if (fi.isRelative()) {
            fileName = oldPath.absoluteFilePath(fileName);
        }
        
        msg.setFileName(fileName);
        newmm.insert(msg, m.value());
    }
    mm = newmm;
}

void MetaTranslator::setCodec( const char *name )
{
    const int latin1 = 4;

    codecName = name;
    codec = QTextCodec::codecForName( name );
    if ( codec == 0 || codec->mibEnum() == latin1 )
        codec = 0;
}

QString MetaTranslator::toUnicode( const char *str, bool utf8 ) const
{
    if ( utf8 )
        return QString::fromUtf8( str );
    else if ( codec == 0 )
        return QString( str );
    else
        return codec->toUnicode( str );
}

QList<MetaTranslatorMessage> MetaTranslator::messages() const
{
    int n = mm.count();
    TMM::ConstIterator *t = new TMM::ConstIterator[n + 1];
    TMM::ConstIterator m;
    for ( m = mm.begin(); m != mm.end(); ++m )
        t[*m] = m;

    QList<MetaTranslatorMessage> val;
    for ( int i = 0; i < n; i++ )
        val.append( t[i].key() );

    delete[] t;
    return val;
}

QList<MetaTranslatorMessage> MetaTranslator::translatedMessages() const
{
    QList<MetaTranslatorMessage> val;
    TMM::ConstIterator m;
    for ( m = mm.begin(); m != mm.end(); ++m ) {
        if ( m.key().type() == MetaTranslatorMessage::Finished )
            val.append( m.key() );
    }
    return val;
}

// the grammatical numerus is the number of plural forms + singular forms.
// i.e english has two forms: singular og plural.
// and polish has three forms: 
// 1. singular (1), 
// 2. plural form 1 (numbers that ends with 2,3,4 except 12,13,14)
// 3. plural form 2 (all others)
// Thus, english returns 2, polish returns 3
int MetaTranslator::grammaticalNumerus(QLocale::Language language, QLocale::Country country)
{
    QStringList forms;
    getNumerusInfo(language, country, 0, &forms);
    return forms.count();
}

QStringList MetaTranslator::normalizedTranslations(const MetaTranslatorMessage& m, 
                                                   QLocale::Language language, 
                                                   QLocale::Country country)
{
    QStringList translations = m.translations();
    int numTranslations = 1;
    if (m.isPlural()) {
        numTranslations = grammaticalNumerus(language, country);
    }

    // make sure that the stringlist always have the size of the language's current numerus, or 1 if its not plural
    if (translations.count() > numTranslations) {
        for (int i = translations.count(); i >  numTranslations; --i)
            translations.removeLast();
    } else if (translations.count() < numTranslations) {
        for (int i = translations.count(); i < numTranslations; ++i)
            translations << QString();
    }
    return translations;    
}
