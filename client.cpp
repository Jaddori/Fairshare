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

bool LoadConfig( Config* config )
{
    bool result = false;
    
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
        cout << "No configuration file found." << endl;
        
        cout << "Please enter username: ";
        getline( cin, config->username );
        
        cout << "Please enter hub folder: ";
        getline( cin, config->folder );

        result = SaveConfig( config );
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

void Sync( Config* config, vector<string>& split )
{
    NetSocket nsocket;
    ifOpenSocket( &nsocket, return );

    ifNetConnect( nsocket, "127.0.0.1", DEFAULT_PORT, return );
    cout << "Client: Connected!" << endl;

    ifNetRecvFile( nsocket, "./hubfiles.tmp", return );
    cout << "Client: Received list of hub files." << endl;

    vector<string> hubFiles;
    ifReadWholeFile( "./hubfiles.tmp", hubFiles, return );

    cout << "Client: Hub files (" << hubFiles.size() << "):" << endl;
    for( int i=0; i<hubFiles.size(); i++ )
    {
        cout << (i+1) << ". " << hubFiles[i] << endl;
    }

    // LOOP THROUGH DIR
    vector<string> locFiles;
    ifDirectoryGetFiles( config->folder, locFiles, return );

    cout << "Client: Read list of local files." << endl;
    cout << "Client: Local files(" << locFiles.size() << "):" << endl;

    for( int i=0; i<locFiles.size(); i++ )
    {
        cout << (i+1) << ". " << locFiles[i] << endl;
    }

    vector<string> unsyncedFiles;
    StrCompare( hubFiles, locFiles, unsyncedFiles );

    cout << "Client: Comparing hub files to local files." << endl;
    cout << "Client: Unsynced files (" << unsyncedFiles.size() << "):" << endl;

    for( int i=0; i<unsyncedFiles.size(); i++ )
    {
        cout << (i+1) << ". " << unsyncedFiles[i] << endl;
    }
    
    ifWriteWholeFile( "./unsynced.txt", unsyncedFiles, return );
    cout << "Client: Writing unsynced files to file." << endl;

    ifNetSendFile( nsocket, "./unsynced.txt", return );
    cout << "Client: Sending unsynced file list." << endl;

    if( unsyncedFiles.size() > 0 )
    {
        char filebuf[1024];

        int r = 1024;
        while( r > 0 )
        {
            memset( filebuf, 0, 1024 );
            r = NetRecv( nsocket, filebuf, 1024 );

            ifNetRecv( r, return )
            else if( r > 0 )
            {
                filebuf[1023] = 0; // make sure string is null-terminated

                unsigned long filesize;
                memcpy( &filesize, filebuf, sizeof(filesize) );
                string filename( filebuf+sizeof(filesize) );

                cout << "Client: Got fileinfo \"" << filename << ":" << filesize << "\"." << endl;

                string path = config->folder + string( "/" ) + filename;
                cout << "Client: PATH = \"" << path << "\"." << endl;
                FileHandle filehandle = FSOpenFile( path.c_str(), FSWrite );
                if( FSValidHandle( filehandle ) )
                {
                    ifNetRecvFileHandle( nsocket, filehandle, return )

                    cout << "Client: Closing file." << endl;
                    FSCloseFile( filehandle );
                    cout << "Client: Closed file." << endl;
                }
                else
                {
                    cout << "Client: File error \"" << strerror(errno) << "\"." << endl;
                }
            }
        }
    }

    #if WIN32
    if( unsyncedFiles.size() > 0 )
    {
        char filebuf[1024];

        int r = 1024;
        while( r > 0 )
        {
            memset( filebuf, 0, 1024 );
            r = NetRecv( nsocket, filebuf, 1024 );
        
            ifNetRecv( r, return )
            else if( r > 0 )
            {
                unsigned long filesize;
                unsigned int namelen;

                int offset = 0;
                memcpy( &filesize, filebuf, sizeof(filesize) );
                offset += sizeof(filesize);
                memcpy( &namelen, filebuf+offset, sizeof(namelen) );
                offset += sizeof(namelen);
                string filename( filebuf+offset, namelen );

                cout << "Client: got fileinfo \"" << filename << ":" << filesize << "\"." << endl;

                string path = config->folder + string("\\") + filename;
                HANDLE filehandle = CreateFile( path.c_str(),
                                                GENERIC_WRITE,
                                                0,
                                                0,
                                                CREATE_ALWAYS,
                                                FILE_ATTRIBUTE_NORMAL,
                                                0 );

                do
                {
                    r = NetRecv( nsocket, filebuf, 1024 );
                    ifNetRecv( r, return );

                    cout << "Client: Received \"" << r << " bytes\"." << endl;

                    DWORD bytesWritten;
                    WriteFile( filehandle,
                               filebuf,
                               r,
                               &bytesWritten,
                               0 );
                } while( r >= 1024 );

                cout << "Client: Closing file." << endl;
                CloseHandle( filehandle );
                cout << "Client: Closed file." << endl;
            }
        }
    }
    #endif

    cout << "Client: Closing socket." << endl;
    CloseSocket( nsocket );

    cout << "Client: Syncronization complete." << endl;
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
                Sync( config, split );
            }
            else if( split[0].compare( "-config" ) == 0 ||
                     split[0].compare( "-cfg" ) == 0 )
            {
                
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
