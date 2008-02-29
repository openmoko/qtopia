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
#ifndef LAPSETTINGS_H
#define LAPSETTINGS_H

#include <QDialog>

#include <qpowerstatus.h>

#include "ui_lightsettingsbase.h"
#ifdef QTOPIA_PHONE
#include "ui_displaysettingsbase.h"
#endif

class QSettings;

class LightSettings : public QDialog
{
    Q_OBJECT

public:
    LightSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~LightSettings();

protected:
    void accept();
    void reject();

private slots:
    void applyBrightness();
    void applyMode();
    void powerTypeClicked(QAbstractButton *);
#ifdef QTOPIA_PHONE
    void backToHomeScreenClicked( int state );
    void pushSettingStatus();
    void receive( const QString& msg, const QByteArray& data );
#endif
    void sysMessage(const QString&, const QByteArray&);
    void updateEditBoxes();
    void updateLightOffMinValue( int dimValue );
    void updateSuspendMinValue( int );

#ifdef QTOPIA_PHONE
private:
    QString status();
    void setStatus( QString details );
    void pullSettingStatus();
#endif
    void saveConfig();

private:
    struct PowerMode {
        PowerMode()
        {
            intervalDim = 20;
            intervalLightOff = 30;
            intervalSuspend = 60;
            initbright = 255;

            dim = true;
            lightoff = true;
            suspend = true;
            networkedsuspend = false;
        }

        bool dim;
        bool lightoff;
        bool suspend;
        bool networkedsuspend;
        int intervalDim;
        int intervalLightOff;
        int intervalSuspend;
        int initbright;
        bool canSuspend;
    };

    class LightSettingsContainer : public QFrame, private Ui::LightSettingsBase
    {
        public:
            LightSettingsContainer( QWidget *parent = 0, Qt::WFlags f = 0 )
                : QFrame( parent, f )
                {
                    setFrameShape( QFrame::NoFrame );
                    setupUi( this );
                    biginfo->setWordWrap(true);

                }
            QButtonGroup * powerSource;
            friend class LightSettings;
    };

#ifdef QTOPIA_PHONE
    class DisplaySettingsContainer : public QFrame, private Ui::DisplaySettingsBase
    {
        public:
            DisplaySettingsContainer( QWidget *parent = 0, Qt::WFlags f = 0 )
                : QFrame( parent, f )
                {
                    setFrameShape( QFrame::NoFrame );
                    setupUi( this );
                }
            friend class LightSettings;
    };


#endif


    void writeMode(QSettings &config, PowerMode *mode);

private:
    int initbright;

    PowerMode batteryMode;
    PowerMode externalMode;
    PowerMode *currentMode;

    QPowerStatus powerStatus;

    LightSettingsContainer *b;
#ifdef QTOPIA_PHONE
    QMenu* contextMenu;
    DisplaySettingsContainer *d;
    bool showHomeScreen;
    int intervalShowHomeScreen;
    QString keyLock;
    bool isFromActiveProfile; // when viewing the status from profile
#endif
bool isStatusView;
};


#endif // LAPSETTINGS_H

