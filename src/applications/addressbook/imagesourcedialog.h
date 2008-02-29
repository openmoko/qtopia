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

#ifndef IMAGESOURCEDIALOG_H
#define IMAGESOURCEDIALOG_H

#include <qdialog.h>

class QDSServices;
class ImageSourceWidgetPrivate;
class ImageSourceWidget : public QWidget
{
    Q_OBJECT
public:
    ImageSourceWidget( QWidget *parent, const char *name = 0 );
    ImageSourceWidget( const QPixmap &pix, QWidget *parent, const char *name = 0 );
    ~ImageSourceWidget();

    void setMaximumImageSize(const QSize &s);
    QSize maximumImageSize() const;

    virtual QPixmap pixmap() const;

public slots:
    virtual void setPixmap( const QPixmap & );

protected slots:
    void change();
    void remove();

private slots:
    void getImageRequest( int id );
    void editImageRequest( int id );

protected:
    void haveImage( bool f );
    void resizeEvent(QResizeEvent *);

private:
    void init();
#ifdef Q_WS_QWS
    QDSServices* getImageServices;
    QDSServices* editImageServices;
#endif

    ImageSourceWidgetPrivate *d;
};

class ImageSourceDialog : public QDialog
{
    Q_OBJECT
public:
    ImageSourceDialog( QWidget *parent, const char *name = 0, bool modal = true, Qt::WFlags fl = 0 );
    ImageSourceDialog( const QPixmap &pix, QWidget *parent, const char *name = 0, bool modal = true, Qt::WFlags fl = 0 );

    void setMaximumImageSize(const QSize &s);
    QSize maximumImageSize() const;

    virtual void setPixmap( const QPixmap &pix );
    virtual QPixmap pixmap() const;
private:
    void init();
    ImageSourceWidget *mWidget;
};

#endif
