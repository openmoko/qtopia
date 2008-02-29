/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef QTOPIASERVICESELECTOR_H
#define QTOPIASERVICESELECTOR_H

#include <qdialog.h>
#include <qtopiaservices.h>

class QtopiaServiceDescriptionPrivate;
class QTOPIA_EXPORT QtopiaServiceDescription
{
public:
    QtopiaServiceDescription();
    QtopiaServiceDescription(const QtopiaServiceRequest& r, const QString& l, const QString& ic, const QVariantMap& p = QVariantMap());
    ~QtopiaServiceDescription();

    QtopiaServiceDescription(const QtopiaServiceDescription& other);
    QtopiaServiceDescription& operator=(const QtopiaServiceDescription& other);

    QtopiaServiceRequest request() const;
    QString label() const;
    QString iconName() const;

    void setRequest(const QtopiaServiceRequest& r);
    void setLabel(const QString& l);
    void setIconName(const QString& i);

    QVariant optionalProperty(const QString& name) const;
    void setOptionalProperty(const QString& name, const QVariant &value);
    void removeOptionalProperty(const QString& name);

    QVariantMap optionalProperties() const;
    void setOptionalProperties(QVariantMap properties);

private:
    QtopiaServiceDescriptionPrivate* d;
};

class QListWidgetItem;
class QTranslatableSettings;
class QLabel;
class QListWidget;

class QTOPIA_EXPORT QtopiaServiceSelector : public QDialog
{
    Q_OBJECT
public:
    explicit QtopiaServiceSelector(QWidget* parent);

    void addApplications();

    QtopiaServiceDescription descriptionFor(const QtopiaServiceRequest& req) const;

protected:
    void closeEvent(QCloseEvent *e);
    void keyPressEvent(QKeyEvent* e);

public slots:
    bool edit(const QString& targetlabel, QtopiaServiceDescription& item);

private slots:
    void selectAction(int a);
    void selectAction(QListWidgetItem *i);

private:
    QtopiaServiceDescription descFor(QListWidgetItem* item) const;
    void populateActionsList();
    void populateActionsList(const QString& srv, QTranslatableSettings &cfg);

    QLabel *label;
    QListWidget *actionlist;
    int selection;
};

#endif
