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
#define HUBFILE_UPDATE_DELAY 10
#define FILESIZE_THRESHOLD 1024*1024 // 1 mb

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

void StrPrint( const char* str )
{
    cout << str << endl;
}

void StrSplit( const string& str, vector<string>& buf, char delimiter )
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

void StrCompare( const vector<string>& a,
                 const vector<string>& b,
                 vector<string>& dif )
{
    for( vector<string>::const_iterator ait = a.begin(); ait != a.end(); ait++ )
    {
        bool found = false;
        for( vector<string>::const_iterator bit = b.begin(); bit != b.end() && !found; bit++ )
        {
            if( ait->compare( *bit ) == 0 )
            {
                found = true;
            }
        }

        if( !found )
        {
            dif.push_back( *ait );
        }
    }
}

bool ReadWholeFile( const char* file, vector<string>& buf )
{
    bool result = false;
    
    ifstream stream( file );
    if( stream.is_open() )
    {
        string line;
        while( stream )
        {
            getline( stream, line );
            if( stream )
            {
                buf.push_back( line );
            }
        }
        stream.close();
        result = true;
    }

    return result;
}

bool WriteWholeFile( const char* file, const vector<string>& buf )
{
    bool result = false;

    ofstream stream( file );
    if( stream.is_open() )
    {
        for( vector<string>::const_iterator it = buf.begin(); it != buf.end(); it++ )
        {
            stream << *it << '\n';
        }
        stream.close();
        
        result = true;
    }

    return result;
}

#define ifReadWholeFile( _file, _buf, _expr )     \
    if( !ReadWholeFile( (_file), (_buf) ) ) \
    { \
    cout << "Failed to read file \"" << (_file) << "\"." << endl; \
    _expr; \
    }

#define ifWriteWholeFile( _file, _buf, _expr ) \
    if( !WriteWholeFile( (_file), (_buf) ) ) \
    { \
    cout << "Failed to write file \"" << (_file) << "\"." << endl; \
    _expr; \
    }

#define ifFSDirectoryGetFiles( _path, _buf, _expr ) \
    if( !FSDirectoryGetFiles( (_path), (_buf) ) ) \
    { \
    cout << "Failed to open directory \"" << (_path) << "\"." << endl; \
    _expr; \
    }

#define ifFSWriteFile( _filehandle, _buf, _size, _expr ) \
    if( !FSWriteFile( (_filehandle), (_buf), (_size) ) ) \
    { \
    cout << "Failed to write to file." << endl; \
    _expr; \
    }

#define ifFSReadFile( _filehandle, _buf, _size, _expr ) \
    if( !FSReadFile( (_filehandle), (_buf), (_size) ) ) \
    { \
    cout << "Failed to read from file." << endl; \
    _expr; \
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

#define ifNetSendFileHandle( _socket, _filehandle, _size, _expr )  \
    if( !NetSendFileHandle( (_socket), (_filehandle), (_size) ) ) \
    { \
    cout << "Failed to send filehandle." << endl; \
    _expr; \
    }

#define ifNetSendFile( _socket, _file, _expr ) \
    if( !NetSendFile( (_socket), (_file) ) ) \
    { \
    cout << "Failed to send file." << endl; \
    _expr; \
    }

#define ifNetRecvFileHandle( _socket, _filehandle, _expr ) \
    if( !NetRecvFileHandle( (_socket), (_filehandle) ) ) \
    { \
    cout << "Failed to receive filehandle." << endl; \
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

// IO
#define FileHandle HANDLE
#define FSWrite GENERIC_WRITE, 0, CREATE_ALWAYS
#define FSRead GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING

bool FSDirectoryGetFiles( const string& path, vector<string>& buf )
{
    bool result = false;
    
    string p = path + string( "\\*" );
    WIN32_FIND_DATA fd;
    
    HANDLE findhandle = FindFirstFileA( p.c_str(), &fd );
    if( findhandle != INVALID_HANDLE_VALUE )
    {
        do
        {
            if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            {
                continue;
            }

            buf.push_back( string(fd.cFileName) );
        } while( FindNextFile( findhandle, &fd ) );

        FindClose( findhandle );
        result = true;
    }

    return result;
}

uint64_t FSGetFileSize( FileHandle handle )
{
    LARGE_INTEGER result;
    GetFileSizeEx( handle, &result );
    return result.QuadPart;
}

FileHandle FSOpenFile( const char* path, DWORD access, DWORD share, DWORD disp )
{
    FileHandle result = CreateFile( path,
                                    access,
                                    share,
                                    0,
                                    disp,
                                    FILE_ATTRIBUTE_NORMAL,
                                    0 );
    return result;
}

void FSCloseFile( FileHandle handle )
{
    CloseHandle( handle );
}

bool FSWriteFile( FileHandle handle, const char* buf, int size )
{
    DWORD bytesWritten;
    WriteFile( handle,
               buf,
               size,
               &bytesWritten,
               0 );
    return ( bytesWritten == size );
}

bool FSReadFile( FileHandle handle, char* buf, int size )
{
    DWORD bytesRead;
    ReadFile( handle,
              buf,
              size,
              &bytesRead,
              0 );
    return ( bytesRead > 0 );
}

bool FSValidHandle( FileHandle handle )
{
    bool result = ( handle != INVALID_HANDLE_VALUE );
    return result;
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

bool NetSendFileHandle( NetSocket s, FileHandle filehandle, uint64_t size )
{
    uint64_t remaining = size;
    char filebuf[1024];
        
    do
    {
        int sendsize = ( remaining > 1024 ? 1024 : (int)remaining );

        DWORD bytesRead;
        ReadFile( filehandle,
                  filebuf,
                  sendsize,
                  &bytesRead,
                  0 );

        ifNetSend( s, filebuf, sendsize, return false );
            
        remaining -= sendsize;
    } while( remaining > 0 );

    return true;
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

        uint64_t remaining = filesize.QuadPart;
        char filebuf[1024];
        
        do
        {
            int sendsize = ( remaining > 1024 ? 1024 : (int)remaining );

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

bool NetRecvFileHandle( NetSocket s, FileHandle filehandle )
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

    return true;
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
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

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

// IO
#define FileHandle int
#define FileDisposition int
#define FSWrite O_WRONLY | O_CREAT
#define FSRead O_RDONLY

bool FSDirectoryGetFiles( const string& path, vector<string>& buf )
{
    bool result = false;

    DIR* dir = opendir( path.c_str() );
    if( dir != 0 )
    {
        struct dirent* dirinfo;
        while( dirinfo = readdir( dir ) )
        {
            if( dirinfo->d_type == DT_REG ) // this is a regular file
            {
                buf.push_back( string(dirinfo->d_name) );
            }
        }
        closedir( dir );
        result = true;
    }

    return result;
}

uint64_t FSGetFileSize( FileHandle handle )
{
    uint64_t result = 0;
    
    struct stat filestats;
    if( fstat( handle, &filestats ) >= 0 )
    {
        result = filestats.st_size;
    }

    return result;
}

FileHandle FSOpenFile( const char* path, FileDisposition disp )
{
    FileHandle result = open( path, disp, S_IRWXU );
    return result;
}

void FSCloseFile( FileHandle handle )
{
    close( handle );
}

bool FSWriteFile( FileHandle handle, const char* buf, int size )
{
    int bytesWritten = write( handle, buf, size );
    return ( bytesWritten == size );
}

bool FSReadFile( FileHandle handle, char* buf, int size )
{
    int bytesRead = read( handle, buf, size );
    return ( bytesRead == size );
}

bool FSValidHandle( FileHandle handle )
{
    bool result = ( handle >= 0 );
    return result;
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

bool NetSendFileHandle( NetSocket s, FileHandle filehandle, uint64_t size )
{
    uint64_t remaining = size;
    char filebuf[1024];

    do
    {
        int sendsize = ( remaining > 1024 ? 1024 : (int)remaining );

        if( read( filehandle, filebuf, sendsize ) < 0 )
        {
            return false;
        }

        ifNetSend( s, filebuf, sendsize, return false );
        remaining -= sendsize;
    } while( remaining > 0 );

    return true;
}

bool NetSendFile( NetSocket s, const char* file )
{
    int filehandle = open( file, O_RDONLY | O_CREAT, S_IRWXU );
    if( filehandle < 0 )
    {
        return false;
    }

    struct stat filestats;
    if( fstat( filehandle, &filestats ) < 0 )
    {
        return false;
    }

    uint64_t remaining = filestats.st_size;
    char filebuf[1024];

    do
    {
        int sendsize = ( remaining > 1024 ? 1024 : (int)remaining );

        if( read( filehandle, filebuf, sendsize ) < 0 )
        {
            return false;
        }

        ifNetSend( s, filebuf, sendsize, return false );
        remaining -= sendsize;
    } while( remaining > 0 );
                
    close( filehandle );

    return true;
}

bool NetRecvFile( NetSocket s, const char* file )
{
    int filehandle = open( file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU );

    if( filehandle >= 0 )
    {
        char filebuf[1024];
        int r;

        do
        {
            r = NetRecv( s, filebuf, 1024 );
            ifNetRecv( r, return false );

            if( write( filehandle, filebuf, r ) < 0 )
            {
                return false;
            }
        } while( r >= 1024 );

        close( filehandle );
    }

    return true;
}

#else // MacOS

#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

// Threading
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

inline void ThreadWait( ThreadHandle handle )
{
    pthread_join( handle, 0 );
}

inline void SleepSeconds( int seconds )
{
    sleep( seconds );
}

// IO
#define FileHandle int
#define FileDisposition int
#define FSWrite O_WRONLY | O_CREAT
#define FSRead O_RDONLY

bool FSDirectoryGetFiles( const string& path, vector<string>& buf )
{
    bool result = false;
    
    DIR* dir = opendir( path.c_str() );
    if( dir != 0 )
    {
        struct dirent* dirinfo;
        while( ( dirinfo = readdir( dir ) ) )
        {
            if( dirinfo->d_type == DT_REG ) // this is a regular file
            {
                buf.push_back( string(dirinfo->d_name) );
            }
        }
        
        closedir( dir );
        result = true;
    }
    
    return result;
}

uint64_t FSGetFileSize( FileHandle handle )
{
    uint64_t result = 0;
    
    struct stat filestat;
    if( fstat( handle, &filestat ) >= 0 )
    {
        result = filestat.st_size;
    }
    
    return result;
}

FileHandle FSOpenFile( const char* path, FileDisposition disp )
{
    FileHandle handle = open( path, disp, S_IRWXU );
    return handle;
}

void FSCloseFile( FileHandle handle )
{
    close( handle );
}

bool FSWriteFile( FileHandle handle, const char* buf, int size )
{
    int bytesWritten = (int)write( handle, buf, size );
    return ( bytesWritten == size );
}

bool FSReadFile( FileHandle handle, char* buf, int size )
{
    int bytesRead = (int)read( handle, buf, size );
    return ( bytesRead == size );
}

bool FSValidHandle( FileHandle handle )
{
    return ( handle >= 0 );
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
    return true;
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
    
    int err = ::bind( s, (struct sockaddr*)&addr, sizeof(addr) );
    return ( err != -1 );
}

inline bool NetListen( NetSocket s )
{
    int err = listen( s, DEFAULT_BACKLOG );
    return ( err != -1 );
}

inline bool NetConnect( NetSocket s, const char* ip, int port )
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr( ip );
    addr.sin_port = htons( port );
    
    int err = connect( s, (struct sockaddr*)&addr, sizeof(addr) );
    return ( err != -1 );
}

inline bool NetSelect( NetSocket s )
{
    fd_set readSet;
    FD_ZERO( &readSet );
    FD_SET( s, &readSet );
    
    timeval timeout;
    timeout.tv_sec = DEFAULT_SELECT_TIMEOUT;
    timeout.tv_usec = 0;
    
    int available = select( s+1, &readSet, 0, 0, &timeout );
    return ( available > 0 );
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
        len = (int)strlen( buf );
    }
    
    int bytesSent = (int)send( s, buf, len, 0 );
    
    return ( bytesSent >= 0 );
}

inline int NetRecv( NetSocket s, char* buf, int len )
{
    return (int)recv( s, buf, len, 0 );
}

#define ifNetRecv( _recv, _expr ) \
if( _recv < 0 ) \
{ \
cout << "Failed to receive network data." << endl; \
_expr; \
}

bool NetSendFileHandle( NetSocket s, FileHandle filehandle, uint64_t size )
{
    uint64_t remaining = size;
    char filebuf[1024];
    
    do
    {
        int sendsize = ( remaining > 1024 ? 1024 : (int)remaining );
        
        if( read( filehandle, filebuf, sendsize ) < 0 )
        {
            return false;
        }
        
        ifNetSend( s, filebuf, sendsize, return false );
        remaining -= sendsize;
    } while( remaining > 0 );
    
    return true;
}

bool NetSendFile( NetSocket s, const char* file )
{
    int filehandle = open( file, O_RDONLY | O_CREAT, S_IRWXU );
    if( filehandle < 0 )
    {
        return false;
    }
    
    struct stat filestats;
    if( fstat( filehandle, &filestats ) < 0 )
    {
        return false;
    }
    
    uint64_t remaining = filestats.st_size;
    char filebuf[1024];
    
    do
    {
        int sendsize = ( remaining > 1024 ? 1024 : (int)remaining );
        
        if( read( filehandle, filebuf, sendsize ) < 0 )
        {
            return false;
        }
        
        ifNetSend( s, filebuf, sendsize, return false );
        remaining -= sendsize;
    } while( remaining > 0 );
    
    close( filehandle );
    
    return true;
}

bool NetRecvFile( NetSocket s, const char* file )
{
    int filehandle = open( file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU );
    
    if( filehandle >= 0 )
    {
        char filebuf[1024];
        int r;
        
        do
        {
            r = NetRecv( s, filebuf, 1024 );
            ifNetRecv( r, return false );
            
            if( write( filehandle, filebuf, r ) < 0 )
            {
                return false;
            }
        } while( r >= 1024 );
        
        close( filehandle );
    }
    
    return true;
}
#endif
