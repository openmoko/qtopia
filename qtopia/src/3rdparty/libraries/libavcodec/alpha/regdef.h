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
/* Some BSDs don't seem to have regdef.h... sigh  */
#ifndef alpha_regdef_h
#define alpha_regdef_h

#define v0      $0      /* function return value */

#define t0      $1      /* temporary registers (caller-saved) */
#define t1      $2
#define t2      $3
#define t3      $4
#define t4      $5
#define t5      $6
#define t6      $7
#define t7      $8

#define s0      $9      /* saved-registers (callee-saved registers) */
#define s1      $10
#define s2      $11
#define s3      $12
#define s4      $13
#define s5      $14
#define s6      $15
#define fp      s6      /* frame-pointer (s6 in frame-less procedures) */

#define a0      $16     /* argument registers (caller-saved) */
#define a1      $17
#define a2      $18
#define a3      $19
#define a4      $20
#define a5      $21

#define t8      $22     /* more temps (caller-saved) */
#define t9      $23
#define t10     $24
#define t11     $25
#define ra      $26     /* return address register */
#define t12     $27

#define pv      t12     /* procedure-variable register */
#define AT      $at     /* assembler temporary */
#define gp      $29     /* global pointer */
#define sp      $30     /* stack pointer */
#define zero    $31     /* reads as zero, writes are noops */

#endif /* alpha_regdef_h */
