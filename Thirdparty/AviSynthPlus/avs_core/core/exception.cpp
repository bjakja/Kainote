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

/* First cut for breaking out system exceptions from the evil and most
 * unhelpful "Unrecognized exception!".
 *
 * This initial version just decodes the exception code, latter if one
 * is so inclined the info structure could be pulled apart and the
 * state of the machine presented. So far just knowing "Integer Divide
 * by Zero" was happening has been a real boon.
 */

#include "exception.h"

#ifdef AVS_WINDOWS
    #include <avs/win.h>
#else
    #include <avs/posix.h>
#endif

#include <cassert>

#ifdef AVS_WINDOWS
static const char * StringSystemError(const unsigned code)
{
  switch (code) {
  case STATUS_GUARD_PAGE_VIOLATION:      // 0x80000001
    return "System exception - Guard Page Violation";
  case STATUS_DATATYPE_MISALIGNMENT:     // 0x80000002
    return "System exception - Datatype Misalignment";
  case STATUS_BREAKPOINT:                // 0x80000003
    return "System exception - Breakpoint";
  case STATUS_SINGLE_STEP:               // 0x80000004
    return "System exception - Single Step";
//------------------------------------------------------------------------------
  case STATUS_ACCESS_VIOLATION:          // 0xc0000005
    return "System exception - Access Violation";
  case STATUS_IN_PAGE_ERROR:             // 0xc0000006
    return " System exception - In Page Error";
  case STATUS_INVALID_HANDLE:            // 0xc0000008
    return "System exception - Invalid Handle";
  case STATUS_NO_MEMORY:                 // 0xc0000017
    return "System exception - No Memory";
  case STATUS_ILLEGAL_INSTRUCTION:       // 0xc000001d
    return "System exception - Illegal Instruction";
  case STATUS_NONCONTINUABLE_EXCEPTION:  // 0xc0000025
    return "System exception - Noncontinuable Exception";
  case STATUS_INVALID_DISPOSITION:       // 0xc0000026
    return "System exception - Invalid Disposition";
  case STATUS_ARRAY_BOUNDS_EXCEEDED:     // 0xc000008c
    return "System exception - Array Bounds Exceeded";
  case STATUS_FLOAT_DENORMAL_OPERAND:    // 0xc000008d
    return "System exception - Float Denormal Operand";
  case STATUS_FLOAT_DIVIDE_BY_ZERO:      // 0xc000008e
    return "System exception - Float Divide by Zero";
  case STATUS_FLOAT_INEXACT_RESULT:      // 0xc000008f
    return "System exception - Float Inexact Result";
  case STATUS_FLOAT_INVALID_OPERATION:   // 0xc0000090
    return "System exception - Float Invalid Operation";
  case STATUS_FLOAT_OVERFLOW:            // 0xc0000091
    return "System exception - Float Overflow";
  case STATUS_FLOAT_STACK_CHECK:         // 0xc0000092
    return "System exception - Float Stack Check";
  case STATUS_FLOAT_UNDERFLOW:           // 0xc0000093
    return "System exception - Float Underflow";
  case STATUS_INTEGER_DIVIDE_BY_ZERO:    // 0xc0000094
    return "System exception - Integer Divide by Zero";
  case STATUS_INTEGER_OVERFLOW:          // 0xc0000095
    return "System exception - Integer Overflow";
  case STATUS_PRIVILEGED_INSTRUCTION:    // 0xc0000096
    return "System exception - Privileged Instruction";
  case STATUS_STACK_OVERFLOW:            // 0xc00000fd
    return "System exception - Stack Overflow";
//------------------------------------------------------------------------------
  case 0xC0000135:                       // 0xc0000135
    return "DLL Not Found";
  case 0xC0000142:                       // 0xc0000142
    return "DLL Initialization Failed";
  case 0xC06d007E:                       // 0xc06d007e
    return "Delay-load Module Not Found";
  case 0xC06d007F:                       // 0xc06d007e
    return "Delay-load Proceedure Not Found";
//------------------------------------------------------------------------------
  default:
    assert(0);
    return "Unrecognized system exception!";
  }

  // Should never-ever get here
  assert(0);
}

// Seh is Windows-only, right?
void SehTranslatorFunction(unsigned int code, struct _EXCEPTION_POINTERS *record)
{
  throw SehException(code, record->ExceptionRecord->ExceptionAddress, StringSystemError(code));
}
#endif
