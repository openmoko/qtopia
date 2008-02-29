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

#include <QAction>
#include <QFileInfo>
#include <QDialog>
#ifndef QTOPIA_DESKTOP
# ifdef MEDIA_SERVER
#  include <qsoundcontrol.h>
#  include <QSound>
# else
#  include <qsoundqss_qws.h>
# endif
#endif
#include <QLayout>
#include <QListWidget>
#include <QPainter>
#include <QTimer>

#include <qtopiaapplication.h>
#include <qcontent.h>
#include <qcontentset.h>

#include <qtranslatablesettings.h>
#include <qdocumentselector.h>
#ifdef QTOPIA_PHONE
# include <qsoftmenubar.h>
#endif

#include "ringtoneeditor.h"

#include <QPhoneProfile>
#include <QPhoneProfileManager>

// RingToneSelect
// up to 4 most recently used user ringtones
// get from settings.
// up to ? system ringtones
// get from etc
// other... select from documents.
// play on highlight.

static const int maxCustomRingTones = 4;

class RingToneLink : public QListWidgetItem
{
public:
    RingToneLink(const QContent &d, QListWidget *l)
        : QListWidgetItem(d.name(), l), dl(d) {}

    ~RingToneLink() {}

    QString text() const { return dl.name(); }
    QContent link() const { return dl; }

    int width( const QListWidget *lb ) const;
    int height( const QListWidget *lb ) const;

    void paint( QPainter *p );

private:
    QContent dl;
};

int RingToneLink::width( const QListWidget *lb ) const
{
    return qMax( lb ? lb->viewport()->width() : 0,
        QApplication::globalStrut().width() );
}

int RingToneLink::height( const QListWidget *lb ) const
{
    if( !lb )
        return 0;
    QFontMetrics fm = lb->fontMetrics();
    return qMax( fm.boundingRect( 0, 0, width( lb ), fm.lineSpacing(), 0, text() ).height(),
        QApplication::globalStrut().height() );
}

void RingToneLink::paint( QPainter *p )
{
    const int w = width( listWidget() );
    const int h = height( listWidget() );
    p->drawText( 3, 0, w-3*2, h, 0, text() );
}

RingToneSelect::RingToneSelect(QWidget *parent)
    : QListWidget(parent), volume(0), volumeSet(false),
# ifdef MEDIA_SERVER
    scontrol(0)
# else
    sclient(0)
# endif
{
    init();
}

void RingToneSelect::init()
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

# ifndef MEDIA_SERVER
    sclient = new QWSSoundClient(this);
# endif
    stimer = new QTimer(this);
    aNone = false;
    m_currentItem = -1;

    connect(stimer, SIGNAL(timeout()), this, SLOT(hoverTimeout()));
    connect(this, SIGNAL(currentRowChanged(int)), this, SLOT(startHoverTimer(int)));
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(selectItem(QListWidgetItem*)));
    connect(this, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(selectItem(QListWidgetItem*)));
    // load up list of items.
    // first list of most recently accessed items, is from Settings/RingTones.conf

    QSettings c("Trolltech","RingTones"); // No tr
    c.beginGroup("Recent"); // No tr
    int num = c.value("count", 0).toInt(); // No tr
    int i;
    //int regg = 0;
    customCount = 0;
    for (i = 1; i <= num; i++)
    {
        QString linkname = c.value("item" + QString::number(i)).toString(); // No tr
        QContent dl(linkname);
        if (dl.fileKnown()) {
            customCount++;
            new RingToneLink(dl, this);
        }
    }

    QContentSet rtones;
    rtones.setCriteria(QContentFilter::Location, Qtopia::qtopiaDir() + "etc/SystemRingTones");
    rtones.addCriteria(QContentFilter::MimeType, "audio/*", QContentFilter::And);

    QContentSetModel model(&rtones);
    for(i = 0; i < model.rowCount(); i++)
    {
        const QContent &dl = model.content(i);
        if (dl.fileKnown())
            new RingToneLink(dl, this);
    }

    otherItem = new QListWidgetItem(tr("Other...", "Select Ringtone from documents"), this);
    noneItem = new QListWidgetItem(tr("None", "Set no ringtone selected"), 0);
}

RingToneSelect::~RingToneSelect()
{
    stopSound();
}

void RingToneSelect::showEvent( QShowEvent *e )
{
    QListWidget::showEvent( e );
    update();
}

bool RingToneSelect::event( QEvent *e )
{
    if ( e->type() == QEvent::WindowDeactivate ) {
        stopSound();
    }
    return QListWidget::event( e );
}

void RingToneSelect::setAllowNone(bool a)
{
    if (a == aNone)
        return;

    aNone = a;
    if (aNone) {
        // have to add none item
        insertItem(0, noneItem);
    } else {
        // have to remove none item.
        takeItem(row(noneItem));
    }

}

void RingToneSelect::closeEvent( QCloseEvent *e )
{
    stopSound();
    QListWidget::closeEvent(e);
}


RingToneLink *RingToneSelect::linkItem(int index) const
{
    if (index == (int)count() - 1 || (index == 0 && aNone))
        return 0;
    return (RingToneLink*)item(index);
}

RingToneLink *RingToneSelect::currentLinkItem() const
{
    return linkItem(currentRow());
}

void RingToneSelect::setCurrentTone(const QContent &d)
{
    stopSound();
    if (!d.fileKnown()) {
        if (aNone)
            setCurrentRow(0);
    } else {
    // for each item, check its link.  same as d, then set as Current Item
        for (int i = aNone ? 1 : 0; i < (int)count()-1; i++) {
            QContent lnk = linkItem(i)->link();
            if (lnk.linkFileKnown() && d.linkFileKnown() && lnk.linkFile() == d.linkFile()
                || lnk.file() == d.file()) {

                if (m_currentItem == i) // Already been here, replay
                    stimer->start(200);
                else
                    setCurrentRow(i);

                break;
            }
        }
    }
}

QContent RingToneSelect::currentTone() const
{
    if (currentLinkItem())
       return currentLinkItem()->link();
    return QContent();
}

void RingToneSelect::selectItem(int pos)
{
    if (item(pos) == otherItem) // other,
        addFromDocuments();
    else {
        stopSound();
        if (linkItem(pos))
            emit selected(linkItem(pos)->link());
        else
            emit selected(QContent());
    }
}

void RingToneSelect::selectItem(QListWidgetItem *item)
{
    if( !item )
        return;
    selectItem( row( item ) );
}

void RingToneSelect::addFromDocuments()
{
    QString dlgcaption = tr("Select Ringtone");

    QDocumentSelectorDialog *dlg = new QDocumentSelectorDialog(this);
    dlg->setModal(true);
    dlg->setWindowTitle(dlgcaption);
    dlg->setSelectPermission( QDrmRights::Play );
    dlg->setMandatoryPermissions( QDrmRights::Play | QDrmRights::Automated );

    QContentFilter audiofilter(QContent::Document);

    audiofilter &= QContentFilter( QContentFilter::MimeType, QLatin1String( "audio/*" ) );

    dlg->setFilter( audiofilter );
    dlg->disableOptions( QDocumentSelector::ContextMenu );

    if (QtopiaApplication::execDialog(dlg)) {
        addCustom( dlg->selectedDocument() );
    }
}

void RingToneSelect::addCustom( const QContent &content )
{
    QContent link = content;

    // first find out if custom is in the list already.
    for (int i = 0; i < customCount; i++) {
        QContent lnk = linkItem(aNone ? i+1 : i)->link();
        if (lnk.linkFileKnown() && link.linkFileKnown()
            && lnk.linkFile() == link.linkFile()
            || lnk.file() == link.file()) {
           // ok, just move it to the first of the list.
           RingToneLink *rtli = linkItem(aNone ? i+1 : i);
           takeItem(row(rtli));
           insertItem(aNone ? 1 : 0, rtli);
           setCurrentRow(aNone ? 1 : 0);
           return;
       }
    }

    if (customCount == maxCustomRingTones) {
        // need to remove the last.
        customCount--;
        takeItem(aNone ? customCount+1 : customCount);
    }
    if( !link.isValid() )
        link.commit();
    // done in two steps because it needs to go at the start.
    RingToneLink *rtli = new RingToneLink(link, 0);
    insertItem(aNone ? 1 : 0, rtli);
    setCurrentRow(aNone ? 1 : 0);
    customCount++;

    saveCustom();
}

void RingToneSelect::saveCustom()
{
    QSettings c("Trolltech","RingTones"); // No tr
    c.beginGroup("Recent"); // No tr
    c.setValue("count", customCount); // No tr
    int i;
    for (i = 0; i < customCount; i++) {
        QContent lnk = linkItem(aNone ? i+1 : i)->link();
        c.setValue("item" + QString::number(i+1),
        lnk.linkFileKnown() ? lnk.linkFile() : lnk.file());
    }
}

void RingToneSelect::startHoverTimer(int selectedItem)
{
    if (m_currentItem != selectedItem)
    {
        stimer->stop();

        stopSound();

        m_currentItem = selectedItem;

        if (m_currentItem != int(count()) - 1)
        {
            stimer->start(200);
        }
    }
}

void RingToneSelect::hoverTimeout()
{
    playCurrentSound();
}

void RingToneSelect::playDone()
{
    stopSound();
    stimer->start(0);
}

void RingToneSelect::playCurrentSound()
{
    // shut off - use done()->playDone() to know when to start again
    stimer->stop();

    if (currentLinkItem()) {
#ifdef MEDIA_SERVER
        if (scontrol == NULL)
        {
            scontrol = new QSoundControl( new QSound( currentLinkItem()->link().file() ) );

            // uncomment this to play a tone more than once
            //connect(scontrol, SIGNAL(done()), this, SLOT(playDone()));
        }

        scontrol->setPriority( QSoundControl::RingTone );
        if ( !volumeSet ) {
            QPhoneProfileManager manager;
            QPhoneProfile curProfile = manager.activeProfile();
            if( curProfile.volume() < 0 || curProfile.volume() > 5 ) {
                qWarning("BUG: Invalid volume setting for profile %d", curProfile.volume());
                volume = 5;
            } else {
                volume = curProfile.volume();
            }
            volumeSet = true;
        }
        scontrol->setVolume( volume * 20 );

        scontrol->sound()->play();
#else
        sclient->play(0, currentLinkItem()->link().file(), volume * 20);
#endif
    }
}

void RingToneSelect::stopSound()
{
#ifdef MEDIA_SERVER
    if (scontrol)
    {
        scontrol->sound()->stop();

        delete scontrol->sound();
        delete scontrol;

        scontrol = NULL;
    }
#else
    if ( sclient ) sclient->stop(0);
#endif
}

RingToneButton::RingToneButton( QWidget *parent )
    : QPushButton( parent ), rtl(0), dlg(0), aNone(false)
{
    init();
}

RingToneButton::RingToneButton( const QContent &tone, QWidget *parent )
    : QPushButton( parent ), rtl(0), dlg(0), aNone(false)
{
    init();
    setTone( tone );
}

void RingToneButton::init()
{
    setText(tr("None", "no ring tone selected"));
    //construction should not be used.
    connect(this, SIGNAL(clicked()), this, SLOT(selectTone()));
    dlg = new QDialog(this);
    dlg->setModal(true);
#ifdef QTOPIA_PHONE
    QtopiaApplication::setMenuLike(dlg, true);
#endif
    QVBoxLayout *vbl = new QVBoxLayout(dlg);

    dlg->setWindowTitle(tr("Select Ringtone"));

    rtl = new RingToneSelect(dlg);
    connect(rtl, SIGNAL(selected(const QContent &)), dlg, SLOT(accept()));
    vbl->addWidget(rtl);
    if (aNone)
        rtl->setAllowNone(true);
}

void RingToneButton::setTone( const QContent &tone )
{
    if( !tone.fileKnown() ) {
        setText(tr("None", "no ring tone selected"));
        mTone = QContent();
    } else {
        mTone = tone;
        // limit total string length
        if (mTone.name().length() > 15) {
            static const QString overlength = tr("%1...");
            setText(overlength.arg(mTone.name().left(12)));
        } else {
            setText( mTone.name() );
        }
    }
}

QContent RingToneButton::tone() const
{
    return mTone;
}

void RingToneButton::setAllowNone(bool b)
{
    aNone = b;
    if (rtl)
        rtl->setAllowNone(b);
}

bool RingToneButton::allowNone() const
{
    return aNone;
}

void RingToneButton::selectTone()
{
    rtl->setCurrentTone( mTone );
    if (QtopiaApplication::execDialog(dlg)) {
        setTone( rtl->currentTone() );
    }
    rtl->stopSound();
}

