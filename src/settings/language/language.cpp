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

#include "languagesettings.h"
#include "langmodel.h"

#include <qtopianamespace.h>

#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qtopiaipcenvelope.h>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif

#include <QLabel>
#include <QFile>
#include <QMessageBox>
#include <QListView>
#include <QAbstractListModel>
#include <QDir>
#include <QDebug>
#include <QMenu>
#include <QSettings>

using namespace Ui;

//copied from PkimMatcher in order to remove dependency on pkim
QStringList LanguageSettings::langs;

QStringList LanguageSettings::dictLanguages()
{
    if ( langs.isEmpty() ) {
        QString basename = Qtopia::qtopiaDir() + "/etc/dict/";
        QDir dir(basename);
        QStringList dftLangs = dir.entryList(QDir::Dirs);
        for (QStringList::ConstIterator it = dftLangs.begin(); it != dftLangs.end(); ++it){
            QString lang = *it;
            if (QFile::exists(basename+lang+"/words.dawg")) {
                langs.append(lang);
            }
        }
    }

    return langs;
}

#include "langname.h"

LanguageSettings::LanguageSettings( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ), confirmChange(true)
{
    setupUi(this);
    setModal( true );

    reset();

    QList<FontedItem> itemList;
    QStringList qpepaths = Qtopia::installPaths();
    foreach(QString tfn, qpepaths) {
        tfn +="i18n/";
        QDir langDir = tfn;
        QStringList list = langDir.entryList(QStringList("*") );
        QStringList::Iterator it;
        for( it = list.begin(); it != list.end(); ++it ) {
            QString id = (*it);
            if ( !langAvail.contains(id) ) {
                QFileInfo desktopFile( tfn + "/" + id + "/.directory" );
                if( desktopFile.exists() ) {
                    langAvail.append(id);
                    QFont font("dejavu");
                    font.setPointSize( 8 );
                    //we need to start with dejavu or all names would be written
                    //in unifont if this application is using unifont
                    bool rightToLeft = false;
                    QString langName = languageName(id, &font, &rightToLeft);
                    qLog(I18n) << "Found language:" << langName
                        << id << "Font:" << font.family() << "Font size:"<< font.pointSize()<< "RTL:" << rightToLeft;
                    FontedItem item (langName, font,
                        dictLanguages().contains(id) && inputLanguages.contains(id) );
                    item.direction = rightToLeft ? Qt::RightToLeft : Qt::LeftToRight;
                    itemList.append(item);
                }
            }
        }
    }

    model = new LanguageModel(this, itemList);

    listView = new QListView(this);
    listView->setModel(model);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);
    listView->setAlternatingRowColors( true );
    //LanguageDelegate * delegate = new LanguageDelegate( listView );
    //listView->setItemDelegate( delegate );

    //set current language
    int currentLanguage = langAvail.indexOf(chosenLanguage);
    if (currentLanguage >= 0)
        listView->setCurrentIndex(model->index(currentLanguage));

    vboxLayout->addWidget(listView);

    QItemSelectionModel *selectionModel = listView->selectionModel();
    connect(selectionModel,SIGNAL(currentChanged(const QModelIndex &, const QModelIndex&)),
            this, SLOT(applyLanguage(const QModelIndex&)));
#ifdef QTOPIA_PHONE
    connect(listView,SIGNAL(activated(const QModelIndex&)), this, SLOT(newLanguageSelected()));
    a_input = new QAction( tr("Use for input"), this );
    connect( a_input, SIGNAL( triggered(bool) ), this, SLOT( inputToggled() ) );
    a_input->setCheckable(true);
    updateActions(listView->currentIndex());

    QMenu *menu = QSoftMenuBar::menuFor(this);
    menu->addAction(a_input);
#else
    connect(listView, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(inputToggled(const QModelIndex&)));
#endif
}

LanguageSettings::~LanguageSettings()
{
}

void LanguageSettings::setConfirm(bool c)
{
    confirmChange = c;
}

void LanguageSettings::inputToggled(const QModelIndex &index)
{
    QString lang = langAvail.at( index.row() );
    inputLanguages.removeAll(lang);
    bool selected = model->data(index,Qt::UserRole).toBool();
    if (!selected && dictLanguages().contains(lang)) {
        inputLanguages.prepend(lang);
        model->setData(index, QVariant(true), Qt::UserRole);
    } else {
        model->setData(index, QVariant(false), Qt::UserRole);
    }

    updateActions(index);
}

void LanguageSettings::updateActions(const QModelIndex& index)
{
#ifdef QTOPIA_PHONE
    if (!index.isValid())
        return;
    QString lang = langAvail.at( listView->currentIndex().row() );
    bool dictLang = dictLanguages().contains(lang);

    a_input->setEnabled(dictLang);
    a_input->setVisible(dictLang);
    a_input->setChecked(inputLanguages.contains(lang));
#else
    Q_UNUSED(index);
#endif
}

#ifdef QTOPIA_PHONE
void LanguageSettings::inputToggled()
{
    inputToggled(listView->currentIndex());
}
#endif

void LanguageSettings::forceChosen()
{
    // For simplicity, make primary reading language also primary writing language.
    QString l = chosenLanguage;
    while (1) {
        if ( dictLanguages().contains(l) )
            break;
        int e = l.indexOf(QRegExp("[._]"));
        if ( e >= 0 ) {
            l = l.left(e);
        } else {
            // Give up.
            l = chosenLanguage;
            break;
        }
    }
    inputLanguages.removeAll(l);
    inputLanguages.prepend(l);
}

void LanguageSettings::newLanguageSelected()
{
    chosenLanguage = langAvail.at( listView->currentIndex().row() );
    QSettings config("Trolltech","locale");
    config.beginGroup( "Language" );
    QString lang = config.value( "Language" ).toString();

    if (lang == chosenLanguage) {
        accept();
        return;
    }


    if( lang != chosenLanguage && confirmChange
        && QMessageBox::warning( this, tr("Language Change"),
                                  tr("<qt>This will cause "
                                  "Qtopia to restart, closing all applications."
                                  "<p>Change Language?</qt>"),
                                  QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) {
        return;
   }

    // because reading language is primary writing language (see forceChosen())
    // we have to remove previous language from inputLanguages if it does not
    // have a dictionary.
    if (!dictLanguages().contains(lang))
        inputLanguages.removeAll(lang);

    config.setValue( "Language", chosenLanguage );
    forceChosen();
    config.setValue("InputLanguages", inputLanguages.join(QString(' ' )));
    config.sync();

    qLog(I18n) << "New language:" << chosenLanguage;
    qLog(I18n) << "Using following dictionaries:" << inputLanguages;

    //we have to set the new default font for the new language

    QSettings qpeconfig("Trolltech","qpe");
    QSettings fmcfg( QSettings::SystemScope, "Trolltech", "FontMap" );
    fmcfg.beginGroup("Map");
    QString bf=fmcfg.value("Font1[]").toString();

    QTranslator t(0);
    QString tfn = Qtopia::qtopiaDir() +"/i18n/";
    if (t.load(tfn+chosenLanguage+"/QtopiaDefaults.qm")) {
        //we assume the user wants normal font size
        QStringList fs = t.translate("FontMap",bf.toAscii().constData(),0).split(',');
        if ( fs.count() == 3 ) {
            qLog(I18n) << "New Font:" << fs[1] << "Font size:" << fs[2];
            qpeconfig.setValue("Appearance/FontFamily", fs[1]);
            qpeconfig.setValue("Appearance/FontSize", fs[2]);
        }
    } else{ //drop back to default font
        // simply read what is in etc/default/Fontmap.conf
        //if we cannot find FontMap.conf use hardcoded value
        QStringList fs = bf.split(',');
        if ( fs.count() == 3 ) {
            qLog(I18n) << "New Font:" << fs[1] << "Font size:" << fs[2];
            qpeconfig.setValue("Appearance/FontFamily", fs[1]);
            qpeconfig.setValue("Appearance/FontSize", fs[2]);
        } else {
            qLog(I18n) << "Could not determine font, dropping back to dejavu";
            qpeconfig.setValue("Appearance/FontFamily", "dejavau");
            qpeconfig.setValue("Appearance/FontSize", "7");
        }
    }
    //remove the current date format and let the new language determine
    //what the format should be
    qpeconfig.remove("Date/DateFormat");

    qpeconfig.sync();


    if( lang != chosenLanguage ) {
        QtopiaIpcEnvelope e("QPE/System","language(QString)");
        e << chosenLanguage;
    }

    accept();
}

void LanguageSettings::accept()
{
    QSettings config("Trolltech","locale");
    config.beginGroup( "Language" );
    config.setValue("InputLanguages", inputLanguages.join(QString(' ' )));
    config.sync();

    QDialog::accept();
}

void LanguageSettings::applyLanguage(const QModelIndex& idx)
{
    if (!idx.isValid())
        return;
    chosenLanguage = langAvail.at(idx.row());
    updateActions(idx);
}


void LanguageSettings::reject()
{
    reset();
    QDialog::reject();
}

void LanguageSettings::reset()
{
    QSettings config("Trolltech","locale");
    config.beginGroup( "Language" );
    QString l = getenv("LANG");
    l = config.value( "Language", l ).toString();
    if(l.isEmpty())
        l = "en_US"; // No tr
    else
    {
        int index = l.indexOf(QChar('.'));
        if (index > 0)
            l = l.left(index);
    }

    chosenLanguage = l;
    inputLanguages = config.value( "InputLanguages").toString().split( ' ' );
    forceChosen();
}


void LanguageSettings::done(int r)
{
    QDialog::done(r);
    close();
}
