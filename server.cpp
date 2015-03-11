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

                if( unsyncedFiles.size() > 0 )
                {
                    char filebuf[1024];
                    for( vector<string>::iterator it = unsyncedFiles.begin(); it != unsyncedFiles.end(); it++ )
                    {
                        string path = config->folder + string( "/" ) + *it;
                        FileHandle filehandle = FSOpenFile( path.c_str(), FSRead );
                        if( FSValidHandle( filehandle ) )
                        {
                            cout << "Server: Opening file \"" << path << "\"." << endl;

                            unsigned long remaining = FSGetFileSize( filehandle );
                            memcpy( filebuf, &remaining, sizeof(remaining) );
                            strcpy( filebuf+sizeof(remaining), it->c_str() );

                            cout << "Server: sending filesize and filename." << endl;
                            cout << "Server: \"" << *it << ":" << remaining << "\"." << endl;
                            ifNetSend( com, filebuf, 1024, return 0 );

                            ifNetSendFileHandle( com, filehandle, remaining, return 0 );

                            cout << "Server: Closing file." << endl;
                            FSCloseFile( filehandle );
                            cout << "Server: Closed file." << endl;
                        }
                        else
                        {
                            cout << "Failed to open requested file \"" << path << "\"." << endl;
                        }
                    }
                }

                cout << "Server: Closing socket." << endl;
                CloseSocket( com );

                cout << "Server: Synchronization complete." << endl;
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
