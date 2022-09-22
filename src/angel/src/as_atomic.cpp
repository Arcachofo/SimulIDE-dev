/*
   AngelCode Scripting Library
   Copyright (c) 2003-2014 Andreas Jonsson

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/
 
//
// as_atomic.cpp
//
// The implementation of the atomic class for thread safe reference counting
//

#include "as_atomic.h"

BEGIN_AS_NAMESPACE

asCAtomic::asCAtomic()
{
	value = 0;
}

asDWORD asCAtomic::get() const
{
	// A very high ref count is highly unlikely. It most likely a problem with
	// memory that has been overwritten or is being accessed after it was deleted.
	asASSERT(value < 1000000);

	return value;
}

void asCAtomic::set(asDWORD val)
{
	// A very high ref count is highly unlikely. It most likely a problem with
	// memory that has been overwritten or is being accessed after it was deleted.
	asASSERT(value < 1000000);

	value = val;
}

asDWORD asCAtomic::atomicInc()
{
	// A very high ref count is highly unlikely. It most likely a problem with
	// memory that has been overwritten or is being accessed after it was deleted.
	asASSERT(value < 1000000);

	return asAtomicInc((int&)value);
}

asDWORD asCAtomic::atomicDec()
{
	// A very high ref count is highly unlikely. It most likely a problem with
	// memory that has been overwritten or is being accessed after it was deleted.
	asASSERT(value < 1000000);

	return asAtomicDec((int&)value);
}

//
// The following code implements the atomicInc and atomicDec on different platforms
//

int asAtomicInc(int &value) { return ++value; }
int asAtomicDec(int &value){ return --value; }

END_AS_NAMESPACE

