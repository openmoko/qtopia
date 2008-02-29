/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "password.h"
#include "config.h"
#include "global.h"
#include "backend/contact.h"
#include "qpeapplication.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <qtextview.h>
#include <qstring.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qfile.h>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif

#include <qdialog.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextbar.h>
#include <qtopia/contextmenu.h>
#include <qlayout.h>
#endif

#ifndef Q_OS_WIN32
#include <unistd.h> //for sleep
#else
#include <windows.h>
#include <winbase.h>
#endif
#include "passwordbase_p.h"

#include "../qtopia1/qpe_show_dialog.cpp"

#ifdef QTOPIA_PHONE
extern bool mousePreferred; // can't call Global::mousePreferred in libqtopia2 from libqtopia
#endif

class PasswordDialog : public PasswordBase
{
    Q_OBJECT

public:
    PasswordDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~PasswordDialog();

    void clear();
    void setPrompt( const QString& );

signals:
    void passwordEntered( const QString& );

public slots:
    void key();

protected:
    void keyPressEvent( QKeyEvent * );

private:
    void input( QString );
    friend class Password;
    QString text;
};

//////
// START COPIED CODE to password2.cpp
//////
#ifndef Q_OS_WIN32
extern "C" char *crypt(const char *key, const char *salt);
#else
static char *crypt(const char *key, const char *salt) {
    //#### revise  
    Q_UNUSED(salt)
    return (char*)key;
}
#endif

static QString qcrypt(const QString& k, const char *salt)
{
    const QCString c_str = k.utf8();
    int		len = k.length();
    QString	result;
    const char	*ptr = c_str;

    for (; len > 8; len -= 8) {
	result += QString::fromUtf8(crypt(ptr, salt));
	ptr += 8;
    }

    if (len) {
	result += QString::fromUtf8(crypt(ptr, salt));
    }

    return result;
}
//////
// END COPIED CODE to password2.cpp
//////

/*
 *  Constructs a PasswordDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
PasswordDialog::PasswordDialog( QWidget* parent,  const char* name, WFlags fl )
    : PasswordBase( parent, name, fl )
{
    QRect desk = qApp->desktop()->geometry();
    int dlgfontsize, promptfontsize;
#ifdef QTOPIA_PHONE
    if  (mousePreferred) {
	dlgfontsize = 12;
	promptfontsize = 10;
    } else 
#endif
    {
	dlgfontsize = 18;
	promptfontsize = 12;
    }

    if ( desk.width() < 220  || desk.height() < 320) {
	QFont f( font() );
	f.setPointSize( dlgfontsize );
	setFont( f );
	f.setPointSize( promptfontsize );
	prompt->setFont( f );
    }

#ifdef QTOPIA_PHONE
    if  (!mousePreferred) {
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

#ifdef QTOPIA_PHONE
    ContextBar::setLabel(this, Key_Select, ContextBar::NoLabel);
#endif    

    QPalette pal = display->palette();
    QBrush base = pal.brush(QPalette::Normal, QColorGroup::Base);
    QColor text = pal.color(QPalette::Normal, QColorGroup::Text);
    pal.setBrush(QPalette::Disabled, QColorGroup::Background, base);
    pal.setColor(QPalette::Disabled, QColorGroup::Text, text);
    display->setPalette(pal);

    setFocusPolicy(StrongFocus);
    setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
PasswordDialog::~PasswordDialog()
{
    // no need to delete child widgets, Qt does it all for us
}



/*!
  \reimp
*/

void PasswordDialog::key()
{
    QPushButton* s = (QPushButton*)sender();
    if ( s == button_OK ) {
	emit passwordEntered( text );
    } else {
	input(s->text());
    }
}


/*!
  \reimp
*/

void PasswordDialog::keyPressEvent( QKeyEvent *e )
{
#ifdef QTOPIA_PHONE
    if ( (Global::mousePreferred() || (!Global::mousePreferred() && isModalEditing())) && (e->key() == Key_Back || e->key() == Key_No) ) {
	if( !mousePreferred )
	    setModalEditing(FALSE);
	e->ignore();
	return;
    }
#endif
    if ( e->key() == Key_Enter || e->key() == Key_Return )
	emit passwordEntered( text );
    else {
	QString t = e->text().left(1);
	if ( t[0]>='0' && t[0]<='9' )
	    input(t);
    }
}


/*!

*/

void PasswordDialog::input( QString c )
{
    if (text.length() < 8) {
	text += c;
	display->setText( text );
    }
}

/*!

*/

void PasswordDialog::setPrompt( const QString& s )
{
    prompt->setText( "<qt>"+s+"</qt>" ); //no tr
}

void PasswordDialog::clear()
{
    text = "";
    input("");
}

class PasswdDlg : public QDialog
{
    Q_OBJECT
public:
    PasswdDlg( QWidget *parent, const char * name, bool modal, bool fullscreen = FALSE )
	: QDialog( parent, name, modal, fullscreen ? WStyle_NoBorder | WStyle_Customize | WStyle_StaysOnTop : 0 ),
	    modl(modal)
    {
	setCaption(tr("Password"));

#ifdef QTOPIA_PHONE
	ContextMenu *cm = new ContextMenu(this);
	cm->setEnableHelp(FALSE);
#endif

	passw = new PasswordDialog( this );
#ifdef QTOPIA_PHONE
	if ( fullscreen || mousePreferred )
#else
	if ( fullscreen )
#endif
	{
	    QRect desk = qApp->desktop()->geometry();
	    setGeometry( 0, 0, desk.width(), desk.height() );
	}

	connect( passw, SIGNAL(passwordEntered(const QString&)),
		 this, SLOT(accept()) );
    }

    void resizeEvent( QResizeEvent * )
    {
	passw->resize( size() );
    }

    void reset()
    {
	passw->clear();
    }

    void execNonModal()
    {
	if ( !modl ) {
	    reset();
	    showFullScreen();
	    do {
		qApp->enter_loop();
	    } while (result()!=1);
	}
    }

    void accept()
    {
	if ( !modl )
	    qApp->exit_loop();
	QDialog::accept();
    }

    PasswordDialog *passw;
    bool modl;
};

class OwnerDlg : public QDialog
{
    Q_OBJECT
public:

    OwnerDlg( QWidget *parent, const char * name, Contact c,
	    bool modal, bool fullscreen = FALSE )
	: QDialog( parent, name, modal, 
		fullscreen ? 
		WStyle_NoBorder | WStyle_Customize | WStyle_StaysOnTop : 0 )
    {
	if ( fullscreen ) {
	    QRect desk = qApp->desktop()->geometry();
	    setGeometry( 0, 0, desk.width(), desk.height() );
	}

	// set up contents.
	QString text = "<H1>" + tr("Owner Information") + "</H1>";
	text += c.toRichText();
	tv = new QTextView(this);
	tv->setText(text);

	tv->viewport()->installEventFilter(this);
    }

    void resizeEvent( QResizeEvent * )
    {
	tv->resize( size() );
    }
   
    bool eventFilter(QObject *o, QEvent *e)
    {
	if (e->type() == QEvent::KeyPress || e->type() == QEvent::MouseButtonPress ) {
	    accept();
	    return TRUE;
	}
	return QWidget::eventFilter(o, e);
    }

    void mousePressEvent( QMouseEvent * ) { accept(); }

private:
    QTextView *tv;
};

/*!
  Returns a crypted password entered by the user when prompted with \a prompt
  The returned value is QString::null if the user cancels the operation,
  or the empty string if the user enters no password (but confirms the
  dialog).

  Qtopia 2: If the prompt is prefixed with "@:", the dialog is not maximized,
  and the returned value is not crypted.
*/

QString Password::getPassword( const QString& prompt )
{
    bool max = TRUE;
    bool cry = TRUE;

    QString p = prompt;
    if ( prompt.left(2) == "@:" ) {
	max = FALSE;
	cry = FALSE;
	p = prompt.mid(2);
    }
    PasswdDlg pd(0,0,TRUE); //, TRUE);
    pd.passw->setPrompt( p );

    int r;
    if ( max ) {
	pd.showMaximized();
	r = pd.exec();
    } else {
	qpe_show_dialog(&pd,FALSE);
	r = pd.exec();
    }

    if ( r == QDialog::Accepted ) {
	if (pd.passw->text.isEmpty())
	    return "";
	else
	    return cry ? qcrypt(pd.passw->text,"a0") : pd.passw->text;
    } else {
	return QString::null;
    }
}


/*!
  Prompt, fullscreen, for the user's passcode until they get it right.

  If \a at_poweron is TRUE, the dialog is only used if the user's
  preference request it at poweron; either way, the screen is always repainted
  by this function.  (this functionality may move to the caller of this function).
*/

void Password::authenticate(bool at_poweron)
{
    Config cfg("Security");
    cfg.setGroup("Passcode");
    QString passcode = cfg.readEntry("passcode");
    if ( !passcode.isEmpty()
	    && (!at_poweron || cfg.readNumEntry("passcode_poweron",0)) )
    {
	// Do it as a fullscreen modal dialog
	PasswdDlg pd(0,0,TRUE,TRUE);

	// see if there is contact information.
	OwnerDlg *oi = 0;
	QString vfilename = Global::applicationFileName("addressbook", 
		"businesscard.vcf");
	if (QFile::exists(vfilename)) {
	    Contact c;
	    c = Contact::readVCard( vfilename )[0];

	    oi = new OwnerDlg(0, 0, c, TRUE, TRUE);
	}

	pd.reset();
	pd.exec();
	while (qcrypt(pd.passw->text, "a0") != passcode) {
	    if (oi)
		oi->exec();
	    pd.reset();
	    pd.exec();
	}
    } else if ( at_poweron ) {
	// refresh screen   #### should probably be in caller
	// Not needed (we took away the screen blacking)
	//if ( qwsServer )
	    //qwsServer->refresh();
    }
}

#include "password.moc"
