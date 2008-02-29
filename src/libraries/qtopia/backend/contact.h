/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef __CONTACT_H__
#define __CONTACT_H__

#include <qtopia/private/palmtoprecord.h>
#include <qtopia/private/recordfields.h>

#include <qstringlist.h>

#if defined(QTOPIA_TEMPLATEDLL)
// MOC_SKIP_BEGIN
QTOPIA_TEMPLATEEXTERN template class QTOPIA_EXPORT QMap<int, QString>;
// MOC_SKIP_END
#endif

class ContactPrivate;
class QTOPIA_EXPORT Contact : public Qtopia::Record
{
    friend class DataSet;
public:
    Contact();
    Contact( const QMap<int, QString> &fromMap );
    virtual ~Contact();

    static void writeVCard( const QString &filename, const QValueList<Contact> &contacts);
    static void writeVCard( const QString &filename, const Contact &c );
    static QValueList<Contact> readVCard( const QString &filename );

    enum journal_action { ACTION_ADD, ACTION_REMOVE, ACTION_REPLACE };

    void setTitle( const QString &v ) { replace( Qtopia::Title, v ); }
    void setFirstName( const QString &v ) { replace( Qtopia::FirstName, v ); }
    void setMiddleName( const QString &v ) { replace( Qtopia::MiddleName, v ); }
    void setLastName( const QString &v ) { replace( Qtopia::LastName, v ); }
    void setSuffix( const QString &v ) { replace( Qtopia::Suffix, v ); }
    void setFileAs( const QString &v ) { replace( Qtopia::FileAs, v ); }
    void setFileAs();

    // default email address
    void setDefaultEmail( const QString &v );
    // inserts email to list and ensure's doesn't already exist
    void insertEmail( const QString &v );
    void removeEmail( const QString &v );
    void clearEmails();
    void insertEmails( const QStringList &v );

    // home
    void setHomeStreet( const QString &v ) { replace( Qtopia::HomeStreet, v ); }
    void setHomeCity( const QString &v ) { replace( Qtopia::HomeCity, v ); }
    void setHomeState( const QString &v ) { replace( Qtopia::HomeState, v ); }
    void setHomeZip( const QString &v ) { replace( Qtopia::HomeZip, v ); }
    void setHomeCountry( const QString &v ) { replace( Qtopia::HomeCountry, v ); }
    void setHomePhone( const QString &v ) { replace( Qtopia::HomePhone, v ); }
    void setHomeFax( const QString &v ) { replace( Qtopia::HomeFax, v ); }
    void setHomeMobile( const QString &v ) { replace( Qtopia::HomeMobile, v ); }
    void setHomeWebpage( const QString &v ) { replace( Qtopia::HomeWebPage, v ); }

    // business
    void setCompany( const QString &v ) { replace( Qtopia::Company, v ); }
    void setBusinessStreet( const QString &v ) { replace( Qtopia::BusinessStreet, v ); }
    void setBusinessCity( const QString &v ) { replace( Qtopia::BusinessCity, v ); }
    void setBusinessState( const QString &v ) { replace( Qtopia::BusinessState, v ); }
    void setBusinessZip( const QString &v ) { replace( Qtopia::BusinessZip, v ); }
    void setBusinessCountry( const QString &v ) { replace( Qtopia::BusinessCountry, v ); }
    void setBusinessWebpage( const QString &v ) { replace( Qtopia::BusinessWebPage, v ); }
    void setJobTitle( const QString &v ) { replace( Qtopia::JobTitle, v ); }
    void setDepartment( const QString &v ) { replace( Qtopia::Department, v ); }
    void setOffice( const QString &v ) { replace( Qtopia::Office, v ); }
    void setBusinessPhone( const QString &v ) { replace( Qtopia::BusinessPhone, v ); }
    void setBusinessFax( const QString &v ) { replace( Qtopia::BusinessFax, v ); }
    void setBusinessMobile( const QString &v ) { replace( Qtopia::BusinessMobile, v ); }
    void setBusinessPager( const QString &v ) { replace( Qtopia::BusinessPager, v ); }
    void setProfession( const QString &v ) { replace( Qtopia::Profession, v ); }
    void setAssistant( const QString &v ) { replace( Qtopia::Assistant, v ); }
    void setManager( const QString &v ) { replace( Qtopia::Manager, v ); }

    // personal
    void setSpouse( const QString &v ) { replace( Qtopia::Spouse, v ); }
    void setGender( const QString &v ) { replace( Qtopia::Gender, v ); }
    void setBirthday( const QString &v ) { replace( Qtopia::Birthday, v ); }
    void setAnniversary( const QString &v ) { replace( Qtopia::Anniversary, v ); }
    void setNickname( const QString &v ) { replace( Qtopia::Nickname, v ); }
    void setChildren( const QString &v );

    // other
    void setNotes( const QString &v ) { replace( Qtopia::Notes, v); }

    bool match( const QString &regexp ) const;

// DON'T ATTEMPT TO USE THIS
#ifdef QTOPIA_INTERNAL_CONTACT_MRE
    bool match( const QRegExp &regexp ) const;
#endif

//     // custom
//     void setCustomField( const QString &key, const QString &v )
//         { replace(Custom- + key, v ); }

    // name
    QString fullName() const;
    QString title() const { return find( Qtopia::Title ); }
    QString firstName() const { return find( Qtopia::FirstName ); }
    QString middleName() const { return find( Qtopia::MiddleName ); }
    QString lastName() const { return find( Qtopia::LastName ); }
    QString suffix() const { return find( Qtopia::Suffix ); }
    QString fileAs() const { return find( Qtopia::FileAs ); }

    // email
    QString defaultEmail() const { return find( Qtopia::DefaultEmail ); }
    QStringList emailList() const;

    // home
    QString homeStreet() const { return find( Qtopia::HomeStreet ); }
    QString homeCity() const { return find( Qtopia::HomeCity ); }
    QString homeState() const { return find( Qtopia::HomeState ); }
    QString homeZip() const { return find( Qtopia::HomeZip ); }
    QString homeCountry() const { return find( Qtopia::HomeCountry ); }
    QString homePhone() const { return find( Qtopia::HomePhone ); }
    QString homeFax() const { return find( Qtopia::HomeFax ); }
    QString homeMobile() const { return find( Qtopia::HomeMobile ); }
    QString homeWebpage() const { return find( Qtopia::HomeWebPage ); }
    /** Multi line string containing all non-empty address info in the form
    * Street
    * City, State Zip
    * Country
    */
    QString displayHomeAddress() const;

    // business
    QString company() const { return find( Qtopia::Company ); }
    QString businessStreet() const { return find( Qtopia::BusinessStreet ); }
    QString businessCity() const { return find( Qtopia::BusinessCity ); }
    QString businessState() const { return find( Qtopia::BusinessState ); }
    QString businessZip() const { return find( Qtopia::BusinessZip ); }
    QString businessCountry() const { return find( Qtopia::BusinessCountry ); }
    QString businessWebpage() const { return find( Qtopia::BusinessWebPage ); }
    QString jobTitle() const { return find( Qtopia::JobTitle ); }
    QString department() const { return find( Qtopia::Department ); }
    QString office() const { return find( Qtopia::Office ); }
    QString businessPhone() const { return find( Qtopia::BusinessPhone ); }
    QString businessFax() const { return find( Qtopia::BusinessFax ); }
    QString businessMobile() const { return find( Qtopia::BusinessMobile ); }
    QString businessPager() const { return find( Qtopia::BusinessPager ); }
    QString profession() const { return find( Qtopia::Profession ); }
    QString assistant() const { return find( Qtopia::Assistant ); }
    QString manager() const { return find( Qtopia::Manager ); }
     /** Multi line string containing all non-empty address info in the form
    * Street
    * City, State Zip
    * Country
    */
    QString displayBusinessAddress() const;

    //personal
    QString spouse() const { return find( Qtopia::Spouse ); }
    QString gender() const { return find( Qtopia::Gender ); }
    QString birthday() const { return find( Qtopia::Birthday ); }
    QString anniversary() const { return find( Qtopia::Anniversary ); }
    QString nickname() const { return find( Qtopia::Nickname ); }
    QString children() const { return find( Qtopia::Children ); }
    QStringList childrenList() const;

    // other
    QString notes() const { return find( Qtopia::Notes ); }
    QString groups() const { return find( Qtopia::Groups ); }
    QStringList groupList() const;

//     // custom
//     const QString &customField( const QString &key )
//         { return find( Custom- + key ); }

    static QStringList fields();
    static QStringList trfields();

    QString toRichText() const;
    QMap<int, QString> toMap() const;
    QString field( int key ) const { return find( key ); }


    // journaling...
    void saveJournal( journal_action action, const QString &key = QString::null );
    void save( QString &buf ) const;

    void setUid( int i )
{ Record::setUid(i); replace( Qtopia::AddressUid , QString::number(i)); }

private:
    friend class AbEditor;
    friend class AbTable;
    friend class AddressBookAccessPrivate;
    friend class XMLIO;

    QString emailSeparator() const { return " "; }
     // the emails should be seperated by a comma
    void setEmails( const QString &v );
    QString emails() const { return find( Qtopia::Emails ); }

    void insert( int key, const QString &value );
    void replace( int key, const QString &value );
    QString find( int key ) const;

    QString displayAddress( const QString &street,
			    const QString &city,
			    const QString &state,
			    const QString &zip,
			    const QString &country ) const;

    Qtopia::UidGen &uidGen() { return sUidGen; }
    static Qtopia::UidGen sUidGen;
    QMap<int, QString> mMap;
    ContactPrivate *d;
};

// these methods are inlined to keep binary compatability with Qtopia 1.5
inline void Contact::insertEmail( const QString &v )
{
    //qDebug("insertEmail %s", v.latin1());
    QString e = v.simplifyWhiteSpace();
    QString def = defaultEmail();

    // if no default, set it as the default email and don't insert
    if ( def.isEmpty() ) {
	setDefaultEmail( e ); // will insert into the list for us
	return;
    }

    // otherwise, insert assuming doesn't already exist
    QString emailsStr = find( Qtopia::Emails );
    if ( emailsStr.contains( e ))
	return;
    if ( !emailsStr.isEmpty() )
	emailsStr += emailSeparator();
    emailsStr += e;
    replace( Qtopia::Emails, emailsStr );
}

inline void Contact::removeEmail( const QString &v )
{
    QString e = v.simplifyWhiteSpace();
    QString def = defaultEmail();
    QString emailsStr = find( Qtopia::Emails );
    QStringList emails = emailList();
	
    // otherwise, must first contain it
    if ( !emailsStr.contains( e ) )
	return;

    // remove it
    //qDebug(" removing email from list %s", e.latin1());
    emails.remove( e );
    // reset the string
    emailsStr = emails.join(emailSeparator()); // Sharp's brain dead separator
    replace( Qtopia::Emails, emailsStr );

    // if default, then replace the default email with the first one
    if ( def == e ) {
	//qDebug("removeEmail is default; setting new default");
	if ( !emails.count() )
	    clearEmails();
	else // setDefaultEmail will remove e from the list
	    setDefaultEmail( emails.first() );
    }
}
inline void Contact::clearEmails()
{
    mMap.remove( Qtopia::DefaultEmail );
    mMap.remove( Qtopia::Emails );
}
inline void Contact::setDefaultEmail( const QString &v )
{
    QString e = v.simplifyWhiteSpace();

    //qDebug("Contact::setDefaultEmail %s", e.latin1());
    replace( Qtopia::DefaultEmail, e );

    if ( !e.isEmpty() )	
	insertEmail( e );
	
}

inline void Contact::insertEmails( const QStringList &v )
{
    for ( QStringList::ConstIterator it = v.begin(); it != v.end(); ++it )
	insertEmail( *it );
}

#endif
