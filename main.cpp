/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Tunder $
   $Notice: (C) Copyright 2014 by SpaceCat, Inc. All Rights Reserved. $
   ======================================================================== */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
using namespace std;

static bool g_running;

#include "fairshare.cpp"
#include "client.cpp"
#include "server.cpp"

int main( int argc, char** argv )
{
    int port;
    
    // running with spec. port
    if( argc > 1 )
    {
        port = (int)strtol( argv[1], (char**)0, 10 );
    }
    else
    {
        port = DEFAULT_PORT;
    }

    Config config;
    
    if( !LoadConfig( &config ) )
    {
        cout << "Try restarting the application." << endl;
        return -1;
    }

    g_running = true;
    
    ThreadHandle serverThread = StartServer( &config );
    StartClient( &config );

    ThreadWait( serverThread );
    
    return 0;
}