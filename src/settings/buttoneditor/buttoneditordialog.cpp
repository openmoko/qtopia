/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "buttonrow.h"

#include <qlayout.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <qtopia/qpeapplication.h>
#include <qtopia/applnk.h>
#include <qtopia/config.h>
#include <qtopia/resource.h>
#include <qtopia/services.h>

#include <qtopia/devicebuttonmanager.h>
#include "buttoneditordialog.h"

ButtonEditorDialog::ButtonEditorDialog() 
  : QDialog(0L, "ButtonEditorDialog", TRUE)
{
  setCaption(tr("Configure Hardware Buttons"));
  initAppList();
  buildGUI();
}

ButtonEditorDialog::~ButtonEditorDialog()
{
  m_AppList.clear();
  m_PressComboBoxes.clear();
  m_HoldComboBoxes.clear();
}

/*!
  Called when the user accepts the settings in the dialog.  Iterates through the
  combo boxes and remaps the buttons accordingly.
*/
void ButtonEditorDialog::accept()
{
    const QValueList<DeviceButton>& buttonList = DeviceButtonManager::instance().buttons();
    QValueListConstIterator<DeviceButton> it = buttonList.begin();
    int index = 0;
    for (; it != buttonList.end(); ++it, ++index) {
        QComboBox* comboBox = m_PressComboBoxes.at(index);
	ServiceRequest qcopMess = actionFor(comboBox->currentItem());
	DeviceButtonManager::instance().remapPressedAction(index, qcopMess);
        comboBox = m_HoldComboBoxes.at(index);
	qcopMess = actionFor(comboBox->currentItem());
	DeviceButtonManager::instance().remapHeldAction(index, qcopMess);
    }
    QDialog::accept();
}

void ButtonEditorDialog::done(int r)
{ 
    QDialog::done(r);
    close();
}

ServiceRequest ButtonEditorDialog::actionFor(int cur) const
{
    QCString service,action;
    if ( cur < (int)m_AppList.count() ) {
	AppLnk* appLnk = (((ButtonEditorDialog*)this)->m_AppList).at(cur);
	service = appLnk->exec();
	action = "raise()";
    } else {
	cur-=m_AppList.count();
	if ( cur < (int)m_Actions.count() ) {
	    QString s = m_Actions[cur];
	    int c = s.find(':');
	    service = s.left(c);
	    action = s.mid(c+1);
	} else {
	    // This must be a special function...
	    cur-=m_Actions.count();
	    if ( cur == 0 ) {
		service = "qpe";
		action = "raise()";
	    }
	}
    }
    return ServiceRequest(service, action);
}

static QStringList readActions(const QString& srv, Config& cfg)
{
    QStringList actions = cfg.readListEntry("Actions",';');
    QStringList r;
    for (QStringList::ConstIterator ait=actions.begin(); ait!=actions.end(); ++ait) {
	if ( (*ait).right(2)=="()" ) {
	    cfg.setGroup(*ait);
	    if ( !cfg.readEntry("Name").isEmpty() )
		r.append(srv+":"+*ait);
	}
    }
    return r;
}

/*!
  Gets the list of AppLnks from the AppLnkSet and extracts only
  the applications.
*/
void ButtonEditorDialog::initAppList()
{
    // Copy all of the AppLnks to our own list
    AppLnkSet lnkSet(QPEApplication::qpeDir() + "apps");
    const QList<AppLnk>& apps = lnkSet.children();
    m_AppList.setAutoDelete(TRUE);
    for (QListIterator<AppLnk> it(apps); it.current() ; ++it)
        if (it.current()->type() != "Separator") // No tr
            m_AppList.append(new AppLnk(*it.current())); 

    // Get the list of parameterless actions for all services
    m_Actions.clear();
    QStringList s = Service::list();
    for (QStringList::ConstIterator itService = s.begin(); itService!=s.end(); ++itService) {
	Config srv(Service::config(*itService),Config::File);
	if ( srv.isValid() ) {
	    srv.setGroup("Service");
	    m_Actions += readActions(*itService,srv);
	}
	Config srvapp(Service::appConfig(*itService),Config::File);
	if ( srvapp.isValid() ) {
	    srvapp.setGroup("Extensions");
	    m_Actions += readActions(*itService,srvapp);
	}
    }
}

/*!
  Dynamically builds the button configuration GUI based on how many buttons the
  device offers.  If the button has a pixmap, that is used in preference to the 
  user text.  However, if no pixmap is available, the user text is shown.
*/
void ButtonEditorDialog::buildGUI()
{
    const QValueList<DeviceButton>& buttonList = DeviceButtonManager::instance().buttons();
    QValueListConstIterator<DeviceButton> it = buttonList.begin();
    QVBoxLayout* vbox = new QVBoxLayout(this,6,0);
    vbox->setAutoAdd(TRUE);
    m_PressComboBoxes.setAutoDelete(FALSE);
    m_HoldComboBoxes.setAutoDelete(FALSE);

    // Sanity check - is $QPEDIR/etc/defaultbuttons.conf missing?
    if ( buttonList.count() == 0 )
	new QLabel(tr("No buttons!"),this);

    for(; it != buttonList.end(); it++) {
	ButtonRow* row = new ButtonRow(this);
        if (!(*it).pixmap().isNull())
            row->label->setPixmap((*it).pixmap());
	else
            row->label->setText((*it).userText());

	populate(row->press,(*it).pressedAction());
        m_PressComboBoxes.append(row->press);

	populate(row->hold,(*it).heldAction());
        m_HoldComboBoxes.append(row->hold);
    }
}

void ButtonEditorDialog::populate(QComboBox* comboBox, const ServiceRequest& current)
{
    QListIterator<AppLnk> appIter(m_AppList);

    // Add apps
    for (; appIter.current(); ++appIter)
	comboBox->insertItem(appIter.current()->pixmap(),appIter.current()->name());

    // Add actions
    for ( QStringList::ConstIterator sit = m_Actions.begin(); sit!=m_Actions.end(); ++sit) {
	int c = (*sit).find(':');
	Config srv(Service::config((*sit).left(c)),Config::File);
	srv.setGroup((*sit).mid(c+1));
	int s = AppLnk::smallIconSize();
	QImage img = Resource::loadImage(srv.readEntry("Icon")).smoothScale(s,s);
	QPixmap pm; pm.convertFromImage(img);
	comboBox->insertItem(pm, srv.readEntry("Name"));
    }

    // Since we don't have an applnk for QPE, we hardwire this one..
    comboBox->insertItem(Resource::loadIconSet("home").pixmap(),tr("Home"));
    comboBox->insertItem(tr("No action"));

    int i;
    int cur2=-1;
    if ( current.isNull() ) {
	i = comboBox->count();
    } else {
	// Now set the combo box to "current"
	// Also look for "dereferenced" service application
	ServiceRequest current2(Service::app(current.service()),current.message());
	for (i=0; i<comboBox->count(); ++i) {
	    ServiceRequest m = actionFor(i);
	    if ( m == current ) {
		comboBox->setCurrentItem(i);
		break;
	    } else if ( m == current2 )
		cur2 = i;
	}
    }
    if ( i==comboBox->count() ) // not found (eg. old binding)
	comboBox->setCurrentItem(cur2 < 0 ? i-1 : cur2); // The fallback, or the "No action"
}
