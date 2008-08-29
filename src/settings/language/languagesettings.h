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
#ifndef LANGUAGESETTINGS_H
#define LANGUAGESETTINGS_H


#include <QStringList>
#include "ui_languagesettingsbase.h"

class QListWidgetItem;
class LanguageModel;
class QModelIndex;
class QListView;
class QAction;

class LanguageSettings : public QDialog, public Ui::LanguageSettingsBase
{
    Q_OBJECT
public:
    LanguageSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    virtual ~LanguageSettings();

    void setConfirm(bool cfm=true);
    static QStringList dictLanguages();

protected:
    void accept();
    void reject();
    void done(int);

    QStringList langAvail;

private slots:
    void applyLanguage(const QModelIndex&);
    void reset();
    void inputToggled(const QModelIndex&);
    void inputToggled();
    void newLanguageSelected();

private:
    void updateActions(const QModelIndex& idx);
    void forceChosen();

    QString chosenLanguage;
    QStringList inputLanguages;
    bool confirmChange;

    static QStringList langs;
    LanguageModel *model;
    QListView *listView;
    QAction *a_input;
};

#endif // LANGUAGESETTINGS_H
