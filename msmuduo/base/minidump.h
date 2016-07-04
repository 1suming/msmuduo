#ifndef _dbg_h
#define _dbg_h



#ifdef WIN



#include <windows.h>  
#include <DbgHelp.h>  
#include <stdlib.h>  
#include<tchar.h> //_T
#pragma comment(lib, "dbghelp.lib")  

#ifndef _M_IX86  
#error "The following code only works for x86!"  
#endif  


inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName);
inline BOOL CALLBACK MiniDumpCallback(PVOID                            pParam,
	const PMINIDUMP_CALLBACK_INPUT   pInput,
	PMINIDUMP_CALLBACK_OUTPUT        pOutput);
inline void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName);
 
LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo);

void DisableSetUnhandledExceptionFilter();
void InitMinDump();


#endif



#endif