/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "messageviewer.h"
#include "messagedelegate.h"
#include "messagemodel.h"
#include <QContactModel>
#include <QContactListView>
#include <QKeyEvent>
#include <QLabel>
#include <QMailId>
#include <QSoftMenuBar>
#include <QStandardItemModel>
#include <QtopiaServiceRequest>
#include <QVBoxLayout>

// A stackable widget allowing the selection of a Contact
class ContactSelector : public QWidget
{
    Q_OBJECT

public:
    ContactSelector(QWidget* parent = 0) 
        : QWidget(parent), listView(new QContactListView)
    {
        QLabel* label = new QLabel;
        label->setText(tr("Select a contact to view messages exchanged:"));
        label->setWordWrap(true);

        listView->setModel(&model);
        listView->setFrameStyle(QFrame::NoFrame);

        connect(listView, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(label);
        layout->addWidget(listView);
        // Adjust ContactSelector layout for attractive layout - not relevant to example discussion
        int horizontal = style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
        int vertical = style()->pixelMetric(QStyle::PM_LayoutTopMargin);
        label->setContentsMargins(horizontal, vertical, horizontal, 0);
        layout->setContentsMargins(0, 0, 0, 0);
        // end-Adjust
    }

signals:
    void contactSelected(const QContact& contact);

protected slots:
    void activated(const QModelIndex& index) { emit contactSelected(model.contact(index)); }

private:
    QContactModel model;
    QContactListView* listView;
};

// A stackable widget allowing the selection of a message
class MessageSelector : public QWidget
{
    Q_OBJECT

public:
    MessageSelector(QWidget* parent = 0)
        : QWidget(parent), label(new QLabel), listView(new QListView)
    {
        label->setWordWrap(true);

        listView->setModel(&model);
        listView->setItemDelegate(&delegate);
        listView->setResizeMode(QListView::Adjust);
        listView->setLayoutMode(QListView::Batched);
        listView->setSelectionMode(QAbstractItemView::SingleSelection);
        listView->installEventFilter(this);
        listView->setFrameStyle(QFrame::NoFrame);

        connect(listView, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(label);
        layout->addWidget(listView);
        // Adjust MessageSelector layout for attractive layout - not relevant to example discussion
        int horizontal = style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
        int vertical = style()->pixelMetric(QStyle::PM_LayoutTopMargin);
        label->setContentsMargins(horizontal, vertical, horizontal, 0);
        layout->setContentsMargins(0, 0, 0, 0);
        // end-Adjust
    }

signals:
    void listPrepared();
    void messageSelected(const QMailId& id);
    void done();

public slots:
    void listMessages(const QContact& contact);

protected slots:
    void activated(const QModelIndex& index) { emit messageSelected(model.messageId(index)); }

protected:
    virtual bool eventFilter(QObject* obj, QEvent* event);

private:
    MessageModel model;
    MessageDelegate delegate;
    QLabel* label;
    QListView* listView;
};

void MessageSelector::listMessages(const QContact& contact) 
{
    model.setContact(contact); 

    if (model.rowCount() == 0)
    {
        label->setText(tr("No messages exchanged with %1").arg(contact.label()));
    }
    else
    {
        listView->selectionModel()->select(model.index(0, 0), QItemSelectionModel::Select);
        listView->scrollToTop();
        label->setText(tr("Select a message to view the content:"));
    }

    emit listPrepared();
}
// end-listMessages

bool MessageSelector::eventFilter(QObject* obj, QEvent* event)
{
    // We need to capture the back key, so it doesn't close our window
    if ((obj == listView) && (event->type() == QEvent::KeyPress))
    {
        if (QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event))
        {
            if (keyEvent->key() == Qt::Key_Back)
            {
                emit done();
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}


MessageViewer::MessageViewer(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f),
      contactSelector(new ContactSelector),
      messageSelector(new MessageSelector)
{
    setupUi(this);

    // Necessary to create the menu (which contains our help entry):
    (void)QSoftMenuBar::menuFor(this);

    // Connect our components
    connect(contactSelector, SIGNAL(contactSelected(QContact)), messageSelector, SLOT(listMessages(QContact)));
    connect(messageSelector, SIGNAL(listPrepared()), this, SLOT(showMessageList()));
    connect(messageSelector, SIGNAL(messageSelected(QMailId)), this, SLOT(viewMessage(QMailId)));
    connect(messageSelector, SIGNAL(done()), this, SLOT(showContactList()));

    widgetStack->addWidget(contactSelector);
    widgetStack->addWidget(messageSelector);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(widgetStack);

    showContactList();
}

MessageViewer::~MessageViewer()
{
}

void MessageViewer::showMessageList()
{
    widgetStack->setCurrentWidget(messageSelector);
}

void MessageViewer::viewMessage(const QMailId& id)
{
    // Request that some application display the selected message
    QtopiaServiceRequest req( "Messages", "viewMessage(QMailId)" );
    req << id;
    req.send();
}

void MessageViewer::showContactList()
{
    widgetStack->setCurrentWidget(contactSelector);
}

#include "messageviewer.moc"
