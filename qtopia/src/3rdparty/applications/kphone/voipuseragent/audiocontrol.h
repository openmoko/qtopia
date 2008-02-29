/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/**********************************************************************
** This file is part of the KPhone project.
** 
** KPhone is a SIP (Session Initiation Protocol) user agent for Linux, with
** which you can initiate VoIP (Voice over IP) connections over the Internet,
** send Instant Messages and subscribe your friends' presence information.
** Read INSTALL for installation instructions, and CHANGES for the latest
** additions in functionality and COPYING for the General Public License
** (GPL).
** 
** More information about Wirlab available at http://www.wirlab.net/
** 
** Note: "This program is released under the GPL with the additional
** exemption that compiling, linking, and/or using OpenSSL is allowed."
***********************************************************************/
#ifndef AUDIOCONTROL_H_INCLUDED
#define AUDIOCONTROL_H_INCLUDED
#include <qdialog.h>
#include <qbutton.h>
#include <qbuttongroup.h>

class QLineEdit;
class QRadioButton;
class CallAudio;

class AudioControl : public QDialog
{
	Q_OBJECT
public:
	AudioControl( QWidget *parent = 0, const char *name = 0 );
	~AudioControl( void );

protected slots:
	void ossButtonClicked( void );
	void alsaButtonClicked( void );
	void jackButtonClicked( void );
	void rwButtonClicked( void );
	void rowoButtonClicked( void );
	void pcmuButtonClicked( void );
	void pcmaButtonClicked( void );
	void gsmButtonClicked( void );
	void ilbcButtonClicked( void );
	void slotOk( void );
	void slotApply( void );
	void slotCancel( void );

private:
	enum audiomode { oss, alsa, jack };
	enum rdwr { rw, rowo };
	enum Codecs { pcmu, gsm, pcma, ilbc, ilbc_20, ilbc_30 };
	enum SizeOfPayload { size80, size160, size240 };
	enum Ringingtone { ringingtoneYes, ringingtoneNo };
	QButtonGroup *bg1;
	QButtonGroup *bg2;
	QButtonGroup *bg3;
	QButtonGroup *bg4;
	QRadioButton *pcmuButton;
	QRadioButton *pcmaButton;
	QRadioButton *gsmButton;
	QRadioButton *ilbcButton;
	QRadioButton *s80;
	QRadioButton *s160;
	QRadioButton *s240;
	CallAudio *a;
	QLineEdit *dspdev;
	QLineEdit *dspdev2;
	QButtonGroup *ringing;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
	QRadioButton *rwButton;
	QRadioButton *rowoButton;
	QRadioButton *ossButton;
	QRadioButton *alsaButton;
	QRadioButton *jackButton;
};

class VideoControl : public QDialog
{
	Q_OBJECT
public:
	VideoControl( QWidget *parent = 0, const char *name = 0 );
	~VideoControl( void );

protected slots:
	void slotOk( void );
	void slotApply( void );
	void slotCancel( void );

private:
	enum Codecs { h261, h263 };
	QButtonGroup *bg1;
	QLineEdit *videoSW;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
};

#endif // AUDIOCONTROL_H_INCLUDED
