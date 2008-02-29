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

#include "detailspage.h"

#include <qsoftmenubar.h>
#include <qtopiaapplication.h>
#include <qtopia/pim/qcontact.h>
#ifdef QTOPIA4_TODO
#include <qtopia/pim/private/contactfieldselector_p.h>
#endif
#include <qtopia/pim/qcontactview.h>

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qtoolbutton.h>
#include <qmenu.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <QScrollArea>
//#include <qwhatsthis.h>

#include "composer.h"

static QStringList splitAddresses(const QString& txt)
{
    // only separates them ( use parseContact to fetch email/name )
    QStringList out, list = txt.split(',');
    QString str;

    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
        str = (*it).trimmed();
        if ( !str.isEmpty() ) {
            out.append( str );
        }
    }
    return out;
}

DetailsPage::DetailsPage( QWidget *parent, const char *name )
    : QWidget( parent ), m_type( -1 )
{
    m_ignoreFocus = true;
    setObjectName( name );
    QIcon abicon(":icon/addressbook/AddressBook");
    QMenu *menu = QSoftMenuBar::menuFor( this );
    if( !Qtopia::mousePreferred() )
    {
        menu->addAction( abicon, tr("From Contacts", "Find recipient's phone number or email address from Contacts application"),
                         this, SLOT(editRecipients()) );
        menu->addSeparator();
        menu->addAction( QIcon(":icon/copy"), tr("Copy"),
                         this, SLOT(copy()) );
        menu->addAction( QIcon(":icon/paste"), tr("Paste"),
                         this, SLOT(paste()) );
    }

    const int margin = 2;
    setMaximumWidth( qApp->desktop()->width() - 2 * margin );
    QGridLayout *l = new QGridLayout( this );
    int rowCount = 0;

    m_toFieldLabel = new QLabel( this );
    m_toFieldLabel->setText( tr( "To" ) );
    m_toBox = new QHBoxLayout( );
    m_toField = new QLineEdit( this );
    m_toBox->addWidget( m_toField );
    connect( m_toField, SIGNAL(textChanged(const QString&)), this, SIGNAL(recipientsChanged()) );
    l->addWidget( m_toFieldLabel, rowCount, 0 );
    QSoftMenuBar::addMenuTo(m_toField, menu);

#ifdef QTOPIA_PHONE
    m_toPicker = 0;
    if( Qtopia::mousePreferred() )
    {
        m_toPicker = new QToolButton( this );
        m_toBox->addWidget( m_toPicker );
        m_toPicker->setText( tr( "..." ) );
        //QWhatsThis::add(  m_toPicker, tr( "Select recipients from the addressbook." ) );
        m_toPicker->setIcon( abicon );
        connect( m_toPicker, SIGNAL(clicked()), this, SLOT(editRecipients()) );
    }
#endif
    l->addLayout( m_toBox, rowCount, 2 );
    ++rowCount;

    m_ccFieldLabel = new QLabel( this );
    m_ccFieldLabel->setText( tr( "CC" ) );
    m_ccBox = new QHBoxLayout( );
    m_ccField = new QLineEdit( this );
    m_ccBox->addWidget( m_ccField );
    connect( m_ccField, SIGNAL(textChanged(const QString&)), this, SIGNAL(recipientsChanged()) );
    l->addWidget( m_ccFieldLabel, rowCount, 0 );
    QSoftMenuBar::addMenuTo( m_ccField, menu );

#ifdef QTOPIA_PHONE
    m_ccPicker = 0;
    if( Qtopia::mousePreferred() )
    {
        m_ccPicker = new QToolButton( this );
        m_ccBox->addWidget( m_ccPicker );
        m_ccPicker->setText( tr( "..." ) );
        m_ccPicker->setIcon( abicon );
        connect( m_ccPicker, SIGNAL(clicked()), this, SLOT(editRecipients()) );
        //QWhatsThis::add( m_ccPicker, tr( "Select carbon copies from the addressbook." ) );
    }
#endif
    l->addLayout( m_ccBox, rowCount, 2 );
    ++rowCount;

    m_bccFieldLabel = new QLabel( this );
    m_bccFieldLabel->setText( tr( "BCC" ) );
    m_bccBox = new QHBoxLayout( );
    m_bccField = new QLineEdit( this );
    m_bccBox->addWidget( m_bccField );
    connect( m_bccField, SIGNAL(textChanged(const QString&)), this, SIGNAL(recipientsChanged()) );
    l->addWidget( m_bccFieldLabel, rowCount, 0 );
#ifdef QTOPIA_PHONE
    m_bccPicker = 0;
    if( Qtopia::mousePreferred() )
    {
        m_bccPicker = new QToolButton( this );
        m_bccBox->addWidget( m_bccPicker );
        m_bccPicker->setEnabled( true );
        m_bccPicker->setText( tr( "..." ) );
        m_bccPicker->setIcon( abicon );
        connect( m_bccPicker, SIGNAL(clicked()), this, SLOT(editRecipients()) );
    ////QWhatsThis::add(  m_bccPicker, tr( "Select blind carbon copies from the addressbook." ) );
    }
#endif
    l->addLayout( m_bccBox, rowCount, 2 );
    ++rowCount;
    QSoftMenuBar::addMenuTo( m_bccField, menu );

    m_subjectFieldLabel = new QLabel( this );
    m_subjectFieldLabel->setText( tr( "Subject" ) );
    m_subjectField = new QLineEdit( this );
    l->addWidget( m_subjectFieldLabel, rowCount, 0 );
    l->addWidget( m_subjectField, rowCount, 2 );
    ++rowCount;
    QSoftMenuBar::addMenuTo( m_subjectField, menu );

    m_deliveryReportField = new QCheckBox( tr("Delivery Report"), this );
    l->addWidget( m_deliveryReportField, rowCount, 0, 1, 3, Qt::AlignLeft );
    ++rowCount;

    m_readReplyField = new QCheckBox( tr("Read Reply"), this );
    l->addWidget( m_readReplyField, rowCount, 0, 1, 3, Qt::AlignLeft );
    ++rowCount;

    m_fromFieldLabel = new QLabel( this );
    m_fromFieldLabel->setEnabled( true );
    m_fromFieldLabel->setText( tr( "From" ) );
    m_fromField = new QComboBox( this );
    m_fromField->setEditFocus( false );
    m_fromField->setEnabled( true );
    m_fromField->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum )); // Why not automatic?
    //QWhatsThis::add(  m_fromField, tr( "Select the from address used for this mail." ) );
    l->addWidget( m_fromFieldLabel, rowCount, 0 );
    l->addWidget( m_fromField, rowCount, 2 );
    ++rowCount;
    QSoftMenuBar::addMenuTo( m_fromField, menu );

    QSpacerItem* spacer1 = new QSpacerItem( 4, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    l->addItem( spacer1, rowCount, 1 );
    ++rowCount;

    QList<QWidget*> tabOrderList;

    tabOrderList.append( m_toField );
#ifdef QTOPIA_PHONE
    if( Qtopia::mousePreferred() )
#endif
        if (m_toPicker)
            tabOrderList.append( m_toPicker );
    tabOrderList.append( m_ccField );
#ifdef QTOPIA_PHONE
    if( Qtopia::mousePreferred() )
#endif
        if (m_ccPicker)
            tabOrderList.append( m_ccPicker );
    tabOrderList.append( m_bccField );
#ifdef QTOPIA_PHONE
    if( Qtopia::mousePreferred() )
#endif
        if (m_bccPicker)
            tabOrderList.append( m_bccPicker );
    tabOrderList.append( m_subjectField );
    tabOrderList.append( m_fromField );

    QListIterator<QWidget*> it( tabOrderList );
    QWidget *prev = 0;
    QWidget *next;
    while ( it.hasNext() ) {
        next = it.next();
        if ( prev )
            setTabOrder( prev, next );
        prev = next;
    }
    m_toField->installEventFilter(this);
    m_ccField->installEventFilter(this);
    m_bccField->installEventFilter(this);
}

void DetailsPage::editRecipients()
{
    QLineEdit *le = 0;
#ifdef QTOPIA_PHONE
    if( Qtopia::mousePreferred() )
    {
        if( sender() == m_toPicker )
            le = m_toField;
        else if( sender() == m_ccPicker )
            le = m_ccField;
        else if( sender() == m_bccPicker )
            le = m_bccField;
    }
    else
    {
        QWidget *w = focusWidget();
        if( w && w->inherits("QLineEdit") )
            le = (QLineEdit *)w;
    }
#endif
    if( !le )
    {
        qWarning("DetailsPage::editRecipients: Couldn't find line edit for recipients.");
        return;
    }

    // Use the contactselector for now
    QContactSelector picker( false, this );
    QContactModel *model = new QContactModel( this );

    QSettings config( "Trolltech", "Contacts" );

    // load SIM/No SIM settings.
    config.beginGroup( "default" );
    if (config.contains("SelectedSources/size")) {
        int count = config.beginReadArray("SelectedSources");
        QSet<QPimSource> set;
        for(int i = 0; i < count; ++i) {
            config.setArrayIndex(i);
            QPimSource s;
            s.context = QUuid(config.value("context").toString());
            s.identity = config.value("identity").toString();
            set.insert(s);
        }
        config.endArray();
        model->setVisibleSources(set);
    }

    if( m_allowPhoneNumbers )
        model->setFilter( "", QContactModel::ContainsPhoneNumber );
    else // m_allowEmails
        model->setFilter( "", QContactModel::ContainsEmail );
    picker.setModel( model );

    int r = QtopiaApplication::execDialog( &picker );
    if ( r == QDialog::Accepted && picker.contactSelected() ) {
        QString newText;
        if( m_allowPhoneNumbers )
            newText = picker.selectedContact().defaultPhoneNumber();
        else //  m_allowEmails
            newText = picker.selectedContact().defaultEmail();
        if ( le->text().trimmed().isEmpty() )
            le->setText( newText );
        else
            le->setText( le->text()  + ", " + newText );
    }

#ifdef QTOPIA4_TODO
    QList<QContact::ContactFields> fields;
    if( m_allowPhoneNumbers )
    {
        fields.append( PimContact::HomeMobile );
        fields.append( PimContact::BusinessMobile );
    }
    if( m_allowEmails )
    {
        fields.append( PimContact::Emails );
    }
    ContactFieldSelector picker( fields, "", true, this, "addressPicker", true );
    QStringList curAddr = QStringList::split( ",", le->text() );
    for( uint i = 0 ; i < curAddr.count() ; ++i )
        curAddr[i] = curAddr[i].stripWhiteSpace();
    picker.setSelectedFields( curAddr );
    int r = QtopiaApplication::execDialog( &picker );
    if (  r == QDialog::Accepted ) {
        QStringList unknownFieldData = picker.unknownFieldData();
        QString userData;
        if (unknownFieldData.count())
            userData = unknownFieldData.join( ", " );
        QString selectedFields;
        QValueList<int> selectedIndices = picker.selected();
        for( QValueList<int>::Iterator it = selectedIndices.begin() ;
                                        it != selectedIndices.end() ; ++it )
        {
            if( !selectedFields.isEmpty() )
                selectedFields += ", ";
            selectedFields += picker.fieldDataAt( *it );
        }
        if( !userData.isEmpty() && !selectedFields.isEmpty() )
            userData += ", ";
        le->setText( userData + selectedFields );
    }
#endif
}

void DetailsPage::setType( int t )
{
    int imHint = QtopiaApplication::Normal;
    if( m_type != t )
    {
        m_allowPhoneNumbers = false;
        m_allowEmails = false;
        m_type = t;
        m_ccField->hide();
        if (m_ccPicker)
            m_ccPicker->hide();
        m_ccFieldLabel->hide();
        m_bccField->hide();
        if (m_bccPicker)
            m_bccPicker->hide();
        m_bccFieldLabel->hide();
        m_subjectField->hide();
        m_subjectFieldLabel->hide();
        m_fromField->hide();
        m_fromFieldLabel->hide();
        m_readReplyField->hide();
        m_deliveryReportField->hide();
        // composer interface can be mms OR ems OR sms AND/OR email
        if( t == MailMessage::MMS )
        {
            m_allowPhoneNumbers = true;
            m_allowEmails = true;
            m_ccFieldLabel->show();
            m_ccField->show();
            if (m_ccPicker)
                m_ccPicker->show();
            m_bccFieldLabel->show();
            m_bccField->show();
            if (m_bccPicker)
                m_bccPicker->show();
            m_subjectField->show();
            m_subjectFieldLabel->show();
            m_readReplyField->show();
            m_deliveryReportField->show();
        }
        else if( t == MailMessage::EMS )
        {
            m_allowPhoneNumbers = true;
        }
        else
        {
            if( t & MailMessage::SMS )
            {
                m_allowPhoneNumbers = true;
            }
            if( t & MailMessage::Email )
            {
                m_allowEmails = true;
                if( t == MailMessage::Email )
                { // only allowed if explicitly email
                    m_ccFieldLabel->show();
                    m_ccField->show();
                    if (m_ccPicker)
                        m_ccPicker->show();
                    m_bccFieldLabel->show();
                    m_bccField->show();
                    if (m_bccPicker)
                        m_bccPicker->show();
                    m_subjectField->show();
                    m_subjectFieldLabel->show();
                    m_fromField->show();
                    m_fromFieldLabel->show();
                }
            }
        }
        if( m_allowPhoneNumbers )
            imHint = QtopiaApplication::PhoneNumber;
        else if( m_allowEmails )
            imHint = QtopiaApplication::Words;

        QtopiaApplication::setInputMethodHint( m_toField,
                                        (QtopiaApplication::InputMethodHint)imHint );
        QtopiaApplication::setInputMethodHint( m_ccField,
                                        (QtopiaApplication::InputMethodHint)imHint );
        QtopiaApplication::setInputMethodHint( m_bccField,
                                        (QtopiaApplication::InputMethodHint)imHint );
    }
    m_toField->setFocus();
    layout()->activate();
}

void DetailsPage::getDetails( MailMessage &mail )
{
    mail.setTo( QStringList( to() ) );
    mail.setCc( QStringList( cc() ) );
    mail.setBcc( QStringList( bcc() ) );
    mail.setSubject( subject() );
    if( mail.subject().isEmpty() ) {
        if ( (m_type & MailMessage::Email) ||
             mail.plainTextBody().trimmed().isEmpty() )
            mail.setSubject( tr("(no subject)") );
        else
            mail.setSubject( mail.plainTextBody() );
    }
    mail.setFrom( from() );
    if( m_type == MailMessage::MMS ) {
        if ( m_deliveryReportField->isChecked() )
            mail.setExtraHeader( "X-Mms-Delivery-Report", "Yes" );
        if ( m_readReplyField->isChecked() )
            mail.setExtraHeader( "X-Mms-Read-Reply", "Yes" );
    }
    if( m_type & MailMessage::SMS ) {
        // For the time being limit sending SMS messages so that they can
        // only be sent to phone numbers and not email addresses
        QString number = to();
        QString n;
        QStringList numbers;
        uint posn;
        uint ch;
        for ( posn = 0; posn < (uint)number.length(); ++posn ) {
            ch = number[posn].unicode();
            if ( ch >= '0' && ch <= '9' ) {
                n += (QChar)ch;
            } else if ( ch == '+' || ch == '#' || ch == '*' ) {
                n += (QChar)ch;
            } else if ( ch == '-' || ch == '(' || ch == ')' ) {
                n += (QChar)ch;
            } else if ( ch == ' ' ) {
                n += (QChar)ch;
            } else if ( ch == ',' ) {
                if (!n.isEmpty())
                    numbers.append( n );
                n = "";
            } // else ignore
        }
        if (!n.isEmpty())
            numbers.append( n );
        mail.setTo( numbers );
    }
}

void DetailsPage::setBcc( const QString &a_bcc )
{
    m_bccField->setText( a_bcc );
}

QString DetailsPage::bcc() const
{
    QString text;
    if( !m_bccField->isHidden() )
        text = m_bccField->text();
    return text;
}


void DetailsPage::setCc( const QString &a_cc )
{
    m_ccField->setText( a_cc );
}

QString DetailsPage::cc() const
{
    QString text;
    if( !m_ccField->isHidden() )
        text = m_ccField->text();
    return text;
}

void DetailsPage::setTo( const QString &a_to )
{
    m_toField->setText( a_to );
}

QString DetailsPage::to() const
{
    // TODO : only include addresses according to allow flags
    QStringList list = splitAddresses( m_toField->text() );
    return m_toField->text();
}

QString DetailsPage::subject() const
{
    return m_subjectField->text();
}

void DetailsPage::setSubject( const QString &sub )
{
    m_subjectField->setText( sub );
}

QString DetailsPage::from() const
{
    return m_fromField->currentText();
}

void DetailsPage::setFrom( const QString &from )
{
    int i;
    for( i = 0 ; i < (int)m_fromField->count() ; ++i )
    {
        if( m_fromField->itemText( i ) == from )
        {
            m_fromField->setCurrentIndex( i );
            break;
        }
    }
    if( i < m_fromField->count() )
    {
        m_fromField->insertItem( 0, from );
        m_fromField->setCurrentIndex( 0 );
    }
}

void DetailsPage::setFromFields( const QStringList &from )
{
    m_fromField->clear();
    m_fromField->addItems( from );
}

void DetailsPage::copy()
{
    QWidget *fw = focusWidget();
    if( !fw )
        return;
    if( fw->inherits( "QLineEdit" ) )
        ((QLineEdit*)(fw))->copy();
    else if( fw->inherits( "QTextEdit" ) )
        ((QTextEdit*)(fw))->copy();
}

void DetailsPage::paste()
{
    QWidget *fw = focusWidget();
    if( !fw )
        return;
    if( fw->inherits( "QLineEdit" ) )
        ((QLineEdit*)(fw))->paste();
    else if( fw->inherits( "QTextEdit" ))
        ((QTextEdit*)(focusWidget()))->paste();
}

void DetailsPage::clear()
{
    m_toField->clear();
    m_ccField->clear();
    m_bccField->clear();
    m_subjectField->clear();
    m_readReplyField->setChecked( false );
    // don't clear from fields
}

bool DetailsPage::eventFilter(QObject* obj, QEvent* event)
{
    if (((obj == m_toField) || (obj == m_ccField) || (obj == m_bccField))
        && (event->type() == QEvent::KeyPress)
        && (qobject_cast<QLineEdit *>(obj)->text().isEmpty())) {
        QKeyEvent *keyEvent = (QKeyEvent*)event;
        if (keyEvent->key() == Qt::Key_Select) {
            editRecipients();
            return true;
        }
        return false;
    } else if (obj == m_toField) {
        if (event->type() == QEvent::EnterEditFocus && m_ignoreFocus) {
            if(!m_toField->text().isEmpty())
                m_toField->setEditFocus(false);
            m_ignoreFocus = false;
            return true;
        }
        return false;
    } else {
        return QWidget::eventFilter(obj, event);
    }
}

