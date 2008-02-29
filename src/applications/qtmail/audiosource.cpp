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

#include "audiosource.h"
#include "../mediarecorder/audioparameters.h"

#include <qtopiaipcenvelope.h>
#include <qtopiaapplication.h>
#include <qdocumentselector.h>
#include <qsoftmenubar.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qpainter.h>
#include <qstring.h>
#include <qtopialog.h>
#include <QDSServices>
#include <QButtonGroup>
#include <QDSData>
#include <QDSAction>

//===========================================================================

class AudioSelectorDialog : public QDialog
{
    Q_OBJECT
public:
    AudioSelectorDialog(QWidget *parent)
        : QDialog(parent)
    {
        setWindowTitle(tr("Select Audio"));
        setModal( true );
        QtopiaApplication::setMenuLike(this, true);
        QVBoxLayout *vb = new QVBoxLayout(this);
        fileSelector = new QDocumentSelector(this);
        fileSelector->setFilter( QContentFilter( QContent::Document ) & QContentFilter( QContentFilter::MimeType, "audio/*" ) );
        fileSelector->disableOptions( QDocumentSelector::ContextMenu );

        vb->addWidget(fileSelector);
        connect(fileSelector, SIGNAL(documentSelected(const QContent&)),
                this, SLOT(documentSelected(const QContent&)));
    }

    QContent selected() const { return file; }

protected slots:
    void documentSelected(const QContent &doc) {
        file = doc;
        accept();
    }

private:
    QDocumentSelector *fileSelector;
    QContent file;
};

class AudioSourceWidgetPrivate
{
public:
    AudioSourceWidgetPrivate() : audioServices( 0 )
    {
    }
    QContent audio;
    QLabel *label;
    QDSServices* audioServices;
    AudioParameters audioParameters;
    QPushButton *removePB, *changePB;
};

AudioSourceWidget::AudioSourceWidget( QWidget *parent )
:   QWidget( parent )
{
    init();
    haveAudio( false );
}

AudioSourceWidget::~AudioSourceWidget()
{
    delete d;
}

void AudioSourceWidget::init()
{
    d = new AudioSourceWidgetPrivate();

    d->label = new QLabel( this );
    d->label->setAlignment( Qt::AlignCenter );

    QVBoxLayout *vl = new QVBoxLayout( this );
    vl->addWidget( d->label );

    d->changePB = new QPushButton( tr("Audio Files"), this );
    connect( d->changePB, SIGNAL(clicked()), this, SLOT(change()) );
    vl->addWidget( d->changePB );

    // Find available QDS audio services
    d->audioServices = new QDSServices( QString( "x-parameters/x-audioparameters" ),
                                        QString( "audio/x-qstring" ) );

    QButtonGroup* audioGroup = new QButtonGroup( this );
    connect( audioGroup,
             SIGNAL( buttonClicked ( int ) ),
             this,
             SLOT( audioRequest( int ) ) );

    int audioId = 0;
    foreach ( QDSServiceInfo serviceInfo, *( d->audioServices ) ) {
        QPushButton* pb = 0;
        QIcon icon;
        if ( !serviceInfo.icon().isEmpty() ) {
            icon = QIcon( ":icon/" + serviceInfo.icon() );
        }

        if ( !icon.isNull() ) {
            pb = new QPushButton( /*icon,*/ serviceInfo.description() );
        } else {
            pb = new QPushButton( serviceInfo.description() );
        }

        vl->addWidget( pb );

        audioGroup->addButton( pb, audioId );
        ++audioId;
    }

    d->removePB = new QPushButton( tr("Remove"), this );
    connect( d->removePB, SIGNAL(clicked()), this, SLOT(remove()) );

    vl->addWidget( d->removePB );
    vl->addStretch();
}

void AudioSourceWidget::change()
{
#ifndef QTOPIA_DESKTOP
    AudioSelectorDialog *dlg = new AudioSelectorDialog(parentWidget());
    dlg->showMaximized();
    if (QtopiaApplication::execDialog(dlg) == QDialog::Accepted) {
        if (dlg->selected().fileKnown())
            setAudio(dlg->selected());
    }
    delete dlg;
#else
    QString formatStr = "Audio (*.amr)";
    QFileDialog *fd = new QFileDialog( QDir::homeDirPath(), formatStr, this, "audioSelector", true );
    fd->setMode( QFileDialog::ExistingFile );
    if( fd->exec() == QDialog::Accepted && fd->selectedFile().length() )
    {
        setAudio( DocLnk(fd->selectedFile()) );
        haveAudio( true );
    }
#endif
}

void AudioSourceWidget::setDefaultAudio( const QString &type, const QString &subFormat, int fr, int ch )
{
    d->audioParameters.setMimeType( QMimeType(type) );
    d->audioParameters.setSubFormat( subFormat );
    d->audioParameters.setFrequency( fr );
    d->audioParameters.setChannels( ch );
}

void AudioSourceWidget::setAudio( const QContent &doc )
{
    qLog(Messaging) <<"AudioSourceWidget::setAudio:" << doc.name();

    d->audio = doc;
    if ( !doc.fileKnown() ) {
        haveAudio( false );
        QFont f = font();
        f.setItalic( true );
        d->label->setFont( f );
        d->label->setText( tr("No Audio") );
    } else {
        haveAudio( true );
        d->label->setFont( font() );
        d->label->setText(doc.name());
    }
}

QContent AudioSourceWidget::audio() const
{
    return d->audio;
}

void AudioSourceWidget::remove()
{
    setAudio(QContent());
}

void AudioSourceWidget::audioRequest( int id )
{
    if ( d->audioServices == 0 )
        return;

    QByteArray parametersArray;
    {
        QDataStream stream( &parametersArray, QIODevice::WriteOnly );
        stream << d->audioParameters;
    }

    QDSData parameters( parametersArray,
                        QMimeType( "x-parameters/x-audioparameters" ) );

    QDSAction action( d->audioServices->operator[]( id ) );
    if ( action.exec( parameters ) == QDSAction::CompleteData ) {
        QDataStream stream( action.responseData().toIODevice() );
        QString filename;
        stream >> filename;
        setAudio( QContent( filename ) );
    } else {
        qWarning( action.errorMessage().toLatin1() );
    }
}

void AudioSourceWidget::haveAudio( bool f )
{
    d->removePB->setEnabled( f );
}


//dialog convenience wrapper
AudioSourceDialog::AudioSourceDialog( QWidget *parent, const QString name, bool modal, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setModal( modal );
    setObjectName( name );
    init();
}

AudioSourceDialog::~AudioSourceDialog()
{
}

void AudioSourceDialog::init()
{
    QVBoxLayout *l = new QVBoxLayout( this );
    mWidget = new AudioSourceWidget( this );
    l->addWidget( mWidget );
#if defined(QTOPIA_DESKTOP) || !defined(QTOPIA_PHONE)
    QHBoxLayout *bottomLayout = new QHBoxLayout( l );
    bottomLayout->addStretch();
    QPushButton *okButton = new QPushButton( tr("OK"), this );
    okButton->setDefault( true );
    connect( okButton, SIGNAL(clicked()), this, SLOT(accept()) );
    bottomLayout->addWidget( okButton );
    QPushButton *cancelButton = new QPushButton( tr("Cancel"), this );
    cancelButton->setAutoDefault( true );
    connect( cancelButton, SIGNAL(clicked()), this, SLOT(reject()) );
    bottomLayout->addWidget( cancelButton );
#endif
#ifdef QTOPIA_DESKTOP
    l->setResizeMode( QLayout::FreeResize );
    QSize sh = sizeHint();
    setMinimumSize( sh );
    setMaximumSize( sh.width()*2, sh.height()*2 );
#endif
    setWindowTitle( tr("Audio") );
}

void AudioSourceDialog::setDefaultAudio( const QString &type, const QString &subFormat, int fr, int ch )
{
    mWidget->setDefaultAudio(type, subFormat, fr, ch);
}

void AudioSourceDialog::setAudio( const QContent &fn )
{
    mWidget->setAudio( fn );
}

QContent AudioSourceDialog::audio() const
{
    return mWidget->audio();
}

#include "audiosource.moc"
