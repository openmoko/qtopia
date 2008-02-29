/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef ABEDITOR_H
#define ABEDITOR_H

#include <qtopia/pim/contact.h>

#include <qdialog.h>
#include <qlist.h>
#include <qmap.h>
#include <qstringlist.h>

class QScrollView;
class QMultiLineEdit;
class QLineEdit;
class QLabel;
class QComboBox;
class QHBox;
class QTabWidget;
class CategorySelect;
class QPEDateButton;

class FileAsCombo;
class AbEditor : public QDialog
{
    Q_OBJECT

    friend class FileAsCombo;
public:
    AbEditor( QWidget* parent = 0, const char* name = 0,  WFlags fl = 0 );
    ~AbEditor();

    void setNameFocus();
    PimContact entry() const { return ent; }

public slots:
    void setEntry( const PimContact &entry );

protected slots:
    void accept();
    void tabClicked( QWidget *tab );
    void editEmails();

private:
    void init();
    QWidget *addScrollBars( QWidget *tab );
    void addFields( QWidget *tabContainer, const QValueList<int> &keys );

    void contactFromFields(PimContact &);
    bool isEmpty();

private:
    PimContact ent;
    QMultiLineEdit *txtNote;
    QTabWidget *tabs;
    QWidget *personalTab;
    QWidget *summaryTab;
    QLabel *summary;
    QMap<int, QLineEdit *> lineEdits;
    QMap<int, QLabel *> labels;
    QComboBox *genderCombo;
    QComboBox *titleCombo;
    QComboBox *suffixCombo;
    FileAsCombo *fileAsCombo;
    CategorySelect *cmbCat;
    QPEDateButton *bdayButton;
    QPEDateButton *anniversaryButton;
    QHBox *ehb;
    QLineEdit *emailLE;
    QPushButton *emailBtn;
};

#endif
