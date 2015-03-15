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
    if( !NetOpenSocket( &nsocket ) )
	{
		cout << "Server: Failed to open socket." << endl;
		return -1;
	}

    // bind socket
    if( !NetBind( nsocket, config->port ) )
	{
		cout << "Server: Failed to bind socket." << endl;
		return -1;
	}

    // listen for connections
    if( !NetListen( nsocket ) )
	{
		cout << "Server: Failed to listen on socket." << endl;
		return -1;
	}

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
            if( FSDirectoryGetFiles( config->folder, locFiles ) )
            {
                if( !WriteWholeFile( "./hubfiles.txt", locFiles ) )
				{
					cout << "Server: Failed to write to disk." << endl;
					return -1;
				}
            }
            else
            {
                cout << "Server: Failed to open hub folder \"" << config->folder << "\"." << endl;
                cout << "Server: Shutting down." << endl;
                return -1;
            }

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
                if( !NetSendFile( com, "./hubfiles.txt" ) )
				{
					cout << "Server: Failed to send list of hub files." << endl;
					return -1;
				}
				
                // receive list of unsynced files
                if( !NetRecvFile( com, "./unsynced.tmp" ) )
				{
					cout << "Server: Failed to receive list of unsynced files." << endl;
					return -1;
				}
                
                vector<string> unsyncedFiles;
                if( !ReadWholeFile( "./unsynced.tmp", unsyncedFiles ) )
				{
					cout << "Server: Failed to read parse list of unsynced files." << endl;
					return -1;
				}

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
                            uint64_t remaining = FSGetFileSize( filehandle );
                            memcpy( filebuf, &remaining, sizeof(remaining) );
                            strcpy( filebuf+sizeof(remaining), it->c_str() );

                            // send filesize and filename to client
                            if( !NetSend( com, filebuf, 1024 ) )
							{
								cout << "Server: Failed to send network data." << endl;
								return -1;
							}

                            // send unsynced file to client
                            if( !NetSendFileHandle( com, filehandle, remaining ) )
							{
								cout << "Server: Failed to send file." << endl;
								return -1;
							}

                            // close requested file
                            FSCloseFile( filehandle );
                            cout << "Server: Synced file \"" << *it << "\"." << endl;

                            SleepSeconds( 1 );
                        }
                        else
                        {
                            cout << "Server: Failed to sync file \"" << *it << "\"." << endl;
                            cout << "Error: " << strerror( errno ) << endl;
                        }
                    }
                }

                // close accepted socket and go back to listening for connections
                NetCloseSocket( com );
            }
        }
    }

    NetCloseSocket( nsocket );

    return 0;
}

ThreadHandle StartServer( Config* config )
{
    ThreadHandle threadHandle = MakeThread( ServerFunc, (ThreadArgs)config );
    return threadHandle;
}
