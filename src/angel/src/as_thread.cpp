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

// Functions for multi threading support

#include "as_config.h"
#include "as_thread.h"
#include "as_atomic.h"

BEGIN_AS_NAMESPACE

//=== Singleton ========================================================
static asCThreadManager *threadManager = 0;
//======================================================================

// Global API functions
extern "C"
{
AS_API int asThreadCleanup(){ return asCThreadManager::CleanupLocalData(); }
AS_API asIThreadManager *asGetThreadManager() { return threadManager; }
AS_API int asPrepareMultithread(asIThreadManager *externalThreadMgr) { return asCThreadManager::Prepare(externalThreadMgr); }
AS_API void asUnprepareMultithread() { asCThreadManager::Unprepare(); }
}

//======================================================================

asCThreadManager::asCThreadManager()
{
	// We're already in the critical section when this function is called
	tld = 0;
	refCount = 1;
}

int asCThreadManager::Prepare(asIThreadManager *externalThreadMgr)
{
	// Don't allow an external thread manager if there
	// is already a thread manager defined
    if( externalThreadMgr && threadManager ) return asINVALID_ARG;

	// The critical section cannot be declared globally, as there is no
	// guarantee for the order in which global variables are initialized
	// or uninitialized.

	// For this reason it's not possible to prevent two threads from calling
	// AddRef at the same time, so there is a chance for a race condition here.

	// To avoid the race condition when the thread manager is first created,
	// the application must make sure to call the global asPrepareForMultiThread()
	// in the main thread before any other thread creates a script engine.
	if( threadManager == 0 && externalThreadMgr == 0 )
		threadManager = asNEW(asCThreadManager);
	else
	{
		// If an application uses different dlls each dll will get it's own memory
		// space for global variables. If multiple dlls then uses AngelScript's
		// global thread support functions it is then best to share the thread
		// manager to make sure all dlls use the same critical section.
        if( externalThreadMgr ) threadManager = reinterpret_cast<asCThreadManager*>(externalThreadMgr);

        threadManager->refCount++;
    }
    return 0; // Success
}

void asCThreadManager::Unprepare()
{
	asASSERT(threadManager);

    if( threadManager == 0 ) return;

	if( --threadManager->refCount == 0 )
	{
		// Make sure the local data is destroyed, at least for the current thread
		CleanupLocalData();

		// As the critical section will be destroyed together
		// with the thread manager we must first clear the global
		// variable in case a new thread manager needs to be created;
		asCThreadManager *mgr = threadManager;
        threadManager = 0;

		asDELETE(mgr,asCThreadManager);
    }
}

asCThreadManager::~asCThreadManager()
{
    if( tld ) asDELETE(tld,asCThreadLocalData);
	tld = 0;
}

int asCThreadManager::CleanupLocalData()
{
    if( threadManager == 0 ) return 0;

	if( threadManager->tld )
	{
		if( threadManager->tld->activeContexts.GetLength() == 0 )
		{
			asDELETE(threadManager->tld,asCThreadLocalData);
			threadManager->tld = 0;
		}
        else return asCONTEXT_ACTIVE;
	}
	return 0;
}

asCThreadLocalData *asCThreadManager::GetLocalData()
{
    if( threadManager == 0 ) return 0;
    if( threadManager->tld == 0 ) threadManager->tld = asNEW(asCThreadLocalData)();
	return threadManager->tld;
}

//=========================================================================

asCThreadLocalData::asCThreadLocalData(){}
asCThreadLocalData::~asCThreadLocalData(){}

END_AS_NAMESPACE
