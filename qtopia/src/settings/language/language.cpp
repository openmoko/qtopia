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

#include "languagesettings.h"
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
#include <qtopia/resource.h>

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

class FontedItem : public QListBoxPixmap {
public:
    FontedItem(const QPixmap& pxm, const QString& langName,const QFont& font) :
	QListBoxPixmap(pxm, langName),
	fnt(font), showPixmap(FALSE)
    {
    }

    int height( const QListBox * ) const
    {
	return QMAX(QFontMetrics(fnt).lineSpacing(), pixmap()->height());
    }

    int width( const QListBox * ) const
    {
	return QMAX(QFontMetrics(fnt).width(text()), pixmap()->width());
    }

    void setSelected(bool active)
    {
        showPixmap = active;
    }

    bool selected() { return showPixmap; }
    
protected:
    void paint(QPainter* painter)
    {
	painter->setFont(fnt);
        if (showPixmap)
            painter->drawPixmap( 3, 0, *pixmap() );
        if ( !text().isEmpty() ) {
	    QFontMetrics fm = painter->fontMetrics();
            if (showPixmap) {
                int yPos;			// vertical text position
                if ( pixmap()->height() < fm.height() )
                    yPos = fm.ascent() + fm.leading()/2;
                else
                    yPos = pixmap()->height()/2 - fm.height()/2 + fm.ascent();
                painter->drawText( pixmap()->width() + 5, yPos, text() );
            } else {
                painter->drawText( 3, fm.ascent() + fm.leading()/2, text() );
            }
        }
    }

private:
    QFont fnt;
    bool showPixmap;
};


//remove dependency on pkim
QStringList LanguageSettings::langs;

QStringList LanguageSettings::dictLanguages()
{
    if ( langs.isEmpty() ) {
        QString basename = QPEApplication::qpeDir() + "/etc/dict/";
        QDir dir(basename);
        QStringList dftLangs = dir.entryList(QDir::Dirs);
        for (QStringList::ConstIterator it = dftLangs.begin(); it != dftLangs.end(); ++it){
            QString lang = *it;
            if (QFile::exists(basename+lang+"/words.dawg")) 
                langs.append(lang);
        }
    }

    return langs;
}

#include "langname.h"

LanguageSettings::LanguageSettings( QWidget* parent,  const char* name, WFlags fl )
    : LanguageSettingsBase( parent, name, TRUE, fl ), confirmChange(TRUE)
{
#ifdef QWS
    if ( FontManager::hasUnicodeFont() )
       languages->setFont(FontManager::unicodeFont(FontManager::Proportional));
#endif

    QPixmap pix = Resource::loadPixmap("selectedDict");
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
		    languages->insertItem( new FontedItem(pix, langName,font) );
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
#else
    connect(languages,SIGNAL(selected(int)), this, SLOT(inputToggled()));
#endif
    updateActions();
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
    int index = languages->currentItem();
    QString lang = langAvail.at( index );
    inputLanguages.remove(lang);
    FontedItem* item = (FontedItem*) languages->item(index);
    if (!item->selected() && dictLanguages().contains(lang)) {
	inputLanguages.prepend(lang);
        item->setSelected(TRUE);
    } else {
        item->setSelected(FALSE);
    }
    updateActions(FALSE);
}

void LanguageSettings::updateActions(bool allLanguages)
{
    QString lang;
    if (allLanguages) {
        int numEntries = languages->numRows();
        for(int index = 0; index < numEntries; index++) {
            FontedItem* item = (FontedItem* ) languages->item(index);
            lang = langAvail.at(index);
            item->setSelected(dictLanguages().contains(lang) && 
                    inputLanguages.contains(lang));
        }
    }
    languages->triggerUpdate(FALSE);
#ifdef QTOPIA_PHONE 
    lang = langAvail.at( languages->currentItem() );
    if (dictLanguages().contains(lang)) 
        a_input->setEnabled(TRUE);
    else
        a_input->setEnabled(FALSE);
    a_input->setOn(inputLanguages.contains(lang));
#endif
}

void LanguageSettings::forceChosen()
{
    // For simplicity, make primary reading language also primary writing language.
    QString l = chosenLanguage;
    while (1) {
	if ( dictLanguages().contains(l) )
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

void LanguageSettings::accept()
{
    chosenLanguage = langAvail.at( languages->currentItem() );
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

    // because reading lang is primary writing lang (see forceChosen())
    // we have to remove previous lang from inputLanguages if it does not
    // have a dictionary.
    if (!dictLanguages().contains(lang))
        inputLanguages.remove(lang);
    
    config.writeEntry( "Language", chosenLanguage );
    forceChosen();
    config.writeEntry( "InputLanguages", inputLanguages, ' ' );
    config.write();
   
    /*we have to set the new default font for the new language */
    Config qpeconfig("qpe");
        
    QTranslator t(0);
    QString tfn = QPEApplication::qpeDir()+"/i18n/";
    if (t.load(tfn+chosenLanguage+"/QtopiaDefaults.qm")) {
        Config fmcfg(QPEApplication::qpeDir()+"/etc/default/FontMap.conf", Config::File);
        fmcfg.setGroup("Map");
        QString bf;
        //we assume the user wants normal font size
        bf=fmcfg.readEntry("Font1[]"); 
        QStringList fs = QStringList::split(',',t.findMessage("FontMap",bf,0).translation());
        if ( fs.count() == 3 ) {
            qDebug("%s %s", fs[1].latin1(), fs[2].latin1());
            qpeconfig.writeEntry("FontFamily", fs[1]);
            qpeconfig.writeEntry("FontSize", fs[2]);
        }
    } else{ //drop back to default font
        qpeconfig.writeEntry("FontFamily", "helvetica");
        qpeconfig.writeEntry("FontSize", "12");
    }
        
    qpeconfig.write();

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
    updateActions(FALSE);
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
    forceChosen();

    int n = langAvail.find( l );
    languages->setCurrentItem( n );
}


void LanguageSettings::done(int r)
{
    QDialog::done(r);
    close();
}
