/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
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

#include "languagesettings.h"

#ifdef QTOPIA_PHONE
#include "../../3rdparty/plugins/inputmethods/pkim/pkimmatcher.h"
#endif

#include <qtopia/global.h>
#include <qtopia/fontmanager.h>
#include <qtopia/config.h>
#include <qtopia/applnk.h>
#include <qtopia/qpedialog.h>
#include <qtopia/qpeapplication.h>
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif
#ifdef QTOPIA_PHONE
#include <qtopia/contextbar.h>
#include <qtopia/contextmenu.h>
#endif

#include <qlabel.h>
#include <qaction.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qslider.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qdir.h>
#if QT_VERSION >= 0x030000
#include <qstylefactory.h>
#endif
#include <qaccel.h>

//#if defined(QT_QWS_IPAQ) || defined(QT_QWS_SL5XXX)
//#include <unistd.h>
//#include <linux/fb.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <sys/ioctl.h>
//#endif
#include <stdlib.h>

class FontedItem : public QListBoxText {
public:
    FontedItem(const QString& langName,const QFont& font) :
	QListBoxText(langName),
	fnt(font)
    {
    }

    int height( const QListBox * ) const
    {
	return QFontMetrics(fnt).lineSpacing();
    }

    int width( const QListBox * ) const
    {
	return QFontMetrics(fnt).width(text());
    }


protected:
    void paint(QPainter* p)
    {
	p->setFont(fnt);
	QListBoxText::paint(p);
    }

private:
    QFont fnt;
};

#include "langname.h"

LanguageSettings::LanguageSettings( QWidget* parent,  const char* name, WFlags fl )
    : LanguageSettingsBase( parent, name, TRUE, fl ), confirmChange(TRUE)
{
#ifdef QWS
    if ( FontManager::hasUnicodeFont() )
       languages->setFont(FontManager::unicodeFont(FontManager::Proportional));
#endif

//#ifndef QTOPIA_DESKTOP
    //QPEApplication::setMenuLike( this, TRUE );
//#endif

    QStringList qpepaths = Global::qtopiaPaths();
    for (QStringList::Iterator qit=qpepaths.begin(); qit != qpepaths.end(); ++qit ) {
	QString tfn = *qit+"i18n/";
	QDir langDir = tfn;
	QStringList list = langDir.entryList("*", QDir::Dirs );
	QStringList::Iterator it;
	for( it = list.begin(); it != list.end(); ++it ) {
	    QString id = (*it);
	    if ( !langAvail.contains(id) ) {
		QFileInfo desktopFile( tfn + "/" + id + "/.directory" );
		if( desktopFile.exists() ) {
		    langAvail.append(id);
		    QFont font = languages->font();
		    QString langName = languageName(id, &font);
		    languages->insertItem( new FontedItem(langName,font) );
		}
	    }
	}
    }

    dl = new QPEDialogListener(this);

    reset();
    connect(languages,SIGNAL(highlighted(int)), this, SLOT(applyLanguage()));
#ifdef QTOPIA_PHONE
    connect(languages,SIGNAL(selected(int)), this, SLOT(accept()));
    a_input = new QAction( tr("Use for input"), QString::null, 0, this );
    connect( a_input, SIGNAL( activated() ), this, SLOT( inputToggled() ) );
    a_input->setToggleAction(TRUE);

    ContextMenu *menu = new ContextMenu(this);
    a_input->addTo(menu);

    updateActions();
#endif
}

LanguageSettings::~LanguageSettings()
{
}

void LanguageSettings::setConfirm(bool c)
{
    confirmChange = c;
}

void LanguageSettings::inputToggled()
{
#ifdef QTOPIA_PHONE
    QString lang = langAvail.at( languages->currentItem() );
    inputLanguages.remove(lang);
    if ( a_input->isOn() )
	inputLanguages.prepend(lang);
#endif
}

#ifdef QTOPIA_PHONE
void LanguageSettings::updateActions()
{
    QString lang = langAvail.at( languages->currentItem() );
    a_input->setOn(inputLanguages.contains(lang));
    a_input->setEnabled(PkIMMatcher::languages().contains(lang));
}

void LanguageSettings::forceChosen()
{
    // For simplicity, make primary reading language also primary writing language.
    QString l = chosenLanguage;
    while (1) {
	if ( PkIMMatcher::languages().contains(l) )
	    break;
	int e = l.findRev(QRegExp("[._]"));
	if ( e >= 0 ) {
	    l = l.left(e);
	} else {
	    // Give up.
	    l = chosenLanguage;
	    break;
	}
    }
    inputLanguages.remove(l);
    inputLanguages.prepend(l);
}
#endif

void LanguageSettings::accept()
{
    chosenLanguage = langAvail.at( languages->currentItem() );
    qDebug("accept");
    Config config("locale");
    config.setGroup( "Language" );
    QString lang = config.readEntry( "Language" );

    if( lang != chosenLanguage && confirmChange
	&& QMessageBox::warning( this, tr("Language Change"),
                                  tr("<qt>This will cause "
                                  "Qtopia to restart, closing all applications."
                                  "<p>Change Language?</qt>"),
                                  QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) {
            
            // user should be able to leave the app by pressing BACK
            // user doesn't want to change lang anymore => reset chosenLanguage
            chosenLanguage = lang;
	    return; // Cancel accept.
    }

    config.writeEntry( "Language", chosenLanguage );
#ifdef QTOPIA_PHONE
    forceChosen();
#else
    inputLanguages = chosenLanguage;
#endif
    config.writeEntry( "InputLanguages", inputLanguages, ' ' );
    qDebug("write");
    config.write();

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    if( lang != chosenLanguage ) {
	QCopEnvelope e("QPE/System","language(QString)");
	e << chosenLanguage;
    }
#endif

    QDialog::accept();
}

void LanguageSettings::applyLanguage()
{
    chosenLanguage = langAvail.at( languages->currentItem() );
#ifdef QTOPIA_PHONE
    updateActions();
#endif
}


void LanguageSettings::reject()
{
    reset();
    QDialog::reject();
}

void LanguageSettings::reset()
{
    Config config("locale");
    config.setGroup( "Language" );
    QString l = getenv("LANG");
    l = config.readEntry( "Language", l );
    if(l.isEmpty()) l = "en_US"; // No tr
    chosenLanguage = l;
    inputLanguages = config.readListEntry( "InputLanguages", ' ' );
#ifdef QTOPIA_PHONE
    forceChosen();
#endif

    int n = langAvail.find( l );
    languages->setCurrentItem( n );
}


void LanguageSettings::done(int r)
{
    QDialog::done(r);
    close();
}
