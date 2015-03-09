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

    const char* msg = "Testing";
    char buf[32] = {};

    ifNetRecv( nsocket, buf, 32, return );
    
    buf[31] = 0;
    cout << "Client: server says \"" << buf << "\"." << endl;

    ifNetSend( nsocket, msg, strlen(msg), return );
    
    CloseSocket( nsocket );
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
