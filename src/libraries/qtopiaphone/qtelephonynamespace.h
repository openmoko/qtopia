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

#ifndef QTELEPHONYNAMESPACE_H
#define QTELEPHONYNAMESPACE_H

#include <qtopiaipcmarshal.h>

class QTOPIAPHONE_EXPORT QTelephony : public QObject
{
    Q_OBJECT
    Q_ENUMS(RegistrationState OperatorMode OperatorAvailability)
    Q_ENUMS(CallClass Result SimFileType SimFileError)
private:
    QTelephony() {}
    ~QTelephony() {}

public:
    enum RegistrationState
    {
        RegistrationNone,
        RegistrationHome,
        RegistrationSearching,
        RegistrationDenied,
        RegistrationUnknown,
        RegistrationRoaming
    };

    enum OperatorMode
    {
        OperatorModeAutomatic,
        OperatorModeManual,
        OperatorModeDeregister,
        OperatorModeManualAutomatic
    };

    enum OperatorAvailability
    {
        OperatorUnavailable,
        OperatorAvailable,
        OperatorCurrent,
        OperatorForbidden
    };

    enum CallClass
    {
        CallClassNone                   = 0,
        CallClassVoice                  = 1,
        CallClassData                   = 2,
        CallClassFax                    = 4,
        CallClassSMS                    = 8,
        CallClassDataCircuitSync        = 16,
        CallClassDataCircuitAsync       = 32,
        CallClassDedicatedPacketAccess  = 64,
        CallClassDedicatedPADAccess     = 128,
        CallClassDefault                = 7
    };

    enum Result
    {
        OK                          = -1,
        Connect                     = -2,
        NoCarrier                   = -3,
        Error                       = -4,
        NoDialtone                  = -5,
        Busy                        = -6,
        NoAnswer                    = -7,
        Dead                        = -8,

        // General errors (GSM 27.07, section 9.2.1).
        PhoneFailure                = 0,
        NoConnectionToPhone         = 1,
        PhoneAdapterLinkReserved    = 2,
        OperationNotAllowed         = 3,
        OperationNotSupported       = 4,
        PhSimPinRequired            = 5,
        PhFSimPinRequired           = 6,
        PhFSimPukRequired           = 7,
        SimNotInserted              = 10,
        SimPinRequired              = 11,
        SimPukRequired              = 12,
        SimFailure                  = 13,
        SimBusy                     = 14,
        SimWrong                    = 15,
        IncorrectPassword           = 16,
        SimPin2Required             = 17,
        SimPuk2Required             = 18,
        MemoryFull                  = 20,
        InvalidIndex                = 21,
        NotFound                    = 22,
        MemoryFailure               = 23,
        TextStringTooLong           = 24,
        InvalidCharsInTextString    = 25,
        DialStringTooLong           = 26,
        InvalidCharsInDialString    = 27,
        NoNetworkService            = 30,
        NetworkTimeout              = 31,
        NetworkNotAllowed           = 32,
        NetPersPinRequired          = 40,
        NetPersPukRequired          = 41,
        NetSubsetPersPinRequired    = 42,
        NetSubsetPersPukRequired    = 43,
        ServProvPersPinRequired     = 44,
        ServProvPersPukRequired     = 45,
        CorpPersPinRequired         = 46,
        CorpPersPukRequired         = 47,   // 23 according to spec ???
        HiddenKeyRequired           = 48,   // 24 according to spec ???
        Unknown                     = 100,

        // GPRS-related errors (GSM 27.07, section 9.2.2).
        IllegalMS                   = 103,
        IllegalME                   = 106,
        GPRSServicesNotAllowed      = 107,
        PLMNNotAllowed              = 111,
        LocationAreaNotAllowed      = 112,
        RoamingNotAllowed           = 113,
        ServiceOptionNotSupported   = 132,
        ServiceOptionNotSubscribed  = 133,
        ServiceOptionOutOfOrder     = 134,
        UnspecifiedGPRSError        = 148,
        PDPAuthenticationFailure    = 149,
        InvalidMobileClass          = 150,

        // VBS/VGCS and eMLPP errors (GSM 27.07, section 9.2.3).
        VBSVGCSNotSupported         = 151,
        NoServiceSubscriptionOnSim  = 152,
        NoSubscriptionForGroupId    = 153,
        GroupIdNotActivatedOnSim    = 154,
        NoMatchingNotification      = 155,
        VBSVGCSCallAlreadyPresent   = 156,
        Congestion                  = 157,
        NetworkFailure              = 158,
        UplinkBusy                  = 159,
        NoAccessRightsForSimFile    = 160,
        NoSubscriptionForPriority   = 161,
        OperationNotApplicable      = 162,

        // SMS errors (GSM 27.05, section 3.2.5).
        MEFailure                   = 300,
        SMSServiceOfMEReserved      = 301,
        SMSOperationNotAllowed      = 302,
        SMSOperationNotSupported    = 303,
        InvalidPDUModeParameter     = 304,
        InvalidTextModeParameter    = 305,
        USimNotInserted             = 310,
        USimPinRequired             = 311,
        PHUSimPinRequired           = 312,
        USimFailure                 = 313,
        USimBusy                    = 314,
        USimWrong                   = 315,
        USimPukRequired             = 316,
        USimPin2Required            = 317,
        USimPuk2Required            = 318,
        SMSMemoryFailure            = 320,
        InvalidMemoryIndex          = 321,
        SMSMemoryFull               = 322,
        SMSCAddressUnknown          = 330,
        SMSNoNetworkService         = 331,
        SMSNetworkTimeout           = 332,
        NoCNMAAckExpected           = 340,
        UnknownError                = 500
    };

    enum SimFileType
    {
        SimFileRootDirectory,
        SimFileDirectory,
        SimFileTransparent,
        SimFileLinearFixed,
        SimFileCyclic,
        SimFileUnknown,
    };

    enum SimFileError
    {
        SimFileServiceUnavailable,
        SimFileSimUnavailable,
        SimFileNotFound,
        SimFileInvalidRead,
        SimFileInvalidWrite
    };

}; // namespace QTelephony

Q_DECLARE_USER_METATYPE_ENUM(QTelephony::RegistrationState)
Q_DECLARE_USER_METATYPE_ENUM(QTelephony::OperatorMode)
Q_DECLARE_USER_METATYPE_ENUM(QTelephony::OperatorAvailability)
Q_DECLARE_USER_METATYPE_ENUM(QTelephony::CallClass)
Q_DECLARE_USER_METATYPE_ENUM(QTelephony::Result)
Q_DECLARE_USER_METATYPE_ENUM(QTelephony::SimFileType)
Q_DECLARE_USER_METATYPE_ENUM(QTelephony::SimFileError)

#endif // QTELEPHONYNAMESPACE_H
