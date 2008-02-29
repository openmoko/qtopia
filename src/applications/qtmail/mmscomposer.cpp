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

#include "mmscomposer.h"
#include "../addressbook/imagesourcedialog.h"
#include "audiosource.h"

#include <qsoftmenubar.h>
#include <qtopiaapplication.h>
#include <qcolorselector.h>
#include <qtopia/mail/mailmessage.h>
#include <qmimetype.h>

#include <QAction>
#include <QPainter>
#include <QLayout>
#include <QStackedWidget>
#include <QDialog>
#include <QImage>
#include <QSpinBox>
#include <QFile>
#include <QStringList>
#include <QDir>
#include <QTextStream>
#include <QDataStream>
//#include <qgfx_qws.h>
#include <QBitArray>
#include <QXmlDefaultHandler>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QMenu>
#include <QDebug>

MMSSlideImage::MMSSlideImage(QWidget *parent)
    : QLabel(parent)
{
    setAlignment( Qt::AlignCenter );
    connect( this, SIGNAL(clicked()), this, SLOT(select()) );
    setImage( QPixmap() );
    setFocusPolicy( Qt::StrongFocus );
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    // would like to specify a value relative to parent here but Qt makes it hard..
    setMinimumSize( 0, 30 );
}

void MMSSlideImage::mousePressEvent( QMouseEvent *event )
{
    if( rect().contains( event->pos() ) )
        m_pressed = true;
}

void MMSSlideImage::mouseReleaseEvent( QMouseEvent *event )
{
    if( m_pressed && rect().contains( event->pos() ) )
        emit clicked();
    m_pressed = false;
}

void MMSSlideImage::keyPressEvent( QKeyEvent *event )
{
    if( event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = (QKeyEvent *)event;
        switch( keyEvent->key() )
        {
            case Qt::Key_Left:
                emit leftPressed();
                break;
            case Qt::Key_Right:
                emit rightPressed();
                break;
            case Qt::Key_Select:
                emit clicked();
                break;
            default:
                QLabel::keyPressEvent( event );
                break;
        }
    }
}

void MMSSlideImage::paintEvent( QPaintEvent *event )
{
    QLabel::paintEvent( event );
    if( hasFocus() )
    {
        QPainter p( this );
        QPen pen(palette().highlight().color());
        p.setPen( pen );
        p.drawRect( 0, 0, width(), height() );
        p.drawRect( 1, 1, width()-2, height()-2 );
    }
}

QRect MMSSlideImage::contentsRect() const
{
    QPixmap img = image();
    if( img.isNull() )
        return QRect();
    QPoint pnt( rect().x() + (width() - img.width()) / 2 + 2,
                rect().y() + (height() - img.height()) / 2 + 2
              );
    pnt = mapToParent( pnt );
    return QRect( pnt.x(), pnt.y(), img.width(), img.height() );
}

QSize MMSSlideImage::sizeHint() const
{
    QWidget *par = 0;
    if( parent() && parent()->isWidgetType() )
        par = (QWidget *)parent();
    QRect mwr = QApplication::desktop()->availableGeometry();
    int w = par ? par->width() : mwr.width(),
        h = par ? par->height() : mwr.height();
    return QSize(w/3*2, h/3*2);
}

void MMSSlideImage::select()
{
    ImageSourceDialog *selector = new ImageSourceDialog( m_image, this,
                                                        "slideImageSelector" );
    selector->setWindowTitle( tr("Slide Photo") );
    int result = QtopiaApplication::execDialog( selector );
    if( result == QDialog::Accepted )
    {
        setImage(selector->pixmap());
    }
    delete selector;
}

void MMSSlideImage::resizeEvent( QResizeEvent *event )
{
    QLabel::resizeEvent( event );
    setImage( m_image );
}

QPixmap MMSSlideImage::scale( const QPixmap &src ) const
{
    QPixmap pix = src;
    if( !src.isNull() && (src.width() >= width() || src.height() >= height()) )
        pix = pix.scaled(width(), height(), Qt::KeepAspectRatio);
    return pix;
}

void MMSSlideImage::setImage( const QPixmap &image )
{
    m_image = image;
    setPixmap( scale( m_image ) );
    if( image.isNull() )
        setText( tr("Slide Image") );
    emit changed();
}

QPixmap MMSSlideImage::image() const
{
    QPixmap img;
    const QPixmap *p = pixmap();
    if( p )
        img = *p;
    return img;
}

const QString MMSSlideText::defaultText = QObject::tr("Your text here..");
MMSSlideText::MMSSlideText(QWidget *parent)
    : QTextEdit(parent), m_hasFocus( false )
{
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                            QSizePolicy::MinimumExpanding ) );
    setWordWrapMode(QTextOption::WordWrap);
    setText( QString() );
}

bool MMSSlideText::event( QEvent *e )
{
#ifdef QTOPIA_PHONE
    bool a = QTextEdit::event( e );
    if( e->type() == QEvent::EnterEditFocus && text().isNull() )
        clear();
    else if( e->type() == QEvent::LeaveEditFocus && text().isEmpty() )
        setText( QString() );
    else if( e->type() == QEvent::MouseButtonPress && !m_hasFocus )
    {
        selectAll();
        m_hasFocus = true;
    }
    else if( e->type() == QEvent::FocusOut )
        m_hasFocus = false;

    return a;
#else
    return QTextEdit::event( e );
#endif
}

void MMSSlideText::keyPressEvent( QKeyEvent *e )
{
#ifdef QTOPIA_PHONE
    if (!Qtopia::mousePreferred()) {
        if (!hasEditFocus()) {
            if (e->key() == Qt::Key_Left) {
                emit leftPressed();
                e->accept();
                return;
            } else if (e->key() == Qt::Key_Right) {
                emit rightPressed();
                e->accept();
                return;
            }
            //else fall through
        }
    }
#endif
    QTextEdit::keyPressEvent( e );
    updateGeometry();
}

QRect MMSSlideText::contentsRect() const
{
    if( text().isNull() )
        return QRect();
    QPoint pnt = rect().topLeft();
    pnt = mapToParent( pnt );
    return QRect( pnt.x(), pnt.y(), rect().width()-2, rect().height() - 2 );
}

void MMSSlideText::setText( const QString &txt )
{
    QString t = txt;
    if( t.simplified().isEmpty() )
    {
        t = MMSSlideText::defaultText;
        selectAll();
    }
    QTextEdit::setPlainText( t );
    updateGeometry();
}

QString MMSSlideText::text() const
{
    QString t = QTextEdit::toPlainText().simplified();
    if( t == MMSSlideText::defaultText )
        t = QString();
    return t;
}

QSize MMSSlideText::sizeHint() const
{
    QFontMetrics fm( font() );
    return QSize( QTextEdit::sizeHint().width(),
                    qMax( fm.boundingRect( 0, 0, width()-2, 32768,
                        Qt::TextWordWrap | Qt::AlignHCenter, text() ).height()+2,
                        QApplication::globalStrut().height() ) );
}

//---------------------------------------------------------------------------

MMSSlideAudio::MMSSlideAudio(QWidget *parent)
    : QPushButton(parent)
{
    setIcon(QIcon(":icon/sound"));
    connect(this, SIGNAL(clicked()), this, SLOT(select()));
}

void MMSSlideAudio::setAudio( const QContent &doc )
{
    QFile f(doc.file());
    if (f.open(QIODevice::ReadOnly)) {
        setText(doc.name());
        audioData = f.readAll();
        audioName = doc.name();
        audioType = doc.type();
    } else {
        setText("");
        audioData.resize(0);
        audioName = QString();
        audioType = QString();
    }

    emit changed();
}

void MMSSlideAudio::setAudio( const QByteArray &d, const QString &loc )
{
    audioData = d;
    audioName = loc;
    setText(loc.toLatin1());
}

QByteArray MMSSlideAudio::audio() const
{
    return audioData;
}

void MMSSlideAudio::select()
{
    AudioSourceDialog *selector
        = new AudioSourceDialog(this, "slideAudioSelector", true);
    selector->setAudio(QContent(audioName));
    selector->setDefaultAudio("audio/amr", "amr", 8000, 1);
    int result = QtopiaApplication::execDialog( selector );
    if ( result == QDialog::Accepted ) {
        setAudio(selector->audio());
    }
    delete selector;
}

QString MMSSlideAudio::mimeType() const
{
    if (audioType.isEmpty()) {
        // guess
        char buf[7];
        memcpy(buf, audioData.data(), qMin(6, audioData.size()));
        buf[6] = '\0';
        QString head(buf);
        if (head == "#!AMR") {
            audioType = "audio/amr";
        } else if (head == "RIFF") {
            audioType = "audio/x-wav";
        }
    }

    return audioType;
}

void MMSSlideAudio::keyPressEvent( QKeyEvent *e )
{
    switch (e->key()) {
        case Qt::Key_Left:
            emit leftPressed();
            break;
        case Qt::Key_Right:
            emit rightPressed();
            break;
        default:
            QPushButton::keyPressEvent(e);
    }
}

//===========================================================================

MMSSlide::MMSSlide(QWidget *parent)
    : QWidget(parent), m_firstShow( true ), m_duration( 5000 )
{
    setFocusPolicy( Qt::NoFocus );
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    QVBoxLayout *l = new QVBoxLayout( this );
    l->setMargin(0);
    l->setSpacing(0);

    m_imageContent = new MMSSlideImage( this );
    l->addWidget( m_imageContent, 6 );
    connect( m_imageContent, SIGNAL(leftPressed()), this, SIGNAL(leftPressed()) );
    connect( m_imageContent, SIGNAL(rightPressed()), this, SIGNAL(rightPressed()) );

    m_textContent = new MMSSlideText( this );
    l->addWidget( m_textContent, 3 );
    connect( m_textContent, SIGNAL(leftPressed()), this, SIGNAL(leftPressed()) );
    connect( m_textContent, SIGNAL(rightPressed()), this, SIGNAL(rightPressed()) );

    m_audioContent = new MMSSlideAudio( this );
    l->addWidget( m_audioContent, 1 );
    connect( m_audioContent, SIGNAL(leftPressed()), this, SIGNAL(leftPressed()) );
    connect( m_audioContent, SIGNAL(rightPressed()), this, SIGNAL(rightPressed()) );
}

void MMSSlide::setDuration( int t )
{
    if( t != m_duration )
    {
        m_duration = t;
        emit durationChanged( m_duration );
    }
}

int MMSSlide::duration() const
{
    return m_duration;
}

MMSSlideImage *MMSSlide::imageContent() const
{
    return m_imageContent;
}

MMSSlideText *MMSSlide::textContent() const
{
    return m_textContent;
}

MMSSlideAudio *MMSSlide::audioContent() const
{
    return m_audioContent;
}

void MMSSlide::showEvent( QShowEvent *event )
{
    QWidget::showEvent( event );
    if( m_firstShow )
    {
        m_imageContent->setFocus();
        m_firstShow = false;
    }
}

//==============================================================================

MMSComposer::MMSComposer(QWidget *parent)
    : QWidget(parent), m_curSlide(-1), m_internalUpdate(false)
{
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    setFocusPolicy( Qt::NoFocus );

    QVBoxLayout *l = new QVBoxLayout( this );
    l->setMargin(0);
    QHBoxLayout *labelLayout = new QHBoxLayout;
    m_durationLabel = new QLabel( this );
    labelLayout->addWidget( m_durationLabel );
    m_slideLabel = new QLabel( this );
    m_slideLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    labelLayout->addWidget( m_slideLabel );
    l->addLayout( labelLayout );
    m_slideStack = new QStackedWidget( this );
    m_slideStack->setFocusPolicy( Qt::NoFocus );
    l->addWidget( m_slideStack, 1 );

    connect( this, SIGNAL(currentChanged(uint)), this, SLOT(updateLabels()) );

    setBackgroundColor(palette().color(QPalette::Base));
    setTextColor(palette().color(QPalette::Text));

    QMenu *thisMenu = QSoftMenuBar::menuFor( this );

    QAction *action = new QAction(tr("Add Slide"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(addSlide()) );
    thisMenu->addAction(action);
    action = new QAction(tr("Remove Slide"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(removeSlide()) );
    thisMenu->addAction(action);
    thisMenu->addSeparator();

    action = new QAction(tr("Slide Options"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(slideOptions()) );
    thisMenu->addAction(action);

    addSlide();
}

MMSComposer::~MMSComposer()
{
    qDeleteAll(m_slides);
}

void MMSComposer::slideOptions()
{
    MMSSlide *cur = slide( currentSlide() );
    if( !cur )
        return;
    QDialog *dlg = new QDialog(this);
    dlg->setModal(true);
    dlg->setWindowTitle( tr("Slide Options") );
    QGridLayout *l = new QGridLayout( dlg );
    l->setMargin(3);
    int rowCount = 0;
    QLabel *la = new QLabel( tr("Duration", "duration between images in a slide show"), dlg );
    l->addWidget( la, rowCount, 0 );
    QHBoxLayout *durCon = new QHBoxLayout( dlg );
    QSpinBox *durBox = new QSpinBox(dlg );
    durCon->addWidget(durBox);
    durBox->setMinimum( 1 );
    durBox->setMaximum( 10 );
    durBox->setValue( cur->duration()/1000 );
    la = new QLabel( dlg );
    durCon->addWidget(la);
    la->setText( tr("seconds") );
    l->addLayout( durCon, rowCount, 1 );
    ++rowCount;
/*
    la = new QLabel( tr("Text Color"), dlg );
    l->addWidget( la, rowCount, 0 );
    QColorButton *tc = new QColorButton( dlg );
    tc->setColor( textColor() );
    l->addWidget( tc, rowCount, 1 );
    ++rowCount;
*/
    la = new QLabel( tr("Slide Color"), dlg );
    l->addWidget( la, rowCount, 0 );
    QColorButton *bg = new QColorButton( dlg );
    bg->setColor( backgroundColor() );
    l->addWidget( bg, rowCount, 1 );
    int r = QtopiaApplication::execDialog( dlg );
    if( r == QDialog::Accepted )
    {
//      setTextColor( tc->color() );
        setBackgroundColor( bg->color() );
        cur->setDuration( durBox->value()*1000 );
    }
}

QRect MMSComposer::contentsRect() const
{
    QRect r = rect();
    r.setHeight( r.height() - qMax( m_slideLabel->height(), m_durationLabel->height() ) );
    return r;
}

void MMSComposer::addSlide()
{
    addSlide( -1 );
}

void MMSComposer::addSlide( int a_slide )
{
    if( a_slide < 0 )
    {
        if( currentSlide() == -1 )
            a_slide = 0;
        else
            a_slide = currentSlide();
    }
    else if( a_slide >= (int)slideCount() )
    {
        a_slide = slideCount() - 1;
    }
    if( slideCount() )
        ++a_slide; // add to the next slide

    MMSSlide *newSlide = new MMSSlide( m_slideStack );
    connect( newSlide, SIGNAL(leftPressed()), this, SLOT(previousSlide()) );
    connect( newSlide, SIGNAL(rightPressed()), this, SLOT(nextSlide()) );
    connect( newSlide, SIGNAL(durationChanged(int)), this, SLOT(updateLabels()) );
    m_slides.insert( a_slide, newSlide );
    m_slideStack->addWidget(newSlide);

    QMenu *thisMenu = QSoftMenuBar::menuFor( this );
    QSoftMenuBar::addMenuTo( newSlide, thisMenu );
    QSoftMenuBar::addMenuTo( newSlide->m_textContent, thisMenu );
    QSoftMenuBar::addMenuTo( newSlide->m_imageContent, thisMenu );
    connect( newSlide->m_textContent, SIGNAL(textChanged()), this, SIGNAL(contentChanged()) );
    connect( newSlide->m_imageContent, SIGNAL(changed()), this, SIGNAL(contentChanged()) );
    connect( newSlide->m_audioContent, SIGNAL(changed()), this, SIGNAL(contentChanged()) );
    m_internalUpdate = true;
    setCurrentSlide( a_slide );
}

void MMSComposer::removeSlide()
{
    removeSlide( -1 );
}

void MMSComposer::removeSlide( int a_slide )
{
    if( slideCount() <= 1 )
        return;
    int s = a_slide;
    if( s == -1 )
        s = currentSlide();
    if( s < 0 || s >= (int)slideCount() )
        return;
    m_slideStack->removeWidget( slide( s ) );
    delete m_slides.takeAt( s );
    if( s >= (int)slideCount() )
        s = slideCount() - 1;
    if( s >= 0 )
        m_internalUpdate = true;
    setCurrentSlide( s );
}

void MMSComposer::setTextColor( const QColor &col )
{
    m_textColor = col;
    QPalette pal = m_slideStack->palette();
    pal.setColor( QPalette::Foreground, m_textColor );
    pal.setColor( QPalette::Text, m_textColor );
    m_slideStack->setPalette( pal );
}

QColor MMSComposer::textColor() const
{
    return m_textColor;
}

QColor MMSComposer::backgroundColor() const
{
    return m_backgroundColor;
}

void MMSComposer::setBackgroundColor( const QColor &col )
{
    m_backgroundColor = col;
    QPalette pal = m_slideStack->palette();
    pal.setColor( QPalette::Background, m_backgroundColor );
    pal.setColor( QPalette::Base, m_backgroundColor );
    m_slideStack->setPalette( pal );
}

void MMSComposer::setCurrentSlide( int a_slide )
{
    if( a_slide >= (int)slideCount() )
        return;
    if( a_slide < 0 )
    {
        m_curSlide = -1;
        return;
    }
    if( m_internalUpdate || a_slide != m_curSlide )
    {
        m_internalUpdate = false;
        m_curSlide = a_slide;
        m_slideStack->setCurrentWidget( slide( m_curSlide ) );
        slide( m_curSlide )->m_imageContent->setFocus();
        emit currentChanged( m_curSlide );
    }
}

void MMSComposer::nextSlide()
{
    if( !slideCount() )
        return;
    int cur = currentSlide();
    if( cur == -1 || ++cur >= (int)slideCount() )
        cur = 0;
    setCurrentSlide( cur );
}

void MMSComposer::previousSlide()
{
    if( !slideCount() )
        return;
    int cur = currentSlide();
    --cur;
    if( cur < 0 )
        cur = slideCount() - 1;
    setCurrentSlide( cur );
}

void MMSComposer::updateLabels()
{
    QString baseLabel = tr("Slide %1 of %2");
    m_slideLabel->setText( baseLabel.arg( QString::number( currentSlide()+1 ) )
                               .arg( QString::number( slideCount() ) ) );
    baseLabel = tr("Duration: %1secs", "duration between images in a slide show");
    m_durationLabel->setText(
    baseLabel.arg( QString::number( slide( currentSlide() )->duration()/1000 ) ) );
}

int MMSComposer::currentSlide() const
{
    return m_curSlide;
}

uint MMSComposer::slideCount() const
{
    return m_slides.count();
}

MMSSlide *MMSComposer::slide( uint slide ) const
{
    if( slide >= slideCount() )
        return 0;
    return m_slides.at(slide);
}

class SmilHandler : public QXmlDefaultHandler
{
public:
    QList<MMSSmilPart> parts;
    MMSSmil smil;
    SmilHandler() : m_insidePart( false ) {}

    bool startElement( const QString &, const QString &, const QString &qName,
                                                const QXmlAttributes & atts )
    {
        if( qName == "smil" )
        {
            smil.fgColor = QColor();
            smil.bgColor = QColor();
            smil.parts.clear();
        }
        else if( qName == "par" )
        {
            m_insidePart = true;
            MMSSmilPart newPart;
            if( atts.value( "duration" ).length() )
                newPart.duration = atts.value( "duration" ).toInt();
            smil.parts.append( newPart );
        }
        else if( qName == "region" )
        {
            if( atts.value("background-color").length() )
                smil.bgColor.setNamedColor( atts.value("background-color") );
        }
        else if( m_insidePart )
        {
            if( qName == "img" && atts.value( "src" ).length() )
                smil.parts.last().image = atts.value( "src" );

            else if( qName == "text" && atts.value( "src" ).length() )
                smil.parts.last().text = atts.value( "src" );

            else if( qName == "audio" && atts.value( "src" ).length() )
                smil.parts.last().audio = atts.value( "src" );
        }
        return true;
    }

    bool endElement( const QString &, const QString &, const QString &qName )
    {
        if( qName == "par" )
            m_insidePart = false;
        return true;
    }

private:
    bool m_insidePart;
};

MMSSmil MMSComposer::parseSmil( const QString &smil )
{
    QXmlInputSource input;
    input.setData( smil );
    QXmlSimpleReader reader;
    SmilHandler *handler = new SmilHandler;
    reader.setContentHandler( handler );
    if( !reader.parse( input ) )
        qWarning( "MMSComposer unable to parse smil message." );
    MMSSmil s = handler->smil;
    delete handler;
    return s;
}

void MMSComposer::setMailMessage( Email &mail )
{
    QBitArray handledParts(mail.messagePartCount());
    handledParts.fill(false);
    clear();
    MMSSlide *curSlide = slide( currentSlide() );
    QString doc;
    int smilPartIndex = 0;
    for( uint i = 0 ; i < mail.messagePartCount() ; ++i )
    {
        if( mail.messagePartAt( i ).contentID().toLower().contains("presentation-part") )
        {
            doc = mail.messagePartAt( i ).decodedBody();
            smilPartIndex = i;
            handledParts.setBit(i);
            break;
        }
    }
    if( !doc.isNull() )
    {
        // parse smil
        MMSSmil smil = parseSmil( doc );
        QList<MMSSmilPart> smilParts = smil.parts;
        int numSlides = 0;
        for( QList<MMSSmilPart>::Iterator it = smilParts.begin() ;
                                    it != smilParts.end() ; ++it, ++numSlides )
        {
            if( numSlides )
                addSlide();
            MMSSlide *curSlide = slide( slideCount() -1 );
            for( uint i = 0 ; i < mail.messagePartCount() ; ++i )
            {
                if(i == smilPartIndex)
                    continue;
                QString partName = mail.messagePartAt( i ).contentLocation();
                if( partName == (*it).text )
                {
                    MailMessagePart part = mail.messagePartAt( i );
                    QByteArray data = QByteArray::fromBase64(part.rawEncodedBody());
                    QTextStream stream( data, QIODevice::ReadOnly );
                    QString t = stream.readAll();
                    curSlide->textContent()->setText( t );
                    handledParts.setBit(i);
                }
                else if( partName == (*it).image )
                {
                    MailMessagePart part = mail.messagePartAt( i );
                    QByteArray data = QByteArray::fromBase64(part.rawEncodedBody());
                    QPixmap pix;
                    pix.loadFromData(data);
                    curSlide->imageContent()->setImage( pix );
                    handledParts.setBit(i);
                }
                else if ( partName == (*it).audio )
                {
                    MailMessagePart part = mail.messagePartAt( i );
                    QByteArray data = QByteArray::fromBase64(part.rawEncodedBody());
                    curSlide->audioContent()->setAudio( data, part.contentLocation() );
                    handledParts.setBit(i);
                }
            }
        }
        if( smil.bgColor.isValid() )
            setBackgroundColor( smil.bgColor );
        if( smil.fgColor.isValid() )
            setTextColor( smil.fgColor );
        setCurrentSlide( 0 );
    } else {
        if (!mail.plainTextBody().isEmpty()) {
            curSlide->textContent()->setText(mail.plainTextBody());
        }
        for (uint i = 0 ; i < mail.messagePartCount() ; ++i) {
            MailMessagePart part = mail.messagePartAt( i );
            QByteArray data = QByteArray::fromBase64(part.rawEncodedBody());
            if( part.contentType().toLower().contains( "text" ) )
            {
                QTextStream stream( data, QIODevice::ReadOnly );
                QString t = stream.readAll();
                curSlide->textContent()->setText( t );
                handledParts.setBit(i);
            } else if( part.contentType().toLower().contains( "image" ) ) {
                QPixmap pix;
                QDataStream stream( &data, QIODevice::ReadOnly );
                stream >> pix;
                curSlide->imageContent()->setImage( pix );
                handledParts.setBit(i);
            } else if( part.contentType().toLower().contains( "audio" ) ) {
                curSlide->audioContent()->setAudio( data, part.contentLocation() );
                handledParts.setBit(i);
            }
        }
    }

    // attach anything that hasn't been dealt with yet
    for (uint i = 0 ; i < mail.messagePartCount() ; ++i) {
        if (!handledParts[i]) {
            MailMessagePart &part = mail.messagePartAt(i);
            mail.validateFile(part);
            // XXX save these parts
        }
    }
}

bool MMSComposer::hasContent() const
{
    for( uint i = 0 ; i < slideCount() ; ++i )
        if( !slide( i )->imageContent()->image().isNull()
            || !slide( i )->textContent()->text().isEmpty()
            || !slide( i )->audioContent()->audio().isEmpty()) {
            return true;
        }
    return false;
}

void MMSComposer::clear()
{
    while( slideCount() > 1 )
        removeSlide( slideCount() - 1 );
    if( slideCount() )
    {
        MMSSlide *cur = slide( currentSlide() );
        cur->imageContent()->setImage( QPixmap() );
        cur->textContent()->setText( QString() );
        cur->audioContent()->setAudio( QContent() );
    }
}

MMSComposerInterface::MMSComposerInterface( QObject *parent, const char *name )
    : ComposerInterface( parent, name )
{
    QWidget *par = 0;
    if( parent && parent->isWidgetType() )
        par = (QWidget *)parent;
    m_composer = new MMSComposer(par);
    connect( m_composer, SIGNAL(contentChanged()), this, SIGNAL(contentChanged()) );
}

MMSComposerInterface::~MMSComposerInterface()
{
    delete m_composer;
}

/*
QCString MMSComposerInterface::id()
{
    QCString t = QString::number( type() ).toLatin1();
    return t + "-Default"; // default mms composer
}

ComposerInterface::ComposerType MMSComposerInterface::type()
{
    return MailMessage::MMS;
}

QString MMSComposerInterface::displayName()
{
    return tr("MMS");
}
*/

bool MMSComposerInterface::hasContent() const
{
    return m_composer->hasContent();
}

void MMSComposerInterface::setMailMessage( Email &mail )
{
    m_composer->setMailMessage( mail );
}

void MMSComposerInterface::getContent( MailMessage &mmsMail ) const
{
    QString doc;
    QStringList files;
    // calling this will remove any previous smil messages' data
    static QString contentDir;
    if (contentDir.isEmpty())
        contentDir = Qtopia::tempDir() + "mms/";
    if( QFile::exists( contentDir ) )
    {
        QFile f( contentDir );
        f.remove();
    }
    QDir dir( contentDir );
    if( !dir.exists() )
        dir.mkdir( contentDir );
    if( !dir.exists() )
    {
        qWarning() << "Can't create directory for MMS content" << contentDir;
        return;
    }
    QStringList oldFiles = dir.entryList();
    for( QStringList::ConstIterator it = oldFiles.begin() ; it != oldFiles.end() ; ++it )
    {
        QFile f( contentDir + *it );
        f.remove();
    }

    //clean slate, generate document
    QString rootLayout, regions, parts, part;
    static const QString docTemplate =
    "<smil>\n"
    "   <head>\n"
    "       <meta name=\"title\" content=\"mms\"/>\n"
    "       <meta name=\"author\" content=\"%1\"/>\n"
    "       <layout>\n"
    "           %2\n"
    "       </layout>\n"
    "   </head>\n"
    "   <body>\n"
    "       %3\n"
    "   </body>\n"
    "</smil>\n"
    ; // 1.author 2.rootlayout&regions 3.parts
    static const QString rootLayoutTemplate =
    "<root-layout width=\"%1\" height=\"%2\"/>\n"
    ; // 1.width 2.height
    static const QString regionTemplate =
    "<region id=\"%1\" width=\"%2\" height=\"%3\" left=\"%4\" top=\"%5\" background-color=\"%6\"/>\n"
    ; // 1.id 2.width 3.height 4.left 5.top
    static const QString partTemplate =
    "<par dur=\"%1\">\n"
    "   %2\n"
    "</par>\n"
    ; // 1.duration 2.contentitems
    static const QString imageTemplate =
    "<img src=\"%1\" region=\"%2\"/>\n"
    ; // 1.src 2.region
    static const QString textTemplate =
    "<text src=\"%1\" region=\"%2\"/>\n"
    ; // 1.src 2.region
    static const QString audioTemplate =
    "<audio src=\"%1\"/>\n"
    ; // 1.src

    /*
       if the composer only contains one piece of content
       either an image or some text, then we don't need to
       generate smil output
   */
    int contentCount = 0;
    MMSSlideImage *imageContent = 0;
    MMSSlideText *textContent = 0;
    MMSSlideAudio *audioContent = 0;
    for( uint s = 0 ; s < m_composer->slideCount() ; ++s )
    {
        MMSSlide *curSlide = m_composer->slide( s );
        if( !curSlide->imageContent()->image().isNull() )
        {
            imageContent = curSlide->imageContent();
            ++contentCount;
        }
        if( !curSlide->textContent()->text().isNull() )
        {
            textContent = curSlide->textContent();
            ++contentCount;
        }
        if( !curSlide->audioContent()->audio().isEmpty() )
        {
            audioContent = curSlide->audioContent();
            ++contentCount;
        }
        if( contentCount > 1 )
            break;
    }
    if( contentCount == 1 )
    {
        if( imageContent )
        {
            QString f = QString( contentDir ) + "mmsimage.jpg";
            imageContent->image().save( f, "JPEG" );
            files.append( f );
        } else if( textContent ) {
            doc = textContent->text();
            mmsMail.setPlainTextBody(doc);
        } else if (audioContent) {
            QMimeType mime(audioContent->mimeType());
            QString ext = mime.extension();
            QString fn = QString( contentDir ) + "mmsaudio" + '.' + ext;
            QFile f(fn);
            if (f.open(QIODevice::WriteOnly)) {
                f.write(audioContent->audio().data(), audioContent->audio().size());
                files.append( fn );
            }
        }
    }
    else if( contentCount > 1 )
    {
        rootLayout = rootLayoutTemplate.arg( m_composer->contentsRect().width() )
                                   .arg( m_composer->contentsRect().height() );
        QRect largestText, largestImage;
        for( int s = 0 ; s < (int)m_composer->slideCount() ; ++s )
        {
            MMSSlide *curSlide = m_composer->slide( s );
            imageContent = curSlide->imageContent();
            textContent = curSlide->textContent();
            audioContent = curSlide->audioContent();

            part = QString();
            if( !imageContent->image().isNull() )
            {
                QRect cr = imageContent->contentsRect();
                if( cr.width() > largestImage.width() )
                    largestImage.setWidth( cr.width() );
                if( cr.height() > largestImage.height() )
                    largestImage.setHeight( cr.height() );

                QString imgFileName = "image" + QString::number( s ) + ".jpg";
                if( !imageContent->image().save( contentDir + imgFileName, "JPEG" ) )
                {
                    qWarning() << "Cannot save image" << (contentDir+imgFileName);
                    return;
                }
                files.append( contentDir + imgFileName );
                part += imageTemplate.arg( imgFileName )
                                            .arg( "image" );

            }
            if( !textContent->text().isNull() )
            {
                QRect cr = textContent->contentsRect();
                if( cr.height() > largestText.height() )
                {
                    largestText.setHeight( cr.height() );
                }

                QString textFileName = "text" + QString::number( s ) + ".txt";
                QFile f( contentDir + textFileName );
                if( !f.open( QIODevice::WriteOnly ) )
                {
                    qWarning() << "Cannot open text file for writing:" << textFileName;
                    return;
                }
                QTextStream stream( &f );
                stream << textContent->text();
                files.append( contentDir + textFileName );
                part += textTemplate.arg( textFileName )
                                    .arg( "text" );
            }
            if( !audioContent->audio().isEmpty() ) {
                QMimeType mime(audioContent->mimeType());
                QString ext = mime.extension();
                QString fn = "mmsaudio" + QString::number( s ) + '.' + ext;
                QFile f(contentDir + fn);
                if (f.open(QIODevice::WriteOnly)) {
                    f.write(audioContent->audio().data(), audioContent->audio().size());
                    files.append( contentDir + fn );
                }
                part += audioTemplate.arg(fn);
            }
            if( !part.isEmpty() )
            {
                part = partTemplate.arg( QString::number( curSlide->duration() ) + "ms" )
                                   .arg( part )
                                   ;
                parts += part;
            }

        }
        QRect imageRect;
        imageRect.setX( 0 );
        imageRect.setY( 0 );
        imageRect.setWidth( m_composer->contentsRect().width() );
        imageRect.setHeight( m_composer->contentsRect().height() - largestText.height() );

        largestText.setX( 0 );
        int h = largestText.height();
        largestText.setY( imageRect.height() );
        largestText.setHeight( h );
        largestText.setWidth(m_composer->contentsRect().width());

        if( imageRect.width() > 0 && imageRect.height() > 0 )
        {
            regions += regionTemplate.arg( "image" )
                                   .arg( imageRect.width() )
                                   .arg( imageRect.height() )
                                   .arg( imageRect.x() )
                                   .arg( imageRect.y() )
                                   .arg( m_composer->backgroundColor().name().toUpper() );
                                   ;
        }
        if( largestText.width() > 0 && largestText.height() > 0 )
        {
            regions += regionTemplate.arg( "text" )
                               .arg( largestText.width() )
                               .arg( largestText.height() )
                               .arg( largestText.x() )
                               .arg( largestText.y() )
                               .arg( m_composer->backgroundColor().name().toUpper() );
                               ;
        }
        doc = docTemplate.arg( Qtopia::ownerName() )
                         .arg( rootLayout + regions )
                         .arg( parts )
                         ;
        // add the smil document to the message
        mmsMail.setMultipartRelated(true);
        MailMessagePart docPart;
        docPart.setContentType( "application/smil" );
        docPart.setContentID( "<presentation-part>" );
        docPart.setEncodedBody( doc, EightBit );
        mmsMail.addMessagePart( docPart );
    }

    mmsMail.setType( MailMessage::MMS );

    // add binary data as mail attachments
    for( QStringList::ConstIterator it = files.begin() ; it != files.end() ;
                                                                        ++it )
    {
        MailMessagePart part;

        QFileInfo fi( *it );
        part.setName( fi.baseName() );
        part.setContentLocation( fi.fileName() );
        part.setFilename( fi.absoluteFilePath() );
        part.setStoredFilename( fi.absoluteFilePath() );
        part.setLinkFile( fi.absoluteFilePath() );
        QMimeType mt( fi.absoluteFilePath() );
        part.setContentType( mt.id() );

        QFile f( fi.absoluteFilePath() );
        if( !f.open( QIODevice::ReadOnly ) )
        {
            qWarning() << "Could not open MMS attachment for reading" << fi.absoluteFilePath();
            continue;
        }
        QDataStream in(&f);
        part.setBody(in,Base64);
        mmsMail.addMessagePart( part );
    }
}

void MMSComposerInterface::attach( const QContent &lnk )
{
    if (!m_composer->slideCount())
        m_composer->addSlide();
    MMSSlide *curSlide = m_composer->slide( m_composer->slideCount()-1 );

    if (lnk.type().startsWith("image/")) {
        QPixmap pm(lnk.file());
        if (!pm.isNull()) {
            if (!curSlide->imageContent()->image().isNull()) {
                // If there is already an image in the last slide, add a new one
                m_composer->addSlide();
                curSlide = m_composer->slide( m_composer->slideCount()-1 );
            }
            curSlide->imageContent()->setImage(pm);
        }
    } else if (lnk.type() == "text/plain") {
        QFile file(lnk.file());
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream ts(&file);
            if (!curSlide->textContent()->text().isEmpty()) {
                // If there is already text in the last slide, add a new one
                m_composer->addSlide();
                curSlide = m_composer->slide( m_composer->slideCount()-1 );
            }
            curSlide->textContent()->setText(ts.readAll());
        }
    } else if (lnk.type().startsWith("audio/")) {
        if (!curSlide->audioContent()->audio().isEmpty()) {
            // If there is already audio in the last slide, add a new one
            m_composer->addSlide();
            curSlide = m_composer->slide( m_composer->slideCount()-1 );
        }
        curSlide->audioContent()->setAudio(lnk);
    } else {
        // XXX deal with other attachments
    }
}

void MMSComposerInterface::attach( const QString &fileName )
{
    attach(QContent(fileName));
}

void MMSComposerInterface::clear()
{
    m_composer->clear();
}

QWidget *MMSComposerInterface::widget() const
{
    return m_composer;
}
