/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Tunder $
   $Notice: (C) Copyright 2014 by SpaceCat, Inc. All Rights Reserved. $
   ======================================================================== */

// fairshare.cpp

#define DEFAULT_PORT 12750
#define DEFAULT_BACKLOG 3
#define DEFAULT_SELECT_TIMEOUT 1

struct Config
{
    string username;
    string folder;
    int port;
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

#include <winsock2.h>
#include <windows.h>

inline void SleepSeconds( DWORD seconds )
{
    Sleep( seconds*1000 );
}

// threading
#define ThreadHandle HANDLE
#define ThreadReturnType DWORD WINAPI
#define ThreadArgs LPVOID
#define ThreadFunc LPTHREAD_START_ROUTINE

inline ThreadHandle MakeThread( ThreadFunc func, ThreadArgs args )
{
    ThreadHandle result = CreateThread( 0, 0, func, args, 0, 0 );
    return result;
}

inline void ThreadWait( ThreadHandle threadHandle )
{
    WaitForSingleObject( threadHandle, INFINITE );
}

// sockets
#define NetData WSADATA
#define NetSocket SOCKET

struct Net
{
    NetData data;
    NetSocket socket;
};

inline bool OpenSocket( NetSocket* s )
{
    return ( ( *s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) != INVALID_SOCKET );
}

inline void CloseSocket( NetSocket s )
{
    closesocket( s );
}

inline bool NetInit( NetData* data )
{
    return ( WSAStartup( MAKEWORD( 2, 2 ), data ) == 0 );
}

inline void NetShutdown()
{
    WSACleanup();
}

inline bool NetBind( NetSocket s, int port )
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons( port );
    
    return ( bind( s, (struct sockaddr*)&addr, sizeof(sockaddr) ) == 0 );
}

inline bool NetListen( NetSocket s )
{
    return ( listen( s, DEFAULT_BACKLOG ) == 0 );
}

inline bool NetConnect( NetSocket s, const char* ip, int port )
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr( ip );
    addr.sin_port = htons( port );
    
    return ( connect( s, (struct sockaddr*)&addr, sizeof(addr) ) != SOCKET_ERROR );
}

inline bool NetSelect( NetSocket s )
{
    fd_set readSet;
    FD_ZERO( &readSet );
    FD_SET( s, &readSet );

    timeval timeout;
    timeout.tv_sec = DEFAULT_SELECT_TIMEOUT;
    timeout.tv_usec = 0;
    
    return ( select( s, &readSet, 0, 0, &timeout ) == 1 );
}

inline NetSocket NetAccept( NetSocket s )
{
    return ( accept( s, 0, 0 ) );
}

inline bool NetValidSocket( NetSocket s )
{
    return ( s != SOCKET_ERROR );
}

inline bool NetSend( NetSocket s, const char* buf, int len = -1 )
{
    if( len < 0 )
    {
        len = strlen( buf );
    }

    return ( send( s, buf, len, 0 ) != SOCKET_ERROR );
}

inline bool NetRecv( NetSocket s, char* buf, int len )
{
    return ( recv( s, buf, len, 0 ) != SOCKET_ERROR );
}

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
