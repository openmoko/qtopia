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

#include "addatt.h"

#include <qtopiaapplication.h>
#include <qtopiaglobal.h>
#include <qdocumentselector.h>
#include <qmimetype.h>

#include <account.h>


#include <QLayout>
#include <QDir>
#include <QStringList>
#include <QMessageBox>
#include <QFileInfo>
#ifdef QTOPIA_PHONE
#include <QAction>
#endif

#include <QToolTip>
#include <QWhatsThis>
#include <qsoftmenubar.h>

AttachmentItem::AttachmentItem(QListWidget *parent, const QContent& att)
  : QListWidgetItem(parent), mattachment(att)
{
  setText(att.name());
  setIcon(att.icon());
  QFileInfo fi(att.file());
  mSizeKB = fi.size() /1024; //to kb
}

AttachmentItem::AttachmentItem(QListWidget *parent, const QString &filename )
  : QListWidgetItem(parent), mFileName(filename)
{
  QFileInfo fi( filename );
  setText(fi.baseName() );
  QMimeType mt( filename );
  setIcon(mt.icon() );
  mSizeKB = fi.size() /1024; //to kb
}

bool AttachmentItem::isDocument() const
{
  return mattachment.isValid();
}

const QContent& AttachmentItem::document() const
{
  return mattachment;
}

QString AttachmentItem::fileName() const
{
  return mFileName;
}

AttachmentItem::~AttachmentItem()
{
}

int AttachmentItem::sizeKB() const
{
    return mSizeKB;
}

AddAtt::AddAtt(QWidget *parent, const char *name, Qt::WFlags f)
  : QWidget(parent,f)
{
  setObjectName(name);
  QGridLayout *top = new QGridLayout(this);

  attView = new QListWidget(this);
  //attView->addColumn(tr("Attachments"));
  connect(attView, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
          this, SLOT(currentItemChanged(QListWidgetItem*,QListWidgetItem*)) );
  top->addWidget(attView, 0,0);
  clear();
}

AddAttBase::~AddAttBase()
{
}

void AddAtt::resizeEvent(QResizeEvent *e)
{
#ifdef QTOPIA_PHONE
    //int styleSpecific = 6;
    //attView->setColumnWidth( 0, e->size().width() - styleSpecific );
    QWidget::resizeEvent( e );
#else
    Q_UNUSED(e)
#endif
}



QStringList AddAttBase::mimeTypes()
{
  QString homedocs = Qtopia::documentDir();
  QDir d( homedocs );
  QStringList l = d.entryList( QDir::Dirs );
  l.removeAll(".");
  l.removeAll("..");
  return l;
}


void AddAttBase::clear()
{
  attView->clear();
  modified = false;
  getFiles();
}

bool AddAttBase::addAttachment( const QString &filename )
{
    AttachmentItem* i = new AttachmentItem(attView,filename);
    if((i->sizeKB() + totalSizeKB())  > MAX_ATTACH_KB)
    {
        delete i;
        return false;
    }
  attView->setCurrentItem(i);
  return true;
}

bool AddAttBase::addAttachment(const QContent &dl)
{
    AttachmentItem* i = new AttachmentItem(attView,dl);
    if((i->sizeKB() + totalSizeKB())  > MAX_ATTACH_KB)
    {
        delete i;
        return false;
    }
  attView->setCurrentItem(i);
  return true;
}


void AddAtt::removeAttachment()
{
  if (attView->currentItem() != NULL) {
    attView->takeItem(attView->row(attView->currentItem()));
  }
  modified = true;
}

QList<AttachmentItem*> AddAttBase::attachedFiles() const
{
  QList<AttachmentItem*> list;
  int i = 0;
  AttachmentItem *iterItem = (AttachmentItem *) attView->item(i);
  while (iterItem != NULL) {
    list.append( iterItem );
    iterItem = (AttachmentItem *) attView->item(++i);
  }

  return list;
}

void AddAttBase::setMailMessageParts(Email *mail)
{
  for ( int i = 0; i < (int)mail->messagePartCount(); i++ ) {
    MailMessagePart &part = mail->messagePartAt( i );
    QContent d;
    d.setName( part.name() );
    d.setFile( part.storedFilename() );
    d.setLinkFile( part.linkFile() );
    d.setType( part.contentType() );

    new AttachmentItem(attView, d );
  }
}

int AddAttBase::totalSizeKB()
{
    QList<AttachmentItem*> list = attachedFiles();
    AttachmentItem* i = 0;
    int total = 0;
    foreach(i, list)
        total += i->sizeKB();
    return total;
}

void AddAtt::currentItemChanged(QListWidgetItem *item, QListWidgetItem*)
{
  bool hasItem = item;
  emit currentChanged( hasItem );
}

void AddAtt::removeCurrentAttachment()
{
  if (attView->currentItem()) {
    attView->takeItem(attView->row(attView->currentItem()));
  }
  modified = true;
}

#ifndef QTOPIA_PHONE
// pda version
AddAttDialog::AddAttDialog(QWidget *parent, const char *name, Qt::WFlags f)
  : QDialog(parent,f)
{
    setObjectName(name);
    setModal(true);
}

void AddAttDialog::removeCurrentAttachment()
{

}

void AddAttDialog::resizeEvent(QResizeEvent *e)
{

}

void AddAttDialog::fileCategorySelected(int id)
{

}

void AddAttDialog::reject()
{

}

void AddAttDialog::accept()
{
  modified = false;
  QDialog::accept();
}

void AddAttDialog::getFiles()
{

}

void AddAttDialog::addCurAttachment()
{

}

#else

//phone version, wraps AddAtt
AddAttDialog::AddAttDialog(QWidget *parent, QString name, Qt::WFlags f)
  : QDialog( parent,f)
{
  setObjectName(name);
  setModal(true);
  QVBoxLayout *l = new QVBoxLayout( this );
  addAtt = new AddAtt( this );
  l->addWidget( addAtt );
  setWindowTitle( tr("Attachments") );
#ifdef QTOPIA_PHONE
    QMenu *context = QSoftMenuBar::menuFor(this);
    QAction *attachAction = context->addAction(QIcon(":icon/attach"),tr("Add Attachment"));
    //attachAction->setVisible( false );
    connect( attachAction, SIGNAL(triggered()), this, SLOT(selectAttachment()) );
    attachAction->setWhatsThis( tr("Attach a document to this mail.") );

    QAction *removeAction = context->addAction(QIcon(":icon/attach"), tr("Remove Attachment"));
    connect( removeAction, SIGNAL(triggered()), addAtt, SLOT(removeCurrentAttachment()) );
    removeAction->setVisible(false);
    connect( addAtt, SIGNAL( currentChanged(bool) ), removeAction, SLOT( setVisible(bool) ) );
#endif

    fileSelectorDialog = new QDialog( this);
    fileSelectorDialog->setObjectName("fileSelectorDialog");
    fileSelectorDialog->setModal(true);
#ifndef QTOPIA_DESKTOP
    QtopiaApplication::setMenuLike( fileSelectorDialog, true );
#endif
    fileSelectorDialog->setWindowTitle( tr("Add Attachment") );
    QVBoxLayout *fl = new QVBoxLayout( fileSelectorDialog );
    fileSelector = new QDocumentSelector( fileSelectorDialog );
    fileSelector->setFilter( QContentFilter( QContent::Document ) );
    fileSelector->enableOptions( QDocumentSelector::TypeSelector | QDocumentSelector::NestTypes );
    fileSelector->setObjectName("fileselector");
    fileSelector->setSelectPermission( QDrmRights::Distribute );
    fileSelector->setMandatoryPermissions( QDrmRights::Distribute );
    fl->addWidget( fileSelector );
   // connect( fileSelector, SIGNAL(closeMe()), fileSelectorDialog, SLOT(accept()) );

    connect( fileSelector, SIGNAL(documentSelected(const QContent&)), this, SLOT(openFile(const QContent&)) );
}

QList<AttachmentItem*> AddAttDialog::attachedFiles() const
{
  return addAtt->attachedFiles();
}

void AddAttDialog::clear()
{
  addAtt->clear();
}

void AddAttDialog::setMailMessageParts(Email *mail)
{
  addAtt->setMailMessageParts( mail );
}

void AddAttDialog::addAttachment(const QString &filename)
{
    if(!addAtt->addAttachment( filename ))
        attachError();
}

void AddAttDialog::addAttachment(const QContent&lnk)
{
    if(!addAtt->addAttachment( lnk ))
        attachError();
}

void AddAttDialog::removeAttachment()
{
  addAtt->removeAttachment();
}

void AddAttDialog::removeCurrentAttachment()
{
  addAtt->removeCurrentAttachment();
}

void AddAttDialog::attach( const QString &filename )
{
    if(!addAtt->addAttachment( filename ))
        attachError();
}

void AddAttDialog::attach( const QContent &doclnk )
{
    if(!addAtt->addAttachment( doclnk ))
        attachError();
}

void AddAttDialog::openFile(const QContent& dl)
{
    fileSelectorDialog->accept();
    if(!addAtt->addAttachment( dl.file() ))
        attachError();

}

void AddAttDialog::selectAttachment()
{
  int result =
#ifdef QTOPIA_DESKTOP
      fileSelectorDialog->exec()
#else
      QtopiaApplication::execDialog( fileSelectorDialog )
#endif
      ;
  if( result == QDialog::Accepted )
  {
    // don't need to do anything, openFile slot called from signal
  }
}

void AddAttDialog::attachError()
{
    QMessageBox::warning(this, tr("Attachment limit"),tr("<qt>Unable to add attachment.<br><br>Total size of attachments cannot exceed %1 MB.</qt>").arg(MAX_ATTACH_KB / 1024), tr("OK") );
}

#endif
