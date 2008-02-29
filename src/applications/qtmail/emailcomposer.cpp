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

#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif
#include <qmimetype.h>
#include <qtopiaglobal.h>

#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QAction>
#ifndef QTOPIA_DESKTOP
#include <qtopiaapplication.h>
#endif

#include "emailcomposer.h"
#include "addatt.h"

EmailComposer::EmailComposer( QWidget *parent, const char *name )
  : QTextEdit( parent)
{
  setObjectName(name);
  m_addAttDialog = 0;
  setFrameStyle(NoFrame);
  setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                QSizePolicy::Expanding ) );

  QAction *attachAction = new QAction( QIcon( ":icon/attach" ), tr("Attachments"),this);
  connect( attachAction, SIGNAL(triggered()), this, SLOT(selectAttachment()) );
#ifdef QTOPIA_PHONE
    QMenu *menu = QSoftMenuBar::menuFor( this );
    menu->addSeparator();
    menu->addAction(attachAction);
#endif
    connect( this, SIGNAL(textChanged()), this, SIGNAL(contentChanged()) );
    setWordWrapMode( QTextOption::WordWrap);
}

AddAttDialog* EmailComposer::addAttDialog()
{
  if( !m_addAttDialog )
    m_addAttDialog = new AddAttDialog( this, "attachmentDialog");
  return m_addAttDialog;
}

void EmailComposer::selectAttachment()
{
  AddAttDialog *attDialog = addAttDialog();
  int result =
#ifdef QTOPIA_DESKTOP
      attDialog->exec()
#else
      QtopiaApplication::execDialog( attDialog )
#endif
      ;
  if( result == QDialog::Accepted )
  {
    emit contentChanged();
  }
}

void EmailComposerInterface::setMailMessage( Email &mail )
{
  setText( mail.plainTextBody() );
    // Restore attachments
  for ( int i = 0; i < (int)mail.messagePartCount(); i++ ) {
    MailMessagePart &part = mail.messagePartAt( i );
    mail.validateFile( part );
    attach( part.storedFilename() );
  }
}

void EmailComposer::keyPressEvent( QKeyEvent *e )
{
#ifdef QTOPIA_PHONE
    if ((e->key() == Qt::Key_Back) &&
    toPlainText().isEmpty()) {
    // Override the QTE behavior that makes the back key
    // revert the QMultiLineEdit text when a
    // QMultiLineEdit is empty.
    // So that the composer client code can completely
    // cancel the compose process when the editor is
    // empty and the back key is pressed
        setPlainText("");
    }

    if( Qtopia::mousePreferred() && e->key() == Qt::Key_Back )
      e->ignore();
    else
#endif
        QTextEdit::keyPressEvent( e );
}

EmailComposerInterface::EmailComposerInterface( QObject *parent, const char *name )
  : TextComposerInterface( parent, name )
{
  QWidget *par = 0;
  if( parent && parent->isWidgetType() )
    par = (QWidget *)parent;
  m_composer = new EmailComposer( par, name );
  connect( m_composer, SIGNAL(contentChanged()), this, SIGNAL(contentChanged()) );
}

EmailComposerInterface::~EmailComposerInterface()
{
  delete m_composer;
}

/*
QCString EmailComposerInterface::id()
{
    QCString t = QString::number( type() ).latin1();
    return t + "-Default"; // default sms/email composer
}

ComposerInterface::ComposerType EmailComposerInterface::type()
{
    return ComposerInterface::SMS | ComposerInterface::Email;
}

QString EmailComposerInterface::displayName()
{
    return tr("SMS/Email");
}
*/

bool EmailComposerInterface::hasContent() const
{
  return !m_composer->toPlainText().isEmpty()
      || m_composer->addAttDialog()->attachedFiles().count() != 0;
}

void EmailComposerInterface::getContent( MailMessage &mail ) const
{
  if( !hasContent() )
    return;
  mail.setPlainTextBody( m_composer->toPlainText() );
  QList<AttachmentItem*> attachments = m_composer->addAttDialog()->attachedFiles();
  QListIterator<AttachmentItem*> it( attachments );
  MailMessagePart attachmentPart;
  while( it.hasNext() )
  {
    AttachmentItem* current = it.next();
    QString fileName;
    if( current->isDocument() )
    {
      const QContent &lnk = current->document();
      attachmentPart.setName( lnk.name() );
      attachmentPart.setFilename( lnk.file() );
      attachmentPart.setStoredFilename( lnk.file() );
      attachmentPart.setLinkFile( lnk.linkFile() );
      attachmentPart.setContentType( lnk.type() );
      fileName = lnk.file();
    }
    else
    {
      QFileInfo fi( current->fileName() );
      attachmentPart.setName( fi.baseName() );
      attachmentPart.setFilename( fi.absoluteFilePath() );
      attachmentPart.setStoredFilename( fi.absoluteFilePath() );
      attachmentPart.setLinkFile( fi.absoluteFilePath() );
      QMimeType mt( fi.absoluteFilePath() );
      attachmentPart.setContentType( mt.id() );
      fileName = fi.absoluteFilePath();
    }

    //no need for this now, since we stream directly from the file

//     QFile f( fileName );
//     if (! f.open(QIODevice::ReadOnly) )
//     {
//       qWarning(("could not open file: " + fileName).toAscii() );
//       return;
//     }
//     QDataStream in(&f);
//     attachmentPart.setBody(in,Base64);
    mail.addMessagePart( attachmentPart );
  }
  mail.setType( MailMessage::Email );
}


void EmailComposerInterface::clear()
{
  m_composer->clear();
  m_composer->addAttDialog()->clear();
}

void EmailComposerInterface::setText( const QString &txt )
{
  m_composer->setPlainText( txt );
}

QWidget *EmailComposerInterface::widget() const
{
  return m_composer;
}

void EmailComposerInterface::attach( const QContent &lnk )
{
  m_composer->addAttDialog()->
#ifdef QTOPIA_PHONE
      attach( lnk )
#else
      addAttachment( lnk )
#endif
      ;
}

void EmailComposerInterface::attach( const QString &fileName )
{
  m_composer->addAttDialog()->
#ifdef QTOPIA_PHONE
      attach( fileName )
#else
      addAttachment( fileName )
#endif
      ;
}

