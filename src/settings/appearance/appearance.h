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

#ifndef APPEARANCESETTINGS_H
#define APPEARANCESETTINGS_H

#include <QDialog>
#include <QMap>

#ifdef QTOPIA_PHONE
#include "ui_appearancesettingsbasephone.h"
#else
#include "ui_appearancesettingsbase.h"
#endif

#include "itemcontrol.h"

#ifndef QTOPIA_PHONE
class SampleWindow;
#else
class QWaitWidget;
#endif

class AppearanceSettings : public QDialog, Ui::AppearanceSettingsBase
{
    Q_OBJECT

public:
    AppearanceSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~AppearanceSettings();

protected:
    void accept();
    void reject();
    void resizeEvent( QResizeEvent * );
    bool eventFilter(QObject *o, QEvent *e);

protected slots:
    void populate();
    void applyStyle(AppearanceItemControl::ApplyItemType itemType = AppearanceItemControl::CurrentItem);
#ifdef QTOPIA_PHONE
    void pushSettingStatus();
    void receive( const QString& msg, const QByteArray& data );
    void tabChanged( int curIndex );
#endif

private:
#ifdef QTOPIA_PHONE
    QString status();
    void setStatus( const QString details );
    void pullSettingStatus();
#else
    void fixSampleGeometry();
    bool isWide();
#endif

private:
    QMap<QWidget*,AppearanceItemControl*> itemControls;
#ifdef QTOPIA_PHONE
    BackgroundAppearanceItemControl *bgControl;
    bool isStatusView, isFromActiveProfile;
#else
    SampleWindow *sample;
#endif
};

#endif // APPEARANCESETTINGS_H
