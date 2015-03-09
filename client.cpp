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

bool SaveHubs( Config* config )
{
    return false;
}

bool LoadHubs( Config* config )
{
    return false;
}

inline void PrintHelp()
{
    cout << "-add [hub] [ip] [opt:port]\t : adds a new hub with ip and port." << endl;
    cout << "-remove/rm [hub]\t\t : removes a hub." << endl;
    cout << "-edit [hub]\t\t\t : change ip and/or port of a hub." << endl;
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
    
    while( g_running )
    {
        getline( cin, command );

        split.clear();
        StrSplit( command, split, ' ' );

        if( split.size() > 0 )
        {
            if( split[0].compare( "-add" ) == 0 )
            {
            }
            else if( split[0].compare( "-remove" ) == 0 ||
                     split[0].compare( "-rm" ) == 0 )
            {
            }
            else if( split[0].compare( "-edit" ) == 0 )
            {
            }
            else if( split[0].compare( "-sync" ) == 0 )
            {
            }
            else if( split[0].compare( "-config" ) == 0 ||
                     split[0].compare( "-cfg" ) == 0 )
            {
            }
            else if( split[0].compare( "-reset" ) == 0 ||
                     split[0].compare( "-rs" ) == 0 )
            {
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
