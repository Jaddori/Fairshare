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
            // 1. accept connection
            com = NetAccept( nsocket );

            if( NetValidSocket( com ) )
            {
                // 2. send hub filelist
                ifNetSendFile( com, "./hubfiles.txt", return 0 );

                // 3. recv unsynced files
                ifNetRecvFile( com, "./unsynced.tmp", return 0 );

                // 4. send name and size of requested file

                // 5. send requested file

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
