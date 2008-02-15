/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef RADIOPLAYER_H
#define RADIOPLAYER_H

#include <QMainWindow>
#include <QIcon>
#include <QAction>
#include <QKeyEvent>

#include "ui_radioplayer.h"
#include "radiobandmanager.h"

class RadioPresetsModel;

class RadioPlayer : public QMainWindow
{
    Q_OBJECT
public:
    RadioPlayer( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~RadioPlayer();

protected:
    void keyPressEvent( QKeyEvent *e );

public slots:
    void setMute( bool value );
    void setStation( const QString& band, RadioBand::Frequency frequency );
    void setDocument( const QString& doc );

private slots:
    void muteOrUnmute();
    void removeVolumeDisplay();
    void changeBand();
    void addToSpeedDial();
    void presets();
    void addToPresets();
    void scanFoundStation( RadioBand::Frequency frequency, int band );
    void updateScanActions();
    void updateStationDetails();

private:
    void initUi();
    void updateMute();
    void adjustVolume( int diff );

private:
    Ui_RadioPlayer *ui;
    RadioBandManager *radio;
    QMenu *menu;
    QAction *muteAction;
    QAction *addToSpeedDialAction;
    QAction *presetsAction;
    QAction *addToPresetsAction;
    QAction *scanForwardAction;
    QAction *scanBackwardAction;
    QAction *scanAllAction;
    QAction *stopScanAction;
    QTimer *volumeDisplayTimer;
    QAction **bandActions;
    int numBandActions;
    RadioPresetsModel *presetsModel;
    QString stationGenre;
};

#endif
