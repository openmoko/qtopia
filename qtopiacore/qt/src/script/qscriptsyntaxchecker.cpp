/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtScript module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qscriptsyntaxchecker_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptlexer_p.h"

namespace QScript {


SyntaxChecker::SyntaxChecker():
    tos(0),
    stack_size(0),
    state_stack(0)
{
}

SyntaxChecker::~SyntaxChecker()
{
    if (stack_size) {
        qFree(state_stack);
    }
}

bool SyntaxChecker::automatic(QScript::Lexer *lexer, int token) const
{
    return token == T_RBRACE || token == 0 || lexer->prevTerminator();
}

bool SyntaxChecker::parse(const QString &code)
{
  const int INITIAL_STATE = 0;
  QScript::Lexer lexer (/*engine=*/ 0);
  lexer.setCode(code, /*lineNo*/ 0);

  int yytoken = -1;
  int saved_yytoken = -1;

  reallocateStack();

  tos = 0;
  state_stack[++tos] = INITIAL_STATE;

  while (true)
    {
      if (yytoken == -1 && - TERMINAL_COUNT != action_index [state_stack [tos]])
        {
          if (saved_yytoken == -1)
            yytoken = lexer.lex();
          else
            {
              yytoken = saved_yytoken;
              saved_yytoken = -1;
            }
        }

      int act = t_action (state_stack [tos], yytoken);

      if (act == ACCEPT_STATE)
        return true;

      else if (act > 0)
        {
          if (++tos == stack_size)
            reallocateStack();

          state_stack [tos] = act;
          yytoken = -1;
        }

      else if (act < 0)
        {
          int r = - act - 1;

          tos -= rhs [r];
          act = state_stack [tos++];

          state_stack [tos] = nt_action (act, lhs [r] - TERMINAL_COUNT);
        }

      else
        {
          if (saved_yytoken == -1 && automatic (&lexer, yytoken) && t_action (state_stack [tos], T_AUTOMATIC_SEMICOLON) > 0)
            {
              saved_yytoken = yytoken;
              yytoken = T_SEMICOLON;
              continue;
            }

          break;
        }
    }

  return yytoken != 0;
}

} // namespace QScript

#endif // QT_NO_SCRIPT
