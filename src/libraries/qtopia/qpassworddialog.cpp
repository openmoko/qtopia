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

#include "qpassworddialog.h"
#include <qtopiaapplication.h>
#include <QSettings>
#include <QApplication>
#include <QPushButton>
#include <QDesktopWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

#include <md5hash.h>

#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif

#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#include <QLayout>
#endif

#ifndef Q_OS_WIN32
#include <unistd.h> //for sleep
#else
#include <windows.h>
#include <winbase.h>
#endif
#include "ui_passwordbase_p.h"

static int execDialog( QDialog *dialog, bool nomax )
{
#ifdef QTOPIA_PHONE
    return QtopiaApplication::execDialog( dialog, nomax );
#else
    return dialog.exec();
#endif
}

static int execDialog( QDialog *dialog )
{
    return execDialog( dialog, false ); // nomax defaults to false in QtopiaApp
}


class QPasswordWidget : public QWidget, public Ui::PasswordBase
{
    Q_OBJECT

public:
    QPasswordWidget( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QPasswordWidget();

    void reset();
    void setPrompt( const QString& );
    QString password() const;

    bool eventFilter( QObject *obj, QEvent *e );

signals:
    void passwordEntered( const QString& );

public slots:
    void key();

protected:
    void keyPressEvent( QKeyEvent * );

private:
    void input( QString );
    friend class QPasswordDialog;
    QString text;
    QPasswordDialog::InputMode mode;
};

/*
 *  Constructs a QPasswordWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
QPasswordWidget::QPasswordWidget( QWidget* parent, Qt::WFlags fl )
    : QWidget( parent, fl )
{
    setupUi(this);
    installEventFilter( this );

#ifdef QTOPIA_PHONE
    if  (!Qtopia::mousePreferred()) {
        button_0->hide();
        button_1->hide();
        button_2->hide();
        button_3->hide();
        button_4->hide();
        button_5->hide();
        button_6->hide();
        button_7->hide();
        button_8->hide();
        button_9->hide();
    }
    button_OK->hide();
#endif
    connect(button_0,SIGNAL(clicked()),this,SLOT(key()));
    connect(button_1,SIGNAL(clicked()),this,SLOT(key()));
    connect(button_2,SIGNAL(clicked()),this,SLOT(key()));
    connect(button_3,SIGNAL(clicked()),this,SLOT(key()));
    connect(button_4,SIGNAL(clicked()),this,SLOT(key()));
    connect(button_5,SIGNAL(clicked()),this,SLOT(key()));
    connect(button_6,SIGNAL(clicked()),this,SLOT(key()));
    connect(button_7,SIGNAL(clicked()),this,SLOT(key()));
    connect(button_8,SIGNAL(clicked()),this,SLOT(key()));
    connect(button_9,SIGNAL(clicked()),this,SLOT(key()));
    connect(button_OK,SIGNAL(clicked()),this,SLOT(key()));

    QPalette pal = display->palette();
    QBrush base = pal.brush(QPalette::Normal, QPalette::Base);
    QColor text = pal.color(QPalette::Normal, QPalette::Text);
    pal.setBrush(QPalette::Disabled, QPalette::Background, base);
    pal.setColor(QPalette::Disabled, QPalette::Text, text);
    display->setPalette(pal);

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    reset();
}

/*
 *  Destroys the object and frees any allocated resources
 */
QPasswordWidget::~QPasswordWidget()
{
    // no need to delete child widgets, Qt does it all for us
}

/*!
  \reimp
*/

void QPasswordWidget::key()
{
    QPushButton* s = (QPushButton*)sender();
    if ( s == button_OK )
        emit passwordEntered( text );
    else
        input(s->text());
}


/*!
  \reimp
*/

void QPasswordWidget::keyPressEvent( QKeyEvent *e )
{
#ifdef QTOPIA_PHONE
    if ( (Qtopia::mousePreferred() || (!Qtopia::mousePreferred() && hasEditFocus()))
        && (e->key() == Qt::Key_Back || e->key() == Qt::Key_No) ) {
        if( !Qtopia::mousePreferred() )
            setEditFocus(false);
        e->ignore();
        return;
    }
#endif
    if ( e->key() == Qt::Key_Back ) {
        if (text.length() > 0) {
            text = text.left( text.size() - 1 );
            display->setText( text );
            if ( text.size() == 0 ) {
#ifdef QTOPIA_PHONE
                QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
                QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::Select);
#endif
            }
        } else {
            emit passwordEntered( text );
            return;
        }
    }
#ifdef QTOPIA_PHONE
    if ( e->key() == Qt::Key_Select ) {
        emit passwordEntered( text );
        return;
    }
    if ( e->key() == Qt::Key_NumberSign && mode == QPasswordDialog::Pin ) {
        // Key_NumberSign (#), is required for GCF compliance.
        // GSM 02.30, section 4.6.1, Entry of PIN and PIN2.
        emit passwordEntered( text );
        return;
    }
#else
    if ( e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return ) {
        emit passwordEntered( text );
        return;
    }
#endif
    QString t = e->text().left(1);
    if ( t[0]>='0' && t[0]<='9' ) {
        input(t);
#ifdef QTOPIA_PHONE
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::Ok);
#endif
    }

    QWidget::keyPressEvent( e );
}

void QPasswordWidget::input( QString c )
{
    if (text.length() < 8) {
        text += c;
        display->setText( text );
    }
}

void QPasswordWidget::setPrompt( const QString& s )
{
    prompt->setText( s );
}

void QPasswordWidget::reset()
{
    text = "";
    input("");
}

QString QPasswordWidget::password() const
{
    if (text.isEmpty())
        return "";
    return ( mode == QPasswordDialog::Crypted ? MD5::hash(text) : text );
}

/**
* Implement event filter interface (part of qobject). Filter to prevent
* users escaping from the password dialog by using the "Escape" or other
* device keys when the password widget "m_passw" is active.
*/
bool QPasswordWidget::eventFilter( QObject *obj, QEvent *e )
{
    // only filter events for m_passw
    if ( obj != this )
        return QWidget::eventFilter( obj, e );
    // if event is anything other than keypress allow m_passw to handle it
    if ( e->type() == QEvent::KeyPress )
        return false;
    QKeyEvent *k = (QKeyEvent *)e;
    if ( k->modifiers() == Qt::NoModifier
        || k->key() == Qt::Key_Escape
        || k->key() == Qt::Key_Menu )
        return true;
    // if keypress is other, allow m_passw to handle it
    return false;
}


QPasswordDialog::QPasswordDialog( QWidget* parent, Qt::WFlags flags)
    : QDialog( parent, flags )
{
#ifdef QTOPIA_PHONE
    QSoftMenuBar::menuFor(this);
    QSoftMenuBar::setHelpEnabled(this, false);
#endif

    m_passw = new QPasswordWidget( this );
    QBoxLayout *l = new QVBoxLayout( this );
    l->addWidget( m_passw );

    // defaults
    m_passw->mode = QPasswordDialog::Crypted;
    setWindowTitle(tr("Authentication Required"));

    connect( m_passw, SIGNAL(passwordEntered(const QString&)),
             this, SLOT(accept()) );
}

QPasswordDialog::~QPasswordDialog()
{
}

void QPasswordDialog::setPrompt(const QString& prompt)
{
    m_passw->setPrompt( prompt );
}

QString QPasswordDialog::prompt() const
{
    return m_passw->prompt->text();
}

void QPasswordDialog::setInputMode( QPasswordDialog::InputMode mode )
{
    m_passw->mode = mode;
}

QPasswordDialog::InputMode QPasswordDialog::inputMode() const
{
    return m_passw->mode;
}

void QPasswordDialog::reset()
{
    m_passw->reset();
}

QString QPasswordDialog::password() const
{
    return m_passw->password();
}


/*!
  Returns a crypted password entered by the user when prompted with \a prompt.
  \a mode specifies whether the returned password is one-way encrypted or
  plain text. If the operation is required to accept more than one password, for example,
  if accepting an old password and a new password, set \a last to false to set the context label to Next.

  The returned value is QString() if the user cancels the operation,
  or the empty string if the user enters no password (but confirms the
  dialog).

  The \a parent argument specifies the QWidget parent.
*/

QString QPasswordDialog::getPassword( QWidget* parent,
                                      const QString& prompt,
                                      InputMode mode,
                                      bool last )
{
    bool max = true;

    if ( mode == Plain || mode == Pin )
        max = false;

    QPasswordDialog pd( parent );
    if ( !last ) {
        QSoftMenuBar::setLabel( &pd, Qt::Key_Back, QSoftMenuBar::Next,
                QSoftMenuBar::AnyFocus );
    }
    pd.setPrompt( prompt );
    pd.setInputMode( mode );

    int r;
    if ( max )
        pd.showMaximized();
    r = execDialog( &pd, !max );

    if ( r == QDialog::Accepted ) {
        return pd.password();
    } else {
        return QString();
    }
}


/*!
  Prompt, fullscreen, for the user's passcode until they get it right.

  If \a atPowerOn is true, the dialog is only used if the user's
  preference request it at poweron; either way, the screen is always repainted
  by this function.  (this functionality may move to the caller of this function).
*/

void QPasswordDialog::authenticateUser( QWidget* parent, bool atPowerOn )
{
    QSettings cfg( "Trolltech", "Security" );
    cfg.beginGroup( "Passcode" );
    QString passcode = cfg.value( "passcode" ).toString();
    if ( !passcode.isEmpty()
        && (!atPowerOn || cfg.value( "passcode_poweron", 0 ).toInt()) ) {
        // Do it as a fullscreen modal dialog
        QPasswordDialog pd( parent );
        pd.setInputMode( QPasswordDialog::Crypted );
        pd.setWindowFlags( Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );
#ifdef QTOPIA_PHONE
        if ( Qtopia::mousePreferred() )
#endif
        {
            QRect desk = qApp->desktop()->geometry();
            pd.setGeometry( 0, 0, desk.width(), desk.height() );
        }

        execDialog( &pd );
        while ( pd.password() != passcode ) {
            pd.reset();
            execDialog( &pd );
        }
    } else if ( atPowerOn ) {
    // refresh screen   #### should probably be in caller
    // Not needed (we took away the screen blacking)
    //if ( qwsServer )
        //qwsServer->refresh();
    }
}

bool QPasswordDialog::authenticateUser(const QString &text)
{
    QSettings cfg("Trolltech","Security");
    cfg.beginGroup("Passcode");
    QString passcode = cfg.value("passcode").toString();
    return (passcode.isEmpty() || MD5::hash(text) == passcode);
}

#include "qpassworddialog.moc"
