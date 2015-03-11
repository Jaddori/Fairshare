/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Tunder $
   $Notice: (C) Copyright 2014 by SpaceCat, Inc. All Rights Reserved. $
   ======================================================================== */

// server.cpp

ThreadReturnType ServerFunc( ThreadArgs args )
{
    Config* config = (Config*)args;

    NetSocket nsocket;
    ifOpenSocket( &nsocket, return 0 );
    cout << "Server: Opening socket." << endl;
    
    ifNetBind( nsocket, config->port, return 0 );
    cout << "Server: Binding socket to port \"" << config->port << "\"." << endl;
    
    ifNetListen( nsocket, return 0 );
    cout << "Server: Listening for connections." << endl;

    NetSocket com;
    while( g_running )
    {
        if( NetSelect( nsocket ) )
        {
            com = NetAccept( nsocket );
            cout << "Server: Accepting connection." << endl;

            if( NetValidSocket( com ) )
            {
                cout << "Server: Sending file list." << endl;
                ifNetSendFile( com, "./hubfiles.txt", return 0 );

                cout << "Server: Receiving list of unsynced files." << endl;
                ifNetRecvFile( com, "./unsynced.tmp", return 0 );

                vector<string> unsyncedFiles;
                ifReadWholeFile( "./unsynced.tmp", unsyncedFiles, return 0 );

                cout << "Server: Unsynced files (" << unsyncedFiles.size() << "):" << endl;
                for( int i=0; i<unsyncedFiles.size(); i++ )
                {
                    cout << (i+1) << ". " << unsyncedFiles[i] << endl;
                }

                #if WIN32
                if( unsyncedFiles.size() > 0 )
                {
                    char filebuf[1024];
                    for( vector<string>::iterator it = unsyncedFiles.begin(); it != unsyncedFiles.end(); it++ )
                    {
                        string path = config->folder + string("\\") + *it;
                        HANDLE filehandle = CreateFile( path.c_str(),
                                                        GENERIC_READ,
                                                        FILE_SHARE_READ,
                                                        0,
                                                        OPEN_EXISTING,
                                                        FILE_ATTRIBUTE_NORMAL,
                                                        0 );

                        if( filehandle != INVALID_HANDLE_VALUE )
                        {
                            cout << "Server: Opening file \"" << path << "\"." << endl;
                        
                            LARGE_INTEGER filesize;
                            GetFileSizeEx( filehandle, &filesize );

                            unsigned long remaining = filesize.QuadPart;
                            unsigned int namelen = it->size();

                            memcpy( filebuf, &remaining, sizeof(remaining) );
                            memcpy( filebuf+sizeof(remaining), &namelen, sizeof(namelen) );
                            strcpy( filebuf+sizeof(remaining)+sizeof(namelen), it->c_str() );

                            cout << "Server: sending filesize and filename." << endl;
                            ifNetSend( com, filebuf, 1024, return 0 );

                            while( remaining > 0 )
                            {
                                int sendsize = ( remaining > 1024 ? 1024 : remaining );

                                DWORD bytesRead;
                                ReadFile( filehandle,
                                          filebuf,
                                          sendsize,
                                          &bytesRead,
                                          0 );

                                ifNetSend( com, filebuf, sendsize, return 0 );
                                remaining -= sendsize;

                                cout << "Server: Sending \"" << sendsize << " bytes\"." << endl;
                            }

                            cout << "Server: closing file." << endl;
                            CloseHandle( filehandle );
                            cout << "Server: closed file." << endl;
                        }
                        else
                        {
                            cout << "Failed to open requested file \"" << *it << "\"." << endl;
                        }
                    }
                }
                #endif

                // 5. send requested file

                cout << "Server: Closing socket." << endl;
                CloseSocket( com );
            }
        }
    }

    CloseSocket( nsocket );

    return 0;
}

ThreadHandle StartServer( Config* config )
{
    ThreadHandle threadHandle = MakeThread( ServerFunc, (ThreadArgs)config );
    return threadHandle;
}
