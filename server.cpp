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
    ifNetBind( nsocket, config->port, return 0 );
    ifNetListen( nsocket, return 0 );

    NetSocket com;
    while( g_running )
    {
        if( NetSelect( nsocket ) )
        {
            com = NetAccept( nsocket );

            if( NetValidSocket( com ) )
            {
                const char* msg = "Test";
                char buf[32] = {};

                ifNetSend( com, msg, strlen(msg), continue );
                ifNetRecv( com, buf, 32, continue );

                buf[31] = 0;
                cout << "Server: client says \"" << buf << "\"." << endl;

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
