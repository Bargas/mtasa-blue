/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        xml/CXMLArrayImpl.cpp
*  PURPOSE:     XML array class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

using namespace std;

CStack < unsigned long, 1, INVALID_XML_ID > CXMLArray::m_IDStack;
std::vector < CXMLCommon* >                 CXMLArray::m_Elements;
unsigned long                               CXMLArray::m_ulCapacity = 0;

void CXMLArray::Initialize ( void )
{
    m_ulCapacity = 0;
#ifndef MTA_DEBUG
    ExpandBy ( 10000 );
#endif
}

unsigned long CXMLArray::PopUniqueID ( CXMLCommon* pEntry )
{
    // Add more ID's if required
    if ( m_IDStack.GetUnusedAmount () < 1 )
    {
#ifdef MTA_DEBUG
        ExpandBy ( 5 );
#else
        ExpandBy ( 10000 );
#endif
    }

    // Grab the next unused ID
    unsigned long ulPhysicalIndex = m_IDStack.Pop ();

    // Checks
    assert ( ulPhysicalIndex != INVALID_XML_ID );
    assert ( ulPhysicalIndex <= m_ulCapacity );
    assert ( m_Elements.size () > ulPhysicalIndex );
    assert ( m_Elements [ ulPhysicalIndex ] == NULL );

    m_Elements [ ulPhysicalIndex ] = pEntry;

    // Map to ID
    unsigned long ulLogicalID = ( ulPhysicalIndex + MAX_SERVER_ELEMENTS + MAX_CLIENT_ELEMENTS ) ;
    return ulLogicalID;
}


void CXMLArray::PushUniqueID ( unsigned long ulLogicalID )
{
    // Map to index
    unsigned long ulPhysicalIndex = ulLogicalID - (MAX_SERVER_ELEMENTS + MAX_CLIENT_ELEMENTS);

    // Checks
    assert ( ulLogicalID != INVALID_XML_ID );
    assert ( ulPhysicalIndex <= m_ulCapacity );
    assert ( m_Elements [ ulPhysicalIndex ] );

    m_IDStack.Push ( ulPhysicalIndex );
    m_Elements [ ulPhysicalIndex ] = NULL;
}


void CXMLArray::PushUniqueID ( CXMLCommon* pEntry )
{
    PushUniqueID ( pEntry->GetID () );
}


CXMLCommon* CXMLArray::GetEntry ( unsigned long ulLogicalID )
{
    // Return the element with the given ID

    // Map to index
    unsigned long ulPhysicalIndex = ulLogicalID - (MAX_SERVER_ELEMENTS + MAX_CLIENT_ELEMENTS);

    if ( ulLogicalID != INVALID_XML_ID && ulPhysicalIndex <= m_ulCapacity )
    {
        return m_Elements [ ulPhysicalIndex ];
    }
    return NULL;
}


void CXMLArray::ExpandBy ( unsigned long ulAmount )
{
    m_IDStack.ExpandBy ( ulAmount );
    m_Elements.resize ( m_ulCapacity + ulAmount + 1, NULL );
    m_ulCapacity += ulAmount;
    assert ( m_IDStack.GetCapacity () == m_ulCapacity );
}


unsigned long CXMLArray::GetCapacity ( void )
{
    return m_ulCapacity;
}


unsigned long CXMLArray::GetUnusedAmount ( void )
{
    return m_IDStack.GetUnusedAmount ();
}
