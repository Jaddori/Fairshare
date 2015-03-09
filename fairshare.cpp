/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Tunder $
   $Notice: (C) Copyright 2014 by SpaceCat, Inc. All Rights Reserved. $
   ======================================================================== */

// fairshare.cpp

#define DEFAULT_PORT 12750

struct Config
{
    string username;
    string folder;
};

inline void StrSplit( const string& str, vector<string>& buf, char delimiter )
{
    size_t first = 0;
    size_t last = str.find_first_of( delimiter );

    while( first != last )
    {
        if( last == string::npos )
        {
            buf.push_back( str.substr( first, str.size()-first ) );
            break;
        }
        else
        {        
            buf.push_back( str.substr( first, last-first ) );
        }

        first = last+1;
        last = str.find_first_of( delimiter, first );
    }
}

#if WIN32

#include <windows.h>

// server
#define ThreadHandle HANDLE
#define ThreadReturnType DWORD WINAPI
#define ThreadArgs LPVOID
#define ThreadFunc LPTHRED_START_ROUTINE

inline ThreadHandle MakeThread( ThreadFunc func, ThreadArgs args )
{
    ThreadHandle result = CreateThread( 0, 0, func, args, 0, 0 );
    return result;
}

inline void ThreadWait( ThreadHandle threadHandle )
{
    WaitForSingleObject( threadHandle, INFINITE );
}

inline void SleepSeconds( DWORD seconds )
{
    Sleep( seconds*1000 );
}

// client

#elif LINUX

#include <unistd.h>
#include <pthread.h>

// server
#define ThreadHandle pthread_t
#define ThreadReturnType void*
#define ThreadArgs void*
typedef void*(*ThreadFunc)(void*);

inline ThreadHandle MakeThread( ThreadFunc func, ThreadArgs args )
{
    ThreadHandle result;
    int err = pthread_create( &result, 0, func, args );
    // TODO: Add error handling
    return result;
}

inline void ThreadWait( ThreadHandle threadHandle )
{
    pthread_join( threadHandle, 0 );
}

inline void SleepSeconds( int seconds )
{
    sleep( seconds );
}

// client

#else

#endif
