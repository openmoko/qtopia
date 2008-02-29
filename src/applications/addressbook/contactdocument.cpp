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

#include "contactdocument.h"
#include "contactbrowser.h"

#include <QApplication>
#include <QPalette>
#include <QFont>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextTable>
#include <QUrl>

#include <QTimeString>
#include <qtopialog.h>
#include <QLineEdit>
#include "qcontactmodel.h"
#include "qcontact.h"
#include "qtopiaapplication.h"
#include "qthumbnail.h"
#include <QDL>

#ifdef QTOPIA_VOIP
#include <QPresence>
#endif

#if defined(QTOPIA_CELL) || defined(QTOPIA_VOIP)
#include "qcontent.h"
#endif


class ContactAnchorData {
    public:
        ContactDocument::ContactAnchorType type;
        QContactModel::Field field;
        QString number;
};

/* ContactDocument */
ContactDocument::ContactDocument(QObject* parent)
    : QObject(parent)
{
    bDialer = !QtopiaService::channel( "Dialer" ).isEmpty();
#ifdef QTOPIA_VOIP
    mPresence = new QPresence(QString(), this);
    if ( mPresence->available() ) {
        connect(mPresence, SIGNAL(monitoredPresence(QString,QPresence::Status)),
                this, SLOT(monitoredPresence(QString,QPresence::Status)) );
    }
    mOnlinePixmap = NULL;
    mOfflinePixmap = NULL;
    voipDialer = bDialer;
#else
    voipDialer = false;
#endif
    mDocument = new QTextDocument(this);
    mDocument->setUndoRedoEnabled(false);
}

ContactDocument::~ContactDocument()
{
#ifdef QTOPIA_VOIP
    delete mPresence;
    delete mOnlinePixmap;
    delete mOfflinePixmap;
#endif
}

void ContactDocument::init( QWidget *widget, const QContact& contact, ContactDocumentType docType)
{
    mContact = contact;

    QPalette thisPalette = qApp->palette(widget);
    QFont defaultFont = QApplication::font(widget);
    cfNormal.setFont(defaultFont);

    cfBold = cfNormal;
    cfItalic = cfNormal;
    cfSmall = cfNormal;
    cfSmallBold = cfNormal;
    cfAnchor = cfNormal;
    cfBoldUnderline = cfNormal;

    cfItalic.setFontItalic(true);

    cfBold.setFontWeight(80);

    cfBoldUnderline.setFontWeight(80);
    cfBoldUnderline.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    cfSmall.setFontPointSize(cfNormal.fontPointSize() * 0.8);

    cfSmallBold.setFontWeight(80);
    cfSmallBold.setFontPointSize(cfNormal.fontPointSize() * 0.8);

    cfAnchor.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    cfAnchor.setProperty(QTextFormat::IsAnchor, true);
    cfAnchor.setForeground(thisPalette.color(QPalette::Link));

    bfCenter.setAlignment(Qt::AlignHCenter);
    bfNormal.setAlignment(Qt::AlignLeft);

    tfNoBorder.setCellPadding(4);
    tfNoBorder.setCellSpacing(0);
    tfNoBorder.setBorder(0);
    tfNoBorder.setAlignment(Qt::AlignHCenter);

    mDocument->clear();
    mFields.clear();
#if defined(QTOPIA_VOIP)
    foreach(QString s, mMonitoredURIs) {
        mPresence->stopMonitoring(s);
    }
    mMonitoredURIs.clear();
#endif

    switch(docType) {
        case Details:
            createContactDetailsDocument();
            break;
    }
}

void ContactDocument::createContactDetailsDocument()
{
    QTextCursor curs(mDocument);

    QString value;
    QTextBlockFormat bfHVCenter;
    bfHVCenter.setAlignment(Qt::AlignCenter);
    bfHVCenter.setNonBreakableLines(false);

    /* add thumbnail and name */
    curs.setBlockFormat(bfCenter);

    /* check if we need to have a thumbnail */
    QPixmap thumb = mContact.thumbnail();
    if( !thumb.isNull() )
    {
        QVariant thumbV = thumb;
        QTextImageFormat img;
        mDocument->addResource(QTextDocument::ImageResource, QUrl("addressbookdetailthumbnail"), thumbV);
        img.setName("addressbookdetailthumbnail"); // No tr

        QTextTable *tt = curs.insertTable(1, 2, tfNoBorder);

        QTextCursor tCursor = tt->cellAt(0,0).firstCursorPosition();
        tCursor.setBlockFormat(bfHVCenter);
        tCursor.insertImage(img);

        tCursor = tt->cellAt(0, 1).firstCursorPosition();
        tCursor.setBlockFormat(bfHVCenter);
        addNameFragment(tCursor);
    } else {
        addNameFragment(curs);
    }
    curs.movePosition(QTextCursor::NextBlock);

    addTextBreak(curs);

    //  Job (if this is a business contact)
    QCategoryManager c("addressbook", 0);
    QString bcatid  = "Business"; // no tr
    bool isBus = mContact.categories().contains( bcatid );

    if( isBus )
    {
        curs.insertBlock(bfCenter);
        value = mContact.jobTitle();
        if ( !value.isEmpty() )
            curs.insertText(value + " ", cfSmall);

        value = mContact.company();
        if( !value.isEmpty() ) {
            curs.insertText(value, cfSmallBold);
        }
        curs.movePosition(QTextCursor::NextBlock);
    }

    //
    //  Voice and messaging...
    //
    if( isBus )
    {
        addBusinessPhoneFragment( curs );
        addHomePhoneFragment( curs );
    }
    else
    {
        addHomePhoneFragment( curs );
        addBusinessPhoneFragment( curs );
    }

    addEmailFragment( curs );

    // XXX For now, no 'Messaging' support for faxes,
    // XXX but it's completely feasible to add.
    addPhoneFragment(curs, ":icon/businessfax", mContact.businessFax(),
        bDialer ? Dialer : NoLink, QContactModel::BusinessFax );
    addPhoneFragment( curs,  ":icon/homefax", mContact.homeFax(),
        bDialer ? Dialer : NoLink, QContactModel::HomeFax );

    if( isBus ) {
        addBusinessFragment( curs );
        addPersonalFragment( curs );
    } else {
        addPersonalFragment( curs );
        addBusinessFragment( curs );
    }

    value = mContact.notes();
    if ( !value.isEmpty() )
    {
        // XXX add a Notes: header
        addTextBreak(curs);
        curs.insertBlock();
        curs.insertHtml(value);
        curs.movePosition(QTextCursor::NextBlock);
    }
}

void ContactDocument::addPhoneFragment( QTextCursor &curs, const QString& img, const QString& num, LinkType link, QContactModel::Field phoneType)
{
    if ( ! num.isEmpty() ) {
        QString escnum = num;
        escnum = Qt::escape( escnum.replace( QRegExp(" "), "-" ) );

        QTextCharFormat cfMyAnchor(cfAnchor);
        QTextImageFormat cfMyAnchorImage;
        ContactAnchorData *cfd = 0;
        QString fieldKey = QString("contactdocument:") + QString::number(mFields.count()); // no tr

        switch (link)
        {
            case Dialer:
                cfd = new ContactAnchorData;
                cfd->type = DialLink;
                cfd->field = phoneType;
                cfd->number = escnum;
                cfMyAnchor.setProperty(QTextFormat::AnchorHref, fieldKey);
                mFields.insert(fieldKey, cfd);
                break;

            default:
                cfMyAnchor = cfNormal;
                break;
        }
        cfMyAnchorImage.merge(cfMyAnchor);
        cfMyAnchorImage.setName(img);

        addImageAndTextLine(curs, cfMyAnchorImage, num,
                            cfMyAnchor, bfCenter, Qtopia::mousePreferred() ? cfSmall : cfNormal);
    }
}

void ContactDocument::addNameFragment( QTextCursor &curs )
{
    QString value;

    //  Name
    // attempt full name first.
    QStringList nameList;
    if (!mContact.nameTitle().isEmpty())
        nameList.append(mContact.nameTitle());
    if (!mContact.firstName().isEmpty())
        nameList.append(mContact.firstName());
    if (!mContact.middleName().isEmpty())
        nameList.append(mContact.middleName());
    if (!mContact.lastName().isEmpty())
        nameList.append(mContact.lastName());
    if (!mContact.suffix().isEmpty())
        nameList.append(mContact.suffix());

    if (!nameList.isEmpty()) {
        value = nameList.join(QChar(' '));
        curs.insertText(value, cfBold);
    } else if ( !(value = mContact.label()).isEmpty() ) {
       curs.insertText(value, cfBold);
    }

    //  Also, name pronunciation
    if ( !mContact.firstNamePronunciation().isEmpty() || !mContact.lastNamePronunciation().isEmpty()) {
        curs.insertText(QChar(QChar::LineSeparator));
        curs.insertText("\"", cfItalic);
        curs.insertText(mContact.firstNamePronunciation(), cfItalic);
        if (!mContact.firstNamePronunciation().isEmpty() && !mContact.lastNamePronunciation().isEmpty())
            curs.insertText(" ", cfItalic);
        curs.insertText(mContact.lastNamePronunciation(), cfItalic);
        curs.insertText("\"", cfItalic);
    }
}

void ContactDocument::addBusinessPhoneFragment( QTextCursor &curs )
{
    addPhoneFragment( curs, ":icon/businessphone",
        mContact.businessPhone(), bDialer ? Dialer:NoLink, QContactModel::BusinessPhone );
    addPhoneFragment( curs, ":icon/businessmobile",
        mContact.businessMobile(), bDialer ? Dialer:NoLink, QContactModel::BusinessMobile );
    addVoipFragment( curs, ":icon/businessvoip",
        mContact.businessVOIP(), voipDialer ? Dialer:NoLink, QContactModel::BusinessVOIP );
    addPhoneFragment( curs, ":icon/businesspager",
        mContact.businessPager(), NoLink, QContactModel::BusinessPager );
}

void ContactDocument::addHomePhoneFragment( QTextCursor &curs )
{
    addPhoneFragment( curs, ":icon/homephone",
        mContact.homePhone(), bDialer ? Dialer:NoLink, QContactModel::HomePhone );
    addPhoneFragment( curs, ":icon/homemobile",
        mContact.homeMobile(), bDialer ? Dialer:NoLink, QContactModel::HomeMobile );
    addVoipFragment( curs, ":icon/homevoip",
        mContact.homeVOIP(), voipDialer ? Dialer:NoLink, QContactModel::HomeVOIP );
}

void ContactDocument::addEmailFragment( QTextCursor &curs )
{
    QStringList emails = mContact.emailList();
    QStringList::Iterator it;
    for( it = emails.begin() ; it != emails.end() ; ++it )
    {
        QString trimmed = (*it).trimmed();
        if(!trimmed.isEmpty())
        {
            QTextCharFormat cfMyAnchor(cfAnchor);
            QTextImageFormat cfMyImageAnchor;

            ContactAnchorData *cfd = new ContactAnchorData;
            cfd->type = EmailLink;
            cfd->number = Qt::escape(trimmed);
            cfd->field = QContactModel::Emails;
            QString fieldKey = QString("contactdocument:") + QString::number(mFields.count()); // no tr

            cfMyAnchor.setProperty(QTextFormat::AnchorHref, fieldKey);

            mFields.insert(fieldKey, cfd);
            cfMyImageAnchor.merge(cfMyAnchor);
            cfMyImageAnchor.setName(":icon/email");

            addImageAndTextLine(curs, cfMyImageAnchor, trimmed,
                    cfMyAnchor, bfCenter, Qtopia::mousePreferred() ? cfNormal : cfSmall);
        }
    }
}

void ContactDocument::addVoipFragment( QTextCursor &curs, const QString& img, const QString& num, LinkType link, QContactModel::Field phoneType)
{
    if ( ! num.isEmpty() ) {
        QTextCharFormat cfMyAnchor(cfAnchor);
        ContactAnchorData *cfd = 0;
        QString fieldKey = QString("contactdocument:") + QString::number(mFields.count()); // no tr

        switch (link)
        {
            case Dialer:
                cfd = new ContactAnchorData;
                cfd->type = DialLink;
                cfd->field = phoneType;
                cfd->number = num;
                cfMyAnchor.setProperty(QTextFormat::AnchorHref, fieldKey);
                mFields.insert(fieldKey, cfd);
                break;

            default:
                cfMyAnchor = cfNormal;
                break;
        }
        QTextImageFormat cfMyAnchorImage;
        cfMyAnchorImage.merge(cfMyAnchor);
        cfMyAnchorImage.setName(img);

#if defined(QTOPIA_VOIP)
        mMonitoredURIs += num;
        mPresence->startMonitoring(num);
        mDocument->addResource(QTextDocument::ImageResource, QUrl(num + "addressbookvoipicon"), *getPresencePixmap(mPresence->monitoredUriStatus(num) == QPresence::Available));
        cfMyAnchorImage.setName(num + "addressbookvoipicon");
#endif

        addImageAndTextLine(curs, cfMyAnchorImage, num, cfMyAnchor, bfCenter, cfNormal);
    }
}

#if defined(QTOPIA_VOIP)
QPixmap *ContactDocument::getPresencePixmap(bool online)
{
    if (online) {
        if (!mOnlinePixmap)
            mOnlinePixmap = new QPixmap(QIcon(":icon/online").pixmap(QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)));
        return mOnlinePixmap;
    } else {
        if (!mOfflinePixmap)
            mOfflinePixmap = new QPixmap(QIcon(":icon/offline").pixmap(QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)));
        return mOfflinePixmap;
    }
}

void ContactDocument::monitoredPresence( const QString& uri, QPresence::Status status )
{
    // monitoring information received from the phone server
    qLog(Sip) << "VoIP presence report: " << uri << (int)status;

    if (mMonitoredURIs.contains(uri)) {
        // This updates the current pixmap
        mDocument->addResource(QTextDocument::ImageResource, QUrl("addressbookvoipicon"), *getPresencePixmap(status == QPresence::Available));
    }
}
#endif

void ContactDocument::addBusinessFragment( QTextCursor &curs )
{
    QString value, value2;
    QString str;

    /* Save our position before we add the header, in case we need to remove it afterwards */
    int posBefore = curs.position();

    /* Add a header */
    curs.insertBlock(bfCenter);
    curs.insertImage(":icon/addressbook/business");
    curs.insertText(qApp->translate( "QtopiaPim", "Business Details"), cfBoldUnderline);
    curs.movePosition(QTextCursor::NextBlock);

    /* and save the new position, so we can tell if we added anything */
    int posAfter = curs.position();

    /* And add stuff */
    addTextLine(curs, mContact.jobTitle(), cfNormal, bfCenter, cfNormal);
    addTextLine(curs, mContact.department(), cfItalic, bfCenter, cfNormal);

    value = mContact.company();
    value2 = mContact.companyPronunciation();

    if ( !value.isEmpty() || !value2.isEmpty()) {
        curs.insertBlock(bfCenter);
        if ( !value.isEmpty()) {
            curs.insertText(value, cfBold);
            curs.insertText(" ", cfNormal);
        }
        if ( !value2.isEmpty() )
            curs.insertText("\"" + value2 + "\"", cfItalic);
        curs.movePosition(QTextCursor::NextBlock);
    }

    if ( !mContact.businessStreet().isEmpty() || !mContact.businessCity().isEmpty() ||
         !mContact.businessZip().isEmpty() || !mContact.businessCountry().isEmpty() ) {

        // glom the city/state together
        value = mContact.businessCity();
        value2 = mContact.businessState();
        str = value;
        if ( !value2.isEmpty() && !str.isEmpty())
            str += ", ";
        str += value2;

        addTextBreak(curs);
        addTextLine(curs, qApp->translate( "QtopiaPim",  "Address: " ), cfBold, bfNormal, cfNormal);
        addTextLine(curs, mContact.businessStreet(), cfNormal, bfNormal, cfNormal);
        addTextLine(curs, str, cfNormal, bfNormal, cfNormal);
        addTextLine(curs, mContact.businessZip(), cfNormal, bfNormal, cfNormal);
        addTextLine(curs, mContact.businessCountry(), cfNormal, bfNormal, cfNormal);
        addTextBreak(curs);
    }

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Web Page: "), cfBold,
        mContact.businessWebpage(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Office: "), cfBold,
        mContact.office(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Profession: "), cfBold,
        mContact.profession(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Assistant: "), cfBold,
        mContact.assistant(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Manager: "), cfBold,
        mContact.manager(), cfNormal, bfNormal, cfNormal);

    /* Finally, see if we need to remove our header */
    if (curs.position() == posAfter) {
        curs.setPosition(posBefore, QTextCursor::KeepAnchor);
        curs.removeSelectedText();
    }
}

void ContactDocument::addPersonalFragment( QTextCursor& curs )
{
    QString value, value2;
    QString str;

    /* Save our position before we add the header, in case we need to remove it afterwards */
    int posBefore = curs.position();

    /* Add a header */
    curs.insertBlock(bfCenter);
    curs.insertImage(":icon/home");
    curs.insertText(qApp->translate( "QtopiaPim", "Personal Details"), cfBoldUnderline);
    curs.movePosition(QTextCursor::NextBlock);

    /* and save the new position, so we can tell if we added anything */
    int posAfter = curs.position();

    // home address
    if ( !mContact.homeStreet().isEmpty() || !mContact.homeCity().isEmpty() ||
         !mContact.homeZip().isEmpty() || !mContact.homeCountry().isEmpty() ) {
        // glom the city/state together
        value = mContact.homeCity();
        value2 = mContact.homeState();
        str = value;
        if ( !value2.isEmpty() && !str.isEmpty())
            str += ", ";
        str += value2;

        addTextLine(curs, qApp->translate( "QtopiaPim",  "Address: " ), cfBold, bfNormal, cfNormal);
        addTextLine(curs, mContact.homeStreet(), cfNormal, bfNormal, cfNormal);
        addTextLine(curs, str, cfNormal, bfNormal, cfNormal);
        addTextLine(curs, mContact.homeZip(), cfNormal, bfNormal, cfNormal);
        addTextLine(curs, mContact.homeCountry(), cfNormal, bfNormal, cfNormal);
        addTextBreak(curs);
    }

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Web Page: "), cfBold,
            mContact.homeWebpage(), cfNormal, bfNormal, cfNormal);

    str = mContact.gender();
    if ( !str.isEmpty() && str.toInt() != 0 ) {
        if ( str.toInt() == 1 )
            str = qApp->translate( "QtopiaPim", "Male" );
        else if ( str.toInt() == 2 )
            str = qApp->translate( "QtopiaPim", "Female" );
        else
            str = QString();
        addTextNameValue(curs, qApp->translate( "QtopiaPim","Gender: "), cfBold,
                str, cfNormal, bfNormal, cfNormal);
    }

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Spouse: "), cfBold,
            mContact.spouse(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Children:") + " ", cfBold,
            mContact.children(), cfNormal, bfNormal, cfNormal);

    if ( mContact.birthday().isValid() ) {
        addTextNameValue(curs, qApp->translate( "QtopiaPim","Birthday: "), cfBold,
            QTimeString::localYMD( mContact.birthday() ), cfNormal, bfNormal, cfNormal);
    }
    if ( mContact.anniversary().isValid() ) {
        addTextNameValue(curs, qApp->translate( "QtopiaPim","Anniversary: "), cfBold,
            QTimeString::localYMD( mContact.anniversary()), cfNormal, bfNormal, cfNormal);
    }

    /* Finally, see if we need to remove our header */
    if (curs.position() == posAfter) {
        curs.setPosition(posBefore, QTextCursor::KeepAnchor);
        curs.removeSelectedText();
    }
}

void ContactDocument::addTextBreak( QTextCursor &curs)
{
    curs.insertBlock();
    curs.movePosition(QTextCursor::NextBlock);
}

void ContactDocument::addTextLine( QTextCursor& curs, const QString& text, const QTextCharFormat& cf,
                                   const QTextBlockFormat &bf, const QTextCharFormat& bcf)
{
    if (! text.isEmpty() ) {
        curs.insertBlock(bf, bcf);
        curs.insertText(text, cf);
        curs.movePosition(QTextCursor::NextBlock);
    }
}

void ContactDocument::addImageAndTextLine ( QTextCursor& curs, const QTextImageFormat& imf,
                                            const QString& text, const QTextCharFormat& cf, const QTextBlockFormat& bf,
                                            const QTextCharFormat& bcf)
{
    if (! text.isEmpty() ) {
        curs.insertBlock(bf, bcf);
        curs.insertImage(imf);
        curs.insertText(text, cf);
        curs.movePosition(QTextCursor::NextBlock);
    }
}

void ContactDocument::addTextNameValue( QTextCursor& curs, const QString& name,
                                        const QTextCharFormat &ncf, const QString& value, const QTextCharFormat &vcf,
                                        const QTextBlockFormat& bf, const QTextCharFormat& bcf)
{
    if (! value.isEmpty() ) {
        curs.insertBlock(bf, bcf);
        curs.insertText(name, ncf);
        curs.insertText(value, vcf);
        curs.movePosition(QTextCursor::NextBlock);
    }
}

ContactDocument::ContactAnchorType ContactDocument::getAnchorType(const QString& href)
{
    ContactAnchorType cType = None;

    if (href.startsWith("contactdocument:")) {
        ContactAnchorData *cfd = mFields.value(href);
        if (cfd)
            cType = cfd->type;
    } else if (href.startsWith(QDL::ANCHOR_HREF_PREFIX)) {
        cType = QdlLink;
    }

    return cType;
}

QString ContactDocument::getAnchorTarget(const QString &href)
{
    if (href.startsWith("contactdocument:")) {
        ContactAnchorData *cfd = mFields.value(href);
        if (cfd)
            return cfd->number;
    } else if (href.startsWith(QDL::ANCHOR_HREF_PREFIX)) {
        return href;
    }
    return QString();
}

