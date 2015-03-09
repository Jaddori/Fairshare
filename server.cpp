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
    else
    {
        cout << "Server: Initializing network data." << endl;
    }
    
    if( !OpenSocket( &net.socket ) )
    {
        cout << "Server: Error initializing network socket." << endl;
        return 0;
    }
    else
    {
        cout << "Server: Opening network socket." << endl;
    }

    if( !NetBind( net.socket, config->port ) )
    {
        cout << "Server: Error binding socket." << endl;
        return 0;
    }
    else
    {
        cout << "Server: Binding socket." << endl;
    }

    if( !NetListen( net.socket ) )
    {
        cout << "Server: Error listening on socket." << endl;
        return 0;
    }
    else
    {
        cout << "Server: Listening on socket." << endl;
    }

    NetSocket com;
    while( g_running )
    {
        if( NetSelect( net.socket ) )
        {
            cout << "Server: Pending connection." << endl;
            
            com = NetAccept( net.socket );

            if( NetValidSocket( com ) )
            {
                cout << "Server: Accepting socket." << endl;
                
                const char* msg = "Test";
                char buf[32] = {};

                cout << "Server: Sending message to client." << endl;
                NetSend( com, msg, 4 );

                cout << "Server: Waiting for message from client." << endl;
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
