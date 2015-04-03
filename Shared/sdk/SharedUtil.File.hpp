/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        SharedUtil.File.hpp
*  PURPOSE:
*  DEVELOPERS:  
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifdef WIN32
    #include "shellapi.h"
    #include "shlobj.h"
#endif

//
// Returns true if the file exists
//
bool SharedUtil::FileExists ( const SString& strFilename )
{
    FILE* fh = fopen ( strFilename, "rb" );
    if ( !fh )
        return false;
    fclose ( fh );
    return true;
}


//
// Returns true if the directory exists
//
bool SharedUtil::DirectoryExists ( const SString& strPath )
{
#ifdef WIN32
    DWORD dwAtr = GetFileAttributes ( strPath );
    if ( dwAtr == INVALID_FILE_ATTRIBUTES )
        return false;
    return ( ( dwAtr & FILE_ATTRIBUTE_DIRECTORY) != 0 );     
#else
    struct stat Info;
    if ( stat ( strPath, &Info ) == -1 )
        return false;
    return ( S_ISDIR ( Info.st_mode ) );
#endif
}


bool SharedUtil::FileLoad ( const SString& strFilename, SString& strBuffer )
{
    strBuffer = "";
    std::vector < char > buffer;
    if ( !FileLoad ( strFilename, buffer ) )
        return false;
    if ( buffer.size () )
    {
        buffer.push_back ( 0 );
        strBuffer = SString ( &buffer.at ( 0 ), buffer.size () );
    }
    return true;
}

bool SharedUtil::FileSave ( const SString& strFilename, const SString& strBuffer, bool bForce )
{
    return FileSave ( strFilename, strBuffer.length () ? &strBuffer.at ( 0 ) : NULL, strBuffer.length (), bForce );
}

bool SharedUtil::FileAppend ( const SString& strFilename, const SString& strBuffer, bool bForce )
{
    return FileAppend ( strFilename, strBuffer.length () ? &strBuffer.at ( 0 ) : NULL, strBuffer.length (), bForce );
}

bool SharedUtil::FileDelete ( const SString& strFilename, bool bForce )
{
#ifdef WIN32
    if ( bForce )
        SetFileAttributes ( strFilename, FILE_ATTRIBUTE_NORMAL );
#endif
    return unlink ( strFilename ) == 0;
}

//
// Load binary data from a file into an array
//
bool SharedUtil::FileLoad ( const SString& strFilename, std::vector < char >& buffer )
{
    buffer.clear ();
    // Open
    FILE* fh = fopen ( strFilename, "rb" );
    if ( !fh )
        return false;
    // Get size
    fseek ( fh, 0, SEEK_END );
    int size = ftell ( fh );
    rewind ( fh );

    int bytesRead = 0;
    if ( size > 0 && size < 1e9 )
    {
        // Allocate space
        buffer.assign ( size, 0 );
        // Read into buffer
        bytesRead = fread ( &buffer.at ( 0 ), 1, size, fh );
    }
    // Close
    fclose ( fh );
    return bytesRead == size;
}


//
// Save binary data to a file
//
bool SharedUtil::FileSave ( const SString& strFilename, const void* pBuffer, unsigned long ulSize, bool bForce )
{
#ifdef WIN32
    if ( bForce )
        SetFileAttributes ( strFilename, FILE_ATTRIBUTE_NORMAL );
#endif

    if ( bForce )
        MakeSureDirExists ( strFilename );

    FILE* fh = fopen ( strFilename, "wb" );
    if ( !fh )
        return false;

    bool bSaveOk = true;
    if ( ulSize )
        bSaveOk = ( fwrite ( pBuffer, 1, ulSize, fh ) == ulSize );
    fclose ( fh );
    return bSaveOk;
}


//
// Append binary data to a file
//
bool SharedUtil::FileAppend ( const SString& strFilename, const void* pBuffer, unsigned long ulSize, bool bForce )
{
#ifdef WIN32
    if ( bForce )
        SetFileAttributes ( strFilename, FILE_ATTRIBUTE_NORMAL );
#endif

    FILE* fh = fopen ( strFilename, "ab" );
    if ( !fh )
        return false;

    bool bSaveOk = true;
    if ( ulSize )
        bSaveOk = ( fwrite ( pBuffer, 1, ulSize, fh ) == ulSize );
    fclose ( fh );
    return bSaveOk;
}


//
// Get a file size
//
uint SharedUtil::FileSize ( const SString& strFilename  )
{
    // Open
    FILE* fh = fopen ( strFilename, "rb" );
    if ( !fh )
        return 0;
    // Get size
    fseek ( fh, 0, SEEK_END );
    uint size = ftell ( fh );
    // Close
    fclose ( fh );
    return size;
}


//
// Ensure all directories exist to the file
//
void SharedUtil::MakeSureDirExists ( const SString& strPath )
{
    // Copy the path
    char szCopy [MAX_PATH];
    strncpy ( szCopy, strPath, MAX_PATH );

    // Begin from the start
    char cChar = 0;
    char* szIter = szCopy;
    while ( *szIter != 0 )
    {
        // Met a slash?
        cChar = *szIter;
        if ( cChar == '\\' ||
             cChar == '/' )
        {
            // Replace it temprarily with 0
            *szIter = 0;

            // Call mkdir on this path
            #ifdef WIN32
                mkdir ( szCopy );
            #else
                mkdir ( szCopy ,0775 );
            #endif

            // Make it a slash again
            *szIter = cChar;
        }

        // Increment iterator
        ++szIter;
    }
}


SString SharedUtil::PathConform ( const SString& strInPath )
{
    SString strPath = strInPath;
    // '/' to '\'
#if WIN32
    strPath = strPath.Replace ( "/", PATH_SEPERATOR );
#else
    strPath = strPath.Replace ( "\\", PATH_SEPERATOR );
#endif
    // no '\\'
    strPath = strPath.Replace ( PATH_SEPERATOR PATH_SEPERATOR, PATH_SEPERATOR );
    return strPath;
}

SString SharedUtil::PathJoin ( const SString& str1, const SString& str2, const SString& str3, const SString& str4, const SString& str5 )
{
    SString strResult = str1;
    if ( str2.length () )
       strResult += PATH_SEPERATOR + str2; 
    if ( str3.length () )
       strResult += PATH_SEPERATOR + str3; 
    if ( str4.length () )
       strResult += PATH_SEPERATOR + str4; 
    if ( str5.length () )
       strResult += PATH_SEPERATOR + str5;
    return PathConform ( strResult );
}

#ifdef WIN32
#ifdef MTA_CLIENT
SString SharedUtil::GetMTALocalAppDataPath ( void )
{
    char szResult[MAX_PATH] = "";
    SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szResult );
    return PathJoin ( szResult, "MTA San Andreas" + GetVersionAppendString () );
}

SString SharedUtil::GetMTATempPath ( void )
{
    char szResult[4030] = "";
    GetTempPath( 4000, szResult );
    return PathJoin ( szResult, "MTA" + GetVersionAppendString () );
}
#endif
#endif



#ifdef WIN32
///////////////////////////////////////////////////////////////
//
// DelTree
//
//
//
///////////////////////////////////////////////////////////////
bool SharedUtil::DelTree ( const SString& strPath, const SString& strInsideHere )
{
    // Safety: Make sure strPath is inside strInsideHere
    if ( strPath.ToLower ().substr ( 0, strInsideHere.length () ) != strInsideHere.ToLower () )
    {
        assert ( 0 );
        return false;
    }

    DWORD dwBufferSize = strPath.length () + 3;
    char *szBuffer = static_cast < char* > ( alloca ( dwBufferSize ) );
    memset ( szBuffer, 0, dwBufferSize );
    strncpy ( szBuffer, strPath, strPath.length () );
    SHFILEOPSTRUCT sfos;

    sfos.hwnd = NULL;
    sfos.wFunc = FO_DELETE;
    sfos.pFrom = szBuffer;
    sfos.pTo = NULL;
    sfos.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO;

    int status = SHFileOperation(&sfos);
    return status == 0;
}
#endif

#ifdef WIN32
///////////////////////////////////////////////////////////////
//
// MkDir
//
// Returns true if the directory is created or already exists
// TODO: Make bTree off option work
//
///////////////////////////////////////////////////////////////
bool SharedUtil::MkDir ( const SString& strInPath, bool bTree )
{
    SString strPath = PathConform ( strInPath );
    MakeSureDirExists ( strPath + "\\" );
    return FindFiles ( strPath, false, true ).size () > 0;
}
#endif

///////////////////////////////////////////////////////////////
//
// FileCopy
//
// Copies a single file.
//
///////////////////////////////////////////////////////////////
bool SharedUtil::FileCopy ( const SString& strSrc, const SString& strDest, bool bForce )
{
    if ( bForce )
        MakeSureDirExists ( strDest );

#ifdef WIN32
    if ( bForce )
        SetFileAttributes ( strDest, FILE_ATTRIBUTE_NORMAL );
#endif

    FILE* fhSrc = fopen ( strSrc, "rb" );
    if ( !fhSrc )
    {
        return false;
    }

    FILE* fhDst = fopen ( strDest, "wb" );
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


#ifdef WIN32
///////////////////////////////////////////////////////////////
//
// GetCurrentWorkingDirectory
//
//
//
///////////////////////////////////////////////////////////////
SString SharedUtil::GetCurrentWorkingDirectory ( void )
{
    char szCurDir [ 1024 ] = "";
    GetCurrentDirectory ( sizeof ( szCurDir ), szCurDir );
    return szCurDir;
}
#endif


#ifdef WIN32
///////////////////////////////////////////////////////////////
//
// FindFiles
//
// Find all files or directories at a path
//
///////////////////////////////////////////////////////////////
std::vector < SString > SharedUtil::FindFiles ( const SString& strMatch, bool bFiles, bool bDirectories )
{
    std::vector < SString > strResult;

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile ( strMatch, &findData );
    if( hFind != INVALID_HANDLE_VALUE )
    {
        do
        {
            if ( ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? bDirectories : bFiles )
                if ( strcmp ( findData.cFileName, "." ) && strcmp ( findData.cFileName, ".." ) )
                    strResult.push_back ( findData.cFileName );
        }
        while( FindNextFile( hFind, &findData ) );
        FindClose( hFind );
    }
    return strResult;
}
#endif

void SharedUtil::ExtractFilename ( const SString& strPathFilename, SString* strPath, SString* strFilename )
{
    if ( !strPathFilename.Split ( PATH_SEPERATOR, strPath, strFilename, -1 ) )
        if ( strFilename )
            *strFilename = strPathFilename;
}

bool SharedUtil::ExtractExtention ( const SString& strFilename, SString* strMain, SString* strExt )
{
    return strFilename.Split ( ".", strMain, strExt, -1 );
}


#ifdef WIN32
SString SharedUtil::MakeUniquePath ( const SString& strPathFilename )
{
    SString strBeforeUniqueChar, strAfterUniqueChar;

    SString strPath, strFilename;
    ExtractFilename ( strPathFilename, &strPath, &strFilename );

    SString strMain, strExt;
    if ( ExtractExtention ( strFilename, &strMain, &strExt ) )
    {
        strBeforeUniqueChar = PathJoin ( strPath, strMain );
        strAfterUniqueChar = "." + strExt;
    }
    else
    {
        strBeforeUniqueChar = strPathFilename;
        strAfterUniqueChar = "";
    }

    SString strTest = strPathFilename;
    int iCount = 1;
    while ( GetFileAttributes ( strTest ) != INVALID_FILE_ATTRIBUTES )
    {
        strTest = SString ( "%s_%d%s", strBeforeUniqueChar.c_str (), iCount++, strAfterUniqueChar.c_str () );
    }
    return strTest;
}
#endif

