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

#include "pdacomposer.h"
#include "addatt.h"

#include <qcontent.h>
#include <qtopia/mail/mailmessage.h>
#include <qtopia/pim/qcontactview.h>
#ifndef QTOPIA_DESKTOP
#include <qtopiaapplication.h>
#endif
#include <qmimetype.h>

#include <qstringlist.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qfileinfo.h>
#include <qfile.h>

#ifndef QTOPIA4_TODO

PDAComposer::PDAComposer( QWidget *parent, const char *name )
    : QWidget( parent ), WriteMailBase()
{
    setObjectName( name );
}

void PDAComposer::clear()
{
}

PDAComposer::~PDAComposer()
{
}


bool PDAComposer::hasContent() const
{
    return false;
}

void PDAComposer::getContent( MailMessage & ) const
{
}

void PDAComposer::setText( const QString & )
{
}

void PDAComposer::attach( const QContent & )
{
}

void PDAComposer::attach( const QString & )
{
}

void PDAComposer::updateAddAtt()
{
}

void PDAComposer::selectAttachments()
{
}

QString PDAComposer::to() const
{
    return QString();
}

void PDAComposer::setTo( const QString & )
{
}

QString PDAComposer::cc() const
{
    return QString();
}

void PDAComposer::setCc( const QString & )
{
}

QString PDAComposer::bcc() const
{
    return QString();
}

void PDAComposer::setBcc( const QString & )
{
}

QString PDAComposer::subject() const
{
    return QString();
}

void PDAComposer::setSubject( const QString & )
{
}

void PDAComposer::setFrom( const QString & )
{
}

QString PDAComposer::from() const
{
    return QString();
}

void PDAComposer::setFromFields( const QStringList & )
{
}

AddAttDialog *PDAComposer::addAttDialog() const
{
    return 0;
}

void PDAComposer::toggleCc( bool )
{
}

void PDAComposer::toggleBcc( bool )
{
}

void PDAComposer::toggleFrom( bool )
{
}

void PDAComposer::setWordWrap( bool )
{
}

void PDAComposer::editRecipients()
{
}

#else

PDAComposer::PDAComposer( QWidget *parent, const char *name )
    : WriteMailBase( parent, name )
{
    connect( toInput, SIGNAL(textChanged(const QString&)),
                                            this, SIGNAL(recipientsChanged()) );
    connect( ccInput, SIGNAL(textChanged(const QString&)),
                                            this, SIGNAL(recipientsChanged()) );
    connect( bccInput, SIGNAL(textChanged(const QString&)),
                                            this, SIGNAL(recipientsChanged()) );
    connect( emailInput, SIGNAL(textChanged()),
                                               this, SIGNAL(contentChanged()) );

    connect( toPicker, SIGNAL(clicked()), this, SLOT(editRecipients()) );
    connect( ccPicker, SIGNAL(clicked()), this, SLOT(editRecipients()) );
    connect( bccPicker, SIGNAL(clicked()), this, SLOT(editRecipients()) );

    m_addAttDialog = new AddAttDialog( this );
    updateAddAtt();

    setTabOrder( toInput, toPicker );
    setTabOrder( toPicker, ccInput );
    setTabOrder( ccInput, ccPicker );
    setTabOrder( ccPicker, subjectInput );
    setTabOrder( subjectInput, fromBox );
    setTabOrder( fromBox, bccInput );
    setTabOrder( bccInput, bccPicker );
    setTabOrder( bccPicker, emailInput );
}

void PDAComposer::clear()
{
    toInput->setText( QString() );
    ccInput->setText( QString() );
    bccInput->setText( QString() );
    subjectInput->setText( QString() );
    emailInput->setText( QString() );
    m_addAttDialog->clear();
    addAtt->clear();
    toInput->setFocus();
    addAtt->hide();
}

PDAComposer::~PDAComposer()
{
}


bool PDAComposer::hasContent() const
{
    return !(emailInput->text().stripWhiteSpace().isEmpty() &&
             subjectInput->text().stripWhiteSpace().isEmpty() &&
             (addAtt->attachedFiles().count() == 0));
}

void PDAComposer::getContent( MailMessage &mail ) const
{
    mail.setTo( toInput->text() );
    mail.setCc( ccInput->text() );
    mail.setBcc( bccInput->text() );
    mail.setSubject( subjectInput->text() );
    mail.setFrom( fromBox->currentText() );
    mail.setPlainTextBody( emailInput->text() );
    QList<AttachmentItem> attachments = m_addAttDialog->attachedFiles();
    QListIterator<AttachmentItem> it( attachments );
    MailMessagePart attachmentPart;
    for( ; *it ; ++it )
    {
        QString fileName;
        if( it.current()->isDocument() )
        {
            const QContent &lnk = it.current()->document();
            attachmentPart.setName( lnk.name() );
            attachmentPart.setFilename( lnk.file() );
            attachmentPart.setStoredFilename( lnk.file() );
            attachmentPart.setLinkFile( lnk.linkFile() );
            attachmentPart.setContentType( lnk.type() );
            fileName = lnk.file();
        }
        else
        {
            QFileInfo fi( it.current()->fileName() );
            attachmentPart.setName( fi.baseName() );
            attachmentPart.setFilename( fi.absFilePath() );
            attachmentPart.setStoredFilename( fi.absFilePath() );
            attachmentPart.setLinkFile( fi.absFilePath() );
            QMimeType mt( fi.absFilePath() );
            attachmentPart.setContentType( mt.id() );
            fileName = fi.absFilePath();
        }

        QFile f( fileName );
        if (! f.open(QIODevice::ReadOnly) )
        {
            qWarning("could not open file: " + fileName );
            return;
        }
        QDataStream in(&f);
        attachmentPart.setBody(in,Base64);
        mail.addMessagePart( attachmentPart );
    }
}

void PDAComposer::setText( const QString &txt )
{
    emailInput->setText( txt );
}

void PDAComposer::attach( const QContent &lnk )
{
    m_addAttDialog->addAttachment( lnk );
    updateAddAtt();
}

void PDAComposer::attach( const QString &fileName )
{
    m_addAttDialog->addAttachment( fileName );
    updateAddAtt();
}

void PDAComposer::updateAddAtt()
{
    addAtt->clear();
    QList<AttachmentItem> attachments = m_addAttDialog->attachedFiles();
    QListIterator<AttachmentItem> it( attachments );
    for( ; *it ; ++it )
        if( (*it)->isDocument() )
            addAtt->addAttachment( (*it)->document() );
        else
            addAtt->addAttachment( (*it)->fileName() );

    if( addAtt->attachedFiles().count() )
        addAtt->show();
    else
        addAtt->hide();
}

void PDAComposer::selectAttachments()
{
    int r =
#ifdef QTOPIA_DESKTOP
        m_addAttDialog->exec()
#else
        QtopiaApplication::execDialog( m_addAttDialog )
#endif
        ;
    if( r == QDialog::Accepted )
    {
        updateAddAtt();
    }
}

QString PDAComposer::to() const
{
    return toInput->text();
}

void PDAComposer::setTo( const QString &txt )
{
    toInput->setText( txt );
}

QString PDAComposer::cc() const
{
    return ccInput->text();
}

void PDAComposer::setCc( const QString &txt )
{
    ccInput->setText( txt );
}

QString PDAComposer::bcc() const
{
    return bccInput->text();
}

void PDAComposer::setBcc( const QString &txt )
{
    bccInput->setText( txt );
}

QString PDAComposer::subject() const
{
    return subjectInput->text();
}

void PDAComposer::setSubject( const QString &txt )
{
    subjectInput->setText( txt );
}

void PDAComposer::setFrom( const QString &txt )
{
    for( uint i = 0 ; (int)i < fromBox->count() ; ++i )
    {
        if( fromBox->text( i ) == txt )
        {
            fromBox->setCurrentItem( i );
            break;
        }
    }
}

QString PDAComposer::from() const
{
    return fromBox->currentText();
}

void PDAComposer::setFromFields( const QStringList &fields )
{
    fromBox->clear();
    fromBox->insertStringList( fields );
}

AddAttDialog *PDAComposer::addAttDialog() const
{
    return m_addAttDialog;
}

void PDAComposer::toggleCc( bool t )
{
    if( t )
    {
        ccInput->show();
        ccLabel->show();
        ccPicker->show();
    }
    else
    {
        ccInput->hide();
        ccLabel->hide();
        ccPicker->hide();
    }
}

void PDAComposer::toggleBcc( bool t )
{
    if( t )
    {
        bccInput->show();
        bccLabel->show();
        bccPicker->show();
    }
    else
    {
        bccInput->hide();
        bccLabel->hide();
        bccPicker->hide();
    }
}

void PDAComposer::toggleFrom( bool t )
{
    if( t )
    {
        fromBox->show();
        fromLabel->show();
    }
    else
    {
        fromBox->hide();
        fromLabel->hide();
    }
}

void PDAComposer::setWordWrap( bool w )
{
    if( w )
        emailInput->setWordWrap( QMultiLineEdit::WidgetWidth );
    else
        emailInput->setWordWrap( QMultiLineEdit::NoWrap );
}

void PDAComposer::editRecipients()
{
    QLineEdit *le = 0;
    if( sender() == toPicker )
        le = toInput;
    else if( sender() == ccPicker )
        le = ccInput;
    else if( sender() == bccPicker )
        le = bccInput;
    if( !le )
    {
        qWarning("BUG: Couldn't find line edit for recipients.");
        return;
    }

    QValueList<PimContact::ContactFields> fields;
    fields.append( PimContact::Emails );
    ContactFieldSelector picker( fields, "", true, this, "addressPicker", true );
    QStringList curAddr = QStringList::split( ",", le->text() );
    for( uint i = 0 ; i < curAddr.count() ; ++i )
        curAddr[i] = curAddr[i].stripWhiteSpace();
    picker.setSelectedFields( curAddr );
    int r =
#ifdef QTOPIA_DESKTOP
        picker.exec()
#else
        QtopiaApplication::execDialog( &picker )
#endif
        ;
    if ( r == QDialog::Accepted ) {
        QStringList unknownFieldData = picker.unknownFieldData();
        QString userData;
        if (unknownFieldData.count())
            userData = unknownFieldData.join( ", " );
        QString selectedFields;
        QValueList<int> selectedIndices = picker.selected();
        for( QValueList<int>::Iterator it = selectedIndices.begin() ;
                                        it != selectedIndices.end() ; ++it )
        {
            if( !selectedFields.isEmpty() )
                selectedFields += ", ";
            selectedFields += picker.fieldDataAt( *it );
        }
        if( !userData.isEmpty() && !selectedFields.isEmpty() )
            userData += ", ";
        le->setText( userData + selectedFields );
        le->setFocus();
    }
}

#endif
