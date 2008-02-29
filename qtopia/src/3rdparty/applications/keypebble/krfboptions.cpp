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
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include "krfboptions.h"

KRFBOptions::KRFBOptions()
{
  Config config( "keypebble" );
  config.setGroup( "Encodings" );
  readSettings( &config );
}

KRFBOptions::~KRFBOptions()
{
    save();
}

void KRFBOptions::save()
{
  Config config( "keypebble" );
  config.setGroup( "Encodings" );
  writeSettings( &config );
}

void KRFBOptions::readSettings( Config *config )
{
  hexTile = config->readBoolEntry( "HexTile" );
  corre = config->readBoolEntry( "CORRE" );
  rre = config->readBoolEntry( "RRE" );
  copyrect = config->readBoolEntry( "CopyRect", true );
  colors256 = config->readBoolEntry( "Colors256" );
  shared = config->readBoolEntry( "Shared" );
  readOnly = config->readBoolEntry( "ReadOnly" );
  updateRate = config->readNumEntry( "UpdateRate", 50 );
}

void KRFBOptions::writeSettings( Config *config )
{
  config->writeEntry( "HexTile", hexTile );
  config->writeEntry( "CORRE", corre );
  config->writeEntry( "RRE", rre );
  config->writeEntry( "CopyRect", copyrect );
  config->writeEntry( "Colors256", colors256 );
  config->writeEntry( "Shared", shared );
  config->writeEntry( "ReadOnly", readOnly );
  config->writeEntry( "UpdateRate", updateRate );
}

int KRFBOptions::encodings()
{
  // Initially one because we always support raw encoding
  int count = 1;

  count += hexTile ? 1 : 0;
  count += corre ? 1 : 0;
  count += rre ? 1 : 0;
  count += copyrect ? 1 : 0;

  return count;
}
