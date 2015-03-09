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

    Net net;
    if( !NetInit( &net.data ) )
    {
        cout << "Server: Error intializing network data." << endl;
        return 0;
    }
    
    if( !OpenSocket( &net.socket ) )
    {
        cout << "Server: Error initializing network socket." << endl;
        return 0;
    }

    NetBind( net.socket, config->port );
    NetListen( net.socket );

    NetSocket com;
    while( g_running )
    {
        if( NetSelect( net.socket ) )
        {
            com = NetAccept( net.socket );

            if( NetValidSocket( com ) )
            {
                const char* msg = "Test";
                char buf[32] = {};
            
                NetSend( com, msg );
                NetRecv( com, buf, 32 );

                buf[31] = 0;
                cout << "Server: client says \"" << buf << "\"." << endl;

                CloseSocket( com );
            }
        }
    }

    CloseSocket( net.socket );
    NetShutdown();

    return 0;
}

ThreadHandle StartServer( Config* config )
{
    ThreadHandle threadHandle = MakeThread( ServerFunc, (ThreadArgs)config );
    return threadHandle;
}
