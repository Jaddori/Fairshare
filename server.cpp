/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Tunder $
   $Notice: (C) Copyright 2014 by SpaceCat, Inc. All Rights Reserved. $
   ======================================================================== */

// server.cpp

ThreadReturn ServerFunc( ThreadArgs args )
{
    while( g_running )
    {
        SleepMS( 1000 );
    }

    return 0;
}

ThreadHandle StartServer( Config* config )
{
    ThreadHandle threadHandle = MakeThread( ServerFunc, (ThreadArgs)config );
    return threadHandle;
}
