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

#ifndef HAVE_RINGTONEEDITOR_H
#define HAVE_RINGTONEEDITOR_H

#include <QListWidget>
#include <QPushButton>
#include <qcontent.h>

#ifndef QTOPIA_DESKTOP
# ifdef MEDIA_SERVER
class QSoundControl;
# else
class QWSSoundClient;
# endif
#endif

class RingToneLink;

class RingToneSelect : public QListWidget
{
    Q_OBJECT
public:
    RingToneSelect(QWidget *parent);
    ~RingToneSelect();

    void setAllowNone(bool b);
    bool allowNone() const { return aNone; }

    void closeEvent( QCloseEvent *e );

    void setCurrentTone(const QContent &d);
    QContent currentTone() const;

    void stopSound();
    void setVolume( const int vol ) { volume = vol; volumeSet = true; }
    void playCurrentSound();

signals:
    void selected(const QContent &);

protected:
    void showEvent( QShowEvent *e );
    bool event( QEvent *e );

private slots:

    void selectItem(int pos);
    void selectItem(QListWidgetItem*);

    void startHoverTimer(int);
    void hoverTimeout();
    void playDone();

private:
    // should also make a bunch of QListWidget functions private.
    void init();

    void addFromDocuments();
    void addCustom( const QContent &link );
    void saveCustom();

    RingToneLink *linkItem(int index) const;
    RingToneLink *currentLinkItem() const;

    int customCount;
    int volume;
    bool volumeSet;
# ifdef MEDIA_SERVER
    QSoundControl *scontrol;
# else
    QWSSoundClient *sclient;
# endif
    QTimer *stimer;
    bool aNone;
    int m_currentItem;

    QListWidgetItem *noneItem;
    QListWidgetItem *otherItem;
};

class RingToneButton : public QPushButton
{
    Q_OBJECT
public:
    RingToneButton( QWidget *parent );
    RingToneButton( const QContent &tone, QWidget *parent );

    void setTone( const QContent &tone );
    QContent tone() const;

    void setAllowNone(bool b);
    bool allowNone() const;
    void setVolume( const int vol ) { rtl->setVolume( vol ); }
    void stopSound() { rtl->stopSound(); }

protected slots:
    void selectTone();
    void playCurrentSound() { rtl->setCurrentTone( mTone ); rtl->playCurrentSound(); }

private:
    void init();

    QContent mTone;
    RingToneSelect *rtl;
    QDialog *dlg;
    bool aNone;
};

#endif
