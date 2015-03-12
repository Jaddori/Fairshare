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

    cout << "Running on port \"" << config->port << "\"." << endl;

    // open socket
    NetSocket nsocket;
    ifOpenSocket( &nsocket, return 0 );

    // bind socket
    ifNetBind( nsocket, config->port, return 0 );

    // listen for connections
    ifNetListen( nsocket, return 0 );

    int selectCycles = 0;
    
    NetSocket com;
    while( g_running ) // loop until local client shuts us down
    {
        if( selectCycles > 0 )
        {
            selectCycles--;
        }
        else
        {
            // update list of hub files
            vector<string> locFiles;
            FSDirectoryGetFiles( config->folder, locFiles );
            WriteWholeFile( "./hubfiles.txt", locFiles );

            selectCycles = HUBFILE_UPDATE_DELAY;
        }
        
        // check for pending connection
        if( NetSelect( nsocket ) )
        {
            // accept connection
            com = NetAccept( nsocket );

            // make sure the accepted connection is valid
            if( NetValidSocket( com ) )
            {
                // send list of hub files
                ifNetSendFile( com, "./hubfiles.txt", return 0 );

                // receive list of unsynced files
                ifNetRecvFile( com, "./unsynced.tmp", return 0 );
                
                vector<string> unsyncedFiles;
                ifReadWholeFile( "./unsynced.tmp", unsyncedFiles, return 0 );

                // make sure there is atleast one unsynced file
                if( unsyncedFiles.size() > 0 )
                {
                    char filebuf[1024];

                    // loop through ever unsynced file and send it to the client
                    for( vector<string>::iterator it = unsyncedFiles.begin(); it != unsyncedFiles.end(); it++ )
                    {
                        // open requested file
                        string path = config->folder + string( "/" ) + *it;
                        FileHandle filehandle = FSOpenFile( path.c_str(), FSRead );
                        if( FSValidHandle( filehandle ) )
                        {
                            // put filesize and filename in byte array
                            unsigned long remaining = FSGetFileSize( filehandle );
                            memcpy( filebuf, &remaining, sizeof(remaining) );
                            strcpy( filebuf+sizeof(remaining), it->c_str() );

                            // send filesize and filename to client
                            ifNetSend( com, filebuf, 1024, return 0 );

                            // send unsynced file to client
                            ifNetSendFileHandle( com, filehandle, remaining, return 0 );

                            // close requested file
                            FSCloseFile( filehandle );
                            cout << "Server: Synced file \"" << *it << "\"." << endl;

                            SleepSeconds( 1 );
                        }
                        else
                        {
                            cout << "Failed to open requested file \"" << path << "\"." << endl;
                        }
                    }
                }

                // close accepted socket and go back to listening for connections
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
