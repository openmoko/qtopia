/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BATCHTRANSLATIONDIALOG_H
#define BATCHTRANSLATIONDIALOG_H

#include "ui_batchtranslation.h"
#include <QDialog>
#include <QtGui/QStringListModel>
#include <QtGui/QStandardItemModel>
#include "phrase.h"

class MessageModel;

class CheckableListModel : public QStandardItemModel
{
public:
    CheckableListModel(QObject *parent = 0);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
};

class BatchTranslationDialog : public QDialog
{
    Q_OBJECT
public:
    BatchTranslationDialog(MessageModel *model, QWidget *w = 0);
    void setPhraseBooks(const QList<PhraseBook> &phrasebooks);
    PhraseBook *GetNamedPhraseBook(const QString &name);

signals:
    void finished();

private slots:
    void startTranslation();
    void movePhraseBookUp();
    void movePhraseBookDown();

private:
    Ui::databaseTranslationDialog m_ui;
    CheckableListModel m_model;
    MessageModel *m_messagemodel;
    QList<PhraseBook> m_phrasebooks;
};


#endif  //BATCHTRANSLATIONDIALOG_H

