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


#include "translationsettingsdialog.h"
#include "messagemodel.h"
#include <QtCore/QLocale>

TranslationSettingsDialog::TranslationSettingsDialog(QWidget *w /*= 0*/) : QDialog(w)
{
    m_ui.setupUi(this);    

    for (int i = QLocale::C + 1; i < QLocale::LastLanguage; ++i) {
        QString lang = QLocale::languageToString(QLocale::Language(i));
        m_ui.cbLanguageList->addItem(lang, QVariant(int(i)));
    }
    m_ui.cbLanguageList->model()->sort(0, Qt::AscendingOrder);

    for (int i = QLocale::AnyCountry; i < QLocale::LastCountry; ++i) {
        QString country = QLocale::countryToString(QLocale::Country(i));
        m_ui.cbCountryList->addItem(country, QVariant(int(i)));
    }
    m_ui.cbCountryList->model()->sort(0, Qt::AscendingOrder);
    m_ui.cbCountryList->insertItem(0, tr("Any Country"), QVariant(0));

}

void TranslationSettingsDialog::setMessageModel(MessageModel *model)
{
    m_messageModel = model;
}

void TranslationSettingsDialog::on_buttonBox_accepted()
{
    int itemindex = m_ui.cbLanguageList->currentIndex();
    QVariant var = m_ui.cbLanguageList->itemData(itemindex);
    QLocale::Language lang = QLocale::Language(var.toInt());
    m_messageModel->setLanguage(lang);

    itemindex = m_ui.cbCountryList->currentIndex();
    var = m_ui.cbCountryList->itemData(itemindex);
    QLocale::Country country = QLocale::Country(var.toInt());
    m_messageModel->setCountry(country);
    accept();
}

void TranslationSettingsDialog::showEvent(QShowEvent *e)
{
    Q_UNUSED(e);
    QLocale::Language lang = m_messageModel->language();
    int itemindex = m_ui.cbLanguageList->findData(QVariant(int(lang)));
    m_ui.cbLanguageList->setCurrentIndex(itemindex == -1 ? 0 : itemindex);


    QLocale::Country country = m_messageModel->country();
    itemindex = m_ui.cbCountryList->findData(QVariant(int(country)));
    m_ui.cbCountryList->setCurrentIndex(itemindex == -1 ? 0 : itemindex);
}
