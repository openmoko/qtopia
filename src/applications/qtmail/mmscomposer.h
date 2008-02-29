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

#ifndef MMSCOMPOSER_H
#define MMSCOMPOSER_H

#include "composer.h"
#include <qcontent.h>
#include <qwidget.h>
#include <qlist.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qpixmap.h>
#include <qpushbutton.h>

class QMenu;
class MMSComposer;
class QStackedWidget;
class QEvent;
class QKeyEvent;
class QPaintEvent;
class QShowEvent;
class QMouseEvent;

class MMSSlideImage : public QLabel
{
    Q_OBJECT
public:
    MMSSlideImage(QWidget *parent);

    QRect contentsRect() const;
    QSize sizeHint() const;

    void setImage( const QPixmap &image );
    QPixmap image() const;

signals:
    void clicked();
    void changed();
    void leftPressed(); // for keypad mode only
    void rightPressed(); // ""

protected slots:
    void select();

protected:
    void resizeEvent( QResizeEvent *event );
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void keyPressEvent( QKeyEvent *event );
    void paintEvent( QPaintEvent *event );
    QPixmap scale( const QPixmap &src ) const;

private:
    QPixmap m_image;
    bool m_pressed;
};

class MMSSlideText : public QTextEdit
{
    Q_OBJECT
public:
    MMSSlideText(QWidget *parent);

    void setText( const QString &txt );
    QString text() const;

    QSize sizeHint() const;

    static const QString defaultText;

    QRect contentsRect() const;
signals:
    void leftPressed(); // for keypad mode only
    void rightPressed(); // ""

protected:
    void keyPressEvent( QKeyEvent *e );
    bool event( QEvent *e );

private:
    bool m_hasFocus;
};

class MMSSlideAudio : public QPushButton
{
    Q_OBJECT
public:
    MMSSlideAudio(QWidget *parent);

    void setAudio( const QContent &fn );
    void setAudio( const QByteArray &, const QString & );
    QByteArray audio() const;
    QString mimeType() const;

private slots:
    void select();

protected:
    void keyPressEvent( QKeyEvent *e );

signals:
    void leftPressed(); // for keypad mode only
    void rightPressed(); // ""
    void changed();

private:
    QByteArray audioData;
    QString audioName;
    mutable QString audioType;
};

struct MMSSmilPart
{
    MMSSmilPart() : duration( 5000 ) {}
    int duration;
    QString image;
    QString text;
    QString audio;
};

struct MMSSmil
{
    QColor bgColor;
    QColor fgColor;
    QList<MMSSmilPart> parts;
};


class MMSSlide : public QWidget
{
    friend class MMSComposer; // crap, composer needs to install context menus on widgets

    Q_OBJECT
public:
    MMSSlide(QWidget *parent = 0);

    MMSSlideImage *imageContent() const;
    MMSSlideText *textContent() const;
    MMSSlideAudio *audioContent() const;

    void setDuration( int t );
    int duration() const;

signals:
    void leftPressed();
    void rightPressed();
    void durationChanged(int);

protected:
    void showEvent( QShowEvent *event );

private:
    MMSSlideImage *m_imageContent;
    MMSSlideText *m_textContent;
    MMSSlideAudio *m_audioContent;
    bool m_firstShow;
    int m_duration;
};

class MMSComposer : public QWidget
{
    Q_OBJECT
public:
    MMSComposer(QWidget *parent = 0);
    ~MMSComposer();

    QRect contentsRect() const;

    void setMailMessage( Email &mail );

    bool hasContent() const;

    uint slideCount() const;
    MMSSlide *slide( uint slide ) const;
    int currentSlide() const;

public slots:
    void clear();
    void addSlide();
    void removeSlide();
    void addSlide( int a_slide );
    void removeSlide( int a_slide );
    void nextSlide();
    void previousSlide();
    void setCurrentSlide( int slide );
    void updateLabels();
    void setTextColor( const QColor &col );
    void setBackgroundColor( const QColor &col );
    QColor textColor() const;
    QColor backgroundColor() const;

protected slots:
    void slideOptions();

private:
    MMSSmil parseSmil( const QString &doc );

signals:
    void currentChanged( uint );
    void contentChanged();

private:
    QLabel *m_slideLabel, *m_durationLabel;
    QStackedWidget *m_slideStack;
    int m_curSlide;
    QList<MMSSlide*> m_slides;
    bool m_internalUpdate;
    QColor m_textColor, m_backgroundColor;
};

class MMSComposerInterface : public ComposerInterface
{
    Q_OBJECT

public:
    MMSComposerInterface(QObject *parent = 0, const char *name=0);
    ~MMSComposerInterface();

    /*
    static QCString id();
    static ComposerType type();
    static QString displayName();
    */

    bool hasContent() const;
    void getContent( MailMessage &email ) const;
    void setMailMessage( Email &mail );

    virtual void attach( const QContent &lnk );
    virtual void attach( const QString &fileName );

    QWidget *widget() const;

public slots:
    void clear();

private:
    MMSComposer *m_composer;
};

#endif
