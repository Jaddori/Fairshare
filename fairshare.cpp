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

struct Hub
{
    string name;
    string ip;
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

#define ifOpenSocket( _socket, _expr )    \
    if( !OpenSocket( (_socket) ) ) \
    { \
    cout << "Failed to open socket." << endl; \
    _expr; \
    }

#define ifNetInit( _data, _expr )    \
    if( !NetInit( (_data) ) ) \
    { \
    cout << "Failed to initialize network data." << endl; \
    _expr; \
    }

#define ifNetBind( _socket, _port, _expr )      \
    if( !NetBind( (_socket), (_port) ) ) \
    { \
    cout << "Failed to bind socket." << endl; \
    _expr; \
    }

#define ifNetListen( _socket, _expr )    \
    if( !NetListen( (_socket) ) ) \
    { \
    cout << "Failed to listen on socket." << endl; \
    _expr; \
    }

#define ifNetConnect( _socket, _ip, _port, _expr )  \
    if( !NetConnect( (_socket), (_ip), (_port) ) ) \
    { \
    cout << "Failed to connect to \"" << (_ip) << ":" << (_port) << "\"." << endl; \
    _expr; \
    }

#define ifNetSend( _socket, _buf, _len, _expr )        \
    if( !NetSend( (_socket), (_buf), (_len) ) ) \
    { \
    cout << "Failed to send data." << endl; \
    _expr; \
    }

#define ifNetSendFile( _socket, _file, _expr ) \
    if( !NetSendFile( (_socket), (_file) ) ) \
    { \
    cout << "Failed to send file." << endl; \
    _expr; \
    }

#define ifNetRecvFile( _socket, _file, _expr ) \
    if( !NetRecvFile( (_socket), (_file) ) ) \
    { \
    cout << "Failed to recv file." << endl; \
    _expr; \
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
    
    return ( select( s+1, &readSet, 0, 0, &timeout ) > 0 );
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

inline int NetRecv( NetSocket s, char* buf, int len )
{
    return recv( s, buf, len, 0 );
}

#define ifNetRecv( _recv, _expr ) \
    if( _recv == SOCKET_ERROR ) \
    { \
    cout << "Failed to receive network data." << endl; \
    _expr; \
    }

bool NetSendFile( NetSocket s, const char* file )
{
    bool result = false;
    
    HANDLE filehandle = CreateFile( file,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    0,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    0 );

    if( filehandle != INVALID_HANDLE_VALUE )
    {
        LARGE_INTEGER filesize;
        GetFileSizeEx( filehandle, &filesize );

        unsigned long remaining = filesize.QuadPart;
        char filebuf[1024];
        
        do
        {
            int sendsize = ( remaining > 1024 ? 1024 : remaining );

            DWORD bytesRead;
            ReadFile( filehandle,
                      filebuf,
                      sendsize,
                      &bytesRead,
                      0 );

            ifNetSend( s, filebuf, sendsize, return false );
            
            remaining -= sendsize;
        } while( remaining > 0 );
        
        CloseHandle( filehandle );
        result = true;
    }

    return result;
}

bool NetRecvFile( NetSocket s, const char* file )
{
    bool result = false;
    
    HANDLE filehandle = CreateFile( file,
                                    GENERIC_WRITE,
                                    0,
                                    0,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    0 );

    if( filehandle != INVALID_HANDLE_VALUE )
    {
        char filebuf[1024];
        int r;
        
        do
        {
            r = NetRecv( s, filebuf, 1024 );
            ifNetRecv( r, return false );

            DWORD bytesWritten;
            WriteFile( filehandle,
                       filebuf,
                       r,
                       &bytesWritten,
                       0 );
        } while( r >= 1024 );
        
        CloseHandle( filehandle );
        result = true;
    }

    return result;
}

#elif LINUX

#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// threading
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

// sockets
#define NetData int // not really used in POSIX
#define NetSocket int

struct Net
{
    NetData data;
    NetSocket socket;
};

inline bool OpenSocket( NetSocket* s )
{
    return ( ( *s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) != -1 );
}

inline void CloseSocket( NetSocket s )
{
    close( s );
}

inline bool NetInit( NetData* data )
{
    return true; // I know, I know...
}

inline void NetShutdown()
{
}

inline bool NetBind( NetSocket s, int port )
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons( port );

    return ( bind( s, (struct sockaddr*)&addr, sizeof(addr) ) != -1 );
}

inline bool NetListen( NetSocket s )
{
    return ( listen( s, DEFAULT_BACKLOG ) != -1 );
}

inline bool NetConnect( NetSocket s, const char* ip, int port )
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr( ip );
    addr.sin_port = htons( port );
    
    return ( connect( s, (struct sockaddr*)&addr, sizeof(addr) ) != -1 );
}

inline bool NetSelect( NetSocket s )
{
    fd_set readSet;
    FD_ZERO( &readSet );
    FD_SET( s, &readSet );

    timeval timeout;
    timeout.tv_sec = DEFAULT_SELECT_TIMEOUT;
    timeout.tv_usec = 0;

    return ( select( s+1, &readSet, 0, 0, &timeout ) > 0 );
}

inline NetSocket NetAccept( NetSocket s )
{
    return ( accept( s, 0, 0 ) );
}

inline bool NetValidSocket( NetSocket s )
{
    return ( s >= 0 );
}

inline bool NetSend( NetSocket s, const char* buf, int len = -1 )
{
    if( len < 0 )
    {
        len = strlen( buf );
    }

    return ( send( s, buf, len, 0 ) != -1 );
}

inline int NetRecv( NetSocket s, char* buf, int len )
{
    return recv( s, buf, len, 0 );
}

#define ifNetRecv( _recv, _expr ) \
    if( _recv == -1 ) \
    { \
    cout << "Failed to receive network data." << endl; \
    _expr; \
    }

#else

#endif
