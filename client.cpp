/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Tunder $
   $Notice: (C) Copyright 2014 by SpaceCat, Inc. All Rights Reserved. $
   ======================================================================== */

// client.cpp

bool SaveConfig( Config* config )
{
    bool result = false;
    
    ofstream stream( "config.txt" );
    if( stream.is_open() )
    {
        stream << config->username << endl << config->folder;
        stream.close();

        result = true;
    }
    else
    {
        cout << "Error saving config." << endl;
    }

    return result;
}

bool LoadConfig( Config* config, bool force = false )
{
    bool result = false;
    
    if( force )
    {
        cout << "Please enter username: ";
        getline( cin, config->username );

        cout << "Please enter hub folder: ";
        getline( cin, config->folder );

        result = SaveConfig( config );
    }
    else
    {
        ifstream stream( "config.txt" );
        if( stream.is_open() )
        {
            getline( stream, config->username );
            getline( stream, config->folder );
            stream.close();

            result = true;
        }
        else
        {
            cout << "No configuration file detected." << endl;
            
            cout << "Please enter username: ";
            getline( cin, config->username );

            cout << "Please enter hub folder: ";
            getline( cin, config->folder );

            result = SaveConfig( config );
        }
    }

    return result;
}

int FindHub( vector<Hub>& hubs, const string& name )
{
    int index = -1;
    for( int i=0; i<hubs.size() && index<0; i++ )
    {
        if( hubs[i].name == name )
        {
            index = i;
        }
    }

    return index;
}

bool SaveHubs( vector<Hub>& hubs )
{
    bool result = false;
    
    ofstream stream( "hubs.txt" );
    if( stream.is_open() )
    {
        for( vector<Hub>::iterator it = hubs.begin(); it != hubs.end(); it++ )
        {
            stream << it->name << " " << it->ip << " " << it->port << endl;
        }

        stream.close();
        result = true;
    }
    else
    {
        cout << "Failed to save hubs." << endl;
    }

    return result;
}

bool LoadHubs( vector<Hub>& hubs )
{
    bool result = false;

    ifstream stream( "hubs.txt" );
    if( stream.is_open() )
    {
        Hub hub;
        while( stream )
        {
            getline( stream, hub.name, ' ' );
            getline( stream, hub.ip, ' ' );
            stream >> hub.port;
            stream.ignore();
			
			hub.schedule = 0;

            if( stream )
            {
                hubs.push_back( hub );
            }
        }

        stream.close();
    }

    return result;
}

void AddHub( vector<Hub>& hubs, vector<string>& split )
{
    if( split.size() < 3 )
    {
        cout << "Usage: -add [hub] [ip] [optional:port]" << endl;
    }
    else
    {
        int index = FindHub( hubs, split[1] );

        // not found
        if( index < 0 )
        {
            Hub hub;
            hub.name = split[1];
            hub.ip = split[2];

            if( split.size() >= 4 )
            {
                hub.port = (int)strtoul( split[3].c_str(), (char**)0, 10 );
            }
            else
            {
                hub.port = DEFAULT_PORT;
            }

            hubs.push_back( hub );

            if( SaveHubs( hubs ) )
            {
                cout << "Hub \"" << hub.name << "\" added." << endl;
            }
        }
        else
        {
            cout << "Hub \"" << split[1] << "\" already exist with ip \"";
            cout << hubs[index].ip << ":" << hubs[index].port << "\"." << endl;
        }
    }
}

void RemoveHub( vector<Hub>& hubs, vector<string>& split )
{
    if( split.size() < 2 )
    {
        cout << "Usage: -remove [hub]" << endl;
    }
    else
    {
        int index = FindHub( hubs, split[1] );

        // not found
        if( index < 0 )
        {
            cout << "Couldn't find hub \"" << split[1] << "\"." << endl;
        }
        else
        {
            hubs.erase( hubs.begin()+index );

            if( SaveHubs( hubs ) )
            {
                cout << "Hub \"" << split[1] << "\" removed." << endl;
            }
        }
    }
}

void EditHub( vector<Hub>& hubs, vector<string>& split )
{
    if( split.size() < 2 )
    {
        cout << "Usage: -edit [hub] [ip] [optional:port]" << endl;
    }
    else
    {
        int index = FindHub( hubs, split[1] );

        // not found
        if( index < 0 )
        {
            cout << "Couldn't find hub \"" << split[1] << "\"." << endl;
        }
        else
        {
            hubs[index].ip = split[2];

            if( split.size() >= 4 )
            {
                hubs[index].port = (int)strtoul( split[3].c_str(), (char**)0, 10 );
            }

            if( SaveHubs( hubs ) )
            {
                cout << "Hub \"" << split[1] << "\" edited." << endl;
            }
        }
    }
}

void ListHubs( vector<Hub>& hubs )
{
    for( int i=0; i<hubs.size(); i++ )
    {
        cout << (i+1) << ". " << hubs[i].name << " - " << hubs[i].ip << ":" << hubs[i].port << endl;
    }
}

void SyncHub( Config* config, Hub* hub )
{
    // open socket
    NetSocket nsocket;
    if( !NetOpenSocket( &nsocket ) )
	{
		cout << "Client: Failed to open socket." << endl;
		NetCloseSocket( nsocket );
		return;
	}

    // connect to remote server
    if( !NetConnect( nsocket, hub->ip.c_str(), hub->port ) )
	{
		cout << "Client: Failed to connect to " << hub->ip.c_str() << endl;
		NetCloseSocket( nsocket );
		return;
	}

    // receive available hub files
    if( !NetRecvFile( nsocket, "./hubfiles.tmp" ) )
	{
		cout << "Client: Failed to receive list of hub files." << endl;
		NetCloseSocket( nsocket );
		return;
	}

    vector<string> hubFiles;
    if( !ReadWholeFile( "./hubfiles.tmp", hubFiles ) )
	{
		cout << "Client: Failed to parse list of hub files." << endl;
		NetCloseSocket( nsocket );
		return;
	}
    
    // read local files
    vector<string> locFiles;
    if( !FSDirectoryGetFiles( config->folder, locFiles ) )
	{
		cout << "Client: Failed to parse local files." << endl;
		NetCloseSocket( nsocket );
		return;
	}
    
    // compare local files to hub files
    vector<string> unsyncedFiles;
    StrCompare( hubFiles, locFiles, unsyncedFiles );

    // write unsynced files to file
    if( !WriteWholeFile( "./unsynced.txt", unsyncedFiles ) )
	{
		cout << "Client: Failed to write list of unsynced files." << endl;
		NetCloseSocket( nsocket );
		return;
	}

    // send list of unsynced files
    if( !NetSendFile( nsocket, "./unsynced.txt" ) )
	{
		cout << "Client: Failed to send list of unsynced files." << endl;
		NetCloseSocket( nsocket );
		return;
	}

    // make sure there is atleast one unsynced file
    if( unsyncedFiles.size() > 0 )
    {
        char filebuf[1024];

        int r = 1024;
        while( r > 0 ) // loop until there is no data being received
        {
            memset( filebuf, 0, 1024 );

            // receive filesize and filename
            r = NetRecv( nsocket, filebuf, 1024 );

            if( r == SOCKET_ERROR )
			{
				cout << "Client: Failed to receive network data." << endl;
				NetCloseSocket( nsocket );
				return;
			}
            else if( r > 0 )
            {
                filebuf[1023] = 0; // make sure string is null-terminated

                // extract filesize from the byte array
                uint64_t filesize;
                memcpy( &filesize, filebuf, sizeof(filesize) );
                string filename( filebuf+sizeof(filesize) );

                string path = config->folder + string( "/" ) + filename;

                // open requested file
                FileHandle filehandle = FSOpenFile( path.c_str(), FSWrite );
                if( FSValidHandle( filehandle ) )
                {
                    // receive requested file
                    //ifNetRecvFileHandle( nsocket, filehandle, return );

                    char filebuf[1024];
                    int r;
                    uint64_t total = 0;

                    uint64_t tenth = filesize / 10;
                    int progress = 1;
                    
                    do
                    {
                        r = NetRecv( nsocket, filebuf, 1024 );
                        if( !NetValidRecv( r ) )
						{
							cout << "Client: Failed to receive network data." << endl;
							NetCloseSocket( nsocket );
							return;
						}

                        if( !FSWriteFile( filehandle, filebuf, r ) )
						{
							cout << "Client: Failed to write to disk." << endl;
							NetCloseSocket( nsocket );
							return;
						}

                        if( filesize > FILESIZE_THRESHOLD )
                        {
                            total += r;

                            if( total > ( progress * tenth ) )
                            {
                                cout << filename << " : " << (progress*10) << "%" << endl;
                                progress++;
                            }
                        }
                    } while( r >= 1024 );

                    // close requested file
                    FSCloseFile( filehandle );
                    SleepSeconds( 1 );

                    cout << "Client: Synced file \"" << filename << "\"." << endl;
                }
                else
                {
                    cout << "Client: File error \"" << strerror(errno) << "\"." << endl;
                }
            }
        }
    }

    // close socket
    NetCloseSocket( nsocket );
}

void Sync( Config* config, vector<Hub>& hubs, vector<string>& split )
{
    // sync specified hub
    if( split.size() > 1 )
    {
        int hubIndex = FindHub( hubs, split[1] );
        if( hubIndex < 0 )
        {
            cout << "Hub \"" << split[1] << "\" was not found." << endl;
        }
        else
        {
            SyncHub( config, &hubs[hubIndex] );
            cout << "Synchronization complete." << endl;
        }
    }
    else // sync all hubs
    {
        if( hubs.size() > 0 )
        {
            for( vector<Hub>::iterator it = hubs.begin(); it != hubs.end(); it++ )
            {
                SyncHub( config, &(*it) );
            }

            cout << "Syncronization complete." << endl;
        }
        else
        {
            cout << "No hubs to sync." << endl;
        }
    }
}

void Schedule( Config* config, vector<Hub>& hubs, vector<string>& split )
{
	// specific hub
	if( split.size() > 2 )
	{
		int index = FindHub( split[1] );
		if( index >= 0 )
		{
			int time = (int)strtol( split[2], (char**)0, 10 );
			hubs[index].schedule = time;
		}
	}
	else if( split.size() > 1 )
	{
		int time = (int)strtol( split[1], (char**)0, 10 );
		for( vector<Hub>::iterator it = hubs.begin(); it != hubs.ens(); it++ )
		{
			it->schedule = time;
		}
	}
	else
	{
		cout << "Usage: -schedule [optional:hub] [seconds]" << endl;
	}
}

inline void PrintHelp()
{
    cout << "-add [hub] [ip] [opt:port]\t : adds a new hub with ip and port." << endl;
    cout << "-remove/rm [hub]\t\t : removes a hub." << endl;
    cout << "-edit [hub]\t\t\t : change ip and/or port of a hub." << endl;
    cout << "-list\t\t\t\t : prints a list of all hubs." << endl;
    cout << "-sync [opt:hub]\t\t\t : synchronizes files with hubs." << endl;
    cout << "-config/cfg\t\t\t : configures user data." << endl;
    cout << "-help\t\t\t\t : prints this list." << endl;
    cout << "-exit\t\t\t\t : exits the application." << endl;
    cout << endl;
}

void StartClient( Config* config )
{
    string command;
    vector<string> split;
    split.reserve( 3 );

    vector<Hub> hubs;
    LoadHubs( hubs );
    
    while( g_running )
    {
        getline( cin, command );

        split.clear();
        StrSplit( command, split, ' ' );

        if( split.size() > 0 )
        {
            if( split[0].compare( "-add" ) == 0 )
            {
                AddHub( hubs, split );
            }
            else if( split[0].compare( "-remove" ) == 0 ||
                     split[0].compare( "-rm" ) == 0 )
            {
                RemoveHub( hubs, split );
            }
            else if( split[0].compare( "-edit" ) == 0 )
            {
                EditHub( hubs, split );
            }
            else if( split[0].compare( "-list" ) == 0 )
            {
                ListHubs( hubs );
            }
            else if( split[0].compare( "-sync" ) == 0 )
            {
                Sync( config, hubs, split );
            }
			else if( split[0].compare( "-schedule" ) == 0 )
			{
				Schedule( config, hubs, split );
			}
            else if( split[0].compare( "-config" ) == 0 ||
                     split[0].compare( "-cfg" ) == 0 )
            {
                LoadConfig( config, true );
            }
            else if( split[0].compare( "-reset" ) == 0 ||
                     split[0].compare( "-rs" ) == 0 )
            {
                hubs.clear();
                SaveHubs( hubs );
            }
            else if( split[0].compare( "-help" ) == 0 ||
                     split[0].compare( "-?" ) == 0 )
            {
                PrintHelp();
            }
            else if( split[0].compare( "-exit" ) == 0 )
            {
                g_running = false;
            }
            else
            {
                cout << "Unrecognized command: \"" << split[0] << "\"." << endl;
                cout << "Type \"help\" for a list of commands." << endl;
            }
        }
    }
}
