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

#ifndef PIM_CONTACT_H
#define PIM_CONTACT_H

#include <qtopia/pim/pimrecord.h>

#include <qstringlist.h>
#include <qmap.h>
#include <qpixmap.h>

struct VObject;
class AddressBookAccessPrivate;
class PimContactPrivate;
class QDate;
class QTOPIAPIM_EXPORT PimContact : public PimRecord
{
    friend class AddressBookAccessPrivate;

public:
    enum ContactFields {
	NameTitle = CommonFieldsEnd,
	FirstName,
	MiddleName,
	LastName,
	Suffix,
	FileAs,

	JobTitle,
	Department,
	Company,
	BusinessPhone,
	BusinessFax,
	BusinessMobile,

	// email
	DefaultEmail,
	Emails,

	HomePhone,
	HomeFax,
	HomeMobile,

	// business
	BusinessStreet,
	BusinessCity,
	BusinessState,
	BusinessZip,
	BusinessCountry,
	BusinessPager,
	BusinessWebPage,

	Office,
	Profession,
	Assistant,
	Manager,

	// home
	HomeStreet,
	HomeCity,
	HomeState,
	HomeZip,
	HomeCountry,
	HomeWebPage,

	//personal
	Spouse,
	Gender,
	Birthday,
	Anniversary,
	Nickname,
	Children,

	// other
	Notes,

	// maps to YomiLastName, or is at least intended to  
	LastNamePronunciation,
	FirstNamePronunciation,
	CompanyPronunciation,

	// DisplayFieldsCount should always be last and is not supposed to 
	// be relied on.  If you need to iteratate, get a map.
	ContactFieldsEnd = 100
    };

    PimContact();

    virtual ~PimContact();
    
    void fromMap( const QMap<int, QString> &m );

    static void writeVCard( const QString &filename, const QValueList<PimContact> &contacts);
    static void writeVCard( const QString &filename, const PimContact &c );
    static QValueList<PimContact> readVCard( const QString &filename );
    static QValueList<PimContact> readVCard( const QByteArray &vcard );
    static QValueList<PimContact> readVCard( VObject* vobject );

    void setNameTitle( const QString &v ) { replace( NameTitle, v ); }
    void setFirstName( const QString &v ) { replace( FirstName, v ); }
    void setMiddleName( const QString &v ) { replace( MiddleName, v ); }
    void setLastName( const QString &v ) { replace( LastName, v ); }
    void setSuffix( const QString &v ) { replace( Suffix, v ); }
    void setFileAs( const QString &v ) { replace( FileAs, v ); }
    void setFileAs();
    void setFirstNamePronunciation( const QString &v ) { replace( FirstNamePronunciation, v ); }
    void setLastNamePronunciation( const QString &v ) { replace( LastNamePronunciation, v ); }
    void setCompanyPronunciation( const QString &v ) { replace( CompanyPronunciation, v ); }

    // default email address
    void setDefaultEmail( const QString &v );
    // inserts email to list and ensure's doesn't already exist
    void insertEmail( const QString &v );
    void removeEmail( const QString &v );
    void clearEmailList();
    void setEmailList( const QStringList &v );

    // home
    void setHomeStreet( const QString &v ) { replace( HomeStreet, v ); }
    void setHomeCity( const QString &v ) { replace( HomeCity, v ); }
    void setHomeState( const QString &v ) { replace( HomeState, v ); }
    void setHomeZip( const QString &v ) { replace( HomeZip, v ); }
    void setHomeCountry( const QString &v ) { replace( HomeCountry, v ); }
    void setHomePhone( const QString &v ) { replace( HomePhone, v ); }
    void setHomeFax( const QString &v ) { replace( HomeFax, v ); }
    void setHomeMobile( const QString &v ) { replace( HomeMobile, v ); }
    void setHomeWebpage( const QString &v ) { replace( HomeWebPage, v ); }

    // business
    void setCompany( const QString &v ) { replace( Company, v ); }
    void setBusinessStreet( const QString &v ) { replace( BusinessStreet, v ); }
    void setBusinessCity( const QString &v ) { replace( BusinessCity, v ); }
    void setBusinessState( const QString &v ) { replace( BusinessState, v ); }
    void setBusinessZip( const QString &v ) { replace( BusinessZip, v ); }
    void setBusinessCountry( const QString &v ) { replace( BusinessCountry, v ); }
    void setBusinessWebpage( const QString &v ) { replace( BusinessWebPage, v ); }
    void setJobTitle( const QString &v ) { replace( JobTitle, v ); }
    void setDepartment( const QString &v ) { replace( Department, v ); }
    void setOffice( const QString &v ) { replace( Office, v ); }
    void setBusinessPhone( const QString &v ) { replace( BusinessPhone, v ); }
    void setBusinessFax( const QString &v ) { replace( BusinessFax, v ); }
    void setBusinessMobile( const QString &v ) { replace( BusinessMobile, v ); }
    void setBusinessPager( const QString &v ) { replace( BusinessPager, v ); }
    void setProfession( const QString &v ) { replace( Profession, v ); }
    void setAssistant( const QString &v ) { replace( Assistant, v ); }
    void setManager( const QString &v ) { replace( Manager, v ); }

    // personal
    void setSpouse( const QString &v ) { replace( Spouse, v ); }
    enum GenderType { UnspecifiedGender=0, Male, Female };
    void setGender( GenderType g );
    void setBirthday( const QDate &d );
    void setAnniversary( const QDate &v );
    void setNickname( const QString &v ) { replace( Nickname, v ); }
    void setChildren( const QString &v );

    // other
    void setNotes( const QString &v ) { replace( Notes, v); }
    
    void changePhoto( const QPixmap &photo, int thumbWidth, int thumbHeight );
    QPixmap photo() const;
    QPixmap thumbnail( int thumbWidth, int thumbHeight ) const;
    
    bool match( const QString &regexp ) const;

// DON'T ATTEMPT TO USE THIS
#ifdef QTOPIA_INTERNAL_CONTACT_MRE
    bool match( const QRegExp &regexp ) const;
#endif

    // name
    QString fullName() const;
    QString nameTitle() const { return find( NameTitle ); }
    QString firstName() const { return find( FirstName ); }
    QString middleName() const { return find( MiddleName ); }
    QString lastName() const { return find( LastName ); }
    QString suffix() const { return find( Suffix ); }
    QString fileAs() const { return find( FileAs ); }
    QString lastNamePronunciation() const { return find( LastNamePronunciation ); }
    QString firstNamePronunciation() const { return find( FirstNamePronunciation ); }
    QString companyPronunciation() const { return find( CompanyPronunciation ); }

    // helper function.  Attempts to find a string to use
    // as a one line representation for the contact.
    QString bestLabel() const;

    // email
    QString defaultEmail() const { return find( DefaultEmail ); }
    QStringList emailList() const;

    // home
    QString homeStreet() const { return find( HomeStreet ); }
    QString homeCity() const { return find( HomeCity ); }
    QString homeState() const { return find( HomeState ); }
    QString homeZip() const { return find( HomeZip ); }
    QString homeCountry() const { return find( HomeCountry ); }
    QString homePhone() const { return find( HomePhone ); }
    QString homeFax() const { return find( HomeFax ); }
    QString homeMobile() const { return find( HomeMobile ); }
    QString homeWebpage() const { return find( HomeWebPage ); }
    /** Multi line string containing all non-empty address info in the form
    * Street
    * City, State Zip
    * Country
    */
    QString displayHomeAddress() const;

    // business
    QString company() const { return find( Company ); }
    QString businessStreet() const { return find( BusinessStreet ); }
    QString businessCity() const { return find( BusinessCity ); }
    QString businessState() const { return find( BusinessState ); }
    QString businessZip() const { return find( BusinessZip ); }
    QString businessCountry() const { return find( BusinessCountry ); }
    QString businessWebpage() const { return find( BusinessWebPage ); }
    QString jobTitle() const { return find( JobTitle ); }
    QString department() const { return find( Department ); }
    QString office() const { return find( Office ); }
    QString businessPhone() const { return find( BusinessPhone ); }
    QString businessFax() const { return find( BusinessFax ); }
    QString businessMobile() const { return find( BusinessMobile ); }
    QString businessPager() const { return find( BusinessPager ); }
    QString profession() const { return find( Profession ); }
    QString assistant() const { return find( Assistant ); }
    QString manager() const { return find( Manager ); }
     /** Multi line string containing all non-empty address info in the form
    * Street
    * City, State Zip
    * Country
    */
    QString displayBusinessAddress() const;

    //personal
    QString spouse() const { return find( Spouse ); }
    GenderType gender() const;
    QDate birthday() const;
    QDate anniversary() const;
    QString nickname() const { return find( Nickname ); }
    QString children() const { return find( Children ); }

    // other
    QString notes() const { return find( Notes ); }
    //QString groups() const { return find( Groups ); }
    //QStringList groupList() const;

    static const QMap<int, QCString> &keyToIdentifierMap();
    static const QMap<QCString,int> &identifierToKeyMap();
    static const QMap<int, QString> & trFieldsMap();
    // needed for Qtopia Desktop synchronization
    static const QMap<int,int> &uniquenessMap();

    QString toRichText() const;

    virtual void setField(int,const QString &);
    virtual QString field(int) const;
    virtual QMap<int,QString> fields() const;

#ifndef QT_NO_DATASTREAM
    friend QTOPIAPIM_EXPORT QDataStream &operator>>( QDataStream &, PimContact & );
    friend QTOPIAPIM_EXPORT QDataStream &operator<<( QDataStream &, const PimContact & );
#endif

protected:
    //virtual int endFieldMarker() const {return ContactFieldCount; }

    static VObject *createVObject( const PimContact &c );
    void p_setUid( QUuid i )
{ PimRecord::p_setUid(i); replace( UID_ID , i.toString()); }

    void insert( int key, const QString &value );
    void replace( int key, const QString &value );
    QString find( int key ) const;

    QString displayAddress( const QString &street,
			    const QString &city,
			    const QString &state,
			    const QString &zip,
			    const QString &country ) const;

    QMap<int, QString> mMap;

private:
    static QString fullName(const PimContact &);
    static void initMaps();

    PimContactPrivate *d;
};

#define QTOPIAPIM_DEFINE_CONTACT
#include <qtopia/pim/qtopiapimwinexport.h>


#ifndef QT_NO_DATASTREAM
QTOPIAPIM_EXPORT QDataStream &operator>>( QDataStream &, PimContact & );
QTOPIAPIM_EXPORT QDataStream &operator<<( QDataStream &, const PimContact & );
#endif

#endif
