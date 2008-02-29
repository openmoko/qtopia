/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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

