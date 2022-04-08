// Avisynth v2.5.  Copyright 2002 Ben Rudiak-Gould et al.
// http://www.avisynth.org

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.

#ifndef __ScriptParser_H__
#define __ScriptParser_H__

#include <avisynth.h>
#include "expression.h"
#include "tokenizer.h"
#include "script.h"


/********************************************************************
********************************************************************/



class ScriptParser
/**
  * Insert intelligent comment here
 **/
{
public:
  ScriptParser(IScriptEnvironment* _env, const char* _code, const char* _filename);

  PExpression Parse(void);

  enum {max_args=1024};
  // fixme: consider using vectors

private:
  IScriptEnvironment2* const env;
  Tokenizer tokenizer;
  const char* const code;
  const char* const filename;
  int loopDepth;    // how many loops are we in during parsing

  void Expect(int op, const char* msg);

  PExpression ParseFunctionDefinition(void);

  PExpression ParseBlock(bool braced, bool *empty);
  PExpression ParseStatement(bool* stop);
  PExpression ParseAssignment(void);
  PExpression ParseAssignmentWithRet(void);
  PExpression ParseConditional(void);
  PExpression ParseOr(void);
  PExpression ParseAnd(void);
  PExpression ParseComparison(void);
  PExpression ParseAddition(bool negationOnHold);
  PExpression ParseMultiplication(bool negationOnHold);
  PExpression ParseUnary(void);
  PExpression ParseOOP(void);
  PExpression ParseArray(PExpression context);
  PExpression ParseFunction(PExpression context);
  PExpression ParseCall(PExpression left, PExpression context, bool isArraySpecifier);
  PExpression ParseAtom(void);

  PExpression ParseIf(void);
  PExpression ParseWhile(void);
  PExpression ParseFor(void);

  // helper for ParseComparison
  int GetTokenAsComparisonOperator();
};

// The following allows us to write multi-character tokens as string literals.
// Originally, non-conformant multi-character characters were used.
// For example: int('012') == 0x303132. Hence, the rightmost character
// is placed at the least significant byte.
static constexpr int operator "" _i(const char s[], const size_t len) {
  int acc = 0;
  for (size_t i = 0; i < len; ++i)
    acc = (acc << 8) + (unsigned char) s[i];
  return acc;
}


#endif  // __ScriptParser_H__
