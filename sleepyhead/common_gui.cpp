/* Common GUI Functions Implementation
 *
 * Copyright (c) 2011-2016 Mark Watkins <jedimark@users.sourceforge.net>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of the Linux
 * distribution for more details. */

#include "common_gui.h"

#ifndef BUILD_WITH_MSVC
# include <unistd.h>
#endif

using namespace SleepyHead::Common;

Qt::DayOfWeek firstDayOfWeekFromLocale()
{
    return QLocale::system().firstDayOfWeek();
}

//#include <windows.h>
//#include <tchar.h>

//typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

//LPFN_ISWOW64PROCESS fnIsWow64Process;

//BOOL IsWow64()
//{
//    BOOL bIsWow64 = FALSE;

//    //IsWow64Process is not available on all supported versions of Windows.
//    //Use GetModuleHandle to get a handle to the DLL that contains the function
//    //and GetProcAddress to get a pointer to the function if available.

//    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
//        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

//    if(NULL != fnIsWow64Process)
//    {
//        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
//        {
//            //handle error
//        }
//    }
//    return bIsWow64;
//}

