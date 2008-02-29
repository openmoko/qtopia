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

#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include <qdialog.h>
#include <qcontent.h>

class QDSServices;
class AudioSourceWidgetPrivate;
class AudioSourceWidget : public QWidget
{
    Q_OBJECT
public:
    AudioSourceWidget( QWidget *parent );
    ~AudioSourceWidget();

    void setDefaultAudio(const QString &type, const QString &subFormat, int fr, int ch);
    virtual QContent audio() const;

public slots:
    virtual void setAudio( const QContent & );

protected slots:
    void change();
    void remove();
    void audioRequest( int id );

protected:
    void haveAudio( bool f );

private:
    void init();
    AudioSourceWidgetPrivate *d;
};

class AudioSourceDialog : public QDialog
{
    Q_OBJECT
public:
    AudioSourceDialog( QWidget *parent, const QString name = QString(), bool modal = true, Qt::WFlags fl =
#ifdef QTOPIA_DESKTOP
    WStyle_Customize | WStyle_Dialog | WStyle_Title
#else
     0
#endif
    );
    virtual ~AudioSourceDialog();

    void setDefaultAudio(const QString &type, const QString &subFormat, int fr, int ch);
    virtual void setAudio( const QContent & );
    virtual QContent audio() const;
private:
    void init();
    AudioSourceWidget *mWidget;
};

#endif

