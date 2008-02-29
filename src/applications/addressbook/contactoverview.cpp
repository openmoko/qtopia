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

#include "contactoverview.h"

#include "qsoftmenubar.h"
#include "qcontactmodel.h"
#include "qcontactview.h"

#include <QKeyEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QStyle>
#include <QScrollArea>
#include <QResizeEvent>

#include <QtopiaService>

// -------------------------------------------------------------
// ContactOverview
// -------------------------------------------------------------
ContactOverview::ContactOverview( QWidget *parent )
    : QWidget( parent ), mInitedGui(false),
    mCall(0), mText(0), mEmail(0), mEdit(0),
    mModel(0), mScrollArea(0), mSetFocus(false)
{
    setObjectName("contactoverview");

    QSoftMenuBar::setLabel(this, Qt::Key_Back,
        QSoftMenuBar::Back, QSoftMenuBar::AnyFocus);
    QSoftMenuBar::setLabel(this, Qt::Key_Select,
        QSoftMenuBar::NoLabel, QSoftMenuBar::AnyFocus);

    // XXX for 4.4, this should be more dynamic
#if defined(QTOPIA_CELL)
    bSMS = true;
#else
    bSMS = false;
#endif

#if defined(QTOPIA_TELEPHONY)
    bDialer = true;
#else
    bDialer = false;
#endif

    bEmail = QtopiaService::apps("Email").count() > 0;
}

ContactOverview::~ContactOverview()
{
}

void ContactOverview::setModel(QContactModel *model)
{
    mModel = model;
    updateCommands();
}

void ContactOverview::updateCommands()
{
    if (mInitedGui) {

        QMap<QContact::PhoneType, QString> numbers = ent.phoneNumbers();

#if !defined(QTOPIA_VOIP)
        // If we don't have VOIP, we can't dial/text VOIP numbers
        numbers.remove(QContact::HomeVOIP);
        numbers.remove(QContact::BusinessVOIP);
        numbers.remove(QContact::VOIP);
#endif

        mCall->setVisible( bDialer && numbers.count() > 0 );
        mEmail->setVisible(bEmail && !ent.defaultEmail().isEmpty() );
        mText->setVisible( bSMS && numbers.count() > 0 );

        if (numbers.count() > 1) {
            mCall->setText(tr("Call..."));
            mText->setText(tr("Text..."));
        } else {
            mCall->setText(tr("Call"));
            mText->setText(tr("Text"));
        }

        if (ent.emailList().count() > 1)
            mEmail->setText(tr("Email..."));
        else
            mEmail->setText(tr("Email"));

        if (mModel) {
            bool editable = mModel->editable(ent.uid());
            mEdit->setVisible(editable);
        } else {
            mEdit->hide();
        }
    }
}

void ContactOverview::init( const QContact &entry )
{
    ent = entry;

    /* Create our UI, if we haven't */
    if (!mInitedGui) {
        mInitedGui = true;

        /* label up the top with the name */
        QVBoxLayout *main = new QVBoxLayout();
        QHBoxLayout *bottom = new QHBoxLayout();
        QVBoxLayout *right = new QVBoxLayout();

        right->setSpacing(2);
        right->setMargin(2);
        bottom->setSpacing(2);
        bottom->setMargin(0);
        main->setSpacing(0);
        main->setMargin(2);

        QWidget *scrollPane = new QWidget();
        mScrollArea = new QScrollArea();
        mScrollArea->setFrameStyle(QFrame::NoFrame);
        mScrollArea->setWidgetResizable(true);
        mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        mScrollArea->setFocusPolicy(Qt::NoFocus);

        mNameLabel = new QLabel();
        mPortrait = new QLabel();
        mPortrait->setFixedSize(QContact::portraitSize());
        mPortrait->setAlignment(Qt::AlignCenter);

        QFont labelFont = font();
        labelFont.setWeight(80);
        labelFont.setPointSizeF(labelFont.pointSize() * 1.2);
        mNameLabel->setFont(labelFont);
        mNameLabel->setAlignment(Qt::AlignHCenter);

        mCall = new QPushButton();
        mText = new QPushButton();
        mEmail = new QPushButton();
        mEdit = new QPushButton();

        buttons << mCall << mText << mEmail << mEdit;

        mEdit->setText(tr("Edit"));

        QSoftMenuBar::setLabel(mCall, Qt::Key_Select,
                "phone/calls", tr("Dial"), QSoftMenuBar::AnyFocus);
        QSoftMenuBar::setLabel(mText, Qt::Key_Select,
                "email", tr("Text"), QSoftMenuBar::AnyFocus);
        QSoftMenuBar::setLabel(mEmail, Qt::Key_Select,
                "email", tr("Email"), QSoftMenuBar::AnyFocus);
        QSoftMenuBar::setLabel(mEdit, Qt::Key_Select,
                               QSoftMenuBar::Edit, QSoftMenuBar::AnyFocus);

        connect(mCall, SIGNAL(clicked()), this, SIGNAL(callContact()));
        connect(mText, SIGNAL(clicked()), this, SIGNAL(textContact()));
        connect(mEmail, SIGNAL(clicked()), this, SIGNAL(emailContact()));
        connect(mEdit, SIGNAL(clicked()), this, SIGNAL(editContact()));

        right->addWidget(mCall, 1);
        right->addWidget(mText, 1);
        right->addWidget(mEmail, 1);
        right->addWidget(mEdit, 1);
        right->addStretch();

        scrollPane->setLayout(right);
        mScrollArea->setWidget(scrollPane);

        bottom->addWidget(mPortrait, 0, Qt::AlignTop | Qt::AlignRight);
        bottom->addWidget(mScrollArea);

        main->addWidget(mNameLabel);
        main->addLayout(bottom, 1);
        main->addStretch();

        setLayout(main);
    }

    if (mModel) {
        QIcon i = qvariant_cast<QIcon>(mModel->data(mModel->index(ent.uid()), Qt::DecorationRole));
        if (!i.isNull())
            mPortrait->setPixmap(i.pixmap(QContact::portraitSize()));
        else
            mPortrait->setPixmap(ent.portrait());
    } else {
        mPortrait->setPixmap(ent.portrait());
    }

    // Until we can get QLabel to do this
    QFontMetrics fm(mNameLabel->font());
    mNameLabel->setText(fm.elidedText(ent.label(), Qt::ElideRight, mNameLabel->width()));

    updateCommands();
    mSetFocus = true;
}

void ContactOverview::focusInEvent( QFocusEvent * )
{
    if (mSetFocus) {
        foreach (QAbstractButton *w, buttons) {
            if (w->isVisibleTo(mScrollArea)) {
                w->setFocus();
                mScrollArea->ensureWidgetVisible(w);
                break;
            }
        }
        mSetFocus = false;
    }
}

void ContactOverview::resizeEvent(QResizeEvent *e)
{
    if (mInitedGui) {
        QFontMetrics fm(mNameLabel->font());
        mNameLabel->setText(fm.elidedText(ent.label(), Qt::ElideRight, mNameLabel->width()));
    }
    QWidget::resizeEvent(e);
}

void ContactOverview::keyPressEvent( QKeyEvent *e )
{
    switch(e->key())
    {
        case Qt::Key_Back:
            emit backClicked();
            return;
        case Qt::Key_Call:
            emit callContact();
            e->setAccepted(true);
            return;
    }

    QWidget::keyPressEvent(e);
}

