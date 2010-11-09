/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/Utils.cpp
*  PURPOSE:     Miscellaneous utility functions
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Oliver Brown <>
*               Cecill Etheredge <>
*               Kevin Whiteside <>
*               aru <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "net/SyncStructures.h"

#ifdef WIN32
    LONGLONG    g_lTimeCounts = 0;
#else
    struct timeval     g_tvInitialTime;
#endif

#ifndef WIN32
    char * strupr (char* a) {
        char *ret = a;
        while (*a != '\0')
        {
            if (islower (*a))
                *a = toupper (*a);
            ++a;
        }
        return ret;
    }
#endif


#ifndef WIN32
#include <unistd.h>     // For access().
#endif

#include <sys/types.h>  // For stat().
#include <sys/stat.h>   // For stat().

bool DoesDirectoryExist ( const char* szPath )
{
    /* Didn't seem to work on Tweak server
    if ( access ( szPath, 0 ) == 0 )
    {
        struct stat status;
        stat ( szPath, &status );

        if ( status.st_mode & S_IFDIR )
        {
            return true;
        }
    }

    return false;
    */

#ifdef WIN32
    DWORD dwAtr = GetFileAttributes ( szPath );
    if ( dwAtr == INVALID_FILE_ATTRIBUTES )
        return false;
    return ( ( dwAtr & FILE_ATTRIBUTE_DIRECTORY) != 0 );     
#else
    struct stat Info;
    stat ( szPath, &Info );
    return ( S_ISDIR ( Info.st_mode ) );
#endif
}


bool CheckNickProvided ( const char* szNick )
{
    if ( stricmp ( szNick, "admin" ) == 0 )
        return false;
    if ( stricmp ( szNick, "console" ) == 0 )
        return false;
    if ( stricmp ( szNick, "server" ) == 0 )
        return false;
    if ( IsIn ( "`", szNick ) )
        return false;
    return true;
}


float DistanceBetweenPoints2D ( const CVector& vecPosition1, const CVector& vecPosition2 )
{
    float fDistanceX = vecPosition2.fX - vecPosition1.fX;
    float fDistanceY = vecPosition2.fY - vecPosition1.fY;

    return sqrt ( fDistanceX * fDistanceX + fDistanceY * fDistanceY );
}


float DistanceBetweenPoints3D ( const CVector& vecPosition1, const CVector& vecPosition2 )
{
    float fDistanceX = vecPosition2.fX - vecPosition1.fX;
    float fDistanceY = vecPosition2.fY - vecPosition1.fY;
    float fDistanceZ = vecPosition2.fZ - vecPosition1.fZ;

    return sqrt ( fDistanceX * fDistanceX + fDistanceY * fDistanceY + fDistanceZ * fDistanceZ );
}


bool IsPointNearPoint2D ( const CVector& vecPosition1, const CVector& vecPosition2, float fDistance )
{
    if ( DistanceBetweenPoints2D ( vecPosition1, vecPosition2 ) <= fDistance )
    {
        return true;
    }
    else
    {
        return false;
    }
}


bool IsPointNearPoint3D ( const CVector& vecPosition1, const CVector& vecPosition2, float fDistance )
{
    if ( DistanceBetweenPoints3D ( vecPosition1, vecPosition2 ) <= fDistance )
    {
        return true;
    }
    else
    {
        return false;
    }
}


void ReplaceCharactersInString ( char* szString, char cWhat, char cWith )
{
    // Loop through the string looking for what
    size_t sizeString = strlen ( szString );
    for ( size_t sizeIndex = 0; sizeIndex < sizeString; sizeIndex++ )
    {
        // Replace matching character with the given replacement
        if ( szString [sizeIndex] == cWhat )
            szString [sizeIndex] = cWith;
    }
}


void ReplaceOccurrencesInString ( std::string &s, const char *a, const char *b )
{
    int idx = 0;
    while( (idx=s.find_first_of(a, idx)) >= 0 )
        s.replace(idx, 1, b);
}


bool IsIn ( const char* szShortText, const char* szLongText)
{
    if ( szShortText && szLongText )
    {
        char* strShort = new char[strlen(szShortText)+1];
        char* strLong = new char[strlen(szLongText)+1];

        strcpy ( strShort, szShortText );
        strcpy ( strLong, szLongText );

        char* szMatch = strstr(uppercase(strLong), uppercase(strShort));
        if (szMatch != NULL) {
            delete [] strShort;
            delete [] strLong;
            return true;
        }
        delete [] strShort;
        delete [] strLong;
    }
    return false;
}


char* uppercase(char* s)
{
    for(unsigned int i = 0; i < strlen(s); i++)
    {
        s[i] = toupper(s[i]);
    }
    return s;
}


void stripString(char* szString)
{
    if ( szString )
    {
        int offset = 0;
        unsigned int i = 0;
        size_t sizeString = strlen ( szString );
        for ( i = 0; i < sizeString; i++ )
        {
            if ( szString[i] < 32 || szString[i] > 126 )
            {
                offset++;
            }
            else
            {
                szString[i-offset] = szString[i];
            }

        }
        szString[i-offset] = '\0'; 
    }
}

void stripControlCodes(char* szString)
{
    if ( szString )
    {
        unsigned char* pWrite = reinterpret_cast < unsigned char* > ( szString );
        const unsigned char* pRead = pWrite;

        while ( *pRead != '\0' )
        {
            if ( *pRead >= 32 )
            {
                *pWrite = *pRead;
                ++pWrite;
            }
            ++pRead;
        }
        *pWrite = '\0';
    }
}

bool StringBeginsWith ( const char* szText, const char* szBegins )
{
    if ( szText && szBegins )
    {
        unsigned int uiTextLength = strlen ( szText );
        unsigned int uiBeginsLength = strlen ( szBegins );

        if ( uiTextLength < uiBeginsLength )
        {
            return false;
        }

        for ( unsigned int ui = 0; ui < uiBeginsLength; ui++ )
        {
            if ( szText[ui] != szBegins[ui] )
            {
                return false;
            }
        }

        return true;
    }

    return false;
}


bool IsNumericString ( const char* szString )
{
    char szSet [] = "-1234567890";
    return strspn ( szString, szSet ) == strlen ( szString );
}


bool IsNumericString ( const char* szString, size_t sizeString )
{
    char szSet [] = "-1234567890";
    return strspn ( szString, szSet ) == sizeString;
}

unsigned int HashString ( const char* szString )
{
    // Implementation of Bob Jenkin's awesome hash function
    // Ref: http://burtleburtle.net/bob/hash/doobs.html

    register const char* k;             //< pointer to the string data to be hashed
    register unsigned int a, b, c;      //< temporary variables
    register unsigned int len;          //< length of the string left

    unsigned int length = (unsigned int) strlen ( szString );

    k = szString;
    len = length;
    a = b = 0x9e3779b9;
    c = 0xabcdef89;                     // initval, arbitrarily set

    while ( len >= 12 )
    {
        a += ( k[0] + ((unsigned int)k[1]<<8) + ((unsigned int)k[2]<<16)  + ((unsigned int)k[3]<<24) );
        b += ( k[4] + ((unsigned int)k[5]<<8) + ((unsigned int)k[6]<<16)  + ((unsigned int)k[7]<<24) );
        c += ( k[8] + ((unsigned int)k[9]<<8) + ((unsigned int)k[10]<<16) + ((unsigned int)k[11]<<24) );

        // Mix
        a -= b; a -= c; a ^= ( c >> 13 );
        b -= c; b -= a; b ^= ( a << 8 );
        c -= a; c -= b; c ^= ( b >> 13 );
        a -= b; a -= c; a ^= ( c >> 12 );
        b -= c; b -= a; b ^= ( a << 16 );
        c -= a; c -= b; c ^= ( b >> 5 );
        a -= b; a -= c; a ^= ( c >> 3 );
        b -= c; b -= a; b ^= ( a << 10 );
        c -= a; c -= b; c ^= ( b >> 15 );

        k += 12;
        len -= 12;
    }

    // Handle the last 11 remaining bytes
    // Note: All cases fall through

    c += length;        // Lower byte of c gets used for length

    switch ( len )
    {
    case 11: 
        c += ((unsigned int)k[10]<<24);
    case 10: 
        c += ((unsigned int)k[9]<<16);
    case 9: 
        c += ((unsigned int)k[8]<<8);
    case 8: 
        b += ((unsigned int)k[7]<<24);
    case 7: 
        b += ((unsigned int)k[6]<<16);
    case 6: 
        b += ((unsigned int)k[5]<<8);
    case 5: 
        b += k[4];
    case 4: 
        a += ((unsigned int)k[3]<<24);
    case 3: 
        a += ((unsigned int)k[2]<<16);
    case 2: 
        a += ((unsigned int)k[1]<<8);
    case 1: 
        a += k[0];
    }

    // Mix
    a -= b; a -= c; a ^= ( c >> 13 );
    b -= c; b -= a; b ^= ( a << 8 );
    c -= a; c -= b; c ^= ( b >> 13 );
    a -= b; a -= c; a ^= ( c >> 12 );
    b -= c; b -= a; b ^= ( a << 16 );
    c -= a; c -= b; c ^= ( b >> 5 );
    a -= b; a -= c; a ^= ( c >> 3 );
    b -= c; b -= a; b ^= ( a << 10 );
    c -= a; c -= b; c ^= ( b >> 15 );

    return c;
}

void InitializeTime ( void )
{
    #ifdef WIN32
        LARGE_INTEGER lFrequency;
        if ( QueryPerformanceFrequency ( &lFrequency ) )
        {
            g_lTimeCounts = lFrequency.QuadPart / 1000;
        }
        else
        {
            // System doesn't support the high-resolution frequency counter. Fail and quit
            CLogger::ErrorPrintf ( "High-resolution frequency counter unsupported!\n" );
            ExitProcess ( 1 );
        }
    #else
        gettimeofday ( &g_tvInitialTime, 0 );
    #endif
}


void DisconnectPlayer ( CGame* pGame, CPlayer& Player, const char* szMessage )
{
    // Send it to the disconnected player
    Player.Send ( CPlayerDisconnectedPacket ( szMessage ) );

    // Quit him
    pGame->QuitPlayer ( Player );
}


void DisconnectConnectionDesync ( CGame* pGame, CPlayer& Player, unsigned int uiCode )
{
    // Populate a disconnection message
    char szBuffer [128];
    _snprintf ( szBuffer, sizeof ( szBuffer ), "Disconnected: Connection desync (%u)", uiCode );
    szBuffer [127] = 0;

    // Send it to the disconnected player
    Player.Send ( CPlayerDisconnectedPacket ( szBuffer ) );

    // Quit him
    pGame->QuitPlayer ( Player, CClient::QUIT_CONNECTION_DESYNC );
}

bool InitializeSockets ( void )
{
#ifdef WIN32
    WSADATA wsaData;
    if ( WSAStartup ( 0x202, &wsaData ) == SOCKET_ERROR )
    {
        WSACleanup ();
        return false;
    }
#endif
    return true;
}

bool CleanupSockets ( void )
{
#ifdef WIN32
    WSACleanup ();
#endif
    return true;
}


float GetRandomFloat ( void )
{
    return static_cast < float > ( rand () ) / ( static_cast < float > ( RAND_MAX ) + 1.0f );
}


double GetRandomDouble ( void )
{
    return static_cast < double > ( rand () ) / ( static_cast < double > ( RAND_MAX ) + 1.0 );
}


int GetRandom ( int iLow, int iHigh )
{
    double dLow = static_cast < double > ( iLow );
    double dHigh = static_cast < double > ( iHigh );

    return static_cast < int > ( floor ( ( dHigh - dLow + 1.0 ) * GetRandomDouble () ) ) + iLow;
}

bool IsValidFilePath ( const char *szDir )
{
    if ( szDir == NULL ) return false;

    unsigned int uiLen = strlen ( szDir );
    unsigned char c, c_d;
    
    // iterate through the char array
    for ( unsigned int i = 0; i < uiLen; i++ ) {
        c = szDir[i];                                       // current character
        c_d = ( i < ( uiLen - 1 ) ) ? szDir[i+1] : 0;       // one character ahead, if any
        if ( !IsVisibleCharacter ( c ) || c == ':' || ( c == '.' && c_d == '.' ) || ( c == '\\' && c_d == '\\' ) )
            return false;
    }
    return true;
}

unsigned int HexToInt ( const char * szHex )
{
    unsigned int value = 0;
    
    if ( szHex[0] == '0' && ( szHex[0] == 'x' || szHex[0] == 'X' ) )
    {
        szHex += 2;
    }

    while ( *szHex != 0 )
    {
        char c = *szHex++;

        value <<= 4;

        if ( c >= 'A' && c <= 'F' )
        {
            value += c - 'A' + 10;
        }
        else if ( c >= 'a' && c <= 'f' )
        {
            value += c - 'a' + 10;
        }
        else
        {
            value += c - '0';
        }
    }

    return value;
}


bool XMLColorToInt ( const char* szColor, unsigned long& ulColor )
{
    const char validHexChars[] = "0123456789ABCDEFabcdef";

    unsigned int uiLength = strlen ( szColor );
    // Make sure it starts with #
    if ( szColor [0] == '#' )
    {
        // #RGB #RGBA #RRGGBB #RRGGBBAA
        if ( uiLength >= 4 && szColor [4] == 0 )
        {
            // Make a RRGGBBAA string
            unsigned char szTemp [12];
            szTemp [0] = 'F';
            szTemp [1] = 'F';
            szTemp [2] = szColor [3];
            szTemp [3] = szColor [3];
            szTemp [4] = szColor [2];
            szTemp [5] = szColor [2];
            szTemp [6] = szColor [1];
            szTemp [7] = szColor [1];
            szTemp [8] = 0;

            // Make sure it only contains 0-9 and A-F
            if ( strspn ( (const char*) szTemp, validHexChars ) == 8 )
            {
                ulColor = HexToInt ( (const char*) szTemp );
                return true;
            }
        }
        else if ( uiLength >= 5 && szColor [5] == 0 )
        {
            // Make a RRGGBBAA string
            unsigned char szTemp [12];
            szTemp [0] = szColor [4];
            szTemp [1] = szColor [4];
            szTemp [2] = szColor [3];
            szTemp [3] = szColor [3];
            szTemp [4] = szColor [2];
            szTemp [5] = szColor [2];
            szTemp [6] = szColor [1];
            szTemp [7] = szColor [1];
            szTemp [8] = 0;

            // Make sure it only contains 0-9 and A-F
            if ( strspn ( (const char*) szTemp, validHexChars ) == 8 )
            {
                ulColor = HexToInt ( (const char*) szTemp );
                return true;
            }
        }
        else if ( uiLength >= 7 && szColor [7] == 0 )
        {
            // Make a RRGGBBAA string
            unsigned char szTemp [12];
            szTemp [0] = 'F';
            szTemp [1] = 'F';
            szTemp [2] = szColor [5];
            szTemp [3] = szColor [6];
            szTemp [4] = szColor [3];
            szTemp [5] = szColor [4];
            szTemp [6] = szColor [1];
            szTemp [7] = szColor [2];
            szTemp [8] = 0;

            // Make sure it only contains 0-9 and A-F
            if ( strspn ( (const char*) szTemp, validHexChars ) == 8 )
            {
                ulColor = HexToInt ( (const char*) szTemp );
                return true;
            }
        }
        else if ( uiLength >= 9 && szColor [9] == 0 )
        {
            // Copy the string without the pre-#
            unsigned char szTemp [12];
            szTemp [0] = szColor [7];
            szTemp [1] = szColor [8];
            szTemp [2] = szColor [5];
            szTemp [3] = szColor [6];
            szTemp [4] = szColor [3];
            szTemp [5] = szColor [4];
            szTemp [6] = szColor [1];
            szTemp [7] = szColor [2];
            szTemp [8] = 0;

            // Make sure it only contains 0-9 and A-F
            if ( strspn ( (const char*) szTemp, validHexChars ) == 8 )
            {
                ulColor = HexToInt ( (const char*) szTemp );
                return true;
            }
        }
    }

    return false;
}


bool XMLColorToInt ( const char* szColor, unsigned char& ucRed, unsigned char& ucGreen, unsigned char& ucBlue, unsigned char& ucAlpha )
{
    // Convert it to an integer first
    unsigned long ulColor;
    if ( !XMLColorToInt ( szColor, ulColor ) )
    {
        return false;
    }

    // Convert it to red, green, blue and alpha
    ucRed = static_cast < unsigned char > ( ulColor );
    ucGreen = static_cast < unsigned char > ( ulColor >> 8 );
    ucBlue = static_cast < unsigned char > ( ulColor >> 16 );
    ucAlpha = static_cast < unsigned char > ( ulColor >> 24 );
    return true;
}


bool ReadSmallKeysync ( CControllerState& ControllerState, const CControllerState& LastControllerState, NetBitStreamInterface& BitStream )
{
    SSmallKeysyncSync keys;
    if ( !BitStream.Read ( &keys ) )
        return false;

    // Put the result into the controllerstate
    ControllerState.LeftShoulder1   = keys.data.bLeftShoulder1;
    ControllerState.RightShoulder1  = keys.data.bRightShoulder1;
    ControllerState.ButtonSquare    = keys.data.bButtonSquare;
    ControllerState.ButtonCross     = keys.data.bButtonCross;
    ControllerState.ButtonCircle    = keys.data.bButtonCircle;
    ControllerState.ButtonTriangle  = keys.data.bButtonTriangle;
    ControllerState.ShockButtonL    = keys.data.bShockButtonL;
    ControllerState.m_bPedWalk      = keys.data.bPedWalk;

    return true;
}


void WriteSmallKeysync ( const CControllerState& ControllerState, const CControllerState& LastControllerState, NetBitStreamInterface& BitStream )
{
    SSmallKeysyncSync keys;
    keys.data.bLeftShoulder1    = ( ControllerState.LeftShoulder1 != 0 );       // Action / Secondary-Fire
    keys.data.bRightShoulder1   = ( ControllerState.RightShoulder1 != 0 );      // Aim-Weapon / Handbrake
    keys.data.bButtonSquare     = ( ControllerState.ButtonSquare != 0 );        // Jump / Reverse
    keys.data.bButtonCross      = ( ControllerState.ButtonCross != 0 );         // Sprint / Accelerate
    keys.data.bButtonCircle     = ( ControllerState.ButtonCircle != 0 );        // Fire // Fire
    keys.data.bButtonTriangle   = ( ControllerState.ButtonTriangle != 0 );      // Enter/Exit/Special-Attack / Enter/exit
    keys.data.bShockButtonL     = ( ControllerState.ShockButtonL != 0 );        // Crouch / Horn
    keys.data.bPedWalk          = ( ControllerState.m_bPedWalk != 0 );          // Walk / -

    // Write it
    BitStream.Write ( &keys );
}


bool ReadFullKeysync ( CControllerState& ControllerState, NetBitStreamInterface& BitStream )
{
    // Read the keysync
    SFullKeysyncSync keys;
    if ( !BitStream.Read ( &keys ) )
        return false;

    // Put the result into the controllerstate
    ControllerState.LeftShoulder1   = keys.data.bLeftShoulder1;
    ControllerState.RightShoulder1  = keys.data.bRightShoulder1;
    ControllerState.ButtonSquare    = keys.data.bButtonSquare;
    ControllerState.ButtonCross     = keys.data.bButtonCross;
    ControllerState.ButtonCircle    = keys.data.bButtonCircle;
    ControllerState.ButtonTriangle  = keys.data.bButtonTriangle;
    ControllerState.ShockButtonL    = keys.data.bShockButtonL;
    ControllerState.m_bPedWalk      = keys.data.bPedWalk;

    ControllerState.LeftStickX      = keys.data.sLeftStickX;
    ControllerState.LeftStickY      = keys.data.sLeftStickY;

    return true;
}


void WriteFullKeysync ( const CControllerState& ControllerState, NetBitStreamInterface& BitStream )
{
    // Put the controllerstate bools into a key byte
    SFullKeysyncSync keys;
    keys.data.bLeftShoulder1    = ( ControllerState.LeftShoulder1 != 0 );
    keys.data.bRightShoulder1   = ( ControllerState.RightShoulder1 != 0 );
    keys.data.bButtonSquare     = ( ControllerState.ButtonSquare != 0 );
    keys.data.bButtonCross      = ( ControllerState.ButtonCross != 0 );
    keys.data.bButtonCircle     = ( ControllerState.ButtonCircle != 0 );
    keys.data.bButtonTriangle   = ( ControllerState.ButtonTriangle != 0 );
    keys.data.bShockButtonL     = ( ControllerState.ShockButtonL != 0 );
    keys.data.bPedWalk          = ( ControllerState.m_bPedWalk != 0 );
    keys.data.sLeftStickX       = ControllerState.LeftStickX;
    keys.data.sLeftStickY       = ControllerState.LeftStickY;

    // Write it
    BitStream.Write ( &keys );
}
bool IsNametagValid ( const char* szNick )
{
    // Grab the size of the nick. Check that it's not to long or short
    size_t sizeNick = strlen ( szNick );
    if ( sizeNick < MIN_NICK_LENGTH || sizeNick > MAX_NAMETAG_LENGTH )
    {
        return false;
    }

    // Check that each character is valid (visible characters exluding space)
    unsigned char ucTemp;
    for ( size_t i = 0; i < sizeNick; i++ )
    {
        ucTemp = szNick [i];
        if ( ucTemp < 32  )
        {
            return false;
        }
    }

    // nametag is valid, return true
    return true;
}


bool IsNickValid ( const char* szNick )
{
    // Grab the size of the nick. Check that it's within the player 
    size_t sizeNick = strlen ( szNick );
    if ( sizeNick < MIN_NICK_LENGTH || sizeNick > MAX_NICK_LENGTH )
    {
        return false;
    }

    // Check that each character is valid (visible characters exluding space)
    unsigned char ucTemp;
    for ( size_t i = 0; i < sizeNick; i++ )
    {
        ucTemp = szNick [i];
        if ( ucTemp < 33 || ucTemp > 126 )
        {
            return false;
        }
    }

    // Nickname is valid, return true
    return true;
}


void RotateVector ( CVector& vecLine, const CVector& vecRotation )
{
    // Rotate it along the X axis
    // [ 1     0        0    0 ]
    // [ 0   cos a   sin a   0 ]
    // [ 0  -sin a   cos a   0 ]

    float fLineY = vecLine.fY;
    vecLine.fY = cos ( vecRotation.fX ) * fLineY  + sin ( vecRotation.fX ) * vecLine.fZ;
    vecLine.fZ = -sin ( vecRotation.fX ) * fLineY + cos ( vecRotation.fX ) * vecLine.fZ;

    // Rotate it along the Y axis
    // [ cos a   0   -sin a   0 ]
    // [   0     1     0      0 ]
    // [ sin a   0    cos a   0 ]

    float fLineX = vecLine.fX;
    vecLine.fX = cos ( vecRotation.fY ) * fLineX - sin ( vecRotation.fY ) * vecLine.fZ;
    vecLine.fZ = sin ( vecRotation.fY ) * fLineX + cos ( vecRotation.fY ) * vecLine.fZ;

    // Rotate it along the Z axis
    // [  cos a   sin a   0   0 ]
    // [ -sin a   cos a   0   0 ]
    // [    0       0     1   0 ]

    fLineX = vecLine.fX;
    vecLine.fX = cos ( vecRotation.fZ ) * fLineX  + sin ( vecRotation.fZ ) * vecLine.fY;
    vecLine.fY = -sin ( vecRotation.fZ ) * fLineX + cos ( vecRotation.fZ ) * vecLine.fY;
}


void LongToDottedIP ( unsigned long ulIP, char* szDottedIP )
{
    in_addr in;
    in.s_addr = ulIP;;
    char* szTemp = inet_ntoa ( in );
    if ( szTemp )
    {
        strncpy ( szDottedIP, szTemp, 22 );
    }
    else
    {
        szDottedIP [0] = 0;
    }
}

const char* HTMLEscapeString ( const char *szSource )
{
    unsigned long ulLength = strlen ( szSource );
    unsigned long ulMaxSize = 0;
    
    // Count the maximum possible size so we can create an appropriate buffer
    for ( unsigned long i = 0; i < ulLength; i++ ) {
        switch ( szSource[i] ) {
            case '<':   ulMaxSize += 4; break;
            case '>':   ulMaxSize += 4; break;
            case '&':   ulMaxSize += 5; break;
            case '"':   ulMaxSize += 6; break;
            case '\'':  ulMaxSize += 6; break;
            default:    ulMaxSize += 1; break;
        }
    }
    
    // Allocate the buffer
    char * szBuffer = new char [ ulMaxSize ];

    // Concatenate all the characters and their potential replacements
    unsigned long c = 0;
    for ( unsigned long i = 0; i < ulLength; i++ ) {
        switch ( szSource[i] ) {
            case '<':   strcat ( szBuffer, "&lt;" );    break;
            case '>':   strcat ( szBuffer, "&gt;" );    break;
            case '&':   strcat ( szBuffer, "&amp;" );   break;
            case '"':   strcat ( szBuffer, "&quot;" );  break;
            case '\'':  strcat ( szBuffer, "&apos;" );  break;
            default:    szBuffer[c] = szSource[i];      break;
            c++;
        }
    }
    return szBuffer;
}


// Copies a single file.
bool FileCopy ( const char* szPathNameSrc, const char* szPathDst )
{
    FILE* fhSrc = fopen ( szPathNameSrc, "rb" );
    if ( !fhSrc )
    {
        return false;
    }

    FILE* fhDst = fopen ( szPathDst, "wb" );
    if ( !fhDst )
    {
        fclose ( fhSrc );
        return false;
    }

    char cBuffer[16384];
    while ( true )
    {
        size_t dataLength = fread ( cBuffer, 1, 16384, fhSrc );
        if ( dataLength == 0 )
            break;
        fwrite ( cBuffer, 1, dataLength, fhDst );
    }

    fclose ( fhSrc );
    fclose ( fhDst );
    return true;
}
