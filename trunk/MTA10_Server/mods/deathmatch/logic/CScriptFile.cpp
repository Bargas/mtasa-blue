/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CScriptFile.cpp
*  PURPOSE:     Script file element class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CScriptFile::CScriptFile ( const char* szFilename, unsigned long ulMaxSize ) : CElement ( NULL )
{
    // Init
    m_iType = CElement::SCRIPTFILE;
    SetTypeName ( "file" );
    m_pFile = NULL;
    m_strFilename = szFilename ? szFilename : "";
    m_ulMaxSize = ulMaxSize;
}


CScriptFile::~CScriptFile ( void )
{
    // Close the file
    Unload ();
}


bool CScriptFile::Load ( eMode Mode )
{
    // If we haven't already got a file
    if ( !m_pFile )
    {
        switch ( Mode )
        {
            // Open file in read only binary mode
            case MODE_READ:
                m_pFile = fopen ( m_strFilename.c_str (), "rb" );
                break;

            // Open file in read write binary mode.
            case MODE_READWRITE:
                // Try to load the file in rw mode. Use existing content.
                m_pFile = fopen ( m_strFilename.c_str (), "rb+" );
                break;

            // Open file in read write binary mode. Truncate size to 0.
            case MODE_CREATE:
                m_pFile = fopen ( m_strFilename.c_str (), "wb+" );
                break;
        }

        // Return whether we successfully opened it or not
        return m_pFile != NULL;
    }

    // Failed
    return false;
}


void CScriptFile::Unload ( void )
{
    // Loaded?
    if ( m_pFile )
    {
        // Close the file
        fclose ( m_pFile );
        m_pFile = NULL;
    }
}


bool CScriptFile::IsEOF ( void )
{
    // Reached end of file? We're at the end of the file if
    // our pointer equals to the filesize - 1
    return GetPointer () >= GetSize () - 1;
}


long CScriptFile::GetPointer ( void )
{
    return ftell ( m_pFile );
}


long CScriptFile::GetSize ( void )
{
    // Remember current position and seek to the end
    long lCurrentPos = ftell ( m_pFile );
    fseek ( m_pFile, 0, SEEK_END );

    // Retrieve size of file
    long lSize = ftell ( m_pFile );

    // Seek back to where the pointer was
    fseek ( m_pFile, lCurrentPos, SEEK_SET );

    // Return the size
    return lSize;
}


unsigned long CScriptFile::SetPointer ( unsigned long ulPosition )
{
    // Is the new position bigger than the file?
    if ( GetSize () < static_cast < long > ( ulPosition ) )
    {
        // Don't make it bigger than our limit
        if ( ulPosition > m_ulMaxSize )
        {
            ulPosition = m_ulMaxSize;
        }
    }

    // Move the pointer
    fseek ( m_pFile, ulPosition, SEEK_SET );

    // Bigger than file size? Tell the script how far we were able to move it
    unsigned long lSize = static_cast < unsigned long > ( GetSize () );
    if ( ulPosition > lSize  )
    {
        ulPosition = lSize;
    }

    // Return the new position
    return ulPosition;
}


void CScriptFile::SetSize ( unsigned long ulNewSize )
{
    // TODO: A way to truncate a file
}


void CScriptFile::Flush ( void )
{
    fflush ( m_pFile );
}


unsigned long CScriptFile::Read ( unsigned long ulSize, char* pData )
{
    // Try to read data into the given block. Return number of bytes we read.
    return fread ( pData, 1, ulSize, m_pFile );
}


unsigned long CScriptFile::Write ( unsigned long ulSize, const char* pData )
{
    // Write the data into the given block. Return number of bytes we wrote.
    return fwrite ( pData, 1, ulSize, m_pFile );
}
