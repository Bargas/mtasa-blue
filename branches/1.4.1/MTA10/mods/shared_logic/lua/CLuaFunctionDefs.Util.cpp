/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Util.cpp
*  PURPOSE:     Lua function definitions class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Chris McArthur <>
*               Derek Abdine <>
*               Christian Myhre Lundheim <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*
*****************************************************************************/

#include "StdInc.h"

int CLuaFunctionDefs::GetTok ( lua_State* luaVM )
{
    SString strInput = "";
    unsigned int uiToken = 0;
    unsigned int uiDelimiter = 0;
    SString strDelimiter;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strInput );
    argStream.ReadNumber ( uiToken );

    if ( argStream.NextIsNumber ( ) )
    {
        argStream.ReadNumber ( uiDelimiter );
        wchar_t wUNICODE[2] = { uiDelimiter, '\0' };
        strDelimiter = UTF16ToMbUTF8(wUNICODE);
    }
    else  // It's already a string
        argStream.ReadString ( strDelimiter );

    if ( !argStream.HasErrors ( ) )
    {
        unsigned int uiCount = 0;

        if ( uiToken > 0 && uiToken < 1024 )
        {
            unsigned int uiCount = 1;
            char* szText = new char [ strInput.length ( ) + 1 ];
            strcpy ( szText, strInput );
            char* szToken = strtok ( szText, strDelimiter );

            // We're looking for the first part?
            if ( uiToken != 1 )
            {
                // strtok count number of times
                do
                {
                    uiCount++;
                    szToken = strtok ( NULL, strDelimiter );
                }
                while ( uiCount != uiToken );
            }

            // Found it?
            if ( szToken )
            {
                // Return it
                lua_pushstring ( luaVM, szToken );
                delete [] szText;
                return 1;
            }

            // Delete the text
            delete [] szText;
        }
        else
            m_pScriptDebugging->LogWarning ( luaVM, "Token parameter sent to split must be greater than 0 and smaller than 1024" );
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::Split ( lua_State* luaVM )
{
    SString strInput = "";
    unsigned int uiDelimiter = 0;
    SString strDelimiter;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strInput );

    if ( argStream.NextIsNumber ( ) )
    {
        argStream.ReadNumber ( uiDelimiter );
        wchar_t wUNICODE[2] = { uiDelimiter, '\0' };
        strDelimiter = UTF16ToMbUTF8(wUNICODE);
    }
    else  // It's already a string
        argStream.ReadString ( strDelimiter );

    if ( !argStream.HasErrors ( ) )
    {
        // Copy the string
        char* strText = new char [ strInput.length ( ) + 1 ];
        strcpy ( strText, strInput );

        unsigned int uiCount = 0;
        char* szToken = strtok ( strText, strDelimiter );

        // Create a new table
        lua_newtable ( luaVM );

        // Add our first token
        lua_pushnumber ( luaVM, ++uiCount );
        lua_pushstring ( luaVM, szToken );
        lua_settable ( luaVM, -3 );

        // strtok until we're out of tokens
        while ( szToken = strtok ( NULL, strDelimiter ) )
        {
            // Add the token to the table
            lua_pushnumber ( luaVM, ++uiCount );
            lua_pushstring ( luaVM, szToken );
            lua_settable ( luaVM, -3 );
        }

        // Delete the text
        delete [] strText;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    return 1;
}


int CLuaFunctionDefs::SetTimer ( lua_State* luaVM )
{
//  timer setTimer ( function theFunction, int timeInterval, int timesToExecute, [ var arguments... ] )
    CLuaFunctionRef iLuaFunction; double dTimeInterval; uint uiTimesToExecute; CLuaArguments Arguments;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadFunction ( iLuaFunction );
    argStream.ReadNumber ( dTimeInterval );
    argStream.ReadNumber ( uiTimesToExecute );
    argStream.ReadLuaArguments ( Arguments );
    argStream.ReadFunctionComplete ();

    if ( !argStream.HasErrors () )
    {
        CLuaMain * luaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( luaMain )
        {
            // Check for the minimum interval
            if ( dTimeInterval < LUA_TIMER_MIN_INTERVAL )
            {
                argStream.SetCustomError( "Interval is below 50" );
            }
            else
            {
                CLuaTimer* pLuaTimer = luaMain->GetTimerManager ()->AddTimer ( iLuaFunction, CTickCount ( dTimeInterval ), uiTimesToExecute, Arguments );
                if ( pLuaTimer )
                {
                    // Set our timer debug info (in case we don't have any debug info which is usually when you do setTimer(destroyElement, 50, 1) or such)
                    pLuaTimer->SetLuaDebugInfo ( g_pClientGame->GetScriptDebugging()->GetLuaDebugInfo( luaVM ) );

                    lua_pushtimer ( luaVM, pLuaTimer );
                    return 1;
                }
            }
        }
    }
    if ( argStream.HasErrors () )
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::KillTimer ( lua_State* luaVM )
{
//  bool killTimer ( timer theTimer )
    CLuaTimer* pLuaTimer;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pLuaTimer );

    if ( !argStream.HasErrors () )
    {
        CLuaMain * luaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( luaMain )
        {
            luaMain->GetTimerManager ()->RemoveTimer ( pLuaTimer );

            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::ResetTimer ( lua_State* luaVM )
{
//  bool resetTimer ( timer theTimer )
    CLuaTimer* pLuaTimer;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pLuaTimer );

    if ( !argStream.HasErrors () )
    {
        CLuaMain * luaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( luaMain )
        {
            luaMain->GetTimerManager ()->ResetTimer ( pLuaTimer );

            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetTimers ( lua_State* luaVM )
{
//  table getTimers ( [ time ] )
    double dTime;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( dTime, 0 );

    if ( !argStream.HasErrors () )
    {
        // Find our VM
        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( pLuaMain )
        {
            // Create a new table
            lua_newtable ( luaVM );

            // Add all the timers with less than ulTime left
            CLuaTimerManager* pLuaTimerManager = pLuaMain->GetTimerManager ();
            CTickCount llCurrentTime = CTickCount::Now ();
            unsigned int uiIndex = 0;
            CFastList < CLuaTimer* > ::const_iterator iter = pLuaTimerManager->IterBegin ();
            for ( ; iter != pLuaTimerManager->IterEnd () ; iter++ )
            {
                CLuaTimer* pLuaTimer = *iter;

                // If the time left is less than the time specified, or the time specifed is 0
                CTickCount llTimeLeft = ( pLuaTimer->GetStartTime () + pLuaTimer->GetDelay () ) - llCurrentTime;
                if ( dTime == 0 || llTimeLeft.ToDouble () <= dTime )
                {
                    // Add it to the table
                    lua_pushnumber ( luaVM, ++uiIndex );
                    lua_pushtimer ( luaVM, pLuaTimer );
                    lua_settable ( luaVM, -3 );
                }
            }
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsTimer ( lua_State* luaVM )
{
//  bool isTimer ( timer theTimer )
    CLuaTimer* pLuaTimer;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pLuaTimer );

    if ( !argStream.HasErrors () )
    {
        lua_pushboolean ( luaVM, true );
        return 1;
    }

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetTimerDetails ( lua_State* luaVM )
{
//  int, int, int getTimerDetails ( timer theTimer )
    CLuaTimer* pLuaTimer;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pLuaTimer );

    if ( !argStream.HasErrors () )
    {
        lua_pushnumber( luaVM, pLuaTimer->GetTimeLeft ().ToDouble () );
        lua_pushnumber( luaVM, pLuaTimer->GetRepeats () );
        lua_pushnumber( luaVM, pLuaTimer->GetDelay ().ToDouble () );
        return 3;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetTickCount_ ( lua_State* luaVM )
{
    double dCount;
    if ( CStaticFunctionDefinitions::GetTickCount_ ( dCount ) )
    {
        lua_pushnumber ( luaVM, dCount );
        return 1;
    }

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetCTime ( lua_State* luaVM )
{
    // table getRealTime( [int seconds = current], bool localTime = true )
    time_t timer;
    time ( &timer );
    bool bLocalTime = true;
    CScriptArgReader argStream ( luaVM );

    if ( argStream.NextCouldBeNumber ( ) )
    {
        argStream.ReadNumber ( timer );
        if ( timer < 0 )
        {
            argStream.SetCustomError ( "seconds cannot be negative" );
        }
    }

    if ( argStream.NextIsBool() )
        argStream.ReadBool( bLocalTime );

    tm * time;
    if ( bLocalTime )
        time = localtime( &timer );
    else
        time = gmtime( &timer );

    if ( time == NULL )
        argStream.SetCustomError ( "seconds is out of range" );

    if ( argStream.HasErrors () )
    {
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );
        lua_pushboolean ( luaVM, false );
        return 1;
    }

    CLuaArguments ret;
    ret.PushString("second");
    ret.PushNumber(time->tm_sec);
    ret.PushString("minute");
    ret.PushNumber(time->tm_min);
    ret.PushString("hour");
    ret.PushNumber(time->tm_hour);
    ret.PushString("monthday");
    ret.PushNumber(time->tm_mday);
    ret.PushString("month");
    ret.PushNumber(time->tm_mon);
    ret.PushString("year");
    ret.PushNumber(time->tm_year);
    ret.PushString("weekday");
    ret.PushNumber(time->tm_wday);
    ret.PushString("yearday");
    ret.PushNumber(time->tm_yday);
    ret.PushString("isdst");
    ret.PushNumber(time->tm_isdst);
    ret.PushString("timestamp");
    ret.PushNumber((double) timer);

    ret.PushAsTable(luaVM);

    return 1;
}
int CLuaFunctionDefs::tocolor ( lua_State* luaVM )
{
//  int tocolor ( int red, int green, int blue [, int alpha = 255] )
    int iRed; int iGreen; int iBlue; int iAlpha;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( iRed );
    argStream.ReadNumber ( iGreen );
    argStream.ReadNumber ( iBlue );
    argStream.ReadNumber ( iAlpha, 255 );

    if ( !argStream.HasErrors () )
    {
        // Make it into an unsigned long
        unsigned long ulColor = COLOR_RGBA ( iRed, iGreen, iBlue, iAlpha );
        lua_pushinteger ( luaVM, static_cast < lua_Integer > ( ulColor ) );
        return 1;
    }

    // Make it black so funcs dont break
    unsigned long ulColor = COLOR_RGBA ( 0, 0, 0, 255 );
    lua_pushnumber ( luaVM, static_cast < lua_Number > ( ulColor ) );
    return 1;
}

int CLuaFunctionDefs::Reference ( lua_State* luaVM )
{
    CScriptArgReader argStream ( luaVM );
    if ( argStream.NextIsTable ( ) || argStream.NextIsFunction ( ) ||
        argStream.NextIsUserData ( ) )
    {
        int iPointer = lua_ref ( luaVM, 1 );
        lua_pushnumber ( luaVM, iPointer );
        return 1;
    }
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::Dereference ( lua_State* luaVM )
{
    int iPointer = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( iPointer );

    if ( !argStream.HasErrors ( ) )
    {
        lua_getref ( luaVM, iPointer );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetColorFromString ( lua_State* luaVM )
{
//  int int int int getColorFromString ( string theColor )
    SString strColor ;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strColor );

    if ( !argStream.HasErrors () )
    {
        unsigned char ucColorRed, ucColorGreen, ucColorBlue, ucColorAlpha;
        if ( XMLColorToInt ( strColor, ucColorRed, ucColorGreen, ucColorBlue, ucColorAlpha ) )
        {
            lua_pushnumber ( luaVM, ucColorRed );
            lua_pushnumber ( luaVM, ucColorGreen );
            lua_pushnumber ( luaVM, ucColorBlue );
            lua_pushnumber ( luaVM, ucColorAlpha );
            return 4;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetValidPedModels ( lua_State* luaVM )
{
    int iIndex = 0;
    lua_newtable ( luaVM );
    for( int i = 0; i <= 312; i++)
    {
        if ( CClientPlayerManager::IsValidModel(i) )
        {
            lua_pushnumber ( luaVM , ++iIndex);
            lua_pushnumber ( luaVM , i);
            lua_settable ( luaVM , -3);
        }
    }

    return 1;
}


int CLuaFunctionDefs::GetDistanceBetweenPoints2D ( lua_State* luaVM )
{
//  float getDistanceBetweenPoints2D ( float x1, float y1, float x2, float y2 )
    CVector2D vecPointA; CVector2D vecPointB;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadVector2D ( vecPointA );
    argStream.ReadVector2D ( vecPointB );

    if ( !argStream.HasErrors () )
    {
        // Return the distance
        lua_pushnumber ( luaVM, DistanceBetweenPoints2D ( vecPointA, vecPointB ) );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetDistanceBetweenPoints3D ( lua_State* luaVM )
{
//  float getDistanceBetweenPoints3D ( float x1, float y1, float z1, float x2, float y2, float z2 )
    CVector vecPointA; CVector vecPointB;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadVector3D ( vecPointA );
    argStream.ReadVector3D ( vecPointB );

    if ( !argStream.HasErrors () )
    {
        // Return the distance
        lua_pushnumber ( luaVM, DistanceBetweenPoints3D ( vecPointA, vecPointB ) );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetEasingValue ( lua_State* luaVM )
{
//  float getEasingValue( float fProgress, string strEasingType [, float fEasingPeriod, float fEasingAmplitude, float fEasingOvershoot] )
    float fProgress; CEasingCurve::eType easingType; float fEasingPeriod; float fEasingAmplitude; float fEasingOvershoot;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( fProgress );
    argStream.ReadEnumString ( easingType );
    argStream.ReadNumber ( fEasingPeriod, 0.3f );
    argStream.ReadNumber ( fEasingAmplitude, 1.0f );
    argStream.ReadNumber ( fEasingOvershoot, 1.70158f );

    if ( argStream.HasErrors () )
    {
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );
        lua_pushboolean ( luaVM, false );
        return 1;
    }

    CEasingCurve easingCurve ( easingType );
    easingCurve.SetParams ( fEasingPeriod, fEasingAmplitude, fEasingOvershoot );
    lua_pushnumber ( luaVM, easingCurve.ValueForProgress ( fProgress ) ); 
    return 1;
}

int CLuaFunctionDefs::InterpolateBetween ( lua_State* luaVM )
{
//  float float float interpolateBetween ( float x1, float y1, float z1, 
//      float x2, float y2, float z2, 
//      float fProgress, string strEasingType, 
//      [ float fEasingPeriod, float fEasingAmplitude, float fEasingOvershoot ] )
    CVector vecPointA; CVector vecPointB;
    float fProgress; CEasingCurve::eType easingType;
    float fEasingPeriod; float fEasingAmplitude; float fEasingOvershoot;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadVector3D ( vecPointA );
    argStream.ReadVector3D ( vecPointB );
    argStream.ReadNumber ( fProgress );
    argStream.ReadEnumString ( easingType );
    argStream.ReadNumber ( fEasingPeriod, 0.3f );
    argStream.ReadNumber ( fEasingAmplitude, 1.0f );
    argStream.ReadNumber ( fEasingOvershoot, 1.70158f );

    if ( argStream.HasErrors () )
    {
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );
        lua_pushboolean ( luaVM, false );
        return 1;
    }

    CVector vecResult = TInterpolation < CVector >::Interpolate ( vecPointA, vecPointB, fProgress, easingType, fEasingPeriod, fEasingAmplitude, fEasingOvershoot );
    lua_pushnumber ( luaVM, vecResult.fX );
    lua_pushnumber ( luaVM, vecResult.fY );
    lua_pushnumber ( luaVM, vecResult.fZ );
    return 3;
}


int CLuaFunctionDefs::Md5 ( lua_State* luaVM )
{
    SString strMd5 = "";
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strMd5 );

    if ( !argStream.HasErrors ( ) )
    {
        MD5 md5bytes;
        char szResult[33];
        CMD5Hasher hasher;
        hasher.Calculate ( strMd5, strMd5.length ( ), md5bytes );
        hasher.ConvertToHex ( md5bytes, szResult );
        lua_pushstring ( luaVM, szResult );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::Sha256 ( lua_State* luaVM )
{
//  string sha256 ( string str )
    SString strSourceData;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strSourceData );

    if ( !argStream.HasErrors () )
    {
        SString strResult = GenerateSha256HexString ( strSourceData );
        lua_pushstring ( luaVM, strResult );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::Hash ( lua_State* luaVM )
{
//  string hash ( string type, string data )
    EHashFunctionType hashFunction; SString strSourceData;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadEnumString ( hashFunction );
    argStream.ReadString ( strSourceData );

    if ( !argStream.HasErrors () )
    {
        SString strResult = GenerateHashHexString ( hashFunction, strSourceData );
        lua_pushstring ( luaVM, strResult.ToLower() );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::TeaEncode ( lua_State* luaVM )
{
    SString str;
    SString key;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( str );
    argStream.ReadString ( key );

    if ( !argStream.HasErrors() )
    {
        SString result;
        SString humanReadableResult;
        SharedUtil::TeaEncode ( str, key, &result );
        humanReadableResult = Base64::encode ( result );
        lua_pushstring ( luaVM, humanReadableResult );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::TeaDecode ( lua_State* luaVM )
{
    SString str;
    SString key;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( str );
    argStream.ReadString ( key );

    if ( !argStream.HasErrors() )
    {
        SString result;
        Base64::decode ( str, result );
        SharedUtil::TeaDecode ( result, key, &str );
        lua_pushstring ( luaVM, str );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::Base64encode ( lua_State* luaVM )
{
    SString str;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( str );

    if ( !argStream.HasErrors() )
    {
        SString result = Base64::encode ( str );
        lua_pushstring ( luaVM, result );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::Base64decode ( lua_State* luaVM )
{
    SString str;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( str );

    if ( !argStream.HasErrors() )
    {
        SString result;
        Base64::decode ( str, result );
        lua_pushlstring ( luaVM, result, result.length() );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetNetworkUsageData ( lua_State* luaVM )
{
    SPacketStat m_PacketStats [ 2 ] [ 256 ];
    memcpy ( m_PacketStats, g_pNet->GetPacketStats (), sizeof ( m_PacketStats ) );

    lua_createtable ( luaVM, 0, 2 );

    lua_pushstring ( luaVM, "in" );
    lua_createtable ( luaVM, 0, 2 );
    {
        lua_pushstring ( luaVM, "bits" );
        lua_createtable ( luaVM, 255, 1 );
        for ( unsigned int i = 0; i < 256; ++i )
        {
            const SPacketStat& statIn = m_PacketStats [ CNet::STATS_INCOMING_TRAFFIC ] [ i ];
            lua_pushnumber ( luaVM, statIn.iTotalBytes * 8 );
            lua_rawseti ( luaVM, -2, i );
        }
        lua_rawset ( luaVM, -3 );

        lua_pushstring ( luaVM, "count" );
        lua_createtable ( luaVM, 255, 1 );
        for ( unsigned int i = 0; i < 256; ++i )
        {
            const SPacketStat& statIn = m_PacketStats [ CNet::STATS_INCOMING_TRAFFIC ] [ i ];
            lua_pushnumber ( luaVM, statIn.iCount );
            lua_rawseti ( luaVM, -2, i );
        }
        lua_rawset ( luaVM, -3 );
    }
    lua_rawset ( luaVM, -3 );

    lua_pushstring ( luaVM, "out" );
    lua_createtable ( luaVM, 0, 2 );
    {
        lua_pushstring ( luaVM, "bits" );
        lua_createtable ( luaVM, 255, 1 );
        for ( unsigned int i = 0; i < 256; ++i )
        {
            const SPacketStat& statOut = m_PacketStats [ CNet::STATS_OUTGOING_TRAFFIC ] [ i ];
            lua_pushnumber ( luaVM, statOut.iTotalBytes * 8 );
            lua_rawseti ( luaVM, -2, i );
        }
        lua_rawset ( luaVM, -3 );

        lua_pushstring ( luaVM, "count" );
        lua_createtable ( luaVM, 255, 1 );
        for ( unsigned int i = 0; i < 256; ++i )
        {
            const SPacketStat& statOut = m_PacketStats [ CNet::STATS_OUTGOING_TRAFFIC ] [ i ];
            lua_pushnumber ( luaVM, statOut.iCount );
            lua_rawseti ( luaVM, -2, i );
        }
        lua_rawset ( luaVM, -3 );
    }
    lua_rawset ( luaVM, -3 );

    return 1;
}


int CLuaFunctionDefs::GetNetworkStats ( lua_State* luaVM )
{
    NetStatistics stats;
    if ( g_pNet->GetNetworkStatistics ( &stats ) )
    {
        lua_createtable ( luaVM, 0, 11 );

        lua_pushstring ( luaVM, "bytesReceived" );
        lua_pushnumber ( luaVM, static_cast < double > ( stats.bytesReceived ) );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "bytesSent" );
        lua_pushnumber ( luaVM, static_cast < double > ( stats.bytesSent ) );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "packetsReceived" );
        lua_pushnumber ( luaVM, stats.packetsReceived );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "packetsSent" );
        lua_pushnumber ( luaVM, stats.packetsSent );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "packetlossTotal" );
        lua_pushnumber ( luaVM, stats.packetlossTotal );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "packetlossLastSecond" );
        lua_pushnumber ( luaVM, stats.packetlossLastSecond );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "messagesInSendBuffer" );
        lua_pushnumber ( luaVM, stats.messagesInSendBuffer );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "messagesInResendBuffer" );
        lua_pushnumber ( luaVM, stats.messagesInResendBuffer );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "isLimitedByCongestionControl" );
        lua_pushnumber ( luaVM, stats.isLimitedByCongestionControl ? 1 : 0 );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "isLimitedByOutgoingBandwidthLimit" );
        lua_pushnumber ( luaVM, stats.isLimitedByOutgoingBandwidthLimit ? 1 : 0 );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "encryptionStatus" );
        lua_pushnumber ( luaVM, 1 );
        lua_settable   ( luaVM, -3 );

        return 1;
    }

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::IsOOPEnabled ( lua_State* luaVM )
{
    CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
    if ( pLuaMain )
        lua_pushboolean ( luaVM, pLuaMain->IsOOPEnabled () );
    else
        lua_pushnil ( luaVM );

    return 1;
}


int CLuaFunctionDefs::GetVersion ( lua_State* luaVM )
{
    lua_createtable ( luaVM, 0, 8 );

    lua_pushstring ( luaVM, "number" );
    lua_pushnumber ( luaVM, CStaticFunctionDefinitions::GetVersion () );
    lua_settable   ( luaVM, -3 );

    lua_pushstring ( luaVM, "mta" );
    lua_pushstring ( luaVM, CStaticFunctionDefinitions::GetVersionString () );
    lua_settable   ( luaVM, -3 );

    lua_pushstring ( luaVM, "name" );
    lua_pushstring ( luaVM, CStaticFunctionDefinitions::GetVersionName () );
    lua_settable   ( luaVM, -3 );

    lua_pushstring ( luaVM, "netcode" );
    lua_pushnumber ( luaVM, CStaticFunctionDefinitions::GetNetcodeVersion () );
    lua_settable   ( luaVM, -3 );

    lua_pushstring ( luaVM, "os" );
    lua_pushstring ( luaVM, CStaticFunctionDefinitions::GetOperatingSystemName () );
    lua_settable   ( luaVM, -3 );

    lua_pushstring ( luaVM, "type" );
    lua_pushstring ( luaVM, CStaticFunctionDefinitions::GetVersionBuildType () );
    lua_settable   ( luaVM, -3 );

    lua_pushstring ( luaVM, "tag" );
    lua_pushstring ( luaVM, CStaticFunctionDefinitions::GetVersionBuildTag () );
    lua_settable   ( luaVM, -3 );

    lua_pushstring ( luaVM, "sortable" );
    lua_pushstring ( luaVM, CStaticFunctionDefinitions::GetVersionSortable () );
    lua_settable   ( luaVM, -3 );

    return 1;
}

int CLuaFunctionDefs::GetLocalization ( lua_State* luaVM )
{
    lua_createtable ( luaVM, 0, 2 );

    lua_pushstring ( luaVM, "code" );
    lua_pushstring ( luaVM, g_pCore->GetLocalization()->GetLanguageCode ().c_str() );
    lua_settable   ( luaVM, -3 );

    lua_pushstring ( luaVM, "name" );
    lua_pushstring ( luaVM, g_pCore->GetLocalization()->GetLanguageName ().c_str() );
    lua_settable   ( luaVM, -3 );

    return 1;
}

int CLuaFunctionDefs::UtfLen ( lua_State* luaVM )
{

    SString strInput = "";
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strInput );

    if ( !argStream.HasErrors ( ) )
    {
        lua_pushnumber ( luaVM, MbUTF8ToUTF16(strInput).size() );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );
    
    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::UtfSeek ( lua_State* luaVM )
{
    SString strInput = "";
    int iPos = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strInput );
    argStream.ReadNumber ( iPos );

    if ( !argStream.HasErrors ( ) )
    {
        std::wstring strUTF = MbUTF8ToUTF16(strInput);
        if ( iPos <= static_cast < int >(strUTF.size()) && iPos >= 0 )
        {
            strUTF = strUTF.substr(0,iPos);
            lua_pushnumber ( luaVM, UTF16ToMbUTF8(strUTF).size() );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::UtfSub ( lua_State* luaVM )
{
    SString strInput = "";
    int iStart = 0, iEnd = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strInput );
    argStream.ReadNumber ( iStart );
    argStream.ReadNumber ( iEnd );

    if ( !argStream.HasErrors ( ) )
    {
        std::wstring strUTF = MbUTF8ToUTF16 ( strInput );
        size_t l = static_cast < int > ( strUTF.size() );

        ptrdiff_t start = iStart;
        ptrdiff_t end = iEnd;

        //posrelat them both
        if (start < 0) start += (ptrdiff_t)l + 1;
            start = (start >= 0) ? start : 0;

        if (end < 0) end += (ptrdiff_t)l + 1;
            end = (end >= 0) ? end : 0;

        if (start < 1) start = 1;
        if (end > (ptrdiff_t)l) end = (ptrdiff_t)l;
        if (start <= end)
        {
            strUTF = strUTF.substr(start-1, end-start+1);
            lua_pushstring(luaVM, UTF16ToMbUTF8(strUTF).c_str());
            return 1;
        }
        else
        {
            lua_pushliteral(luaVM, "");
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );

    return 1;
}

int CLuaFunctionDefs::UtfChar ( lua_State* luaVM )
{
    int iChar = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( iChar );

    if ( !argStream.HasErrors ( ) )
    {
        if ( iChar > 65534 || iChar < 32 )
        {
            m_pScriptDebugging->LogBadType ( luaVM );
            lua_pushnil ( luaVM );
            return 1;
        }

        // Generate a null-terminating string for our character
        wchar_t wUNICODE[2] = { iChar, '\0' };

        // Convert our UTF character into an ANSI string
        std::string strANSI = UTF16ToMbUTF8(wUNICODE);

        lua_pushstring ( luaVM, strANSI.c_str() );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::UtfCode ( lua_State* luaVM )
{
    SString strInput = "";
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strInput );

    if ( !argStream.HasErrors ( ) )
    {
        std::wstring strUTF = MbUTF8ToUTF16(strInput);
        unsigned long ulCode = strUTF.c_str()[0];

        lua_pushnumber ( luaVM, ulCode );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetPerformanceStats ( lua_State* luaVM )
{
//  table getPerformanceStats ( string category, string options, string filter )
    SString strCategory, strOptions, strFilter;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strCategory );
    argStream.ReadString ( strOptions, "" );
    argStream.ReadString ( strFilter, "" );

    if ( !argStream.HasErrors () )
    {
        CClientPerfStatResult Result;
        CClientPerfStatManager::GetSingleton ()->GetStats ( &Result, strCategory, strOptions, strFilter );

        lua_newtable ( luaVM );
        for ( int c = 0; c < Result.ColumnCount () ; c++ )
        {
            const SString& name = Result.ColumnName ( c );
            lua_pushnumber ( luaVM, c+1 );                      // row index number (starting at 1, not 0)
            lua_pushlstring ( luaVM, name.c_str (), name.length() );
            lua_settable ( luaVM, -3 );
        }

        lua_newtable ( luaVM );
        for ( int r = 0; r < Result.RowCount () ; r++ )
        {
            lua_newtable ( luaVM );                             // new table
            lua_pushnumber ( luaVM, r+1 );                      // row index number (starting at 1, not 0)
            lua_pushvalue ( luaVM, -2 );                        // value
            lua_settable ( luaVM, -4 );                         // refer to the top level table

            for ( int c = 0; c < Result.ColumnCount () ; c++ )
            {
                SString& cell = Result.Data ( c, r );
                lua_pushnumber ( luaVM, c+1 );
                lua_pushlstring ( luaVM, cell.c_str (), cell.length () );
                lua_settable ( luaVM, -3 );
            }
            lua_pop ( luaVM, 1 );                               // pop the inner table
        }
        return 2;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetDevelopmentMode ( lua_State* luaVM )
{
//  bool setDevelopmentMode ( bool enable )
//  bool setDevelopmentMode ( string command )
    bool bEnable; SString strCommand;

    CScriptArgReader argStream ( luaVM );
    if ( argStream.NextIsString () )
    {
        argStream.ReadString ( strCommand );
        //g_pClientGame->SetDevSetting ( strCommand );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    argStream.ReadBool ( bEnable );

    if ( !argStream.HasErrors () )
    {
        g_pClientGame->SetDevelopmentMode ( bEnable );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetDevelopmentMode ( lua_State* luaVM )
{
//  bool getDevelopmentMode ()
    bool bResult = g_pClientGame->GetDevelopmentMode ();
    lua_pushboolean ( luaVM, bResult );
    return 1;
}


int CLuaFunctionDefs::DownloadFile ( lua_State* luaVM )
{
    SString strFile = "";
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strFile );

    if ( !argStream.HasErrors ( ) )
    {
        // Grab our VM
        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( pLuaMain )
        {
            // Grab its resource
            CResource * pResource = pLuaMain->GetResource();
            if ( pResource )
            {
                std::list < CResourceFile* > ::const_iterator iter = pResource->IterBeginResourceFiles();
                for ( ; iter != pResource->IterEndResourceFiles() ; iter++ ) 
                {
                    if ( strcmp ( strFile, (*iter)->GetShortName() ) == 0 )
                    {
                        if ( CStaticFunctionDefinitions::DownloadFile ( pResource, strFile, (*iter)->GetServerChecksum() ) )
                        {
                            lua_pushboolean ( luaVM, true );
                            return 1;
                        }
                    }
                }
                m_pScriptDebugging->LogCustom ( luaVM, 255, 255, 255, "%s: File doesn't exist", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ) );
            }
        }
    }
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::PregFind ( lua_State* luaVM )
{
//  bool pregFind ( string base, string pattern, uint/string flags = 0 )
    SString strBase, strPattern;
    pcrecpp::RE_Options pOptions;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strBase );
    argStream.ReadString ( strPattern );
    ReadPregFlags( argStream, pOptions );

    if ( !argStream.HasErrors () )
    {
        pcrecpp::RE pPattern ( strPattern, pOptions );

        if ( pPattern.PartialMatch ( strBase ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::PregReplace ( lua_State* luaVM )
{
//  string pregReplace ( string base, string pattern, string replace, uint/string flags = 0 )
    SString strBase, strPattern, strReplace;
    pcrecpp::RE_Options pOptions;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strBase );
    argStream.ReadString ( strPattern );
    argStream.ReadString ( strReplace );
    ReadPregFlags( argStream, pOptions );

    if ( !argStream.HasErrors () )
    {
        pcrecpp::RE pPattern ( strPattern, pOptions );

        string strNew = strBase;
        if ( pPattern.GlobalReplace ( strReplace, &strNew ) )
        {
            lua_pushstring ( luaVM, strNew.c_str () );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::PregMatch ( lua_State* luaVM )
{
//  table pregMatch ( string base, string pattern, uint/string flags = 0, int maxResults = 100000 )
    SString strBase, strPattern;
    pcrecpp::RE_Options pOptions;
    int iMaxResults;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strBase );
    argStream.ReadString ( strPattern );
    ReadPregFlags( argStream, pOptions );
    argStream.ReadNumber ( iMaxResults, 100000 );

    if ( !argStream.HasErrors () )
    {
        lua_newtable ( luaVM );

        pcrecpp::RE pPattern ( strPattern, pOptions );

        pcrecpp::StringPiece strInput ( strBase );

        string strGet; int i = 1;
        while ( pPattern.FindAndConsume ( &strInput, &strGet ) && i <= iMaxResults ) 
        {
            lua_pushnumber ( luaVM, i );
            lua_pushstring ( luaVM, strGet.c_str () );
            lua_settable ( luaVM, -3 );   
            i++;
        }
        
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::AddDebugHook ( lua_State* luaVM )
{
//  bool AddDebugHook ( string hookType, function callback[, table allowedNames ] )
    EDebugHookType hookType; CLuaFunctionRef callBack; std::vector < SString > allowedNames;

    CScriptArgReader argStream( luaVM );
    argStream.ReadEnumString( hookType );
    argStream.ReadFunction( callBack );
    argStream.ReadStringTable( allowedNames, true );
    argStream.ReadFunctionComplete ();

    if ( !argStream.HasErrors() )
    {
        if ( g_pClientGame->GetDebugHookManager()->AddDebugHook( hookType, callBack, allowedNames ) )
        {
            lua_pushboolean( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom( luaVM, argStream.GetFullErrorMessage() );

    // Failed
    lua_pushboolean( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::RemoveDebugHook ( lua_State* luaVM )
{
//  bool RemoveDebugHook ( string hookType, function callback )
    EDebugHookType hookType; CLuaFunctionRef callBack;

    CScriptArgReader argStream( luaVM );
    argStream.ReadEnumString( hookType );
    argStream.ReadFunction( callBack );
    argStream.ReadFunctionComplete();

    if ( !argStream.HasErrors() )
    {
        if ( g_pClientGame->GetDebugHookManager()->RemoveDebugHook( hookType, callBack ) )
        {
            lua_pushboolean( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom( luaVM, argStream.GetFullErrorMessage() );

    // Failed
    lua_pushboolean( luaVM, false );
    return 1;
}
