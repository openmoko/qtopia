/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#define QTOPIA_INTERNAL_CONTACT_MRE

#include "contact.h"
#include <qtopia/private/vobject_p.h>
#include <qtopia/private/cp1252textcodec_p.h>
#include <qtopia/private/qfiledirect_p.h>

#include <qtopia/stringutil.h>
#include <qtopia/timeconversion.h>
#include <qtopia/timestring.h>
#include <qtopia/config.h>
#include <qtopia/pim/private/xmlio_p.h>
#include <qtopia/global.h>
#include <qtopia/applnk.h>
#include <qtopia/image.h>

#include <qobject.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qstylesheet.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qbuffer.h>
#include <qtextcodec.h>
#include <qpixmap.h>
#include <qdatastream.h>
#include <qdir.h>

#include <stdio.h>

extern QTOPIA_EXPORT int q_DontDecodeBase64Photo;

QString emailSeparator() { return " "; }

static QMap<int, int> *uniquenessMapPtr = 0;
static QMap<QCString, int> *identifierToKeyMapPtr = 0;
static QMap<int, QCString> *keyToIdentifierMapPtr = 0;
static QMap<int, QString> * trFieldsMapPtr = 0;


/*!
  \class PimContact
  \module qpepim
  \ingroup qpepim
  \brief The PimContact class holds the data of an address book entry.

  This data includes information the name of the person, contact
  information, and business information such as deparment and job title.

*/

/*!
  \enum PimContact::ContactFields
  \internal
*/

/*!
  \fn QMap<int,QString> PimContact::fields() const
  \internal
*/

/*!
  Constructs a new PimContact.
*/
PimContact::PimContact()
    : PimRecord(), mMap(), d( 0 )
{
}

/*!
  \internal
*/
void PimContact::fromMap( const QMap<int,QString> &m)
{
    setFields( m );
}

/*!
  Destroys the contact.
*/
PimContact::~PimContact()
{
}

/*!
  Returns a rich text formatted QString of the PimContact.
*/
QString PimContact::toRichText() const
{
    if (!keyToIdentifierMapPtr)
	initMaps();

    QString text;
    QString value, comp, state;

    // name, jobtitle and company
    if ( !(value = fileAs()).isEmpty() )
	text += "<b>" + Qtopia::escapeString(value) + "</b><br>";
    // also part of name is how to pronounce it.

    if ( !(value = firstNamePronunciation()).isEmpty() )
	text += "<b>( " + Qtopia::escapeString(value) + " )</br><br>";

    if ( !(value = lastNamePronunciation()).isEmpty() )
	text += "<b>( " + Qtopia::escapeString(value) + " )</br><br>";

    if ( !(value = jobTitle()).isEmpty() )
	text += Qtopia::escapeString(value) + "<br>";

    comp = company();
    if ( !(value = department()).isEmpty() ) {
	text += Qtopia::escapeString(value);
	if ( comp )
	    text += ", ";
	else
	    text += "<br>";
    }
    if ( !comp.isEmpty() )
	text += Qtopia::escapeString(comp) + "<br>";
    if ( !(value = companyPronunciation()).isEmpty() )
	text += "<b>( " + Qtopia::escapeString(value) + " )</br><br>";


    // business address
    if ( !businessStreet().isEmpty() || !businessCity().isEmpty() ||
	 !businessZip().isEmpty() || !businessCountry().isEmpty() ) {
	text += "<br>";
	text += "<b>" + qApp->translate( "QtopiaPim",  "Work Address: " ) + "</b>";
	text +=  "<br>";
    }

    if ( !(value = businessStreet()).isEmpty() )
	text += Qtopia::escapeString(value) + "<br>";
    state =  businessState();
    if ( !(value = businessCity()).isEmpty() ) {
	text += Qtopia::escapeString(value);
	if ( state )
	    text += ", " + Qtopia::escapeString(state);
	text += "<br>";
    } else if ( !state.isEmpty() )
	text += Qtopia::escapeString(state) + "<br>";
    if ( !(value = businessZip()).isEmpty() )
	text += Qtopia::escapeString(value) + "<br>";
    if ( !(value = businessCountry()).isEmpty() )
	text += Qtopia::escapeString(value) + "<br>";

    // home address
    if ( !homeStreet().isEmpty() || !homeCity().isEmpty() ||
	 !homeZip().isEmpty() || !homeCountry().isEmpty() ) {
	text += "<br>";
	text += "<b>" + qApp->translate( "QtopiaPim",  "Home Address: " ) + "</b>";
	text +=  "<br>";
    }

    if ( !(value = homeStreet()).isEmpty() )
	text += Qtopia::escapeString(value) + "<br>";
    state =  homeState();
    if ( !(value = homeCity()).isEmpty() ) {
	text += Qtopia::escapeString(value);
	if ( !state.isEmpty() )
	    text += ", " + Qtopia::escapeString(state);
	text += "<br>";
    } else if (!state.isEmpty())
	text += Qtopia::escapeString(state) + "<br>";
    if ( !(value = homeZip()).isEmpty() )
	text += Qtopia::escapeString(value) + "<br>";
    if ( !(value = homeCountry()).isEmpty() )
	text += Qtopia::escapeString(value) + "<br>";

    // the others...
    QString str;
    str = emailList().join(", ");
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim", "Email Addresses: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = homePhone();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Home Phone: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = homeFax();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Home Fax: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = homeMobile();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Home Mobile: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = homeWebpage();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Home Web Page: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = businessWebpage();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Business Web Page: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = office();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Office: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = businessPhone();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Business Phone: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = businessFax();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Business Fax: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = businessMobile();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Business Mobile: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = businessPager();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Business Pager: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = profession();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Profession: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = assistant();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Assistant: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = manager();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Manager: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    str = gender();
    if ( !str.isEmpty() && str.toInt() != 0 ) {
	if ( str.toInt() == 1 )
	    str = qApp->translate( "QtopiaPim", "Male" );
	else if ( str.toInt() == 2 )
	    str = qApp->translate( "QtopiaPim", "Female" );
	text += "<b>" + qApp->translate( "QtopiaPim","Gender: ") + "</b>" + str + "<br>";
    }
    str = spouse();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Spouse: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    if ( birthday().isValid() ) {
	str = TimeString::localYMD( birthday() );
	if ( !str.isEmpty() )
	    text += "<b>" + qApp->translate( "QtopiaPim","Birthday: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    }
    if ( anniversary().isValid() ) {
	str = TimeString::localYMD( anniversary() );
	if ( !str.isEmpty() )
	    text += "<b>" + qApp->translate( "QtopiaPim","Anniversary: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";
    }
    str = nickname();
    if ( !str.isEmpty() )
	text += "<b>" + qApp->translate( "QtopiaPim","Nickname: ") + "</b>"
		+ Qtopia::escapeString(str) + "<br>";

    // notes last
    if ( (value = notes()) ) {
	text += "<br>" + Qtopia::escapeMultiLineString(value) + "<br>";
    }
    return text;
}

/*!
  \internal
*/
void PimContact::insert( int key, const QString &v )
{
    QString value = v.stripWhiteSpace();
    if ( value.isEmpty() )
	mMap.remove( key );
    else
	mMap.insert( key, value );
}

/*!
  \internal
*/
void PimContact::setField(int key,const QString &s)
{
    if (key < CommonFieldsEnd || key >= CustomFieldsStart) {
	PimRecord::setField(key,s);
    } else if (key > ContactFieldsEnd) {
	qDebug("unkown field");
    } else {
	QString fixedStr = s;

	//
	// Replace ';' and ',' with appropriate separator.
	//
	if ( key == Emails ){
	    if ( fixedStr.contains(";") )
		fixedStr.replace(QRegExp(";"), emailSeparator() );
	    else if ( fixedStr.contains(",") )
		fixedStr.replace(QRegExp(","), emailSeparator() );
	}

	mMap[key] = fixedStr;
    }
}

/*!
  \internal
*/
QString PimContact::field(int key) const
{
    if (key < CommonFieldsEnd || key >= CustomFieldsStart) {
	return PimRecord::field(key);
    } else if (key > ContactFieldsEnd) {
	qDebug("unkown field");
	return QString::null;
    } else {
	return mMap[key];
    }
}

QMap<int, QString> PimContact::fields() const
{
    QMap<int, QString> m = PimRecord::fields();

    if (!keyToIdentifierMapPtr)
	initMaps();
    QMap<int, QCString>::Iterator it;
    for (it = keyToIdentifierMapPtr->begin();
	    it != keyToIdentifierMapPtr->end(); ++it) {
	int i = it.key();
	QString str = field(i);
	if (!str.isEmpty())
	    m.insert(i, str);
    }

    return m;
}

/*!
  \internal
*/
void PimContact::replace( int key, const QString & v )
{
    QString value = v.stripWhiteSpace();
    if ( value.isEmpty() )
	mMap.remove( key );
    else
	mMap.replace( key, value );
}

/*!
  \internal
*/
QString PimContact::find( int key ) const
{
    return mMap[key];
}

/*!
  \internal
*/
QString PimContact::displayAddress( const QString &street,
				 const QString &city,
				 const QString &state,
				 const QString &zip,
				 const QString &country ) const
{
    QString s = street;
    if ( !street.isEmpty() )
	s+= "\n";
    s += city;
    if ( !city.isEmpty() && !state.isEmpty() )
	s += ", ";
    s += state;
    if ( !state.isEmpty() && !zip.isEmpty() )
	s += "  ";
    s += zip;
    if ( !country.isEmpty() && !s.isEmpty() )
	s += "\n";
    s += country;
    return s;
}

/*!
  Returns the formated business address of the contact.
*/
QString PimContact::displayBusinessAddress() const
{
    return displayAddress( businessStreet(), businessCity(),
			   businessState(), businessZip(),
			   businessCountry() );
}

/*!
  Returns the formated home address of the contact.
*/
QString PimContact::displayHomeAddress() const
{
    return displayAddress( homeStreet(), homeCity(),
			   homeState(), homeZip(),
			   homeCountry() );
}

/*!
  Sets the persons gender to the enum value specified in \a g.
*/
void PimContact::setGender( GenderType g )
{
    switch( g ) {
    case Male: replace( Gender, QString::number( 1 ) );	break;
    case Female: replace( Gender, QString::number( 2 ) ); break;
    case UnspecifiedGender:
    default:
	mMap.remove( Gender );
	break;
    }
}

/*! \enum PimContact::GenderType
  This enum describes the three possible choices for gender.

  \value UnspecifiedGender
  \value Male
  \value Female
*/

/*!
  Returns the gender of the contact as type GenderType.
*/
PimContact::GenderType PimContact::gender() const
{
    QString genderStr = find( Gender );

    switch( genderStr.toInt() ) {
    case 1: return Male;
    case 2: return Female;
    case 0:
    default:
	return UnspecifiedGender;
    }
}

/*!
  Sets the person's birthday to \a d.
*/
void PimContact::setBirthday( const QDate &d )
{
    replace( Birthday, PimXmlIO::dateToXml( d ) );
}

/*!
  Return the person's birthday. QDate will be null if there
  is birthday has not been set.
*/
QDate PimContact::birthday() const
{
    return PimXmlIO::xmlToDate( find( Birthday ) );
}

/*!
  Sets the person's anniversary to \a d.
*/
void PimContact::setAnniversary( const QDate &d )
{
    replace( Anniversary, PimXmlIO::dateToXml( d ) );
}

/*!
  Return the person's anniversary date.  QDate will be null if there
  is  anniversary has not been set.
*/
QDate PimContact::anniversary() const
{
    return PimXmlIO::xmlToDate( find( Anniversary ) );
}


/*!
  Returns the full name of the contact, generated from the
  current user preferences. fileAs() in general will be a cached
  vopy of this.

  \sa setFileAs()
*/
QString PimContact::fullName() const
{
    return fullName(*this);
}

static bool nameFormatCache = FALSE;
static QStringList formatList;

static void readNameFormatConfig()
{
    // NameFormat
    //   FirstName LastName
    //   LastName , _ FirstName
    //   LastName , _ FirstName | Company

    // read config for formatted name order.
    Config cfg("Contacts");
    cfg.setGroup("formatting");
    QStringList sl = cfg.readListEntry("NameFormat", ' ');
    if (sl.isEmpty()) {
	const QMap<int,QCString> k2i = PimContact::keyToIdentifierMap();

	formatList.append(k2i[PimContact::NameTitle]);
	formatList.append(k2i[PimContact::FirstName]);
	formatList.append(k2i[PimContact::MiddleName]);
	formatList.append(k2i[PimContact::LastName]);
	formatList.append(k2i[PimContact::Suffix]);
	formatList.append("|");
	formatList.append(k2i[PimContact::Company]);
    } else {
	formatList = sl;
    }
}

void qpe_setNameFormatCache(bool y)
{
    nameFormatCache = y;
    if ( y )
	readNameFormatConfig();
}

QString PimContact::fullName(const PimContact &cnt)
{
    if (!nameFormatCache)
	readNameFormatConfig();

    QStringList::Iterator it;
    QString name;
    const QMap<QCString, int> i2k = identifierToKeyMap();
    bool any = FALSE;
    bool prev = FALSE;
    for( it = formatList.begin(); it != formatList.end(); ++it ) {
	QCString cit(*it);
	if (i2k.contains(cit)) {
	    QString field = cnt.find(i2k[cit]);
	    if (!field.isEmpty()) {
		if ( prev )
		    name += " ";
		name += field;
		any = TRUE;
		prev = TRUE;
	    }
	} else if ( *it == "|" ) {
	    if ( any )
		break;
	    name = "";
	    prev = FALSE;
	} else if ( *it == "_" ) {
	    name += ' ';
	    prev = FALSE;
	} else {
	    if ( !name.isEmpty() ) {
		name += *it;
	    }
	    prev = FALSE;
	}
    }
    return name.simplifyWhiteSpace();
}

/*!
  Set the contact to be filed by the fullName().
*/
void PimContact::setFileAs()
{
    replace( FileAs, fullName() );
}

/*!
  Adds \a email to the list of email address for the contact.
  If no default email is set, the default email will
  be set to \a email.
*/
void PimContact::insertEmail( const QString &email )
{
    //qDebug("insertEmail %s", v.latin1());
    QString e = email.simplifyWhiteSpace();
    QString def = defaultEmail();

    // if no default, set it as the default email and don't insert
    if ( def.isEmpty() ) {
	setDefaultEmail( e ); // will insert into the list for us
	return;
    }

    // otherwise, insert assuming doesn't already exist
    QString emailsStr = find( Emails );
    if ( emailsStr.contains( e ))
	return;
    if ( !emailsStr.isEmpty() )
	emailsStr += emailSeparator();
    emailsStr += e;
    replace( Emails, emailsStr );
}

/*!
  Removes \a email from the list of email address for the contact.
  If the email is the default email, the default email will
  be set to the first one in the list.
*/
void PimContact::removeEmail( const QString &email )
{
    QString e = email.simplifyWhiteSpace();
    QString def = defaultEmail();
    QString emailsStr = find( Emails );
    QStringList emails = emailList();

    // otherwise, must first contain it
    if ( !emailsStr.contains( e ) )
	return;

    // remove it
    //qDebug(" removing email from list %s", e.latin1());
    emails.remove( e );
    // reset the string
    emailsStr = emails.join(emailSeparator()); // Sharp's brain dead separator
    replace( Emails, emailsStr );

    // if default, then replace the default email with the first one
    if ( def == e ) {
	//qDebug("removeEmail is default; setting new default");
	if ( !emails.count() )
	    clearEmailList();
	else // setDefaultEmail will remove e from the list
	    setDefaultEmail( emails.first() );
    }
}
/*!
  Clear the email list for the contact, including
  the default email.
*/
void PimContact::clearEmailList()
{
    mMap.remove( DefaultEmail );
    mMap.remove( Emails );
}

/*!
  Returns a list of email addresses belonging to the contact, including
  the default email address.
*/
QStringList PimContact::emailList() const
{
    QString emailStr = find( Emails );

    QStringList r;
    if ( !emailStr.isEmpty() ) {
	QStringList l = QStringList::split( emailSeparator(), emailStr );
	for ( QStringList::ConstIterator it = l.begin();it != l.end();++it )
	    r += (*it).simplifyWhiteSpace();
    }

    return r;
}

/*!
  Sets the default email to \a v and adds it to the list.
*/
void PimContact::setDefaultEmail( const QString &v )
{
    QString e = v.simplifyWhiteSpace();

    //qDebug("PimContact::setDefaultEmail %s", e.latin1());
    replace( DefaultEmail, e );

    if ( !e.isEmpty() )
	insertEmail( e );

}

/*!
  Sets the email list to \a emails
*/
void PimContact::setEmailList( const QStringList &emails )
{
    clearEmailList();
    for ( QStringList::ConstIterator it = emails.begin(); it != emails.end(); ++it )
	insertEmail( *it );
}

/*!
  Sets the children of the contact to \a children.
*/
void PimContact::setChildren( const QString &children )
{
    replace( Children, children );
}

// In pimrecord.cpp
void qpe_startVObjectInput();
void qpe_endVObjectInput();
void qpe_startVObjectOutput();
void qpe_setVObjectProperty(const QString&, const QString&, const char* type, PimRecord*);
void qpe_endVObjectOutput(VObject *,const char* type,const PimRecord*);
VObject *qpe_safeAddPropValue( VObject *o, const char *prop, const QString &value );
static inline VObject *safeAddPropValue( VObject *o, const char *prop, const QString &value )
{ return qpe_safeAddPropValue(o,prop,value); }
VObject *qpe_safeAddProp( VObject *o, const char *prop);
static inline VObject *safeAddProp( VObject *o, const char *prop)
{ return qpe_safeAddProp(o,prop); }


static void safeAddAddress( VObject* vcard, const char* prop, const QString& bs, const QString& bc,
    const QString& bst, const QString& bz, const QString& bco)
{
    // Not really needed now, since parse handles empty values
    if ( !!bs || !!bc || !!bst || !!bz || !!bco ) {
	VObject *adr= safeAddProp( vcard, VCAdrProp );
	safeAddProp( adr, prop );
	safeAddPropValue( adr, VCStreetAddressProp, bs );
	safeAddPropValue( adr, VCCityProp, bc );
	safeAddPropValue( adr, VCRegionProp, bst );
	safeAddPropValue( adr, VCPostalCodeProp, bz );
	safeAddPropValue( adr, VCCountryNameProp, bco );
    }
}

/*!
   \internal
*/
VObject *PimContact::createVObject( const PimContact &c )
{
    qpe_startVObjectOutput();

    VObject *vcard = newVObject( VCCardProp );
    safeAddPropValue( vcard, VCVersionProp, "2.1" );
    safeAddPropValue( vcard, VCLastRevisedProp, TimeConversion::toISO8601( QDateTime::currentDateTime() ) );
    safeAddPropValue( vcard, VCUniqueStringProp, QString::number(c.p_uid().data1) );

    // full name
    safeAddPropValue( vcard, VCFullNameProp, c.fileAs() );

    // name properties
    VObject *name = safeAddProp( vcard, VCNameProp );
    safeAddPropValue( name, VCFamilyNameProp, c.lastName() );
    safeAddPropValue( name, VCGivenNameProp, c.firstName() );
    safeAddPropValue( name, VCAdditionalNamesProp, c.middleName() );
    safeAddPropValue( name, VCNamePrefixesProp, c.nameTitle() );
    safeAddPropValue( name, VCNameSuffixesProp, c.suffix() );

    safeAddPropValue( vcard, VCPronunciationProp, c.firstNamePronunciation() );
    safeAddPropValue( vcard, "X-Qtopia-LNSOUND", c.lastNamePronunciation() );

    // home properties
    safeAddAddress( vcard, VCHomeProp, c.homeStreet(), c.homeCity(),
        c.homeState(), c.homeZip(), c.homeCountry() );

    VObject *home_phone = safeAddPropValue( vcard, VCTelephoneProp, c.homePhone() );
    safeAddProp( home_phone, VCHomeProp );
    home_phone = safeAddPropValue( vcard, VCTelephoneProp, c.homeMobile() );
    safeAddProp( home_phone, VCHomeProp );
    safeAddProp( home_phone, VCCellularProp );
    home_phone = safeAddPropValue( vcard, VCTelephoneProp, c.homeFax() );
    safeAddProp( home_phone, VCHomeProp );
    safeAddProp( home_phone, VCFaxProp );

    VObject *url = safeAddPropValue( vcard, VCURLProp, c.homeWebpage() );
    safeAddProp( url, VCHomeProp );

    // work properties
    safeAddAddress( vcard, VCWorkProp, c.businessStreet(), c.businessCity(),
        c.businessState(), c.businessZip(), c.businessCountry() );

    VObject *work_phone = safeAddPropValue( vcard, VCTelephoneProp, c.businessPhone() );
    safeAddProp( work_phone, VCWorkProp );
    work_phone = safeAddPropValue( vcard, VCTelephoneProp, c.businessMobile() );
    safeAddProp( work_phone, VCWorkProp );
    safeAddProp( work_phone, VCCellularProp );
    work_phone = safeAddPropValue( vcard, VCTelephoneProp, c.businessFax() );
    safeAddProp( work_phone, VCWorkProp );
    safeAddProp( work_phone, VCFaxProp );
    work_phone = safeAddPropValue( vcard, VCTelephoneProp, c.businessPager() );
    safeAddProp( work_phone, VCWorkProp );
    safeAddProp( work_phone, VCPagerProp );

    url = safeAddPropValue( vcard, VCURLProp, c.businessWebpage() );
    safeAddProp( url, VCWorkProp );

    VObject *title = safeAddPropValue( vcard, VCTitleProp, c.jobTitle() );
    safeAddProp( title, VCWorkProp );


    QStringList emails = c.emailList();
    //emails.prepend( c.defaultEmail() );
    for( QStringList::Iterator it = emails.begin(); it != emails.end(); ++it ) {
	VObject *email = safeAddPropValue( vcard, VCEmailAddressProp, *it );
	safeAddProp( email, VCInternetProp );
	if (*it == c.defaultEmail())
	    safeAddProp( email, VCPreferredProp );
    }

    safeAddPropValue( vcard, VCNoteProp, c.notes() );

    safeAddPropValue( vcard, VCBirthDateProp, PimXmlIO::dateToXml( c.birthday() ) );

    if ( !c.company().isEmpty() || !c.department().isEmpty() || !c.office().isEmpty() ) {
	VObject *org = safeAddProp( vcard, VCOrgProp );
	safeAddPropValue( org, VCOrgNameProp, c.company() );
	safeAddPropValue( org, VCOrgUnitProp, c.department() );
	safeAddPropValue( org, VCOrgUnit2Prop, c.office() );
    }
    safeAddPropValue( vcard, "X-Qtopia-CSOUND", c.companyPronunciation() );

    //photo
    QString pfn = c.customField("photofile");
    if ( !pfn.isEmpty() ) {
	const char* format = "JPEG";

#if 0 // load-and-save (allows non-JPEG images)
	QImage sp;
	if( !sp.load( pfn ) ) {
	    qWarning("PimContact::createVObject - Unable to load contact photo");
	} else {
	    QBuffer buffer;
	    buffer.open(IO_WriteOnly);
	    QImageIO iio(&buffer,format);
	    iio.setImage(sp);
	    QString prettyEncPhoto;
	    if ( iio.write() ) {
		QByteArray buf = buffer.buffer();
#else // Just use the file (avoid compounding JPEG lossiness)
	QFile f(pfn);
	if ( f.open(IO_ReadOnly) ) {
	    QByteArray buf = f.readAll();
	    if ( buf.size() ) {
#endif

		VObject *po = addPropSizedValue( vcard, VCPhotoProp, buf.data(), buf.size() );
		safeAddPropValue( po, "TYPE", format );
		// should really be done by vobject.cpp, since it decides to use base64
		safeAddPropValue( po, VCEncodingProp, "BASE64" );
	    }
	}
    }

    // some values we have to export as custom fields
    safeAddPropValue( vcard, "X-Qtopia-Profession", c.profession() );
    safeAddPropValue( vcard, "X-Qtopia-Manager", c.manager() );
    safeAddPropValue( vcard, "X-Qtopia-Assistant", c.assistant() );

    safeAddPropValue( vcard, "X-Qtopia-Spouse", c.spouse() );
    if ( c.gender() != PimContact::UnspecifiedGender )
	safeAddPropValue( vcard, "X-Qtopia-Gender", QString::number( (int)c.gender() ) );
    safeAddPropValue( vcard, "X-Qtopia-Anniversary", PimXmlIO::dateToXml( c.anniversary() ) );
    safeAddPropValue( vcard, "X-Qtopia-Nickname", c.nickname() );
    safeAddPropValue( vcard, "X-Qtopia-Children", c.children() );

    qpe_endVObjectOutput(vcard,"Address Book",&c); // No tr

    return vcard;
}

static PimContact parseVObject( VObject *obj )
{
    PimContact c;
//    int ds = 0;
//    char *photodata = 0;

    //  Make sure this is a codec loaded to deal with PalmOS' char format
    static CP1252TextCodec *palmCodec = 0;
    if( !palmCodec )
        palmCodec = new CP1252TextCodec();

    VObjectIterator it;
    initPropIterator( &it, obj );
    while( moreIteration( &it ) ) {
	VObject *o = nextVObject( &it );
	QCString name = vObjectName( o );

	// check this key/value for a CHARSET field.
	VObjectIterator tnit;
	initPropIterator( &tnit, o );
	QTextCodec *tc = 0;
	while( moreIteration( &tnit ) ) {
	    VObject *otc = nextVObject( &tnit );
	    if ( qstrcmp(vObjectName(otc), VCCharSetProp ) == 0) {
		tc = QTextCodec::codecForName(vObjectStringZValue(otc));
		break;
	    }
	}
	QString value;
	if (tc)
	    value = tc->toUnicode( vObjectStringZValue( o ) );
	else
	    value = vObjectStringZValue( o );

	if ( name == VCNameProp ) {
	    VObjectIterator nit;
	    initPropIterator( &nit, o );
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString name = vObjectTypeInfo( o );

		if (tc) {
		    value = tc->toUnicode( vObjectStringZValue( o ) );
		    qDebug("Orig %s, utf %s", vObjectStringZValue( o ), value.utf8().data());
		}
		else
		    value = vObjectStringZValue( o );
		//QString value = vObjectStringZValue( o );
		if ( name == VCNamePrefixesProp )
		    c.setNameTitle( value );
		else if ( name == VCNameSuffixesProp )
		    c.setSuffix( value );
		else if ( name == VCFamilyNameProp ) {
		    c.setLastName( value );
		} else if ( name == VCGivenNameProp ) {
		    c.setFirstName( value );
		} else if ( name == VCAdditionalNamesProp )
		    c.setMiddleName( value );
	    }
	}
	else if ( name == VCPronunciationProp )
	    c.setFirstNamePronunciation( value );
	else if ( name == "X-Qtopia-LNSOUND" )
	    c.setLastNamePronunciation( value );
	else if ( name == VCAdrProp ) {
	    bool work = TRUE; // default address is work address
	    QString street;
	    QString city;
	    QString region;
	    QString postal;
	    QString country;

	    VObjectIterator nit;
	    initPropIterator( &nit, o );
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString name = vObjectName( o );
		if (tc)
		    value = tc->toUnicode( vObjectStringZValue( o ) );
		else
		    value = vObjectStringZValue( o );
		//QString value = vObjectStringZValue( o );
		if ( name == VCHomeProp )
		    work = FALSE;
		else if ( name == VCWorkProp )
		    work = TRUE;
		else if ( name == VCStreetAddressProp )
		    street = value;
		else if ( name == VCCityProp )
		    city = value;
		else if ( name == VCRegionProp )
		    region = value;
		else if ( name == VCPostalCodeProp )
		    postal = value;
		else if ( name == VCCountryNameProp )
		    country = value;
	    }
	    if ( work ) {
		c.setBusinessStreet( street );
		c.setBusinessCity( city );
		c.setBusinessCountry( country );
		c.setBusinessZip( postal );
		c.setBusinessState( region );
	    } else {
		c.setHomeStreet( street );
		c.setHomeCity( city );
		c.setHomeCountry( country );
		c.setHomeZip( postal );
		c.setHomeState( region );
	    }
	}
	else if ( name == VCTelephoneProp ) {
	    enum {
		HOME = 0x01,
		WORK = 0x02,
		VOICE = 0x04,
		CELL = 0x08,
		FAX = 0x10,
		PAGER = 0x20,
		UNKNOWN = 0x80
	    };
	    int type = 0;

	    VObjectIterator nit;
	    initPropIterator( &nit, o );
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString name = vObjectTypeInfo( o );
		if ( name == VCHomeProp )
		    type |= HOME;
		else if ( name == VCWorkProp )
		    type |= WORK;
		else if ( name == VCVoiceProp )
		    type |= VOICE;
		else if ( name == VCCellularProp )
		    type |= CELL;
		else if ( name == VCFaxProp )
		    type |= FAX;
		else if ( name == VCPagerProp )
		    type |= PAGER;
		else if ( name == VCPreferredProp )
		    ;
		else if ( name == VCEncodingProp )
		    ;
		else
		    type |= UNKNOWN;
	    }
	    // evil if, indicates that if there was even one property we didn't know, don't store this value.
	    if ( (type & UNKNOWN) == UNKNOWN ) {
		qWarning("found unkown attribute in vobject, %s", (const char *)name);
	    }

	    if ( ( type & (HOME|WORK) ) == 0 ) // default
		type |= HOME;
	    if ( ( type & (VOICE|CELL|FAX|PAGER) ) == 0 ) // default
		type |= VOICE;

            if ( ( type & (CELL|HOME) ) == (CELL|HOME) )
		c.setHomeMobile( value );
            else if ( ( type & (CELL|WORK) ) == (CELL|WORK) )
		c.setBusinessMobile( value );
            else if ( (type & (VOICE|HOME) ) == (VOICE|HOME) )
		c.setHomePhone( value );
            else if ( ( type & (FAX|HOME) ) == (FAX|HOME) )
		c.setHomeFax( value );
            else if ( ( type & (VOICE|WORK) ) == (VOICE|WORK) )
		c.setBusinessPhone( value );
            else if ( ( type & (FAX|WORK) ) == (FAX|WORK) )
		c.setBusinessFax( value );
            else if ( ( type & (PAGER|WORK) ) == (PAGER|WORK) )
		c.setBusinessPager( value );
	}
	else if ( name == VCEmailAddressProp ) {
	    QString email;
	    if (tc)
		email = tc->toUnicode( vObjectStringZValue( o ) );
	    else
		email = vObjectStringZValue( o );
	    //QString email = vObjectStringZValue( o );
	    VObjectIterator nit;
	    initPropIterator( &nit, o );
	    bool isDefaultEmail = FALSE;
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString name = vObjectTypeInfo( o );
		if ( name == VCPreferredProp)
		    isDefaultEmail = TRUE;
	    }

	    if ( isDefaultEmail )
		c.setDefaultEmail( email );
	    else
		c.insertEmail( email );
	}
	else if ( name == VCURLProp ) {
	    VObjectIterator nit;
	    initPropIterator( &nit, o );
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString name = vObjectTypeInfo( o );
		if ( name == VCHomeProp )
		    c.setHomeWebpage( value );
		else if ( name == VCWorkProp )
		    c.setBusinessWebpage( value );
	    }
	}
	else if ( name == VCOrgProp ) {
	    VObjectIterator nit;
	    initPropIterator( &nit, o );
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString name = vObjectName( o );
		if (tc)
		    value = tc->toUnicode( vObjectStringZValue( o ) );
		else
		    value = vObjectStringZValue( o );
		if ( name == VCOrgNameProp ) {
		    c.setCompany( value );
		} else if ( name == VCOrgUnitProp )
		    c.setDepartment( value );
		else if ( name == VCOrgUnit2Prop )
		    c.setOffice( value );
	    }
	}
	//support for photos
	else if( name == VCPhotoProp )
	{
	    VObject *po;
	    VObjectIterator pit;
	    initPropIterator( &pit, o );
	    while( moreIteration( &pit ) )
	    {
		po = nextVObject( &pit );	

		QString pName( vObjectName( po ) );


		/* Don't care what type it is,
		   we're going to convert it to jpeg
		   in any case.
		if( pName == "TYPE" )
		    pmt = vObjectStringZValue( po );

		if( pName == VCDataSizeProp )
		    ds = vObjectIntegerValue( po );
		*/

		//TODO : Is quoted printable encoding ever used?
	    }

	    c.setCustomField( "phototmp", QString(vObjectStringZValue( o )) );
	    //actually save it in AddressBook::recieveFile when we know if it's a new contact 
	   // or a duplicate
	}
	else if ( name == "X-Qtopia-CSOUND" )
	    c.setCompanyPronunciation( value );
	else if ( name == VCTitleProp ) {
	    c.setJobTitle( value );
	}
	else if ( name == VCNoteProp ) {
	    c.setNotes( value );
	}
	else if (name == VCBirthDateProp ) {
	    c.setBirthday( PimXmlIO::xmlToDate( value)  );
	}
	else if ( name == "X-Qtopia-Profession" ) {
	    c.setProfession( value );
	}
	else if ( name == "X-Qtopia-Manager" ) {
	    c.setManager( value );
	}
	else if ( name == "X-Qtopia-Assistant" ) {
	    c.setAssistant( value );
	}
	else if ( name == "X-Qtopia-Spouse" ) {
	    c.setSpouse( value );
	}
	else if ( name == "X-Qtopia-Gender" ) {
	    c.setGender( (PimContact::GenderType) value.toInt() );
	}
	else if ( name == "X-Qtopia-Anniversary" ) {
	    c.setAnniversary( PimXmlIO::xmlToDate( value ) );
	}
	else if ( name == "X-Qtopia-Nickname" ) {
	    c.setNickname( value );
	}
	else if ( name == "X-Qtopia-Children" ) {
	    c.setChildren( value );
	} 
	else {
	    qpe_setVObjectProperty(name,value,"Address Book",&c); // No tr
	}


#if 0
	else {
	    printf("Name: %s, value=%s\n", name.data(), vObjectStringZValue( o ) );
	    VObjectIterator nit;
	    initPropIterator( &nit, o );
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString name = vObjectName( o );
		QString value = vObjectStringZValue( o );
		printf(" subprop: %s = %s\n", name.data(), value.latin1() );
	    }
	}
#endif
    }
    c.setFileAs();

    return c;
}


/*!
   Write the list of \a contacts as vCard objects to the file
   specified by \a filename.

   \sa readVCard()
*/
void PimContact::writeVCard( const QString &filename, const QValueList<PimContact> &contacts)
{
	QFileDirect f( filename.utf8().data() );
	if ( !f.open( IO_WriteOnly ) ) {
		qWarning("Unable to open vcard write");
		return;
	}

    QValueList<PimContact>::ConstIterator it;
    for( it = contacts.begin(); it != contacts.end(); ++it ) {
	VObject *obj = createVObject( *it );
	writeVObject(f.directHandle() , obj );
	cleanVObject( obj );
    }
    cleanStrTbl();
}

/*!
   Write the \a contact as a vCard object to the file
   specified by \a filename.

   \sa readVCard()
*/
void PimContact::writeVCard( const QString &filename, const PimContact &contact)
{
	QFileDirect f( filename.utf8().data() );
	if ( !f.open( IO_WriteOnly ) ) {
		qWarning("Unable to open vcard write");
		return;
	}

    VObject *obj = createVObject( contact );
	writeVObject( f.directHandle() , obj );
	cleanVObject( obj );

	cleanStrTbl();
}

/*!
  Reads the file specified by \a filename as a list of vCards objects
  and returns the list of near equivalent contacts.

  \sa writeVCard()
*/
QValueList<PimContact> PimContact::readVCard( const QString &filename )
{
    QValueList<PimContact> contacts;
    
    q_DontDecodeBase64Photo++; // don't decode the base64 encoded data automatically - we want the base64 encoded version
    contacts = readVCard( Parse_MIME_FromFileName( (char *)filename.utf8().data() ) );
    q_DontDecodeBase64Photo--;
    
    return contacts;
}

/*!
  Reads the given VCard data in \a vcard, and returns the list of
  near equivalent contacts.
 
  \sa writeVCard()
*/
QValueList<PimContact> PimContact::readVCard( const QByteArray &vcard )
{
    QValueList<PimContact> contacts;
    
    q_DontDecodeBase64Photo++;
    contacts = readVCard( Parse_MIME( (const char*)vcard, vcard.count() ) );
    q_DontDecodeBase64Photo--;
    
    return contacts;
}

/*!
  Reads the given VCard data in \a vobject, and returns the list of
  near equivalent contacts.
 
  \sa writeVCard()
*/
QValueList<PimContact> PimContact::readVCard( VObject* vobject )
{
    QValueList<PimContact> contacts;

    q_DontDecodeBase64Photo++;
    qpe_setNameFormatCache(TRUE);
    qpe_startVObjectInput();
    while ( vobject ) {
	contacts.append( parseVObject( vobject ) );

	VObject *t = vobject;
	vobject = nextVObjectInList(vobject);
	cleanVObject( t );
    }
    qpe_endVObjectInput();
    qpe_setNameFormatCache(FALSE);
    q_DontDecodeBase64Photo--;
  
    return contacts;
}

/*!
  Returns TRUE if the part of contact matches \a regexp. Otherwise returns FALSE.
*/
bool PimContact::match( const QString &regexp ) const
{
    return match(QRegExp(regexp));
}

/*!
  Returns TRUE if the part of contact matches \a r. Otherwise returns FALSE.
*/
bool PimContact::match( const QRegExp &r ) const
{
    bool match;
    match = false;
    QMap<int, QString>::ConstIterator it;
    for ( it = mMap.begin(); it != mMap.end(); ++it ) {
	if ( (*it).find( r ) > -1 ) {
	    match = true;
	    break;
	}
    }
    return match;
}

/*!
  Returns a suitable display string for the contact.

  \sa fileAs(), fullName()
*/
QString PimContact::bestLabel() const
{
    QString str = fileAs();
    // fileAs is _usually_ lastname, Firstname, but not always
    if (str.stripWhiteSpace().isEmpty()) {
	if (lastName().isEmpty() && firstName().isEmpty()) {
	    if (str.stripWhiteSpace().isEmpty()) {
		str = company();
		if (str.stripWhiteSpace().isEmpty()) {
		    str = defaultEmail();
		}
	    }
	} else {
	    // firstname or last name not empty.
	    if (lastName().isEmpty()) {
		// only first,
		return firstName();
	    } else {
		str = lastName();
		if (!firstName().isEmpty())
		    str += ", " + firstName();
	    }
	}
    }
    return str;
}

/*!
   Sets \a photo as the contact's image, storing both a full-sized copy of the image,
   and a thumbnail-sized copy.
   
   If the contact already has a photo (and / or a thumbnail), these files will
   be deleted.
*/
void PimContact::changePhoto( const QPixmap &photo, int thumbWidth, int thumbHeight )
{
    //  Check for existing contact photos, and delete them if necissary.
    QString photoFile = customField( "photofile" );
    QString thumbFile = customField( "thumbfile" );
    QString baseDir = Global::applicationFileName( "addressbook", "contactimages/" );
    
    QDir dir( baseDir );
    if( !dir.exists() )
        dir.mkdir( baseDir );
    
    if( !photoFile.isEmpty() ) {
        QFile pFile( baseDir + photoFile );
        if( pFile.exists() )
            pFile.remove();
    }
    
    if( !thumbFile.isEmpty() ) {
        QFile pFile( baseDir + thumbFile );
        if( pFile.exists() )
            pFile.remove();
    }
    
    if( !photo.isNull() ) {
        //  If either a thumnail, or a main photo is missing, find a new pair of filenames.
        if( thumbFile.isEmpty() || photoFile.isEmpty() ) {
            QString baseFileName = "ci-" + QString::number( time(0) ) + "-";
            int i = 0;
            QFile pFile;
            QFile tFile;
            
            do {
                photoFile = baseFileName + QString::number( i++ ) + ".jpg";
                thumbFile = baseFileName + QString::number( i++ ) + "-thumb.jpg";
                pFile.setName( baseDir + photoFile );
                tFile.setName( baseDir + thumbFile );
            } while( pFile.exists() || tFile.exists() );
        }
        
        //  Save the full-sized image.
        if( !photo.save( baseDir + photoFile, "JPEG", 50 ) ) 
            qWarning( QString( "PimContact::changePhoto - Unable to save contact "
                "image '%1'" ).arg( photoFile ) );
        else
            setCustomField( "photofile", photoFile );

        //  Create and save the thumbnail version
        QImage thumb = Image::sizeToPortrait( photo.convertToImage(), thumbWidth, thumbHeight );
        if( !thumb.save( baseDir + thumbFile, "JPEG", 50 ) )
            qWarning( QString( "PimContact::changePhoto - Unable to save contact "
                "thumbnail '%1'" ).arg( thumbFile ) );
        else
            setCustomField( "thumbfile", thumbFile );     
            
        return;   
    }
    
    removeCustomField( "photofile" );
    removeCustomField( "thumbfile" );
}

/*!
   Returns a QPixmap of the contact's photo, if there is one.
*/
QPixmap PimContact::photo() const
{
    QString photoFile = customField( "photofile" );
    if( photoFile.isEmpty() )
        photoFile = customField( "thumbfile" );
    
    if( !photoFile.isEmpty() ) {
        QPixmap photo;
        QString baseDir = Global::applicationFileName( "addressbook", "contactimages/" );
        
        if( photo.load( baseDir + photoFile ) )
            return photo;
        else
            qWarning( QString( "PimContact::photo - Unable to load contact "
                "image '%1'" ).arg( photoFile ) );
    }

    return QPixmap();
}

/*!
   Returns a QPixmap containing a thumbnail of the contacts photo, if there is one. The thumbnail
   will be sized to \a thumbWidth x \a thumbHeight, if it is not already at that size.
*/
QPixmap PimContact::thumbnail( int thumbWidth, int thumbHeight ) const
{
    QString thumbFile = customField( "thumbfile" );
    if( thumbFile.isEmpty() )
        thumbFile = customField( "photofile" );

    if( !thumbFile.isEmpty() ) {
        QPixmap thumb;
        QString baseDir = Global::applicationFileName( "addressbook", "contactimages/" );
        
        if( thumb.load( baseDir + thumbFile ) ) {
            if( thumb.width() != thumbWidth || thumb.height() != thumbHeight ) {
                thumb.convertFromImage( Image::sizeToPortrait( thumb.convertToImage(), 
                    thumbWidth, thumbHeight ) );
            }
            return thumb;
        }
        else
            qWarning( QString( "PimContact::thumbnail - Unable to load contact "
                "image thumbnail '%1'" ).arg( thumbFile ) );
    }

    return QPixmap();
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator>>( QDataStream &s, PimContact &c )
{
    s >> (PimRecord&)c;
    s >> c.mMap;
    return s;
}

QDataStream &operator<<( QDataStream &s, const PimContact &c )
{
    s << (const PimRecord&)c;
    s << c.mMap;
    return s;
}
#endif


static const QtopiaPimMapEntry addressbookentries[] = {
    // name
    { "Title", // No tr
	    QT_TRANSLATE_NOOP("PimContact",  "Title"), PimContact::NameTitle, 0 },
    { "FirstName", QT_TRANSLATE_NOOP("PimContact",  "First Name" ), PimContact::FirstName, 20 },
    { "MiddleName", QT_TRANSLATE_NOOP("PimContact",  "Middle Name" ), PimContact::MiddleName, 10 },
    { "LastName", QT_TRANSLATE_NOOP("PimContact",  "Last Name" ), PimContact::LastName, 60 },
    { "Suffix", QT_TRANSLATE_NOOP("PimContact",  "Suffix" ), PimContact::Suffix, 0 },
    { "FileAs", QT_TRANSLATE_NOOP("PimContact",  "File As" ), PimContact::FileAs, 0 },

    // email
    { "DefaultEmail", QT_TRANSLATE_NOOP("PimContact",  "Default Email" ), PimContact::DefaultEmail, 50 },
    { "Emails", QT_TRANSLATE_NOOP("PimContact",  "Emails" ), PimContact::Emails, 50 },

    // home
    { "HomeStreet", QT_TRANSLATE_NOOP("PimContact",  "Home Street" ), PimContact::HomeStreet, 10 },
    { "HomeCity", QT_TRANSLATE_NOOP("PimContact",  "Home City" ), PimContact::HomeCity, 0 },
    { "HomeState", QT_TRANSLATE_NOOP("PimContact",  "Home State" ), PimContact::HomeState, 0 },
    { "HomeZip", QT_TRANSLATE_NOOP("PimContact",  "Home Zip" ), PimContact::HomeZip, 0 },
    { "HomeCountry", QT_TRANSLATE_NOOP("PimContact",  "Home Country" ), PimContact::HomeCountry, 0 },
    { "HomePhone", QT_TRANSLATE_NOOP("PimContact",  "Home Phone" ), PimContact::HomePhone, 10 },
    { "HomeFax", QT_TRANSLATE_NOOP("PimContact",  "Home Fax" ), PimContact::HomeFax, 10 },
    { "HomeMobile", QT_TRANSLATE_NOOP("PimContact",  "Home Mobile" ), PimContact::HomeMobile, 30 },
    { "HomeWebPage", QT_TRANSLATE_NOOP("PimContact",  "Home Web Page" ), PimContact::HomeWebPage, 10 },

    // business
    { "Company", QT_TRANSLATE_NOOP("PimContact",  "Company" ), PimContact::Company, 10 },
    { "BusinessStreet", QT_TRANSLATE_NOOP("PimContact",  "Business Street" ), PimContact::BusinessStreet, 10 },
    { "BusinessCity", QT_TRANSLATE_NOOP("PimContact",  "Business City" ), PimContact::BusinessCity, 0 },
    { "BusinessState", QT_TRANSLATE_NOOP("PimContact",  "Business State" ), PimContact::BusinessState, 0 },
    { "BusinessZip", QT_TRANSLATE_NOOP("PimContact",  "Business Zip" ), PimContact::BusinessZip, 0 },
    { "BusinessCountry", QT_TRANSLATE_NOOP("PimContact",  "Business Country" ), PimContact::BusinessCountry, 0 },
    { "BusinessWebPage", QT_TRANSLATE_NOOP("PimContact",  "Business Web Page" ), PimContact::BusinessWebPage, 10 },
    { "JobTitle", QT_TRANSLATE_NOOP("PimContact",  "Job Title" ), PimContact::JobTitle, 10 },
    { "Department", QT_TRANSLATE_NOOP("PimContact",  "Department" ), PimContact::Department, 0 },
    { "Office", QT_TRANSLATE_NOOP("PimContact",  "Office" ), PimContact::Office, 0 },
    { "BusinessPhone", QT_TRANSLATE_NOOP("PimContact",  "Business Phone" ), PimContact::BusinessPhone, 10 },
    { "BusinessFax", QT_TRANSLATE_NOOP("PimContact",  "Business Fax" ), PimContact::BusinessFax, 10 },
    { "BusinessMobile", QT_TRANSLATE_NOOP("PimContact",  "Business Mobile" ), PimContact::BusinessMobile, 20 },
    { "BusinessPager", QT_TRANSLATE_NOOP("PimContact",  "Business Pager" ), PimContact::BusinessPager, 10 },
    { "Profession", QT_TRANSLATE_NOOP("PimContact",  "Profession" ), PimContact::Profession, 0 },
    { "Assistant", QT_TRANSLATE_NOOP("PimContact",  "Assistant" ), PimContact::Assistant, 0 },
    { "Manager", QT_TRANSLATE_NOOP("PimContact",  "Manager" ), PimContact::Manager, 0 },

    //personal
    { "Spouse", QT_TRANSLATE_NOOP("PimContact",  "Spouse" ), PimContact::Spouse, 10 },
    { "Gender", QT_TRANSLATE_NOOP("PimContact",  "Gender" ), PimContact::Gender, 0 },
    { "Birthday", QT_TRANSLATE_NOOP("PimContact",  "Birthday" ), PimContact::Birthday, 50 },
    { "Anniversary", QT_TRANSLATE_NOOP("PimContact",  "Anniversary" ), PimContact::Anniversary, 0 },
    { "Nickname", QT_TRANSLATE_NOOP("PimContact",  "Nickname" ), PimContact::Nickname, 0 },
    { "Children", QT_TRANSLATE_NOOP("PimContact",  "Children" ), PimContact::Children, 20 },

    // other
    { "Notes", QT_TRANSLATE_NOOP("PimContact",  "Notes" ), PimContact::Notes, 0 },

    // Added in Qtopia 1.6
    { "LastNamePronunciation", QT_TRANSLATE_NOOP("PimContact",  "  Pronunciation" ), PimContact::LastNamePronunciation, 0 },
    { "FirstNamePronunciation", QT_TRANSLATE_NOOP("PimContact",  "  Pronunciation" ), PimContact::FirstNamePronunciation, 0 },
    { "CompanyPronunciation", QT_TRANSLATE_NOOP("PimContact",  "  Pronunciation" ), PimContact::CompanyPronunciation, 0 },

    { 0, 0, 0, 0 }
};

void PimContact::initMaps()
{
    delete keyToIdentifierMapPtr;
    keyToIdentifierMapPtr = new QMap<int, QCString>;

    delete identifierToKeyMapPtr;
    identifierToKeyMapPtr = new QMap<QCString, int>;

    delete trFieldsMapPtr;
    trFieldsMapPtr = new QMap<int,QString>;

    delete uniquenessMapPtr;
    uniquenessMapPtr = new QMap<int, int>;

    PimRecord::initMaps("PimContact", addressbookentries, *uniquenessMapPtr, *identifierToKeyMapPtr, *keyToIdentifierMapPtr,
			*trFieldsMapPtr );
}

/*!
  \internal
*/
const QMap<int, QCString> &PimContact::keyToIdentifierMap()
{
    if ( !keyToIdentifierMapPtr )
	initMaps();
    return *keyToIdentifierMapPtr;
}

/*!
  \internal
*/
const QMap<QCString,int> &PimContact::identifierToKeyMap()
{
    if ( !identifierToKeyMapPtr )
	initMaps();
    return *identifierToKeyMapPtr;
}

/*!
  \internal
*/
const QMap<int, QString> & PimContact::trFieldsMap()
{
    if ( !trFieldsMapPtr )
	initMaps();
    return *trFieldsMapPtr;
}

/*!
  \internal
*/
const QMap<int,int> & PimContact::uniquenessMap()
{
    if ( !uniquenessMapPtr )
	initMaps();
    return *uniquenessMapPtr;
}

/*! \fn void PimContact::setNameTitle( const QString &str )
  Sets the title of the contact to \a str.
*/

/*! \fn void PimContact::setFirstName( const QString &str )
  Sets the first name of the contact to \a str.
*/

/*! \fn void PimContact::setMiddleName( const QString &str )
  Sets the middle name of the contact to \a str.
*/

/*! \fn void PimContact::setLastName( const QString &str )
  Sets the last name of the contact to \a str.
*/

/*! \fn void PimContact::setFirstNamePronunciation( const QString &str )
  Sets the pronunciation of the contacts first name to \a str.
*/

/*!
  \fn QString PimContact::firstNamePronunciation() const
  Returns the pronunciation of the contacts first name.
*/

/*! \fn void PimContact::setLastNamePronunciation( const QString &str )
  Sets the pronunciation of the contacts last name to \a str.
*/

/*!
  \fn QString PimContact::lastNamePronunciation() const
  Returns the pronunciation of the contacts last name.
*/

/*! \fn void PimContact::setCompanyPronunciation( const QString &str )
  Sets the pronunciation of the contacts company name to \a str.
*/

/*!
  \fn QString PimContact::companyPronunciation() const
  Returns the pronunciation of the contacts company name.
*/

/*! \fn void PimContact::setSuffix( const QString &str )
  Sets the suffix of the contact to \a str.
*/

/*! \fn void PimContact::setFileAs( const QString &str )
  Sets the contact to filed as \a str.
*/

/*! \fn void PimContact::setHomeStreet( const QString &str )
  Sets the home street address of the contact to \a str.
*/

/*! \fn void PimContact::setHomeCity( const QString &str )
  Sets the home city of the contact to \a str.
*/

/*! \fn void PimContact::setHomeState( const QString &str )
  Sets the home state of the contact to \a str.
*/

/*! \fn void PimContact::setHomeZip( const QString &str )
  Sets the home zip code of the contact to \a str.
*/

/*! \fn void PimContact::setHomeCountry( const QString &str )
  Sets the home country of the contact to \a str.
*/

/*! \fn void PimContact::setHomePhone( const QString &str )
  Sets the home phone number of the contact to \a str.
*/

/*! \fn void PimContact::setHomeFax( const QString &str )
  Sets the home fax number of the contact to \a str.
*/

/*! \fn void PimContact::setHomeMobile( const QString &str )
  Sets the home mobile phone number of the contact to \a str.
*/

/*! \fn void PimContact::setHomeWebpage( const QString &str )
  Sets the home webpage of the contact to \a str.
*/

/*! \fn void PimContact::setCompany( const QString &str )
  Sets the company for contact to \a str.
*/

/*! \fn void PimContact::setJobTitle( const QString &str )
  Sets the job title of the contact to \a str.
*/

/*! \fn void PimContact::setDepartment( const QString &str )
  Sets the department for contact to \a str.
*/

/*! \fn void PimContact::setOffice( const QString &str )
  Sets the office for contact to \a str.
*/

/*! \fn void PimContact::setBusinessStreet( const QString &str )
  Sets the business street address of the contact to \a str.
*/

/*! \fn void PimContact::setBusinessCity( const QString &str )
  Sets the business city of the contact to \a str.
*/

/*! \fn void PimContact::setBusinessState( const QString &str )
  Sets the business state of the contact to \a str.
*/

/*! \fn void PimContact::setBusinessZip( const QString &str )
  Sets the business zip code of the contact to \a str.
*/

/*! \fn void PimContact::setBusinessCountry( const QString &str )
  Sets the business country of the contact to \a str.
*/

/*! \fn void PimContact::setBusinessPhone( const QString &str )
  Sets the business phone number of the contact to \a str.
*/

/*! \fn void PimContact::setBusinessFax( const QString &str )
  Sets the business fax number of the contact to \a str.
*/

/*! \fn void PimContact::setBusinessMobile( const QString &str )
  Sets the business mobile phone number of the contact to \a str.
*/

/*! \fn void PimContact::setBusinessPager( const QString &str )
  Sets the business pager number of the contact to \a str.
*/

/*! \fn void PimContact::setBusinessWebpage( const QString &str )
  Sets the business webpage of the contact to \a str.
*/

/*! \fn void PimContact::setProfession( const QString &str )
  Sets the profession of the contact to \a str.
*/

/*! \fn void PimContact::setAssistant( const QString &str )
  Sets the assistant of the contact to \a str.
*/

/*! \fn void PimContact::setManager( const QString &str )
  Sets the manager of the contact to \a str.
*/

/*! \fn void PimContact::setSpouse( const QString &str )
  Sets the spouse of the contact to \a str.
*/

/*! \fn void PimContact::setNickname( const QString &str )
  Sets the nickname of the contact to \a str.
*/

/*! \fn void PimContact::setNotes( const QString &str )
  Sets the notes about the contact to \a str.
*/

/*! \fn QString PimContact::nameTitle() const
  Returns the title of the contact.
*/

/*! \fn QString PimContact::firstName() const
  Returns the first name of the contact.
*/

/*! \fn QString PimContact::middleName() const
  Returns the middle name of the contact.
*/

/*! \fn QString PimContact::lastName() const
  Returns the last name of the contact.
*/

/*! \fn QString PimContact::suffix() const
  Returns the suffix of the contact.
*/

/*! \fn QString PimContact::fileAs() const
  Returns the string the contact is filed as.
*/

/*! \fn QString PimContact::defaultEmail() const
  Returns the default email address of the contact.
*/

/*! \fn QString PimContact::homeStreet() const
  Returns the home street address of the contact.
*/

/*! \fn QString PimContact::homeCity() const
  Returns the home city of the contact.
*/

/*! \fn QString PimContact::homeState() const
  Returns the home state of the contact.
*/

/*! \fn QString PimContact::homeZip() const
  Returns the home zip of the contact.
*/

/*! \fn QString PimContact::homeCountry() const
  Returns the home country of the contact.
*/

/*! \fn QString PimContact::homePhone() const
  Returns the home phone number of the contact.
*/

/*! \fn QString PimContact::homeFax() const
  Returns the home fax number of the contact.
*/

/*! \fn QString PimContact::homeMobile() const
  Returns the home mobile number of the contact.
*/

/*! \fn QString PimContact::homeWebpage() const
  Returns the home webpage of the contact.
*/

/*! \fn QString PimContact::company() const
  Returns the company for the contact.
*/

/*! \fn QString PimContact::department() const
  Returns the department for the contact.
*/

/*! \fn QString PimContact::office() const
  Returns the office for the contact.
*/

/*! \fn QString PimContact::jobTitle() const
  Returns the job title of the contact.
*/

/*! \fn QString PimContact::profession() const
  Returns the profession of the contact.
*/

/*! \fn QString PimContact::assistant() const
  Returns the assistant of the contact.
*/

/*! \fn QString PimContact::manager() const
  Returns the manager of the contact.
*/

/*! \fn QString PimContact::businessStreet() const
  Returns the business street address of the contact.
*/

/*! \fn QString PimContact::businessCity() const
  Returns the business city of the contact.
*/

/*! \fn QString PimContact::businessState() const
  Returns the business state of the contact.
*/

/*! \fn QString PimContact::businessZip() const
  Returns the business zip of the contact.
*/

/*! \fn QString PimContact::businessCountry() const
  Returns the business country of the contact.
*/

/*! \fn QString PimContact::businessPhone() const
  Returns the business phone number of the contact.
*/

/*! \fn QString PimContact::businessFax() const
  Returns the business fax number of the contact.
*/

/*! \fn QString PimContact::businessMobile() const
  Returns the business mobile number of the contact.
*/

/*! \fn QString PimContact::businessPager() const
  Returns the business pager number of the contact.
*/

/*! \fn QString PimContact::businessWebpage() const
  Returns the business webpage of the contact.
*/

/*! \fn QString PimContact::spouse() const
  Returns the spouse of the contact.
*/

/*! \fn QString PimContact::nickname() const
  Returns the nickname of the contact.
*/

/*! \fn QString PimContact::children() const
  Returns the children of the contact.
*/

/*! \fn QString PimContact::notes() const
  Returns the notes relating to the the contact.
*/

/*!
  \fn void PimContact::p_setUid(QUuid)
  \internal
*/
