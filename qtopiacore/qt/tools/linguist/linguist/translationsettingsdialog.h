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

#ifndef TRANSLATIONSETTINGSDIALOG_H
#define TRANSLATIONSETTINGSDIALOG_H

#include "ui_translationsettings.h"
#include <QDialog>
#include <QtCore/QLocale>
class MessageModel;
class TranslationSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    TranslationSettingsDialog(QWidget *w = 0);
    void setMessageModel(MessageModel *model);

private:
    virtual void showEvent(QShowEvent *e);
private slots:
    void on_buttonBox_accepted();

private:
    Ui::TranslationSettings m_ui;
    MessageModel *m_messageModel;

};

#endif /* TRANSLATIONSETTINGSDIALOG_H */
