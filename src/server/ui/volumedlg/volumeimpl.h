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
#ifndef VOLUMEIMPL_H
#define VOLUMEIMPL_H

#include "ui_volume.h"
#include <QDialog>

#include "qtopiaserverapplication.h"

class VolumeWidget;
class VolumeDialogImplPrivate;

class VolumeDialogImpl : public QDialog
{
    Q_OBJECT
public:
    VolumeDialogImpl( QWidget* parent = 0, Qt::WFlags fl = 0 );

    void setVolume( bool up );

    static const int TIMEOUT = 1500;

signals:
    void volumeChanged( bool up);
    void setText(QString volume);

protected:
    void timerEvent( QTimerEvent *e );

private slots:
    void resetTimer();
    void valueSpaceVolumeChanged();

private:
    int m_tid;
    int m_oldValue;
    VolumeWidget *volumeWidget;
    VolumeDialogImplPrivate  *m_d;
};

#endif

