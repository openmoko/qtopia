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
#ifndef QPC_RECORD_FIELDS_H
#define QPC_RECORD_FIELDS_H

#include <qtopia/qpeglobal.h>

// dataset = "addressbook"
namespace Qtopia
{
    static const int UID_ID = 0;
    static const int CATEGORY_ID = 1;

    enum AddressBookFields {
	AddressUid = UID_ID,	
	AddressCategory = CATEGORY_ID,

	// NOTE: Order of fields dependency in backend/contact.cpp

	Title,
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

	// used for internal record keeping, not for end user.
	Groups,
	rid,
	rinfo,

	// maps to Furigana, or is at least intended to.
	Pronunciation
    };

    // dataset = "todolist"
    enum TaskFields {
	TaskUid = UID_ID,
	TaskCategory = CATEGORY_ID,
	
	HasDate,
	Completed,
	TaskDescription,
	Priority,
	Date,

	TaskRid,
	TaskRinfo
    };

    // dataset = "categories" for todos
    enum CategoryFields {
	CatUid = UID_ID,
	CatName,
	CatAppGroup
    };


// dataset = "datebook"
    enum DatebookFields {
	DatebookUid = UID_ID,
	DatebookCategory = CATEGORY_ID,
	
	DatebookDescription,
	Location,
	TimeZone,
	Note,
	StartDateTime,
	EndDateTime,
	DatebookType,
	HasAlarm,
	SoundType,
	AlarmTime,

	RepeatPatternType,
	RepeatPatternFrequency,
	RepeatPatternPosition,
	RepeatPatternDays,
	RepeatPatternHasEndDate,
	RepeatPatternEndDate,	

	DateBookRid,
	DateBookRinfo
    };
};


#endif
