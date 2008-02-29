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
#ifndef TEMPLATETEXT_H
#define TEMPLATETEXT_H

#include <qdialog.h>
#include <qstring.h>

class QAction;
class QListWidget;
class QListWidgetItem;
class QLineEdit;

class TemplateTextDialog : public QDialog
{
    Q_OBJECT
public:
    TemplateTextDialog(QWidget *parent = 0);
    virtual ~TemplateTextDialog();
    QString text();
    void loadTexts();
    void saveTexts();

private slots:
    void selected();
    void slotRemove();
    void slotReset();
    void slotUpdateActions();
 private:
    QListWidgetItem *mNewTemplateText;
    QListWidget *mTemplateList;
    QAction *removeAction;
    QAction *resetAction;

    int userTemplates;
};

class NewTemplateTextDialog : public QDialog
{
    Q_OBJECT
public:
    NewTemplateTextDialog(QWidget *parent = 0);
    QString text();

private:
    QLineEdit *mEdit;
};

#endif
