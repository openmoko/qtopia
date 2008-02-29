/***************************************************************************
(C) Copyright 1996 Apple Computer, Inc., AT&T Corp., International             
Business Machines Corporation and Siemens Rolm Communications Inc.             
                                                                               
For purposes of this license notice, the term Licensors shall mean,            
collectively, Apple Computer, Inc., AT&T Corp., International                  
Business Machines Corporation and Siemens Rolm Communications Inc.             
The term Licensor shall mean any of the Licensors.                             
                                                                               
Subject to acceptance of the following conditions, permission is hereby        
granted by Licensors without the need for written agreement and without        
license or royalty fees, to use, copy, modify and distribute this              
software for any purpose.                                                      
                                                                               
The above copyright notice and the following four paragraphs must be           
reproduced in all copies of this software and any software including           
this software.                                                                 
                                                                               
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS AND NO LICENSOR SHALL HAVE       
ANY OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS OR       
MODIFICATIONS.                                                                 
                                                                               
IN NO EVENT SHALL ANY LICENSOR BE LIABLE TO ANY PARTY FOR DIRECT,              
INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOST PROFITS ARISING OUT         
OF THE USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH         
DAMAGE.                                                                        
                                                                               
EACH LICENSOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED,       
INCLUDING BUT NOT LIMITED TO ANY WARRANTY OF NONINFRINGEMENT OR THE            
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR             
PURPOSE.                                                                       

The software is provided with RESTRICTED RIGHTS.  Use, duplication, or         
disclosure by the government are subject to restrictions set forth in          
DFARS 252.227-7013 or 48 CFR 52.227-19, as applicable.                         

***************************************************************************/

/*
 * src: vobject.c
 * doc: vobject and APIs to construct vobject, APIs pretty print 
 * vobject, and convert a vobject into its textual representation.
 */

#include <stdlib.h>

#include <qtopia/config.h>
#include "vobject_p.h"
#include "qfiledirect_p.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
//#include <io.h>


#define NAME_OF(o)				o->id
#define VALUE_TYPE(o)			o->valType
#define STRINGZ_VALUE_OF(o)		o->val.strs
#define INTEGER_VALUE_OF(o)		o->val.i
#define LONG_VALUE_OF(o)		o->val.l
#define ANY_VALUE_OF(o)			o->val.any
#define VOBJECT_VALUE_OF(o)		o->val.vobj

static char vobj_cs[10];
static enum { EightBit, QuotedPrintable, Base64 } vobj_enc=EightBit;
static const char *vobj_enc_s=0;

typedef union ValueItem {
    const char *strs;
    unsigned int i;
    unsigned long l;
    void *any;
    VObject *vobj;
    } ValueItem;

struct VObject {
    VObject *next;
    const char *id;
    VObject *prop;
    unsigned short valType;
    ValueItem val;
    };

typedef struct StrItem StrItem;

struct StrItem {
    StrItem *next;
    const char *s;
    unsigned int refCnt;
    };

DLLEXPORT(const char**) fieldedProp;



/*----------------------------------------------------------------------
   The following functions involve with memory allocation:
	newVObject
	deleteVObject
	dupStr
	deleteStr
	newStrItem
	deleteStrItem
   ----------------------------------------------------------------------*/

DLLEXPORT(VObject*) newVObject_(const char *id)
{
    VObject *p = (VObject*)malloc(sizeof(VObject));
    p->next = 0;
    p->id = id;
    p->prop = 0;
    VALUE_TYPE(p) = 0;
    ANY_VALUE_OF(p) = 0;
    return p;
}

DLLEXPORT(VObject*) newVObject(const char *id)
{
    return newVObject_(lookupStr(id));
}

DLLEXPORT(void) deleteVObject(VObject *p)
{
    unUseStr(p->id);
    free(p);
}

DLLEXPORT(char*) dupStr(const char *s, unsigned int size)
{
    char *t;
    if  (size == 0) {
	size = strlen(s);
	}
    t = (char*)malloc(size+1);
    if (t) {
	memcpy(t,s,size);
	t[size] = 0;
	return t;
	}
    else {
	return (char*)0;
	}
}

DLLEXPORT(void) deleteStr(const char *p)
{
    if (p) free((void*)p);
}


static StrItem* newStrItem(const char *s, StrItem *next)
{
    StrItem *p = (StrItem*)malloc(sizeof(StrItem));
    p->next = next;
    p->s = s;
    p->refCnt = 1;
    return p;
}

static void deleteStrItem(StrItem *p)
{
    free((void*)p);
}


/*----------------------------------------------------------------------
  The following function provide accesses to VObject's value.
  ----------------------------------------------------------------------*/

DLLEXPORT(const char*) vObjectName(VObject *o)
{
    return NAME_OF(o);
}

DLLEXPORT(void) setVObjectName(VObject *o, const char* id)
{
    NAME_OF(o) = id;
}

DLLEXPORT(const char*) vObjectStringZValue(VObject *o)
{
    return STRINGZ_VALUE_OF(o);
}

DLLEXPORT(void) setVObjectStringZValue(VObject *o, const char *s)
{
    STRINGZ_VALUE_OF(o) = dupStr(s,0);
    VALUE_TYPE(o) = VCVT_STRINGZ;
}

DLLEXPORT(void) setVObjectStringZValue_(VObject *o, const char *s)
{
    STRINGZ_VALUE_OF(o) = s;
    VALUE_TYPE(o) = VCVT_STRINGZ;
}

DLLEXPORT(unsigned int) vObjectIntegerValue(VObject *o)
{
    return INTEGER_VALUE_OF(o);
}

DLLEXPORT(void) setVObjectIntegerValue(VObject *o, unsigned int i)
{
    INTEGER_VALUE_OF(o) = i;
    VALUE_TYPE(o) = VCVT_UINT;
}

DLLEXPORT(unsigned long) vObjectLongValue(VObject *o)
{
    return LONG_VALUE_OF(o);
}

DLLEXPORT(void) setVObjectLongValue(VObject *o, unsigned long l)
{
    LONG_VALUE_OF(o) = l;
    VALUE_TYPE(o) = VCVT_ULONG;
}

DLLEXPORT(void*) vObjectAnyValue(VObject *o)
{
    return ANY_VALUE_OF(o);
}

DLLEXPORT(void) setVObjectAnyValue(VObject *o, void *t)
{
    ANY_VALUE_OF(o) = t;
    VALUE_TYPE(o) = VCVT_RAW;
}

DLLEXPORT(VObject*) vObjectVObjectValue(VObject *o)
{
    return VOBJECT_VALUE_OF(o);
}

DLLEXPORT(void) setVObjectVObjectValue(VObject *o, VObject *p)
{
    VOBJECT_VALUE_OF(o) = p;
    VALUE_TYPE(o) = VCVT_VOBJECT;
}

DLLEXPORT(int) vObjectValueType(VObject *o)
{
    return VALUE_TYPE(o);
}


/*----------------------------------------------------------------------
  The following functions can be used to build VObject.
  ----------------------------------------------------------------------*/

DLLEXPORT(VObject*) addVObjectProp(VObject *o, VObject *p)
{
    /* circular link list pointed to tail */
    /*
    o {next,id,prop,val}
                V
	pn {next,id,prop,val}
             V
	    ...
	p1 {next,id,prop,val}
             V
	     pn
    -->
    o {next,id,prop,val}
                V
	pn {next,id,prop,val}
             V
	p {next,id,prop,val}
	    ...
	p1 {next,id,prop,val}
             V
	     pn
    */

    VObject *tail = o->prop;
    if (tail) {
	p->next = tail->next;
	o->prop = tail->next = p;
	}
    else {
	o->prop = p->next = p;
	}
    return p;
}

DLLEXPORT(VObject*) addProp(VObject *o, const char *id)
{
    return addVObjectProp(o,newVObject(id));
}

DLLEXPORT(VObject*) addProp_(VObject *o, const char *id)
{
    return addVObjectProp(o,newVObject_(id));
}

DLLEXPORT(void) addList(VObject **o, VObject *p)
{
    p->next = 0;
    if (*o == 0) {
	*o = p;
	}
    else {
	VObject *t = *o;
	while (t->next) {
	   t = t->next;
	   }
	t->next = p;
	}
}

DLLEXPORT(VObject*) nextVObjectInList(VObject *o)
{
    return o->next;
}

DLLEXPORT(VObject*) setValueWithSize_(VObject *prop, void *val, unsigned int size)
{
    VObject *sizeProp;
    setVObjectAnyValue(prop, val);
    sizeProp = addProp(prop,VCDataSizeProp);
    setVObjectLongValue(sizeProp, size);
    return prop;
}

DLLEXPORT(VObject*) setValueWithSize(VObject *prop, void *val, unsigned int size)
{
    void *p = dupStr((const char *)val,size);
    return setValueWithSize_(prop,p,p?size:0);
}

DLLEXPORT(void) initPropIterator(VObjectIterator *i, VObject *o)
{
    i->start = o->prop; 
    i->next = 0;
}

DLLEXPORT(void) initVObjectIterator(VObjectIterator *i, VObject *o)
{
    i->start = o->next; 
    i->next = 0;
}

DLLEXPORT(int) moreIteration(VObjectIterator *i)
{ 
    return (i->start && (i->next==0 || i->next!=i->start));
}

DLLEXPORT(VObject*) nextVObject(VObjectIterator *i)
{
    if (i->start && i->next != i->start) {
	if (i->next == 0) {
	    i->next = i->start->next;
	    return i->next;
	    }
	else {
	    i->next = i->next->next;
	    return i->next;
	    }
	}
    else return (VObject*)0;
}

DLLEXPORT(VObject*) isAPropertyOf(VObject *o, const char *id)
{
    VObjectIterator i;
    initPropIterator(&i,o);
    while (moreIteration(&i)) {
	VObject *each = nextVObject(&i);
	if (!qstricmp(id,each->id))
	    return each;
	}
    return (VObject*)0;
}

DLLEXPORT(VObject*) addGroup(VObject *o, const char *g)
{
    /*
	a.b.c
	-->
	prop(c)
	    prop(VCGrouping=b)
		prop(VCGrouping=a)
     */
    char *dot = strrchr(g,'.');
    if (dot) {
	VObject *p, *t;
	char *gs, *n = dot+1;
	gs = dupStr(g,0);	/* so we can write to it. */
	/* used to be
	* t = p = addProp_(o,lookupProp_(n));
	*/
	t = p = addProp_(o,lookupProp(n));
	dot = strrchr(gs,'.');
	*dot = 0;
	do {
	    dot = strrchr(gs,'.');
	    if (dot) {
		n = dot+1;
		*dot=0;
		}
	    else
		n = gs;
	    /* property(VCGroupingProp=n);
	     *	and the value may have VCGrouping property
	     */
	    t = addProp(t,VCGroupingProp);
	    setVObjectStringZValue(t,lookupProp_(n));
	    } while (n != gs);
	deleteStr(gs);	
	return p;
	}
    else
	return addProp_(o,lookupProp(g));
}

DLLEXPORT(VObject*) addPropValue(VObject *o, const char *p, const char *v)
{
    VObject *prop;
    prop = addProp(o,p);
    setVObjectStringZValue_(prop, strdup( v ) );
    return prop;
}

DLLEXPORT(VObject*) addPropSizedValue_(VObject *o, const char *p, const char *v,
	unsigned int size)
{
    VObject *prop;
    prop = addProp(o,p);
    setValueWithSize_(prop, (void*)v, size);
    return prop;
}

DLLEXPORT(VObject*) addPropSizedValue(VObject *o, const char *p, const char *v,
	unsigned int size)
{
    return addPropSizedValue_(o,p,dupStr(v,size),size);
}


DLLEXPORT(void) cleanVObject(VObject *o)
{
    if (o == 0) return;
    if (o->prop) {
	/* destroy time: cannot use the iterator here.
	   Have to break the cycle in the circular link
	   list and turns it into regular NULL-terminated
	   list -- since at some point of destruction,
	   the reference entry for the iterator to work
	   will not longer be valid.
	   */
	VObject *p;
	p = o->prop->next;
	o->prop->next = 0;
	do {
	   VObject *t = p->next;
	   cleanVObject(p);
	   p = t;
	   } while (p);
	}
    switch (VALUE_TYPE(o)) {
	case VCVT_STRINGZ:
	case VCVT_RAW:
	    // assume they are all allocated by malloc.
	    free((char*)STRINGZ_VALUE_OF(o));
	    break;
	case VCVT_VOBJECT:
	    cleanVObject(VOBJECT_VALUE_OF(o));
	    break;
	}
    deleteVObject(o);
}

DLLEXPORT(void) cleanVObjects(VObject *list)
{
    while (list) {
	VObject *t = list;
	list = nextVObjectInList(list);
	cleanVObject(t);
	}
}

/*----------------------------------------------------------------------
  The following is a String Table Facilities.
  ----------------------------------------------------------------------*/

#define STRTBLSIZE 255

static StrItem *strTbl[STRTBLSIZE];

static unsigned int hashStr(const char *s)
{
    unsigned int h = 0;
    int i;
    for (i=0;s[i];i++) {
	h += s[i]*i;
	}
    return h % STRTBLSIZE;
}

DLLEXPORT(const char*) lookupStr(const char *s)
{
    StrItem *t;
    unsigned int h = hashStr(s);
    if ((t = strTbl[h]) != 0) {
	do {
	    if (qstricmp(t->s,s) == 0) {
		t->refCnt++;
		return t->s;
		}
	    t = t->next;
	    } while (t);
	}
    s = dupStr(s,0);
    strTbl[h] = newStrItem(s,strTbl[h]);
    return s;
}

DLLEXPORT(void) unUseStr(const char *s)
{
    StrItem *t, *p;
    unsigned int h = hashStr(s);
    if ((t = strTbl[h]) != 0) {
	p = t;
	do {
	    if (qstricmp(t->s,s) == 0) {
		t->refCnt--;
		if (t->refCnt == 0) {
		    if (p == strTbl[h]) {
			strTbl[h] = t->next;
			}
		    else {
			p->next = t->next;
			}
		    deleteStr(t->s);
		    deleteStrItem(t);
		    return;
		    }
		}
	    p = t;
	    t = t->next;
	    } while (t);
	}
}

DLLEXPORT(void) cleanStrTbl()
{
    int i;
    for (i=0; i<STRTBLSIZE;i++) {
	StrItem *t = strTbl[i];
	while (t) {
	    StrItem *p;
	    deleteStr(t->s);
	    p = t;
	    t = t->next;
	    deleteStrItem(p);
	    } while (t);
	strTbl[i] = 0;
	}
}


struct PreDefProp {
    const char *name;
    const char *alias;
    const char** fields;
    unsigned int flags;
    };

/* flags in PreDefProp */
#define PD_BEGIN	0x1
#define PD_INTERNAL	0x2

static const char *adrFields[] = {
    VCPostalBoxProp,
    VCExtAddressProp,
    VCStreetAddressProp,
    VCCityProp,
    VCRegionProp,
    VCPostalCodeProp,
    VCCountryNameProp,
    0
};

static const char *nameFields[] = {
    VCFamilyNameProp,
    VCGivenNameProp,
    VCAdditionalNamesProp,
    VCNamePrefixesProp,
    VCNameSuffixesProp,
    NULL
    };

static const char *orgFields[] = {
    VCOrgNameProp,
    VCOrgUnitProp,
    VCOrgUnit2Prop,
    VCOrgUnit3Prop,
    VCOrgUnit4Prop,
    NULL
    };

static const char *AAlarmFields[] = {
    VCRunTimeProp,
    VCSnoozeTimeProp,
    VCRepeatCountProp,
    VCAudioContentProp,
    0
    };

/* ExDate -- has unamed fields */
/* RDate -- has unamed fields */

static const char *DAlarmFields[] = {
    VCRunTimeProp,
    VCSnoozeTimeProp,
    VCRepeatCountProp,
    VCDisplayStringProp,
    0
    };

static const char *MAlarmFields[] = {
    VCRunTimeProp,
    VCSnoozeTimeProp,
    VCRepeatCountProp,
    VCEmailAddressProp,
    VCNoteProp,
    0
    };

static const char *PAlarmFields[] = {
    VCRunTimeProp,
    VCSnoozeTimeProp,
    VCRepeatCountProp,
    VCProcedureNameProp,
    0
    };

static struct PreDefProp propNames[] = {
    { VC7bitProp, 0, 0, 0 },
    { VC8bitProp, 0, 0, 0 },
    { VCAAlarmProp, 0, AAlarmFields, 0 },
    { VCAdditionalNamesProp, 0, 0, 0 },
    { VCAdrProp, 0, adrFields, 0 },
    { VCAgentProp, 0, 0, 0 },
    { VCAIFFProp, 0, 0, 0 },
    { VCAOLProp, 0, 0, 0 },
    { VCAppleLinkProp, 0, 0, 0 },
    { VCAttachProp, 0, 0, 0 },
    { VCAttendeeProp, 0, 0, 0 },
    { VCATTMailProp, 0, 0, 0 },
    { VCAudioContentProp, 0, 0, 0 },
    { VCAVIProp, 0, 0, 0 },
    { VCBase64Prop, 0, 0, 0 },
    { VCBBSProp, 0, 0, 0 },
    { VCBirthDateProp, 0, 0, 0 },
    { VCBMPProp, 0, 0, 0 },
    { VCBodyProp, 0, 0, 0 },
    { VCBusinessRoleProp, 0, 0, 0 },
    { VCCalProp, 0, 0, PD_BEGIN },
    { VCCaptionProp, 0, 0, 0 },
    { VCCardProp, 0, 0, PD_BEGIN },
    { VCCarProp, 0, 0, 0 },
    { VCCategoriesProp, 0, 0, 0 },
    { VCCellularProp, 0, 0, 0 },
    { VCCGMProp, 0, 0, 0 },
    { VCCharSetProp, 0, 0, 0 },
    { VCCIDProp, VCContentIDProp, 0, 0 },
    { VCCISProp, 0, 0, 0 },
    { VCCityProp, 0, 0, 0 },
    { VCClassProp, 0, 0, 0 },
    { VCCommentProp, 0, 0, 0 },
    { VCCompletedProp, 0, 0, 0 },
    { VCContentIDProp, 0, 0, 0 },
    { VCCountryNameProp, 0, 0, 0 },
    { VCDAlarmProp, 0, DAlarmFields, 0 },
    { VCDataSizeProp, 0, 0, PD_INTERNAL },
    { VCDayLightProp, 0, 0, 0 },
    { VCDCreatedProp, 0, 0, 0 },
    { VCDeliveryLabelProp, 0, 0, 0 },
    { VCDescriptionProp, 0, 0, 0 },
    { VCDIBProp, 0, 0, 0 },
    { VCDisplayStringProp, 0, 0, 0 },
    { VCDomesticProp, 0, 0, 0 },
    { VCDTendProp, 0, 0, 0 },
    { VCDTstartProp, 0, 0, 0 },
    { VCDueProp, 0, 0, 0 },
    { VCEmailAddressProp, 0, 0, 0 },
    { VCEncodingProp, 0, 0, 0 },
    { VCEndProp, 0, 0, 0 },
    { VCEventProp, 0, 0, PD_BEGIN },
    { VCEWorldProp, 0, 0, 0 },
    { VCExNumProp, 0, 0, 0 },
    { VCExpDateProp, 0, 0, 0 },
    { VCExpectProp, 0, 0, 0 },
    { VCExtAddressProp, 0, 0, 0 },
    { VCFamilyNameProp, 0, 0, 0 },
    { VCFaxProp, 0, 0, 0 },
    { VCFullNameProp, 0, 0, 0 },
    { VCGeoLocationProp, 0, 0, 0 },
    { VCGeoProp, 0, 0, 0 },
    { VCGIFProp, 0, 0, 0 },
    { VCGivenNameProp, 0, 0, 0 },
    { VCGroupingProp, 0, 0, 0 },
    { VCHomeProp, 0, 0, 0 },
    { VCIBMMailProp, 0, 0, 0 },
    { VCInlineProp, 0, 0, 0 },
    { VCInternationalProp, 0, 0, 0 },
    { VCInternetProp, 0, 0, 0 },
    { VCISDNProp, 0, 0, 0 },
    { VCJPEGProp, 0, 0, 0 },
    { VCLanguageProp, 0, 0, 0 },
    { VCLastModifiedProp, 0, 0, 0 },
    { VCLastRevisedProp, 0, 0, 0 },
    { VCLocationProp, 0, 0, 0 },
    { VCLogoProp, 0, 0, 0 },
    { VCMailerProp, 0, 0, 0 },
    { VCMAlarmProp, 0, MAlarmFields, 0 },
    { VCMCIMailProp, 0, 0, 0 },
    { VCMessageProp, 0, 0, 0 },
    { VCMETProp, 0, 0, 0 },
    { VCModemProp, 0, 0, 0 },
    { VCMPEG2Prop, 0, 0, 0 },
    { VCMPEGProp, 0, 0, 0 },
    { VCMSNProp, 0, 0, 0 },
    { VCNamePrefixesProp, 0, 0, 0 },
    { VCNameProp, 0, nameFields, 0 },
    { VCNameSuffixesProp, 0, 0, 0 },
    { VCNoteProp, 0, 0, 0 },
    { VCOrgNameProp, 0, 0, 0 },
    { VCOrgProp, 0, orgFields, 0 },
    { VCOrgUnit2Prop, 0, 0, 0 },
    { VCOrgUnit3Prop, 0, 0, 0 },
    { VCOrgUnit4Prop, 0, 0, 0 },
    { VCOrgUnitProp, 0, 0, 0 },
    { VCPagerProp, 0, 0, 0 },
    { VCPAlarmProp, 0, PAlarmFields, 0 },
    { VCParcelProp, 0, 0, 0 },
    { VCPartProp, 0, 0, 0 },
    { VCPCMProp, 0, 0, 0 },
    { VCPDFProp, 0, 0, 0 },
    { VCPGPProp, 0, 0, 0 },
    { VCPhotoProp, 0, 0, 0 },
    { VCPICTProp, 0, 0, 0 },
    { VCPMBProp, 0, 0, 0 },
    { VCPostalBoxProp, 0, 0, 0 },
    { VCPostalCodeProp, 0, 0, 0 },
    { VCPostalProp, 0, 0, 0 },
    { VCPowerShareProp, 0, 0, 0 },
    { VCPreferredProp, 0, 0, 0 },
    { VCPriorityProp, 0, 0, 0 },
    { VCProcedureNameProp, 0, 0, 0 },
    { VCProdIdProp, 0, 0, 0 },
    { VCProdigyProp, 0, 0, 0 },
    { VCPronunciationProp, 0, 0, 0 },
    { VCPSProp, 0, 0, 0 },
    { VCPublicKeyProp, 0, 0, 0 },
    { VCQPProp, VCQuotedPrintableProp, 0, 0 },
    { VCQuickTimeProp, 0, 0, 0 },
    { VCQuotedPrintableProp, 0, 0, 0 },
    { VCRDateProp, 0, 0, 0 },
    { VCRegionProp, 0, 0, 0 },
    { VCRelatedToProp, 0, 0, 0 },
    { VCRepeatCountProp, 0, 0, 0 },
    { VCResourcesProp, 0, 0, 0 },
    { VCRNumProp, 0, 0, 0 },
    { VCRoleProp, 0, 0, 0 },
    { VCRRuleProp, 0, 0, 0 },
    { VCRSVPProp, 0, 0, 0 },
    { VCRunTimeProp, 0, 0, 0 },
    { VCSequenceProp, 0, 0, 0 },
    { VCSnoozeTimeProp, 0, 0, 0 },
    { VCStartProp, 0, 0, 0 },
    { VCStatusProp, 0, 0, 0 },
    { VCStreetAddressProp, 0, 0, 0 },
    { VCSubTypeProp, 0, 0, 0 },
    { VCSummaryProp, 0, 0, 0 },
    { VCTelephoneProp, 0, 0, 0 },
    { VCTIFFProp, 0, 0, 0 },
    { VCTimeZoneProp, 0, 0, 0 },
    { VCTitleProp, 0, 0, 0 },
    { VCTLXProp, 0, 0, 0 },
    { VCTodoProp, 0, 0, PD_BEGIN },
    { VCTranspProp, 0, 0, 0 },
    { VCUniqueStringProp, 0, 0, 0 },
    { VCURLProp, 0, 0, 0 },
    { VCURLValueProp, 0, 0, 0 },
    { VCValueProp, 0, 0, 0 },
    { VCVersionProp, 0, 0, 0 },
    { VCVideoProp, 0, 0, 0 },
    { VCVoiceProp, 0, 0, 0 },
    { VCWAVEProp, 0, 0, 0 },
    { VCWMFProp, 0, 0, 0 },
    { VCWorkProp, 0, 0, 0 },
    { VCX400Prop, 0, 0, 0 },
    { VCX509Prop, 0, 0, 0 },
    { VCXRuleProp, 0, 0, 0 },
    { 0,0,0,0 }
    };


static struct PreDefProp* lookupPropInfo(const char* str)
{
    /* brute force for now, could use a hash table here. */
    int i;
	
    for (i = 0; propNames[i].name; i++)
	if (qstricmp(str, propNames[i].name) == 0) {
	    return &propNames[i];
	    }
    
    return 0;
}


DLLEXPORT(const char*) lookupProp_(const char* str)
{
    int i;
	
    for (i = 0; propNames[i].name; i++)
	if (qstricmp(str, propNames[i].name) == 0) {
	    const char* s;
	    s = propNames[i].alias?propNames[i].alias:propNames[i].name;
	    return lookupStr(s);
	    }
    return lookupStr(str);
}


DLLEXPORT(const char*) lookupProp(const char* str)
{
    int i;
	
    for (i = 0; propNames[i].name; i++)
	if (qstricmp(str, propNames[i].name) == 0) {
	    const char *s;
	    fieldedProp = propNames[i].fields;
	    s = propNames[i].alias?propNames[i].alias:propNames[i].name;
	    return lookupStr(s);
	    }
    fieldedProp = 0;
    return lookupStr(str);
}


/*----------------------------------------------------------------------
  APIs to Output text form.
  ----------------------------------------------------------------------*/
#define OFILE_REALLOC_SIZE 256
typedef struct OFile {
    FILE *fp;
    char *s;
    int len;
    int limit;
    int alloc:1;
    int fail:1;
    } OFile;

#if 0
static void appendsOFile(OFile *fp, const char *s)
{
    int slen;
    if (fp->fail) return;
    slen  = strlen(s);
    if (fp->fp) {
	fwrite(s,1,slen,fp->fp);
	}
    else {
stuff:
	if (fp->len + slen < fp->limit) {
	    memcpy(fp->s+fp->len,s,slen);
	    fp->len += slen;
	    return;
	    }
	else if (fp->alloc) {
	    fp->limit = fp->limit + OFILE_REALLOC_SIZE;
	    if (OFILE_REALLOC_SIZE <= slen) fp->limit += slen;
	    fp->s = (char *) realloc(fp->s,fp->limit);
	    if (fp->s) goto stuff;
	    }
	if (fp->alloc)
	    free(fp->s);
	fp->s = 0;
	fp->fail = 1;
	}
}

static void appendcOFile(OFile *fp, char c)
{
    if (fp->fail) return;
    if (fp->fp) {
	fputc(c,fp->fp);
	}
    else {
stuff:
	if (fp->len+1 < fp->limit) {
	    fp->s[fp->len] = c;
	    fp->len++;
	    return;
	    }
	else if (fp->alloc) {
	    fp->limit = fp->limit + OFILE_REALLOC_SIZE;
	    fp->s = (char *) realloc(fp->s,fp->limit);
	    if (fp->s) goto stuff;
	    }
	if (fp->alloc)
	    free(fp->s);
	fp->s = 0;
	fp->fail = 1;
	}
}
#else
static void appendcOFile_(OFile *fp, char c)
{
    if (fp->fail) return;
    if (fp->fp) {
	fputc(c,fp->fp);
	}
    else {
stuff:
	if (fp->len+1 < fp->limit) {
	    fp->s[fp->len] = c;
	    fp->len++;
	    return;
	    }
	else if (fp->alloc) {
	    fp->limit = fp->limit + OFILE_REALLOC_SIZE;
	    fp->s = (char *)realloc(fp->s,fp->limit);
	    if (fp->s) goto stuff;
	    }
	if (fp->alloc)
	    free(fp->s);
	fp->s = 0;
	fp->fail = 1;
	}
}

static void appendcOFile(OFile *fp, char c)
{
    if (c == '\n') {
	/* write out as <CR><LF> */
	appendcOFile_(fp,0xd);
	appendcOFile_(fp,0xa);
	}
    else
	appendcOFile_(fp,c);
}

static void appendsOFile(OFile *fp, const char *s)
{
    int i, slen;
    slen  = strlen(s);
    for (i=0; i<slen; i++) {
	appendcOFile(fp,s[i]);
	}
}

#endif

static void appendsOFileEncCs(OFile *fp)
{
    if ( vobj_enc_s ) {
	appendsOFile(fp, ";" VCEncodingProp "=");
	appendsOFile(fp, vobj_enc_s);
    }
    appendsOFile(fp, ";" VCCharSetProp "=");
    appendsOFile(fp, vobj_cs);
}


static void initOFile(OFile *fp, FILE *ofp)
{
    fp->fp = ofp;
    fp->s = 0;
    fp->len = 0;
    fp->limit = 0;
    fp->alloc = 0;
    fp->fail = 0;
}

static int writeBase64(OFile *fp, unsigned char *s, long len)
{
    long cur = 0;
    int i, numQuads = 0;
    unsigned long trip;
    unsigned char b;
    char quad[5];
#define MAXQUADS 16

    quad[4] = 0;

    while (cur < len) {
	// collect the triplet of bytes into 'trip'
	trip = 0;
	for (i = 0; i < 3; i++) {
	    b = (cur < len) ? *(s + cur) : 0;
	    cur++;
	    trip = trip << 8 | b;
	    }
	// fill in 'quad' with the appropriate four characters
	for (i = 3; i >= 0; i--) {
	    b = (unsigned char)(trip & 0x3F);
	    trip = trip >> 6;
	    if ((3 - i) < (cur - len))
		quad[i] = '='; // pad char
	    else if (b < 26) quad[i] = (char)b + 'A';
	    else if (b < 52) quad[i] = (char)(b - 26) + 'a';
	    else if (b < 62) quad[i] = (char)(b - 52) + '0';
	    else if (b == 62) quad[i] = '+';
	    else quad[i] = '/';
	    }
	// now output 'quad' with appropriate whitespace and line ending
	appendsOFile(fp, (numQuads == 0 ? "    " : ""));
	appendsOFile(fp, quad);
	appendsOFile(fp, ((cur >= len)?"\n" :(numQuads==MAXQUADS-1?"\n" : "")));
	numQuads = (numQuads + 1) % MAXQUADS;
	}
    appendcOFile(fp,'\n');

    return 1;
}

static const char *qpReplaceChar(unsigned char c) 
{
    if (c == '\n') {
	return "=0A=\n";
    } else if (
	    // RFC 1521
	    (c >= 32 && c <= 60) // Note: " " not allowed at EOL
	    ||
	    (c >= 62 && c <= 126)
	)
    { 
	return 0;
    }

    static char trans[4];
    trans[0] = '=';
    trans[3] = '\0';
    int rem = c % 16;
    int div = c / 16;

    if (div < 10)
	trans[1] = '0' + div;
    else
	trans[1] = 'A' + (div - 10);

    if (rem < 10)
	trans[2] = '0' + rem;
    else
	trans[2] = 'A' + (rem - 10);

    return trans;
}

static void writeEncString(OFile *fp, const char *s, bool nosemi)
{
    /*
	only A-Z, 0-9 and 
       "'"  (ASCII code 39)
       "("  (ASCII code 40)
       ")"  (ASCII code 41)
       "+"  (ASCII code 43)
       ","  (ASCII code 44)
       "-"  (ASCII code 45)
       "/"  (ASCII code 47)
       "?"  (ASCII code 63)
       
       should remain un-encoded.
       '=' needs to be encoded as it is the escape character.
       ';' needs to be as it is a field separator.

     */
    const char *p = s;
    switch ( vobj_enc ) {
	case EightBit:
	    while (*p) {
		if ( *p == '\n' || nosemi && ( *p == '\\' || *p == ';' ) )
		    appendcOFile(fp, '\\');
		appendcOFile(fp, *p);
		p++;
	    }
	    break;
	case QuotedPrintable:
	    while (*p) {
		const char *rep = qpReplaceChar(*p);
		if (rep)
		    appendsOFile(fp, rep);
		else if ( *p == ';' && nosemi )
		    appendsOFile(fp, "=3B");
		else if ( *p == ' ' ) {
		    if ( !p[1] || p[1] == '\n' ) // RFC 1521
			appendsOFile(fp, "=20");
		    else
			appendcOFile(fp, *p);
		} else
		    appendcOFile(fp, *p);
		p++;
	    }
	    break;
	case Base64:
	    writeBase64(fp, (unsigned char*)p, strlen(p));
	    break;
    }
}

static bool includesUnprintable(VObject *o)
{
    if (o) {
	if (VALUE_TYPE(o) == VCVT_STRINGZ) {
	    const char *p = STRINGZ_VALUE_OF(o);
	    if (p) {
		while (*p) {
		    if (*p==' ' && (!p[1] || p[1]=='\n') // RFC 1521: spaces at ends need quoting
			     || qpReplaceChar(*p) )
			return TRUE;
		    p++;
		}
	    }
	}
    }
    return FALSE;
}
	    
static void writeVObject_(OFile *fp, VObject *o);

static void writeValue(OFile *fp, VObject *o, unsigned long size, bool nosemi)
{
    if (o == 0) return;
    switch (VALUE_TYPE(o)) {
	case VCVT_STRINGZ: {
	    writeEncString(fp, STRINGZ_VALUE_OF(o), nosemi);
	    break;
	    }
	case VCVT_UINT: {
	    char buf[16];
	    sprintf(buf,"%u", INTEGER_VALUE_OF(o));
	    appendsOFile(fp,buf);
	    break;
	    }
	case VCVT_ULONG: {
	    char buf[16];
	    sprintf(buf,"%lu", LONG_VALUE_OF(o));
	    appendsOFile(fp,buf);
	    break;
	    }
	case VCVT_RAW: {
	    appendcOFile(fp,'\n');
	    writeBase64(fp,(unsigned char*)(ANY_VALUE_OF(o)),size);
	    break;
	    }
	case VCVT_VOBJECT:
	    appendcOFile(fp,'\n');
	    writeVObject_(fp,VOBJECT_VALUE_OF(o));
	    break;
	}
}

static void writeAttrValue(OFile *fp, VObject *o)
{
    if (NAME_OF(o)) {
	struct PreDefProp *pi;
	pi = lookupPropInfo(NAME_OF(o));
	if (pi && ((pi->flags & PD_INTERNAL) != 0)) return;
	if ( includesUnprintable(o) )
	    appendsOFileEncCs(fp);
	appendcOFile(fp,';');
	appendsOFile(fp,NAME_OF(o));
    } else {
	appendcOFile(fp,';');
    }
    if (VALUE_TYPE(o)) {
	appendcOFile(fp,'=');
	writeValue(fp,o,0,TRUE);
    }
}

static void writeGroup(OFile *fp, VObject *o)
{
    char buf1[256];
    char buf2[256];
    strcpy(buf1,NAME_OF(o));
    while ((o=isAPropertyOf(o,VCGroupingProp)) != 0) {
	strcpy(buf2,STRINGZ_VALUE_OF(o));
	strcat(buf2,".");
	strcat(buf2,buf1);
	strcpy(buf1,buf2);
	}
    appendsOFile(fp,buf1);
}

static int inList(const char **list, const char *s)
{
    if (list == 0) return 0;
    while (*list) {
	if (qstricmp(*list,s) == 0) return 1;
	list++;
	}
    return 0;
}

static void writeProp(OFile *fp, VObject *o)
{
    if (NAME_OF(o)) {
	struct PreDefProp *pi;
	VObjectIterator t;
	const char **fields_ = 0;
	pi = lookupPropInfo(NAME_OF(o));
	if (pi && ((pi->flags & PD_BEGIN) != 0)) {
	    writeVObject_(fp,o);
	    return;
	    }
	if (isAPropertyOf(o,VCGroupingProp))
	    writeGroup(fp,o);
	else
	    appendsOFile(fp,NAME_OF(o));
	if (pi) fields_ = pi->fields;
	initPropIterator(&t,o);
	while (moreIteration(&t)) {
	    const char *s;
	    VObject *eachProp = nextVObject(&t);
	    s = NAME_OF(eachProp);
	    if (qstricmp(VCGroupingProp,s) && !inList(fields_,s))
		writeAttrValue(fp,eachProp);
	    }
	if (fields_) {
	    int i = 0, n = 0;
	    const char** fields = fields_;
	    /* output prop as fields */
	    bool printable = TRUE;
	    while (*fields && printable) {
		VObject *t = isAPropertyOf(o,*fields);
		if (includesUnprintable(t))
		    printable = FALSE;
		fields++;
	    }
	    fields = fields_;
	    if (!printable)
		appendsOFileEncCs(fp);
	    appendcOFile(fp,':');
	    while (*fields) {
		VObject *t = isAPropertyOf(o,*fields);
		i++;
		if (t) n = i;
		fields++;
	    }
	    fields = fields_;
	    for (i=0;i<n;i++) {
		writeValue(fp,isAPropertyOf(o,*fields),0,TRUE);
		fields++;
		if (i<(n-1)) appendcOFile(fp,';');
	    }
	    }
	}

	
    if (VALUE_TYPE(o)) {
	    if ( includesUnprintable(o) )
			appendsOFileEncCs(fp);
	unsigned long size = 0;
        VObject *p = isAPropertyOf(o,VCDataSizeProp);
	if (p) size = LONG_VALUE_OF(p);
	appendcOFile(fp,':');
	writeValue(fp,o,size,FALSE);
	}

    appendcOFile(fp,'\n');
}

static void writeVObject_(OFile *fp, VObject *o)
{
    if (NAME_OF(o)) {
	struct PreDefProp *pi;
	pi = lookupPropInfo(NAME_OF(o));

	if (pi && ((pi->flags & PD_BEGIN) != 0)) {
	    VObjectIterator t;
	    const char *begin = NAME_OF(o);
	    appendsOFile(fp,"BEGIN:");
	    appendsOFile(fp,begin);
	    appendcOFile(fp,'\n');
	    initPropIterator(&t,o);
	    while (moreIteration(&t)) {
		VObject *eachProp = nextVObject(&t);
		writeProp(fp, eachProp);
		}
	    appendsOFile(fp,"END:");
	    appendsOFile(fp,begin);
	    appendsOFile(fp,"\n\n");
	    }
	}
}

static void initVObjectEncoding()
{
    Config pimConfig( "Beam" );
    pimConfig.setGroup("Send");
    Config devcfg(pimConfig.readEntry("DeviceConfig"),Config::File);
    QString enc = "QP";
    QString cs = "UTF-8";
    if ( devcfg.isValid() ) {
	devcfg.setGroup("Send");
	enc = devcfg.readEntry("Encoding","QP"); 
	cs = devcfg.readEntry("CharSet","UTF-8"); 
    }
    strncpy(vobj_cs,cs.latin1(),10);
    if ( enc == "QP" ) {
	vobj_enc = QuotedPrintable;
	vobj_enc_s = VCQuotedPrintableProp;
    } else if ( enc == "B64" ) {
	vobj_enc = Base64;
	vobj_enc_s = VCBase64Prop;
    } else {
	vobj_enc = EightBit;
	vobj_enc_s = 0;
    }
}

void writeVObject(FILE *fp, VObject *o)
{
    initVObjectEncoding();

    OFile ofp;
    // #####
    //_setmode(_fileno(fp), _O_BINARY);
    initOFile(&ofp,fp);
    writeVObject_(&ofp,o);
}

DLLEXPORT(void) writeVObjectToFile(char *fname, VObject *o)
{
	QFileDirect f( fname);
	if ( !f.open( IO_WriteOnly ) ) {
		qWarning("Unable to open vobject write %s", fname);
		return;
	}

    writeVObject( f.directHandle(),o );
}

DLLEXPORT(void) writeVObjectsToFile(char *fname, VObject *list)
{
	QFileDirect f( fname);
	if ( !f.open( IO_WriteOnly ) ) {
		qWarning("Unable to open vobject write %s", fname);
		return;
	}

	while (list) {
	    writeVObject(f.directHandle(),list);
	    list = nextVObjectInList(list);
	    }
}

DLLEXPORT(const char *) vObjectTypeInfo(VObject *o)
{
    const char *type = vObjectName( o );
    if ( strcmp( type, "TYPE" ) == 0 )
	type = vObjectStringZValue( o );
    return type;
}


// end of source file vobject.c
