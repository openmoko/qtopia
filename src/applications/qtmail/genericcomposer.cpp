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

#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif
#include <qmimetype.h>
#ifndef QTOPIA_DESKTOP
#include <qtopiaapplication.h>
#endif


#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QAction>
#include <QTextEdit>
#include <QLabel>
#include <QLayout>
#include <QClipboard>
#include <QMenu>
#include <QSettings>
#include <QTextCursor>
#include <QContact>

#include "genericcomposer.h"
#include "smsclient.h"
#ifdef QTOPIA_PHONE
#include "templatetext.h"
#include <qtopia/serial/qgsmcodec.h>
#endif

#define SMS_CHAR_LIMIT 459

//Qtmail sends either GSM or UCS2 encoded SMS messages
//If another encoding is ultimately used to send the message,
//these functions will return inaccurate results
#ifdef QTOPIA_PHONE
void GenericComposer::smsLengthInfo(uint& estimatedBytes, bool& isUnicode)
{
    //calculate the number of consumed bytes
    //considering the gsm charset

    unsigned short c;
    unsigned short d;
    uint count = 0;
    QString unicodestr = text();
    for(int i = 0; i < unicodestr.length(); ++i)
    {
        c = unicodestr[i].unicode();
        if(c >= 256)
        {
            estimatedBytes = unicodestr.length() * 2;
            isUnicode = true;
            return;
        }
        else
        {
            d = QGsmCodec::twoByteFromUnicode(c);
            if(d >= 256)
                count += 2;
            else if(d == 0x10) //0x10 is unrecognised char
            {
                estimatedBytes = unicodestr.length() * 2; //non gsm char, so go unicode
                isUnicode = true;
                return;
            }
            else
                count += 1;
        }
    }
    isUnicode = false;
    estimatedBytes = count;
}
//estimates the number of messages that will be sent

int GenericComposer::smsCountInfo()
{
    bool isUnicode = false;
    uint numBytes = 0;
    int numMessages = 0;
    int len = text().length();

    smsLengthInfo(numBytes,isUnicode);

    if(isUnicode) //all 2 byte UCS2 so ok to use text length
    {
        if (len <= 70 ) {
            numMessages = 1;
        } else {
            // 67 = 70 - fragment_header_size (3).
            numMessages = ( len + 66 ) / 67;
        }
    }
    else
    {
        //use byte length instead of text length
        //as some GSM chars consume 2 bytes
        if ( numBytes <= 160 ) {
            numMessages = 1;
        } else {
        // 153 = 160 - fragment_header_size (7).
            numMessages = ( numBytes + 152 ) / 153;
        }
    }
    return numMessages;
}

#endif

class ComposerTextEdit : public QTextEdit
{
public:
    ComposerTextEdit( QWidget *parent, const char *name = 0 );

    void limitedInsert( const QString &str);
#ifndef QT_NO_CLIPBOARD
    void limitedPaste();
#endif

protected:
    void keyPressEvent( QKeyEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void inputMethodEvent( QInputMethodEvent *e );
};

ComposerTextEdit::ComposerTextEdit( QWidget *parent, const char *name )
    : QTextEdit( parent )
{
    setObjectName( name );
    setFrameStyle(NoFrame);
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                                    QSizePolicy::Expanding ) );
    setLineWrapMode( QTextEdit::WidgetWidth );
}

void ComposerTextEdit::keyPressEvent( QKeyEvent *e )
{
    int charCount = toPlainText().length();
    QChar c = e->text()[0];
    if( charCount >= SMS_CHAR_LIMIT &&
            (c.isLetterOrNumber() || c.isPunct() || c.isSpace()) )
                            //e->key() != Key_Back && e->key() != Key_Delete )
        return;
#ifndef QT_NO_CLIPBOARD
    if( e->modifiers() & Qt::ControlModifier )
    {
        if( e->key() == Qt::Key_V )
        {
            limitedPaste();
            return;
        }
        else if( e->key() == Qt::Key_Y )
            return; //redo could be redo paste, and that could exceed limit
    }
#endif
#ifdef QTOPIA_PHONE
    if( Qtopia::mousePreferred() && e->key() == Qt::Key_Back )
        e->ignore();
    else
#endif
        QTextEdit::keyPressEvent( e );
}

void ComposerTextEdit::limitedInsert( const QString &str)
{
    int curCharCount = toPlainText().length();
    if( curCharCount >= SMS_CHAR_LIMIT )
           return;
    QString strText = str;
    int strCharCount = strText.length();
    while( (strCharCount+curCharCount) > SMS_CHAR_LIMIT )
    {
        strText = strText.left( strText.length() -1 );
        strCharCount = strText.length();
    }
    if( !strText.isEmpty() )
    {
        textCursor().insertText( strText );
        ensureCursorVisible();
        emit textChanged();
    }

}

#ifndef QT_NO_CLIPBOARD
void ComposerTextEdit::limitedPaste()
{
    limitedInsert( QApplication::clipboard()->text() );
}
#endif

void ComposerTextEdit::mousePressEvent( QMouseEvent *e )
{
    if( e->button() == Qt::RightButton )
        return;
    QTextEdit::mousePressEvent( e );
}

void ComposerTextEdit::inputMethodEvent( QInputMethodEvent *e )
{
    if ( !e->commitString().isEmpty() ) {
        // Clear the commit string if it is makes the message longer
        // than the limit
        int charCount = toPlainText().length();
        if ( ( charCount + e->commitString().length() > SMS_CHAR_LIMIT ) ) {
            e->setCommitString( QString() );
        }
    }

    QTextEdit::inputMethodEvent( e );
}

GenericComposer::GenericComposer( QWidget *parent, const char *name )
    : QWidget( parent ),
      m_vCard( false ),
      m_vCardData()
{
    setObjectName( name );
    m_showLimitAction = new QAction( tr("Show Sms Limit"), this );
    m_showLimitAction->setCheckable( true );
    QSettings cfg("Trolltech","qtmail");
    cfg.beginGroup( "GenericComposer" );
    m_showLimitAction->setChecked( cfg.value( "showSmsLimitIndicator", true ).toBool() );
    connect( m_showLimitAction, SIGNAL(triggered(bool)), this, SLOT(updateSmsLimitIndicator()) );

    m_templateTextAction = new QAction( tr("Insert Template"), this );
    connect( m_templateTextAction, SIGNAL(triggered()),
             this, SLOT(templateText()) );

    QVBoxLayout *l = new QVBoxLayout( this );
    m_smsLimitIndicator = new QLabel( this );
    l->addWidget( m_smsLimitIndicator );
    m_smsLimitIndicator->setAlignment( Qt::AlignRight );
    m_textEdit = new ComposerTextEdit( this );
    setFocusProxy( m_textEdit );
    l->addWidget( m_textEdit );
#ifdef QTOPIA4_TODO
    m_textEdit->setMaxLength( SMS_BYTE_LIMIT );
#endif
    connect( m_textEdit, SIGNAL(textChanged()), this, SIGNAL(contentChanged()) );
    connect( m_textEdit, SIGNAL(textChanged()), this, SLOT(updateSmsLimitIndicator()) );
#ifdef QTOPIA_PHONE
    QMenu *menu = QSoftMenuBar::menuFor( m_textEdit, QSoftMenuBar::AnyFocus );
    menu->addSeparator();
    menu->insertAction(menu->actions().count() ? menu->actions()[0] : 0, m_showLimitAction);
    menu->insertAction(m_showLimitAction, m_templateTextAction);
    menu->addSeparator();
    QSoftMenuBar::addMenuTo( this, menu );
#endif
    updateSmsLimitIndicator();
}

GenericComposer::~GenericComposer()
{
    QSettings cfg("Trolltech","qtmail");
    cfg.beginGroup( "GenericComposer" );
    cfg.setValue( "showSmsLimitIndicator", m_showLimitAction->isChecked() );
}

void GenericComposer::updateSmsLimitIndicator()
{
    if ( m_showLimitAction->isChecked() && !m_vCard )
    {
        if( m_smsLimitIndicator->isHidden() )
            m_smsLimitIndicator->show();
    }
    else
    {
        if( !m_smsLimitIndicator->isHidden() )
            m_smsLimitIndicator->hide();
        return;
    }
    int charCount = text().length();
#ifdef QTOPIA_PHONE
    int numMessages = smsCountInfo();
#else
    int numMessages = 1;
#endif
    m_smsLimitIndicator->setText( QString(tr("%1/%2 ","e.g. 5/7"))
                                    .arg( SMS_CHAR_LIMIT - charCount )
                                    .arg( numMessages )
                                );
}

void GenericComposer::templateText()
{
#ifdef QTOPIA_PHONE
    TemplateTextDialog *templateTextDialog = new TemplateTextDialog();
#ifdef QTOPIA_DESKTOP
    templateTextDialog->exec();
#else
    QtopiaApplication::execDialog( templateTextDialog );
#endif
    ComposerTextEdit *composer = qobject_cast<ComposerTextEdit *>( m_textEdit );
    if (templateTextDialog->result() && composer)
        composer->limitedInsert( templateTextDialog->text() );
    delete templateTextDialog;
#endif
}

void GenericComposer::setText( const QString &t )
{
#ifndef QTOPIA_NO_SMS
    if ( t.startsWith( SmsClient::vCardPrefix() ) ) {
        QString copy = t;
        QList<QContact> contacts
            = QContact::readVCard(
                copy.remove( SmsClient::vCardPrefix() ).toLatin1() );

        if ( contacts.count() == 0 ) {
            // Invalid VCard data, so just show raw data
            m_textEdit->setPlainText( t );
        } else if ( contacts.count() == 1 ) {
            QString name = tr( "Message contains vCard for %1" );
            if ( !contacts[0].nickname().isEmpty() ) {
                m_textEdit->setPlainText( name.arg( contacts[0].nickname() ) );
            } else if ( !contacts[0].firstName().isEmpty() &&
                        !contacts[0].lastName().isEmpty() ) {
                m_textEdit->setPlainText( name.arg( contacts[0].firstName() +
                                                    " " +
                                                    contacts[0].lastName() ) );
            } else if ( !contacts[0].firstName().isEmpty() ) {
                m_textEdit->setPlainText( name.arg( contacts[0].firstName() ) );
            } else {
                m_textEdit->setPlainText(
                    tr( "Message contains vCard for a contact" ) );
            }
            m_vCard = true;
            m_vCardData = t;
        } else if ( contacts.count() > 1 ) {
            m_textEdit->setPlainText(
                tr( "Message contains vCard for multiple contacts" ) );
            m_vCard = true;
            m_vCardData = t;
        }
    } else
#endif
    {
        m_textEdit->setPlainText( t );
    }

    // Update GUI state
    m_templateTextAction->setVisible( !m_vCard );
    m_textEdit->setReadOnly( m_vCard );
    m_textEdit->setEditFocus( !m_vCard );
    if ( m_vCard ) {
        setFocusProxy( 0 );
    } else {
        setFocusProxy( m_textEdit );
    }
    updateSmsLimitIndicator();
}

QString GenericComposer::text() const
{
    if ( m_vCard )
        return m_vCardData;
    else
        return m_textEdit->toPlainText();
}

GenericComposerInterface::GenericComposerInterface( QObject *parent, const char *name )
    : TextComposerInterface( parent, name )
{
    QWidget *par = 0;
    if( parent && parent->isWidgetType() )
        par = (QWidget *)parent;
    m_composer = new GenericComposer( par, name );
    connect( m_composer, SIGNAL(contentChanged()), this, SIGNAL(contentChanged()) );
}

GenericComposerInterface::~GenericComposerInterface()
{
    delete m_composer;
}

/*
QCString GenericComposerInterface::id()
{
    QCString t = QString::number( type() ).toLatin1();
    return t + "-Default"; // default sms/email composer
}

ComposerInterface::ComposerType GenericComposerInterface::type()
{
    return ComposerInterface::SMS | ComposerInterface::Email;
}

QString GenericComposerInterface::displayName()
{
    return tr("SMS/Email");
}
*/

bool GenericComposerInterface::hasContent() const
{
    return !m_composer->text().isEmpty();
}

void GenericComposerInterface::setMailMessage( Email &mail )
{
    setText( mail.plainTextBody() );
}

void GenericComposerInterface::getContent( MailMessage &mail ) const
{
    if( !hasContent() )
        return;
    mail.setPlainTextBody( m_composer->text() );
    mail.setType( MailMessage::SMS | MailMessage::Email );
}


void GenericComposerInterface::clear()
{
    m_composer->setText( QString() );
}

void GenericComposerInterface::setText( const QString &txt )
{
    m_composer->setText( txt );
}

QWidget *GenericComposerInterface::widget() const
{
    return m_composer;
}

void GenericComposerInterface::attach( const QContent & )
{
    qWarning("Unimplemented function called %s %d, %s", __FILE__, __LINE__, __FUNCTION__ );
}

void GenericComposerInterface::attach( const QString & )
{
    qWarning("Unimplemented function called %s %d, %s", __FILE__, __LINE__, __FUNCTION__ );
}

