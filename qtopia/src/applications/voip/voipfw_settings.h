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
#ifndef VOIPSETTINGSSTROAGE_H
#define  VOIPSETTINGSSTROAGE_H

#include "voipuseragentstructures.h"
#include <qtopia/global.h>

enum VoIPReturnCode_t{
VOIP_SUCCESS=0,
VOIP_FAILURE
};

class VoIPSettingsStorage
{
public:
	VoIPSettingsStorage(QString persistantStorageFilename);

	VoIPReturnCode_t getSipIdentity(SipIdentity_t *Identity);
	VoIPReturnCode_t setSipIdentity(SipIdentity_t Identity);
	VoIPReturnCode_t getSipPreferences(SipPreferences_t *SipPreferences);
	VoIPReturnCode_t setSipPreferences(SipPreferences_t SipPreferences);
	VoIPReturnCode_t getAudioPreferences(AudioPreferences_t *AudioPreferences);
	VoIPReturnCode_t setAudioPreferences(AudioPreferences_t AudioPreferences);
	VoIPReturnCode_t setListOfCodecs(QValueList <CodecInformation_t> codecs);
	VoIPReturnCode_t getListOfCodecs(QValueList <CodecInformation_t> *codecs);	


	
private:
	SipIdentity_t m_Identity;
	SipPreferences_t  m_SipPreferences;
	AudioPreferences_t m_AudioPreferences;
	QValueList <CodecInformation_t> m_codecs;
	QString m_filename;

};

#endif
