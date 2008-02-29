/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <qsoftmenubar.h>
#include <qmimetype.h>
#include <qmailmessage.h>
#include <qtopiaglobal.h>

#include <QAction>
#include <QDocumentSelector>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <qtopiaapplication.h>

#include "emailcomposer.h"
#include "addatt.h"

EmailComposer::EmailComposer( QWidget *parent, const char *name )
  : QTextEdit( parent)
{
    setObjectName(name);

    setFrameStyle(NoFrame);
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    connect( this, SIGNAL(textChanged()), this, SIGNAL(contentChanged()) );
    connect( this, SIGNAL(textChanged()), this, SLOT(updateLabel()) );
    setWordWrapMode( QTextOption::WordWrap);

    m_addAttDialog = new AddAttDialog(this, "attachmentDialog");

    updateLabel();
}

void EmailComposer::updateLabel()
{
    static const bool keypadAbsent(style()->inherits("QThumbStyle"));

    if (keypadAbsent) {
        if (isEmpty())
            QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel);
        else
            QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Next);
    } else {
        if (isEmpty()) {
            QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel);
            QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
        } else {
            QSoftMenuBar::clearLabel(this, Qt::Key_Back);

            if (toPlainText().isEmpty())
                QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
            else
                QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::Next);
        }
    }
}

bool EmailComposer::isEmpty() const
{
    return (toPlainText().isEmpty() && m_addAttDialog->attachedFiles().isEmpty());
}

AddAttDialog* EmailComposer::addAttDialog()
{
    return m_addAttDialog;
}

void EmailComposer::selectAttachment()
{
    if (addAttDialog()->documentSelector()->documents().isEmpty()) {
        QMessageBox::warning(this, 
                             tr("No documents"),
                             tr("There are no existing documents to attach"),
                             tr("OK") );
    } else {
        if( QtopiaApplication::execDialog( addAttDialog() ) == QDialog::Accepted )
            emit attachmentsChanged();
    }
}

void EmailComposer::keyPressEvent( QKeyEvent *e )
{
    static const bool keypadAbsent(style()->inherits("QThumbStyle"));

    if (keypadAbsent) {
        if (e->key() == Qt::Key_Back) {
            e->accept();
            emit finished();
            return;
        }
    } else {
        if (e->key() == Qt::Key_Select) {
            if (!isEmpty()) {
                e->accept();
                emit finished();
            } else {
                e->ignore();
            }
            return;
        }

        if (e->key() == Qt::Key_Back) {
            if( Qtopia::mousePreferred() ) {
                e->ignore();
                return;
            } else if (toPlainText().isEmpty()) {
                e->accept();
                emit finished();
                return;
            }
        }
    }

    QTextEdit::keyPressEvent( e );
}

EmailComposerInterface::EmailComposerInterface( QWidget *parent )
    : QMailComposerInterface( parent ),
      m_composer( new EmailComposer( parent, "EmailComposer" ) ),
      m_label( new QLabel( parent ) ),
      m_widget( new QWidget( parent ) )
{
    connect( m_composer, SIGNAL(contentChanged()), this, SIGNAL(contentChanged()) );
    connect( m_composer, SIGNAL(attachmentsChanged()), this, SIGNAL(contentChanged()) );
    connect( m_composer, SIGNAL(attachmentsChanged()), this, SLOT(attachmentsChanged()) );
    connect( m_composer, SIGNAL(finished()), this, SIGNAL(finished()) );

    m_label->hide();

    m_widget->setFocusProxy(m_composer);

    QVBoxLayout* layout = new QVBoxLayout( m_widget );
    layout->setMargin(0);
    layout->addWidget(m_composer);
    layout->addWidget(m_label);
}

EmailComposerInterface::~EmailComposerInterface()
{
    delete m_composer;

    // Delete any temporary files we don't need
    while (!m_temporaries.isEmpty())
        m_temporaries.takeFirst().removeFiles();
}

bool EmailComposerInterface::isEmpty() const
{
    return m_composer->isEmpty();
}

QMailMessage EmailComposerInterface::message() const
{
    QMailMessage mail;

    if( isEmpty() )
        return mail;

    QList<AttachmentItem*> attachments = m_composer->addAttDialog()->attachedFiles();

    QMailMessageContentType type("text/plain; charset=UTF-8");
    if(attachments.isEmpty()) {
        mail.setBody( QMailMessageBody::fromData( m_composer->toPlainText(), type, QMailMessageBody::Base64 ) );
    } else {
        QMailMessagePart textPart;
        textPart.setBody(QMailMessageBody::fromData(m_composer->toPlainText().toUtf8(), type, QMailMessageBody::Base64));
        mail.setMultipartType(QMailMessagePartContainer::MultipartMixed);
        mail.appendPart(textPart);

        foreach (AttachmentItem* current, attachments) {
            QString partName;
            QString fileName;
            QString content;

            if( current->isDocument() ) {
                partName = current->document().name();
                fileName = current->document().fileName();
                content = current->document().type();
            } else {
                fileName = current->fileName();
            }

            QFileInfo fi( fileName );
            if (partName.isEmpty())
                partName = fi.fileName();

            fileName = fi.absoluteFilePath();
            if (content.isEmpty())
                content = QMimeType( fileName ).id();

            QMailMessageContentType type( content.toLatin1() );
            type.setName( partName.toLatin1() );

            QMailMessageContentDisposition disposition( QMailMessageContentDisposition::Attachment );
            disposition.setFilename( partName.toLatin1() );

            QMailMessagePart part;

            if (fileName.startsWith(Qtopia::tempDir())) {
                // This file is temporary - extract the data and create a part from that
                QFile dataFile(fileName);
                if (dataFile.open(QIODevice::ReadOnly)) {
                    QDataStream in(&dataFile);

                    part = QMailMessagePart::fromStream(in, disposition, type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding);
                } else {
                    qWarning() << "Unable to open temporary file:" << fileName;
                }
            } else {
                part = QMailMessagePart::fromFile(fileName, disposition, type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding);
            }

            mail.appendPart(part);
        }
    }

    mail.setMessageType( QMailMessage::Email );

    return mail;
}

void EmailComposerInterface::clear()
{
    m_composer->clear();
    m_composer->addAttDialog()->clear();

    // Delete any temporary files we don't need
    while (!m_temporaries.isEmpty())
        m_temporaries.takeFirst().removeFiles();
}

void EmailComposerInterface::setMessage( const QMailMessage &mail )
{
    if (mail.multipartType() == QMailMessagePartContainer::MultipartNone) {
        if (mail.hasBody())
            setText( mail.body().data(), mail.contentType().content() );
    } else {
        // The only type of multipart message we currently compose is Mixed, with
        // all but the first part as out-of-line attachments
        int textPart = -1;
        for ( uint i = 0; i < mail.partCount(); ++i ) {
            QMailMessagePart &part = const_cast<QMailMessagePart&>(mail.partAt(i));
            
            if (textPart == -1 && part.contentType().type().toLower() == "text") {
                // This is the first text part, we will use as the forwarded text body
                textPart = i;
            } else {
                // Detach the part data to a temporary file if necessary
                QString attPath = part.attachmentPath();
                if (attPath.isEmpty()) {
                    if (part.detachAttachment(Qtopia::tempDir())) {
                        attPath = part.attachmentPath();

                        // Create a content object for the file
                        QContent doc(attPath);

                        if (part.hasBody()) {
                            QMailMessageContentType type(part.contentType());

                            if (doc.drmState() == QContent::Unprotected)
                                doc.setType(type.content());
}

                        doc.setName(part.displayName());
                        doc.setRole(QContent::Data);

                        doc.commit();

                        // This needs to be removed after composition
                        m_temporaries.append(doc);
                    }
                }

                if (!attPath.isEmpty())
                    attach(attPath);
            }
        }

        if (textPart != -1) {
            const QMailMessagePart& part = mail.partAt(textPart);
            setText( part.body().data(), part.contentType().content() );
        }
    }
}

void EmailComposerInterface::setText( const QString &txt, const QString & )
{
    m_composer->setPlainText( txt );
}

QWidget *EmailComposerInterface::widget() const
{
    return m_widget;
}

void EmailComposerInterface::addActions(QMenu* menu) const
{
    QAction *attachAction = new QAction( QIcon( ":icon/attach" ), tr("Attachments") + "...", m_composer);
    connect( attachAction, SIGNAL(triggered()), m_composer, SLOT(selectAttachment()) );

    menu->addAction(attachAction);
}

void EmailComposerInterface::attach( const QContent &lnk )
{
    m_composer->addAttDialog()->attach( lnk );
}

void EmailComposerInterface::attachmentsChanged()
{
    int count = 0;
    int sizeKB = 0;

    foreach (const AttachmentItem* item, m_composer->addAttDialog()->attachedFiles()) {
        ++count;
        sizeKB += item->sizeKB();
    }
    
    if (count == 0) {
        m_label->hide();
    } else {
        m_label->setText(QString("<center><small>") + tr("%n Attachment(s): %1KB","", count).arg(sizeKB)+"</small></center>");
        m_label->show();
    }
}

QTOPIA_EXPORT_PLUGIN( EmailComposerPlugin )

EmailComposerPlugin::EmailComposerPlugin()
    : QMailComposerPlugin()
{
}

QString EmailComposerPlugin::key() const
{
    return "EmailComposer";
}

QMailMessage::MessageType EmailComposerPlugin::messageType() const
{
    return QMailMessage::Email;
}

QString EmailComposerPlugin::name() const
{
    return tr("Email");
}

QString EmailComposerPlugin::displayName() const
{
    return tr("Email");
}

QIcon EmailComposerPlugin::displayIcon() const
{
    static QIcon icon(":icon/email");
    return icon;
}

QMailComposerInterface* EmailComposerPlugin::create( QWidget *parent )
{
    return new EmailComposerInterface( parent );
}

