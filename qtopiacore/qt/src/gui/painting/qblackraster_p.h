/****************************************************************************
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_3RDPARTY_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/***************************************************************************/
/*                                                                         */
/*  qblackraster_p.h, derived from ftraster.h                              */
/*                                                                         */
/*    The FreeType glyph rasterizer (specification).                       */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, ../../3rdparty/freetype/docs/FTL.TXT.  By continuing to use,  */
/*  modify, or distribute this file you indicate that you have read        */
/*  the license and understand and accept it fully.                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTRASTER_H__
#define __FTRASTER_H__

/*
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
*/

#ifdef __cplusplus
    extern "C" {
#endif

#include <private/qrasterdefs_p.h>

QT_FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* Uncomment the following line if you are using ftraster.c as a         */
  /* standalone module, fully independent of FreeType.                     */
  /*                                                                       */
/* #define _STANDALONE_ */

#ifndef QT_FT_EXPORT_VAR
#define QT_FT_EXPORT_VAR( x )  extern  x
#endif

  QT_FT_EXPORT_VAR( const QT_FT_Raster_Funcs )  qt_ft_standard_raster;


QT_FT_END_HEADER

#ifdef __cplusplus
    }
#endif


#endif /* __FTRASTER_H__ */


/* END */
