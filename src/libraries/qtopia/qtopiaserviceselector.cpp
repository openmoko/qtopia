/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "qtopiaserviceselector.h"

#include <QBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QCloseEvent>
#include <QListWidget>
#include <qtopianamespace.h>
#include <qsoftmenubar.h>
#include <qtranslatablesettings.h>
#include <qexpressionevaluator.h>
#include <qcontentset.h>

/*!
  \class QtopiaServiceDescription
  \mainclass
  \brief The QtopiaServiceDescription class describes a service request in user terms.

  This data includes what action to undertake when activated, and a display name
  and icon.

  \ingroup ipc
  \sa QtopiaServiceSelector, QtopiaServiceRequest
*/

/*! \fn QtopiaServiceDescription::QtopiaServiceDescription()
  Constructs an empty QtopiaServiceDescription .
*/

/*! \fn QtopiaServiceDescription::QtopiaServiceDescription( QtopiaServiceRequest r, QString l, QString ic )
  Constructs a QtopiaServiceDescription, which describes an action called \a l, with the
  display icon found in the file specified by \a ic, which initiates
  QtopiaServiceRequest \a r when activated.
*/

/*! \fn QtopiaServiceRequest QtopiaServiceDescription::request() const
  Returns the QtopiaServiceRequest described.

  \sa setRequest()
*/

/*! \fn QString QtopiaServiceDescription::label() const
  Returns the display label describing the request.

  \sa setLabel()
*/

/*! \fn QString QtopiaServiceDescription::iconName() const
  Returns the icon name describing the request.

  \sa setIconName()
*/

/*! \fn void QtopiaServiceDescription::setRequest(const QtopiaServiceRequest& r)
  Sets the request to \a r.

  \sa request()
*/

/*! \fn void QtopiaServiceDescription::setLabel(const QString& l)
  Sets the display label to \a l.

  \sa label()
*/

/*! \fn void QtopiaServiceDescription::setIconName(const QString& i)
  Sets the display icon to \a i.

  \sa iconName()
*/

/*!
  \class QtopiaServiceSelector
  \mainclass
  \brief The QtopiaServiceSelector class implements a list dialog for selecting a service.

  \ingroup ipc
  \sa QtopiaServiceDescription, QtopiaServiceRequest
*/

class QListView;

#define SRV_ROLE        Qt::UserRole
#define ICON_ROLE       Qt::UserRole + 1
#define ACTION_ROLE     Qt::UserRole + 2

/*!
    Construct a Qtopia service selector dialog owned by \a parent.
*/
QtopiaServiceSelector::QtopiaServiceSelector(QWidget* parent) : QDialog(parent)
{
    setModal(true);

    QVBoxLayout *vbl = new QVBoxLayout(this);
    vbl->setSpacing(0);
    vbl->setMargin(0);

    QHBoxLayout *hbl = new QHBoxLayout();
    hbl->setSpacing(0);
    hbl->setMargin(0);
    vbl->addLayout(hbl);

    label = new QLabel();
    hbl->addWidget(label);

    actionlist = new QListWidget();
    vbl->addWidget(actionlist);

    QListWidgetItem* item = new QListWidgetItem(tr("No action"), actionlist);
    item->setIcon(QIcon(":icon/reset"));

    populateActionsList();

    connect(actionlist, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(selectAction(QListWidgetItem*)));

#ifdef QTOPIA_PHONE
    //QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel);
#endif
}

/*!
    Add all installed applications to the list of services displayed in this selector.
    By default, only service actions are shown.
*/
void QtopiaServiceSelector::addApplications()
{
    // Copy all of the AppLnks to our own list
    QContentSet lnkSet(QContentFilter::Category, QLatin1String("Applications"));
    QContentList apps = lnkSet.items();
    foreach(const QContent &it, apps )
    {
        if ( it.type() != "Separator" && it.property("Builtin") != "1") // No tr
        {
            QListWidgetItem *item = new QListWidgetItem(it.name(), actionlist);
            item->setData(SRV_ROLE, it.executableName());
            item->setData(ICON_ROLE, it.iconName());
            item->setData(ACTION_ROLE, "raise()");
            item->setIcon(QIcon(":icon/"+it.iconName()));
        }
    }
}

void QtopiaServiceSelector::populateActionsList()
{
    QStringList srvList = QtopiaService::list();
    QStringList::ConstIterator itService;

    for(itService = srvList.begin(); itService != srvList.end(); ++itService)
    {
        if ( !QtopiaService::app(*itService).isEmpty() ) { // only show available services (eg. not 'Beam Business Card' if no Contacts)
            QTranslatableSettings srv(QtopiaService::config(*itService), QSettings::IniFormat);
            if( srv.status()==QSettings::NoError )
            {
                srv.beginGroup("Service");
                populateActionsList(*itService, srv);
            }
        }

        QTranslatableSettings srvapp(QtopiaService::appConfig(*itService, QString()), QSettings::IniFormat);
        if( srvapp.status()==QSettings::NoError )
        {
            srvapp.beginGroup("Extensions");
            populateActionsList(*itService, srvapp);
        }
    }
}

void QtopiaServiceSelector::populateActionsList(const QString& srv, QTranslatableSettings& cfg)
{
    QStringList actions = cfg.value("Actions").toString().split( ';');
    QString name;
    QString icon;
    QListWidgetItem* item;

    cfg.endGroup();

    for(QStringList::ConstIterator ait = actions.begin(); ait != actions.end(); ++ait)
    {
        if((*ait).right(2) == "()")
        {
            cfg.beginGroup(*ait);
            name = cfg.value("Name").toString();
            if(!name.isEmpty())
            {
                QByteArray r = cfg.value(QLatin1String("Requires")).toByteArray();
                QExpressionEvaluator expr(r);
                if ( r.isEmpty() || expr.isValid() && expr.evaluate() && expr.result().toBool() ) {
                    icon = cfg.value("Icon").toString();

                    item = new QListWidgetItem(name, actionlist);
                    item->setData(SRV_ROLE, QVariant(srv));
                    item->setData(ICON_ROLE, QVariant(icon));
                    item->setData(ACTION_ROLE, QVariant(*ait));
                    item->setIcon(QIcon(":icon/"+icon));
                }
            }
            cfg.endGroup();
        }
    }
}

/*!
    \reimp
*/
void QtopiaServiceSelector::closeEvent(QCloseEvent *e)
{
    e->accept();
}

/*!
    Displays this Qtopia service selector dialog, to allow the user to select
    a service to associate with \a targetlabel.  The selected service description
    is returned in \a item.

    This function is typically used to edit the service associated with a device button
    or similar action.  The contents of the service selector are not being edited; rather
    the button or action is being edited.

    Returns true if an item was selected; or false if the dialog was canceled.
*/
bool QtopiaServiceSelector::edit(const QString& targetlabel, QtopiaServiceDescription& item)
{
    bool rowset = false;

    //
    //  Find the currently selected entry, and put the cursor there.
    //

    label->setText(tr("<p>Action for <b>%1</b>...").arg(targetlabel));

    int count = actionlist->count();
    for(int i = 0; i < count; i++)
    {
        QListWidgetItem* actionitem = actionlist->item(i);

        if(item.label() == actionitem->text() &&
            item.request().service() == actionitem->data(SRV_ROLE) &&
            item.iconName() == actionitem->data(ICON_ROLE) &&
            item.request().message() == actionitem->data(ACTION_ROLE))
        {
            actionlist->setCurrentRow(i);
            selection = i;
            rowset = true;
            break;
        }
    }

    if(!rowset)
    {
        actionlist->setCurrentRow(0);
        selection = -1;
    }

    showMaximized();
    if ( exec() != QDialog::Accepted )
        return false;

    bool chose = false;
    if( selection > -1 )
    {
        int ch = actionlist->currentRow();
        if( ch == 0 )
        {
            chose = true;
            item.setRequest(QtopiaServiceRequest());
        }
        else if( ch > 0 )
        {
            QListWidgetItem* actionitem = actionlist->item(ch);
            if( actionitem )
            {
                chose = true;
                item = descFor(actionitem);
            }
        }
    }

    return chose;
}

QtopiaServiceDescription QtopiaServiceSelector::descFor(QListWidgetItem* item) const
{
    QString service = item->data(SRV_ROLE).toString();
    QString message = item->data(ACTION_ROLE).toString();
    QString name = item->text();
    QString icon = item->data(ICON_ROLE).toString();

    QtopiaServiceRequest sr(service, message);
    return QtopiaServiceDescription(sr, name, icon);
}

/*!
    Returns the description of the service request \a req.  Descriptions include
    the service request, a user-visible name, and an icon name.

    \sa QtopiaServiceDescription
*/
QtopiaServiceDescription QtopiaServiceSelector::descriptionFor(const QtopiaServiceRequest& req) const
{
    QString srv = req.service();

    const int count = actionlist->count();

    for (int tryapp=0; tryapp<=1; ++tryapp) {
        if ( tryapp )
            srv = QtopiaService::app(req.service());
        for (int i = 0; i < count; i++)
        {
            QListWidgetItem* actionitem = actionlist->item(i);

            if( srv == actionitem->data(SRV_ROLE) &&
                req.message() == actionitem->data(ACTION_ROLE))
            {
                return descFor(actionitem);
            }
        }
    }

    return QtopiaServiceDescription();
}

void QtopiaServiceSelector::selectAction(QListWidgetItem *i)
{
    int a = actionlist->row(i);
    if ( a >= 0 )
        selectAction(a);
}

void QtopiaServiceSelector::selectAction(int a)
{
    selection = a;
    accept();
}

/*!
    \reimp
*/
void QtopiaServiceSelector::keyPressEvent(QKeyEvent* e)
{
    int k = e->key();
    if ( k == Qt::Key_Back )
        reject();
    else if( k >= Qt::Key_Select )
        if(actionlist->currentItem())
            selectAction(actionlist->currentRow());
}


