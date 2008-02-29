/****************************************************************************
**
** Copyright (C) 2006-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TRPREVIEWTOOL_H
#define TRPREVIEWTOOL_H

#include <QMainWindow>
#include <QWorkspace>
#include <QHash>
#include <QComboBox>
#include <QUiLoader>
#include <QTranslator>
#include <QLayout>

#include "ui_trpreviewtool.h"

class FormHolder : public QWidget
{
    Q_OBJECT

public:
    FormHolder(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    bool loadFormFile(const QString& path);
    void retranslate();
    QString formFilePath();
    virtual QSize sizeHint() const;

private:
    static QUiLoader* uiLoader;
    QString formPath;
    QWidget* form;
    QHBoxLayout* layout;
    QSize m_sizeHint;
};

class QStandardItemModel;

class TrPreviewTool : public QMainWindow
{
    Q_OBJECT

public:
    TrPreviewTool(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~TrPreviewTool();
    bool addFormFile(const QString &path);
    bool loadTranslation(const QString &path, const QString &displayName = QString());
    bool addTranslator(QTranslator *translator, const QString &displayName);
    void cascade();

public slots:
    void openForm();
    void loadTranslation();
    void translationSelected(int idx);
    void showAboutBox();
    void on_viewForms_doubleClicked(const QModelIndex &);
    void reloadTranslations();

private:
    virtual bool event(QEvent *e);  // override from QWidget
    bool addTranslator(QTranslator *translator, const QString &path, const QString &displayName);
    FormHolder* createFormFromFile(const QString& path);
    void recreateForms();
    void showWarning(const QString& warning);
    Ui::TrPreviewToolClass ui;
    QWorkspace* workspace;
    QComboBox* trCombo;
    QTranslator* currentTr;
    QHash<QString,QTranslator*> trDict;
    QStandardItemModel *m_uiFilesModel;
};


#endif // TRPREVIEWTOOL_H
