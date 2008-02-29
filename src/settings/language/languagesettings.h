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
#ifndef LANGUAGESETTINGS_H
#define LANGUAGESETTINGS_H


#include <QStringList>
#include <QAction>
#include "ui_languagesettingsbase.h"

class QListWidgetItem;
class LanguageModel;
class QModelIndex;
class QListView;

class LanguageSettings : public QDialog, public Ui::LanguageSettingsBase
{
    Q_OBJECT
public:
    LanguageSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    virtual ~LanguageSettings();

    void setConfirm(bool cfm=true);

protected:
    void accept();
    void reject();
    void done(int);

    QStringList langAvail;

private slots:
    void applyLanguage(const QModelIndex&);
    void reset();
    void inputToggled(const QModelIndex&);
#ifdef QTOPIA_PHONE
    void inputToggled();
#endif
    void newLanguageSelected();

private:
    void updateActions(const QModelIndex& idx);
    void forceChosen();

    QString chosenLanguage;
    QStringList inputLanguages;
    bool confirmChange;

    static QStringList langs;
    static QStringList dictLanguages();
    LanguageModel *model;
    QListView *listView;
#ifdef QTOPIA_PHONE
    QAction *a_input;
#endif
};


#endif // LANGUAGESETTINGS_H

