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


#ifndef __Tokenizer_H__
#define __Tokenizer_H__

#include <avisynth.h>
#include <vector>

/*********************************************************
*********************************************************/


class Tokenizer
/**
  * Breaks up scripts into tokens
 **/
{
public:
  Tokenizer(const char* pc, IScriptEnvironment* _env);
  explicit Tokenizer(Tokenizer* old);

  void NextToken();

  inline bool IsIdentifier() const { return type == 'd'; }
  inline bool IsOperator() const { return type == 'o'; }
  inline bool IsInt() const { return type == 'i'; }
  inline bool IsFloat() const { return type == 'f'; }
  inline bool IsString() const { return type == 's'; }
#ifdef ARRAYS_AT_TOKENIZER_LEVEL
  inline bool IsArray() const { return type == 'a'; }
#endif
  inline bool IsNewline() const { return type == 'n'; }
  inline bool IsEOF() const { return type == 0; }

  bool IsIdentifier(const char* id) const;
  inline bool IsOperator(int o) const
    { return IsOperator() && o == op; }

  const char* AsIdentifier() const { AssertType('d'); return identifier; }
  int AsOperator() const { AssertType('o'); return op; }
  int AsInt() const { AssertType('i'); return integer; }
  float AsFloat() const { AssertType('f'); return floating_pt; }
  const char* AsString() const { AssertType('s'); return string; }
#ifdef ARRAYS_AT_TOKENIZER_LEVEL
  std::vector<AVSValue>* AsArray() const { AssertType('a'); return array2; }
#endif

  int GetLine() const { return line; }
  int GetColumn(const char* start_of_string) const;

private:
  void SkipWhitespace();
  void SkipNewline();
  void GetNumber();
  void AssertType(char expected_type) const;
  void SetToOperator(int o);

  IScriptEnvironment* const env;
  const char* token_start;
  const char* pc;
  int line;
  char type;   // i'd'entifier, 'o'perator, 'i'nt, 'f'loat, 's'tring, 'n'ewline, 'a'rray, 0=eof
  union
  {
    const char* identifier;
    const char* string;
    int op;   // '+', '++', '.', ',', '(', ')','[', ']', 0=eoln
    int integer;
    float floating_pt;
#ifdef ARRAYS_AT_TOKENIZER_LEVEL
    // not used now, finally arrays are implemented with helper script functions
    std::vector<AVSValue>* array2;
#endif
  };
};



/**** Helper functions ****/

void ThrowTypeMismatch(char expected, char actual, IScriptEnvironment* env);


#endif  // __Tokenizer_H__