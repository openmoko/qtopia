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
/* -------------------------------------------------------------------------- */
/*                                                                            */
/* [keytrans.C]             Keyboard Translation                              */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/* Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>            */
/*                                                                            */
/* This file is part of Konsole - an X terminal for KDE                       */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*									      */
/* Konsole ported to Qt/Embedded by Trolltech                                 */
/*									      */
/* -------------------------------------------------------------------------- */

/*
   The keyboard translation table allows to configure konsoles behavior
   on key strokes.

   FIXME: some bug crept in, disallowing '\0' to be emitted.
*/

#include "keytrans.h"

#include <qtopia/qpeapplication.h>

#include <qnamespace.h>
#include <qbuffer.h>
#include <qobject.h>
#include <qdict.h>
#include <qintdict.h>
#include <qfile.h>
#include <qglobal.h>
#include <qdir.h>

//#include <kstddirs.h>
//nclude <klocale.h>

#include <stdio.h>


#undef USE_APPDATA_DIR


#define HERE printf("%s(%d): here\n",__FILE__,__LINE__)

/* KeyEntry

   instances represent the individual assignments
*/

KeyTrans::KeyEntry::KeyEntry(int _ref, int _key, int _bits, int _mask, int _cmd, QString _txt)
: ref(_ref), key(_key), bits(_bits), mask(_mask), cmd(_cmd), txt(_txt)
{
}

KeyTrans::KeyEntry::~KeyEntry()
{
}

bool KeyTrans::KeyEntry::matches(int _key, int _bits, int _mask)
{ int m = mask & _mask;
  return _key == key && (bits & m) == (_bits & m);
}

QString KeyTrans::KeyEntry::text()
{
  return txt;
}

/* KeyTrans

   combines the individual assignments into a proper map, and takes part in a collection.
*/

KeyTrans::KeyTrans()
{
  path = "";
  numb = 0;
}

KeyTrans::~KeyTrans()
{
}

KeyTrans::KeyEntry* KeyTrans::addEntry(int ref, int key, int bits, int mask, int cmd, QString txt)
// returns conflicting entry
{
  for (QListIterator<KeyEntry> it(table); it.current(); ++it)
  {
    if (it.current()->matches(key,bits,mask))
    {
      return it.current();
    }
  }
  table.append(new KeyEntry(ref,key,bits,mask,cmd,txt));
  return (KeyEntry*)NULL;
}

bool KeyTrans::findEntry(int key, int bits, int* cmd, const char** txt, int* len)
{
  for (QListIterator<KeyEntry> it(table); it.current(); ++it)
    if (it.current()->matches(key,bits,0xffff))
    {
      *cmd = it.current()->cmd;
      *txt = it.current()->txt.ascii();
      *len = it.current()->txt.length();
      return TRUE;
    }
  return FALSE;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* Scanner for keyboard configuration                                        */
/*                                                                           */
/* ------------------------------------------------------------------------- */

// regular tokenizer
/* Tokens
   - Spaces
   - Name    (A-Za-z0-9)+
   - String
   - Opr     on of +-:
*/

#define SYMName    0
#define SYMString  1
#define SYMEol     2
#define SYMEof     3
#define SYMOpr     4
#define SYMError   5

#define inRange(L,X,H) ((L <= X) && (X <= H))
#define isNibble(X) (inRange('A',X,'F')||inRange('a',X,'f')||inRange('0',X,'9'))
#define convNibble(X) (inRange('0',X,'9')?X-'0':X+10-(inRange('A',X,'F')?'A':'a'))

class KeytabReader
{
public:
  KeytabReader(QString p, QIODevice &d);
public:
  void getCc();
  void getSymbol();
  void parseTo(KeyTrans* kt);
  void ReportError(const char* msg);
  void ReportToken(); // diagnostic
private:
  int     sym;
  QString res;
  int     len;
  int     slinno;
  int     scolno;
private:
  int     cc;
  int     linno;
  int     colno;
  QIODevice* buf;
  QString path;
};


KeytabReader::KeytabReader(QString p, QIODevice &d)
{
  path = p;
  buf = &d;
  cc = 0;
}

void KeytabReader::getCc()
{
  if (cc == '\n') { linno += 1; colno = 0; }
  if (cc < 0) return;
  cc = buf->getch();
  colno += 1;
}

void KeytabReader::getSymbol()
{
  res = ""; len = 0; sym = SYMError;
  while (cc == ' ') getCc(); // skip spaces
  if (cc == '#')      // skip comment
  {
    while (cc != '\n' && cc > 0) getCc();
  }
  slinno = linno;
  scolno = colno;
  if (cc <= 0)
  {
    sym = SYMEof; return; // eos
  }
  if (cc == '\n')
  {
    getCc();
    sym = SYMEol; return; // eol
  }
  if (inRange('A',cc,'Z')||inRange('a',cc,'z')||inRange('0',cc,'9'))
  {
    while (inRange('A',cc,'Z') || inRange('a',cc,'z') || inRange('0',cc,'9'))
    {
      res = res + (char)cc;
      getCc();
    }
    sym = SYMName;
    return;
  }
  if (strchr("+-:",cc))
  {
    res = "";
    res = res + (char)cc;
    getCc();
    sym = SYMOpr; return;
  }
  if (cc == '"')
  {
    getCc();
    while (cc >= ' ' && cc != '"')
    { int sc;
      if (cc == '\\') // handle quotation
      {
        getCc();
        switch (cc)
        {
          case 'E'  : sc = 27; getCc(); break;
          case 'b'  : sc =  8; getCc(); break;
          case 'f'  : sc = 12; getCc(); break;
          case 't'  : sc =  9; getCc(); break;
          case 'r'  : sc = 13; getCc(); break;
          case 'n'  : sc = 10; getCc(); break;
          case '\\' : // fall thru
          case '"'  : sc = cc; getCc(); break;
          case 'x'  : getCc();
                      sc = 0;
                      if (!isNibble(cc)) return; sc = 16*sc + convNibble(cc); getCc();
                      if (!isNibble(cc)) return; sc = 16*sc + convNibble(cc); getCc();
                      break;
          default   : return;
        }
      }
      else
      {
        // regular char
        sc = cc; getCc();
      }
      res = res + (char)sc;
      len = len + 1;
    }
    if (cc != '"') return;
    getCc();
    sym = SYMString; return;
  }
}

void KeytabReader::ReportToken() // diagnostic
{
  printf("sym(%d): ",slinno);
  switch(sym)
  {
    case SYMEol    : printf("End of line"); break;
    case SYMEof    : printf("End of file"); break;
    case SYMName   : printf("Name: %s",res.latin1()); break;
    case SYMOpr    : printf("Opr : %s",res.latin1()); break;
    case SYMString : printf("String len %d,%d ",res.length(),len);
                     for (unsigned i = 0; i < res.length(); i++)
                       printf(" %02x(%c)",res.latin1()[i],res.latin1()[i]>=' '?res.latin1()[i]:'?');
                     break;
  }
  printf("\n");
}

void KeytabReader::ReportError(const char* msg) // diagnostic
{
  fprintf(stderr,"%s(%d,%d):error: %s.\n",path.ascii(),slinno,scolno,msg);
}

// local symbol tables ---------------------------------------------------------------------

class KeyTransSymbols
{
public:
  KeyTransSymbols();
protected:
  void defOprSyms();
  void defModSyms();
  void defKeySyms();
  void defKeySym(const char* key, int val);
  void defOprSym(const char* key, int val);
  void defModSym(const char* key, int val);
public:
  QDict<QObject> keysyms;
  QDict<QObject> modsyms;
  QDict<QObject> oprsyms;
};

static KeyTransSymbols * syms = 0L;

// parser ----------------------------------------------------------------------------------
/* Syntax
   - Line :: [KeyName { ("+" | "-") ModeName } ":" (String|CommandName)] "\n"
   - Comment :: '#' (any but \n)*
*/

KeyTrans* KeyTrans::fromDevice(QString path, QIODevice &buf)
{
  KeyTrans* kt = new KeyTrans;
  kt->path = path;
  KeytabReader ktr(path,buf); ktr.parseTo(kt);
  return kt;
}


#define assertSyntax(Cond,Message) if (!(Cond)) { ReportError(Message); goto ERROR; }

void KeytabReader::parseTo(KeyTrans* kt)
{
  // Opening sequence

  buf->open(IO_ReadOnly);
  getCc();
  linno = 1;
  colno  = 1;
  getSymbol();

Loop:
  // syntax: ["key" KeyName { ("+" | "-") ModeName } ":" String/CommandName] ["#" Comment]
  if (sym == SYMName && !strcmp(res.latin1(),"keyboard")) // No tr
  {
    getSymbol(); assertSyntax(sym == SYMString, "Header expected") // No tr
    kt->hdr = res.latin1();
    getSymbol(); assertSyntax(sym == SYMEol, "Text unexpected") // No tr
    getSymbol();                   // eoln
    goto Loop;
  }
  if (sym == SYMName && !strcmp(res.latin1(),"key")) // No tr
  {
//printf("line %3d: ",startofsym);
    getSymbol(); assertSyntax(sym == SYMName, "Name expected") // No tr
    assertSyntax(syms->keysyms[res], "Unknown key name") // No tr
    int key = (int)syms->keysyms[res]-1;
//printf(" key %s (%04x)",res.latin1(),(int)syms->keysyms[res]-1);
    getSymbol(); // + - :
    int mode = 0;
    int mask = 0;
    while (sym == SYMOpr && (!strcmp(res.latin1(),"+") || !strcmp(res.latin1(),"-")))
    {
      bool on = !strcmp(res.latin1(),"+");
      getSymbol();
      // mode name
      assertSyntax(sym == SYMName, "Name expected") // No tr
      assertSyntax(syms->modsyms[res], "Unknown mode name") // No tr
      int bits = (int)syms->modsyms[res]-1;
      if (mask & (1 << bits))
      {
        fprintf(stderr,"%s(%d,%d): mode name used multible times.\n",path.ascii(),slinno,scolno); // No tr
      }
      else
      {
        mode |= (on << bits);
        mask |= (1 << bits);
      }
//printf(", mode %s(%d) %s",res.latin1(),(int)syms->modsyms[res]-1,on?"on":"off");
      getSymbol();
    }
    assertSyntax(sym == SYMOpr && !strcmp(res.latin1(),":"), "':' expected") // No tr
    getSymbol();
    // string or command
    assertSyntax(sym == SYMName || sym == SYMString,"Command or string expected") // No tr
    int cmd = 0;
    if (sym == SYMName)
    {
      assertSyntax(syms->oprsyms[res], "Unknown operator name") // No tr
      cmd = (int)syms->oprsyms[res]-1;
//printf(": do %s(%d)",res.latin1(),(int)syms->oprsyms[res]-1);
    }
    if (sym == SYMString)
    {
      cmd = CMD_send;
//printf(": send");
//for (unsigned i = 0; i < res.length(); i++)
//printf(" %02x(%c)",res.latin1()[i],res.latin1()[i]>=' '?res.latin1()[i]:'?');
    }
//printf(". summary %04x,%02x,%02x,%d\n",key,mode,mask,cmd);
    KeyTrans::KeyEntry* ke = kt->addEntry(slinno,key,mode,mask,cmd,res);
    if (ke)
    {
      fprintf(stderr,"%s(%d): keystroke already assigned in line %d.\n",path.ascii(),slinno,ke->ref); // No tr
    }
    getSymbol();
    assertSyntax(sym == SYMEol, "Unexpected text") // No tr
    goto Loop;
  }
  if (sym == SYMEol)
  {
    getSymbol();
    goto Loop;
  }

  assertSyntax(sym == SYMEof, "Undecodable Line") // No tr

  buf->close();
  return;

ERROR:
  while (sym != SYMEol && sym != SYMEof) getSymbol(); // eoln
  goto Loop;
}


KeyTrans* KeyTrans::defaultKeyTrans()
{
  QCString txt =
#include "default.keytab.h"
  ;
  QBuffer buf(txt);
  return fromDevice("[buildin]",buf);
}

KeyTrans* KeyTrans::fromFile(const char* path)
{
  QFile file(path);
  return fromDevice(path,file);
}

// local symbol tables ---------------------------------------------------------------------
// material needed for parsing the config file.
// This is incomplete work.

void KeyTransSymbols::defKeySym(const char* key, int val)
{
  keysyms.insert(key,(QObject*)(val+1));
}

void KeyTransSymbols::defOprSym(const char* key, int val)
{
  oprsyms.insert(key,(QObject*)(val+1));
}

void KeyTransSymbols::defModSym(const char* key, int val)
{
  modsyms.insert(key,(QObject*)(val+1));
}

void KeyTransSymbols::defOprSyms()
{
  // Modifier
  defOprSym("scrollLineUp",  CMD_scrollLineUp  );
  defOprSym("scrollLineDown",CMD_scrollLineDown);
  defOprSym("scrollPageUp",  CMD_scrollPageUp  );
  defOprSym("scrollPageDown",CMD_scrollPageDown);
  defOprSym("emitSelection", CMD_emitSelection );
  defOprSym("prevSession",   CMD_prevSession   );
  defOprSym("nextSession",   CMD_nextSession   );
}

void KeyTransSymbols::defModSyms()
{
  // Modifier
  defModSym("Shift",      BITS_Shift        ); // No tr
  defModSym("Control",    BITS_Control      ); // No tr
  defModSym("Alt",        BITS_Alt          );
  // Modes
  defModSym("BsHack",     BITS_BsHack       ); // deprecated
  defModSym("Ansi",       BITS_Ansi         ); // No tr
  defModSym("NewLine",    BITS_NewLine      );
  defModSym("AppCuKeys",  BITS_AppCuKeys    );
}

void KeyTransSymbols::defKeySyms()
{
  // Grey keys
  defKeySym("Escape",       Qt::Key_Escape      ); // No tr
  defKeySym("Tab",          Qt::Key_Tab         ); // No tr
  defKeySym("Backtab",      Qt::Key_Backtab     );
  defKeySym("Backspace",    Qt::Key_Backspace   ); // No tr
  defKeySym("Return",       Qt::Key_Return      ); // No tr
  defKeySym("Enter",        Qt::Key_Enter       ); // No tr
  defKeySym("Insert",       Qt::Key_Insert      ); // No tr
  defKeySym("Delete",       Qt::Key_Delete      ); // No tr
  defKeySym("Pause",        Qt::Key_Pause       ); // No tr
  defKeySym("Print",        Qt::Key_Print       ); // No tr
  defKeySym("SysReq",       Qt::Key_SysReq      );
  defKeySym("Home",         Qt::Key_Home        ); // No tr
  defKeySym("End",          Qt::Key_End         ); // No tr
  defKeySym("Left",         Qt::Key_Left        ); // No tr
  defKeySym("Up",           Qt::Key_Up          ); // No tr
  defKeySym("Right",        Qt::Key_Right       ); // No tr
  defKeySym("Down",         Qt::Key_Down        ); // No tr
  defKeySym("Prior",        Qt::Key_Prior       ); // No tr
  defKeySym("Next",         Qt::Key_Next        ); // No tr
  defKeySym("Shift",        Qt::Key_Shift       ); // No tr
  defKeySym("Control",      Qt::Key_Control     ); // No tr
  defKeySym("Meta",         Qt::Key_Meta        ); // No tr
  defKeySym("Alt",          Qt::Key_Alt         );
  defKeySym("CapsLock",     Qt::Key_CapsLock    );
  defKeySym("NumLock",      Qt::Key_NumLock     );
  defKeySym("ScrollLock",   Qt::Key_ScrollLock  );
  defKeySym("F1",           Qt::Key_F1          );
  defKeySym("F2",           Qt::Key_F2          );
  defKeySym("F3",           Qt::Key_F3          );
  defKeySym("F4",           Qt::Key_F4          );
  defKeySym("F5",           Qt::Key_F5          );
  defKeySym("F6",           Qt::Key_F6          );
  defKeySym("F7",           Qt::Key_F7          );
  defKeySym("F8",           Qt::Key_F8          );
  defKeySym("F9",           Qt::Key_F9          );
  defKeySym("F10",          Qt::Key_F10         );
  defKeySym("F11",          Qt::Key_F11         );
  defKeySym("F12",          Qt::Key_F12         );
  defKeySym("F13",          Qt::Key_F13         );
  defKeySym("F14",          Qt::Key_F14         );
  defKeySym("F15",          Qt::Key_F15         );
  defKeySym("F16",          Qt::Key_F16         );
  defKeySym("F17",          Qt::Key_F17         );
  defKeySym("F18",          Qt::Key_F18         );
  defKeySym("F19",          Qt::Key_F19         );
  defKeySym("F20",          Qt::Key_F20         );
  defKeySym("F21",          Qt::Key_F21         );
  defKeySym("F22",          Qt::Key_F22         );
  defKeySym("F23",          Qt::Key_F23         );
  defKeySym("F24",          Qt::Key_F24         );
  defKeySym("F25",          Qt::Key_F25         );
  defKeySym("F26",          Qt::Key_F26         );
  defKeySym("F27",          Qt::Key_F27         );
  defKeySym("F28",          Qt::Key_F28         );
  defKeySym("F29",          Qt::Key_F29         );
  defKeySym("F30",          Qt::Key_F30         );
  defKeySym("F31",          Qt::Key_F31         );
  defKeySym("F32",          Qt::Key_F32         );
  defKeySym("F33",          Qt::Key_F33         );
  defKeySym("F34",          Qt::Key_F34         );
  defKeySym("F35",          Qt::Key_F35         );
  defKeySym("Super_L",      Qt::Key_Super_L     );
  defKeySym("Super_R",      Qt::Key_Super_R     );
  defKeySym("Menu",         Qt::Key_Menu        ); // No tr
  defKeySym("Hyper_L",      Qt::Key_Hyper_L     );
  defKeySym("Hyper_R",      Qt::Key_Hyper_R     );

  // Regular keys
  defKeySym("Space",        Qt::Key_Space       ); // No tr
  defKeySym("Exclam",       Qt::Key_Exclam      );
  defKeySym("QuoteDbl",     Qt::Key_QuoteDbl    );
  defKeySym("NumberSign",   Qt::Key_NumberSign  );
  defKeySym("Dollar",       Qt::Key_Dollar      ); // No tr
  defKeySym("Percent",      Qt::Key_Percent     ); // No tr
  defKeySym("Ampersand",    Qt::Key_Ampersand   ); // No tr
  defKeySym("Apostrophe",   Qt::Key_Apostrophe  ); // No tr
  defKeySym("ParenLeft",    Qt::Key_ParenLeft   );
  defKeySym("ParenRight",   Qt::Key_ParenRight  );
  defKeySym("Asterisk",     Qt::Key_Asterisk    ); // No tr
  defKeySym("Plus",         Qt::Key_Plus        ); // No tr
  defKeySym("Comma",        Qt::Key_Comma       ); // No tr
  defKeySym("Minus",        Qt::Key_Minus       ); // No tr
  defKeySym("Period",       Qt::Key_Period      ); // No tr
  defKeySym("Slash",        Qt::Key_Slash       ); // No tr
  defKeySym("0",            Qt::Key_0           );
  defKeySym("1",            Qt::Key_1           );
  defKeySym("2",            Qt::Key_2           );
  defKeySym("3",            Qt::Key_3           );
  defKeySym("4",            Qt::Key_4           );
  defKeySym("5",            Qt::Key_5           );
  defKeySym("6",            Qt::Key_6           );
  defKeySym("7",            Qt::Key_7           );
  defKeySym("8",            Qt::Key_8           );
  defKeySym("9",            Qt::Key_9           );
  defKeySym("Colon",        Qt::Key_Colon       ); // No tr
  defKeySym("Semicolon",    Qt::Key_Semicolon   ); // No tr
  defKeySym("Less",         Qt::Key_Less        ); // No tr
  defKeySym("Equal",        Qt::Key_Equal       ); // No tr
  defKeySym("Greater",      Qt::Key_Greater     ); // No tr
  defKeySym("Question",     Qt::Key_Question    ); // No tr
  defKeySym("At",           Qt::Key_At          ); // No tr
  defKeySym("A",            Qt::Key_A           );
  defKeySym("B",            Qt::Key_B           );
  defKeySym("C",            Qt::Key_C           );
  defKeySym("D",            Qt::Key_D           );
  defKeySym("E",            Qt::Key_E           );
  defKeySym("F",            Qt::Key_F           );
  defKeySym("G",            Qt::Key_G           );
  defKeySym("H",            Qt::Key_H           );
  defKeySym("I",            Qt::Key_I           );
  defKeySym("J",            Qt::Key_J           );
  defKeySym("K",            Qt::Key_K           );
  defKeySym("L",            Qt::Key_L           );
  defKeySym("M",            Qt::Key_M           );
  defKeySym("N",            Qt::Key_N           );
  defKeySym("O",            Qt::Key_O           );
  defKeySym("P",            Qt::Key_P           );
  defKeySym("Q",            Qt::Key_Q           );
  defKeySym("R",            Qt::Key_R           );
  defKeySym("S",            Qt::Key_S           );
  defKeySym("T",            Qt::Key_T           );
  defKeySym("U",            Qt::Key_U           );
  defKeySym("V",            Qt::Key_V           );
  defKeySym("W",            Qt::Key_W           );
  defKeySym("X",            Qt::Key_X           );
  defKeySym("Y",            Qt::Key_Y           );
  defKeySym("Z",            Qt::Key_Z           );
  defKeySym("BracketLeft",  Qt::Key_BracketLeft );
  defKeySym("Backslash",    Qt::Key_Backslash   ); // No tr
  defKeySym("BracketRight", Qt::Key_BracketRight);
  defKeySym("AsciiCircum",  Qt::Key_AsciiCircum );
  defKeySym("Underscore",   Qt::Key_Underscore  ); // No tr
  defKeySym("QuoteLeft",    Qt::Key_QuoteLeft   );
  defKeySym("BraceLeft",    Qt::Key_BraceLeft   );
  defKeySym("Bar",          Qt::Key_Bar         ); // No tr
  defKeySym("BraceRight",   Qt::Key_BraceRight  );
  defKeySym("AsciiTilde",   Qt::Key_AsciiTilde  );
}

KeyTransSymbols::KeyTransSymbols()
{
  defModSyms();
  defOprSyms();
  defKeySyms();
}

// Global material -----------------------------------------------------------

static int keytab_serial = 0; //FIXME: remove,localize

static QIntDict<KeyTrans> * numb2keymap = 0L;
static QDict<KeyTrans>    * path2keymap = 0L;

KeyTrans* KeyTrans::find(int numb)
{
  KeyTrans* res = numb2keymap->find(numb);
  return res ? res : numb2keymap->find(0);
}

KeyTrans* KeyTrans::find(const char* path)
{
  KeyTrans* res = path2keymap->find(path);
  return res ? res : numb2keymap->find(0);
}

int KeyTrans::count()
{
  return numb2keymap->count();
}

void KeyTrans::addKeyTrans()
{
  this->numb = keytab_serial ++;
  numb2keymap->insert(numb,this);
  path2keymap->insert(path,this);
}

void KeyTrans::loadAll()
{
  if (!numb2keymap)
    numb2keymap = new QIntDict<KeyTrans>;
  if (!path2keymap)
    path2keymap = new QDict<KeyTrans>;
  if (!syms)
    syms = new KeyTransSymbols;

  defaultKeyTrans()->addKeyTrans();


  QString path = QPEApplication::qpeDir() + "etc/keytabs";
  QDir dir(path);
  QStringList lst = dir.entryList("*.keytab");

  for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
      QFile file(path + "/" + *it);
    KeyTrans* sc = KeyTrans::fromDevice(*it, file);
    if (sc) {
	sc->addKeyTrans();
    }
  }

}

// Debugging material -----------------------------------------------------------
/*
void TestTokenizer(QBuffer &buf)
{
  // opening sequence

  buf.open(IO_ReadOnly);
  cc = buf.getch();
  lineno = 1;

  // Test tokenizer

  while (getSymbol(buf)) ReportToken();

  buf.close();
}

void test()
{
  // Opening sequence

  QCString txt =
#include "default.keytab.h"
  ;
  QBuffer buf(txt);
  if (0) TestTokenizer(buf);
  if (1) { KeyTrans kt; kt.scanTable(buf); }
}
*/
