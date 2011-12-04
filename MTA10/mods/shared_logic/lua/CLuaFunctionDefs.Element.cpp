/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Element.cpp
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

using std::list;

int CLuaFunctionDefs::GetRootElement ( lua_State* luaVM )
{
    CClientEntity* pRoot = CStaticFunctionDefinitions::GetRootElement ();

    // Return the root element
    lua_pushelement ( luaVM, pRoot );
    return 1;
}


int CLuaFunctionDefs::IsElement ( lua_State* luaVM )
{
    // Correct argument type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element. If it's valid, return true.
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementID ( lua_State* luaVM )
{
    // Correct argument type specified?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Get its ID and return it
            const char* szName = pEntity->GetName ();
            if ( szName )
            {
                lua_pushstring ( luaVM, szName );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementID", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementID" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementByID ( lua_State* luaVM )
{
    // Eventually read out the index
    int iArgument2 = lua_type ( luaVM, 2 );
    unsigned int uiIndex = 0;
    if ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING )
    {
        uiIndex = static_cast < unsigned int > ( lua_tonumber ( luaVM, 2 ) );
    }

    // Check that we got the name in place
    if ( lua_istype ( luaVM, 1, LUA_TSTRING ) )
    {
        // Grab the string
        const char* szID = lua_tostring ( luaVM, 1 );

        // Try to find the element with that ID. Return it
        CClientEntity* pEntity = CStaticFunctionDefinitions::GetElementByID ( szID, uiIndex );
        if ( pEntity )
        {
            lua_pushelement ( luaVM, pEntity );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementByID" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementByIndex ( lua_State* luaVM )
{
    // Verify element type
    int iArgument1 = lua_type ( luaVM, 1 );
    int iArgument2 = lua_type ( luaVM, 2 );
    if ( ( iArgument1 == LUA_TSTRING ) &&
        ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
    {
        // Grab the element type name and the index
        const char* szType = lua_tostring ( luaVM, 1 );
        unsigned int uiIndex = static_cast < unsigned int > ( lua_tonumber ( luaVM, 2 ) );

        // Try to find it. Return it if we did.
        CClientEntity* pEntity = CStaticFunctionDefinitions::GetElementByIndex ( szType, uiIndex );
        if ( pEntity )
        {
            lua_pushelement ( luaVM, pEntity );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementByIndex" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementData ( lua_State* luaVM )
{
//  var getElementData ( element theElement, string key [, inherit = true] )
    CClientEntity* pEntity; SString strKey; bool bInherit;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadString ( strKey );
    argStream.ReadBool ( bInherit, true );

    if ( !argStream.HasErrors () )
    {
        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( pLuaMain )
        {
            if ( strKey.length () > MAX_CUSTOMDATA_NAME_LENGTH )
            {
                // Warn and truncate if key is too long
                m_pScriptDebugging->LogCustom ( luaVM, SString ( "Truncated argument @ '%s' [%s]", "getElementData", *SString ( "string length reduced to %d characters at argument 2", MAX_CUSTOMDATA_NAME_LENGTH ) ) );
                strKey = strKey.Left ( MAX_CUSTOMDATA_NAME_LENGTH );
            }

            CLuaArgument* pVariable = pEntity->GetCustomData ( strKey, bInherit );
            if ( pVariable )
            {
                pVariable->Push ( luaVM );
                return 1;
            }
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", "getElementData", *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetElementMatrix ( lua_State* luaVM )
{
    // Verify the argument
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element, verify it
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab the position
            CMatrix matrix;
            if ( CStaticFunctionDefinitions::GetElementMatrix ( *pEntity, matrix ) )
            {            
                // Return it
                lua_createtable ( luaVM, 4, 0 );

                // First row
                lua_createtable ( luaVM, 4, 0 );
                lua_pushnumber ( luaVM, matrix.vRight.fX );
                lua_rawseti ( luaVM, -2, 1 );
                lua_pushnumber ( luaVM, matrix.vRight.fY );
                lua_rawseti ( luaVM, -2, 2 );
                lua_pushnumber ( luaVM, matrix.vRight.fZ );
                lua_rawseti ( luaVM, -2, 3 );
                lua_pushnumber ( luaVM, 1.0f );
                lua_rawseti ( luaVM, -2, 4 );
                lua_rawseti ( luaVM, -2, 1 );

                // Second row
                lua_createtable ( luaVM, 4, 0 );
                lua_pushnumber ( luaVM, matrix.vFront.fX );
                lua_rawseti ( luaVM, -2, 1 );
                lua_pushnumber ( luaVM, matrix.vFront.fY );
                lua_rawseti ( luaVM, -2, 2 );
                lua_pushnumber ( luaVM, matrix.vFront.fZ );
                lua_rawseti ( luaVM, -2, 3 );
                lua_pushnumber ( luaVM, 1.0f );
                lua_rawseti ( luaVM, -2, 4 );
                lua_rawseti ( luaVM, -2, 2 );

                // Third row
                lua_createtable ( luaVM, 4, 0 );
                lua_pushnumber ( luaVM, matrix.vUp.fX );
                lua_rawseti ( luaVM, -2, 1 );
                lua_pushnumber ( luaVM, matrix.vUp.fY );
                lua_rawseti ( luaVM, -2, 2 );
                lua_pushnumber ( luaVM, matrix.vUp.fZ );
                lua_rawseti ( luaVM, -2, 3 );
                lua_pushnumber ( luaVM, 1.0f );
                lua_rawseti ( luaVM, -2, 4 );
                lua_rawseti ( luaVM, -2, 3 );

                // Fourth row
                lua_createtable ( luaVM, 4, 0 );
                lua_pushnumber ( luaVM, matrix.vPos.fX );
                lua_rawseti ( luaVM, -2, 1 );
                lua_pushnumber ( luaVM, matrix.vPos.fY );
                lua_rawseti ( luaVM, -2, 2 );
                lua_pushnumber ( luaVM, matrix.vPos.fZ );
                lua_rawseti ( luaVM, -2, 3 );
                lua_pushnumber ( luaVM, 1.0f );
                lua_rawseti ( luaVM, -2, 4 );
                lua_rawseti ( luaVM, -2, 4 );

                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementMatrix", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementMatrix" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementPosition ( lua_State* luaVM )
{
    // Verify the argument
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element, verify it
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab the position
            CVector vecPosition;
            if ( CStaticFunctionDefinitions::GetElementPosition ( *pEntity, vecPosition ) )
            {            
                // Return it
                lua_pushnumber ( luaVM, vecPosition.fX );
                lua_pushnumber ( luaVM, vecPosition.fY );
                lua_pushnumber ( luaVM, vecPosition.fZ );
                return 3;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementPosition", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementPosition" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementRotation ( lua_State* luaVM )
{
    // Verify the argument
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element, verify it
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );

        const char* szRotationOrder = "default";
        if ( lua_type ( luaVM, 2 ) == LUA_TSTRING ) 
        {
            szRotationOrder = lua_tostring ( luaVM, 2 );
        }

        if ( pEntity )
        {
            // Grab the rotation
            CVector vecRotation;
            if ( CStaticFunctionDefinitions::GetElementRotation ( *pEntity, vecRotation, szRotationOrder ) )
            {            
                // Return it
                lua_pushnumber ( luaVM, vecRotation.fX );
                lua_pushnumber ( luaVM, vecRotation.fY );
                lua_pushnumber ( luaVM, vecRotation.fZ );
                return 3;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementRotation", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementRotation" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementVelocity ( lua_State* luaVM )
{
    // Verify the argument
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element, verify it
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab the velocity
            CVector vecVelocity;
            if ( CStaticFunctionDefinitions::GetElementVelocity ( *pEntity, vecVelocity ) )
            {
                // Return it
                lua_pushnumber ( luaVM, vecVelocity.fX );
                lua_pushnumber ( luaVM, vecVelocity.fY );
                lua_pushnumber ( luaVM, vecVelocity.fZ );
                return 3;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementVelocity", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementVelocity" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementType ( lua_State* luaVM )
{
    // Check the arg type
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Get its typename
            const char* szTypeName = pEntity->GetTypeName ();
            if ( szTypeName )
            {
                lua_pushstring ( luaVM, szTypeName );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementType", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementType" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementChildren ( lua_State* luaVM )
{
    // Find our VM
    CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
    if ( pLuaMain )
    {
        // Correct arg type?
        if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the argument
            CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
            if ( pEntity )
            {
                // Create a new table
                lua_newtable ( luaVM );

                // Add all the elements with a matching type to it
                pEntity->GetChildren ( luaVM );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer ( luaVM, "getElementChildren", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( luaVM, "getElementChildren" );
    }

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementChild ( lua_State* luaVM )
{
    // Verify argument types
    int iArgument1 = lua_type ( luaVM, 1 );
    int iArgument2 = lua_type ( luaVM, 2 );
    if ( ( iArgument1 == LUA_TLIGHTUSERDATA ) &&
        ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
    {
        // Grab entity and index
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        unsigned int uiIndex = static_cast < unsigned int > ( lua_tonumber ( luaVM, 2 ) );
        if ( pEntity )
        {
            // Grab the child
            CClientEntity* pChild = CStaticFunctionDefinitions::GetElementChild ( *pEntity, uiIndex );
            if ( pChild )
            {
                lua_pushelement ( luaVM, pChild );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementChild", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementChild" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementChildrenCount ( lua_State* luaVM )
{
    // Correct arg type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab the count
            unsigned int uiCount = pEntity->CountChildren ();
            lua_pushnumber ( luaVM, static_cast < lua_Number > ( uiCount ) );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementChildrenCount", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementChildrenCount" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementParent ( lua_State* luaVM )
{
    // Correct arg type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the entity
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab the parent and return it
            CClientEntity* pParent = pEntity->GetParent ();
            if ( pParent )
            {
                lua_pushelement ( luaVM, pParent );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementParent", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementParent" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementsByType ( lua_State* luaVM )
{
    // Find our VM
    CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
    if ( pLuaMain )
    {
        // Check arg types
        int iArgument2 = lua_type ( luaVM, 2 );
        if ( lua_type ( luaVM, 1 ) == LUA_TSTRING && 
            ( iArgument2 == LUA_TNONE || iArgument2 == LUA_TLIGHTUSERDATA ) )
        {
            // see if a root argument has been specified
            CClientEntity* startAt = m_pRootEntity;
            if ( iArgument2 == LUA_TLIGHTUSERDATA )
            {
                // if its valid, use that, otherwise, produce an error
                CClientEntity* pEntity = lua_toelement ( luaVM, 2 );
                if ( pEntity )
                    startAt = pEntity;
                else
                {
                    m_pScriptDebugging->LogBadPointer ( luaVM, "getElementsByType", "element", 2 );
                    lua_pushboolean ( luaVM, false );
                    return 1;
                }
            }

            bool bStreamedIn = false;
            if ( lua_type ( luaVM, 3 ) == LUA_TBOOLEAN )
            {
                bStreamedIn = lua_toboolean ( luaVM, 3 ) ? true : false;
            }

            // Grab the argument
            const char* szType = lua_tostring ( luaVM, 1 );

            // Create a new table
            lua_newtable ( luaVM );

            // Add all the elements with a matching type to it
            startAt->FindAllChildrenByType ( szType, luaVM, bStreamedIn );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadType ( luaVM, "getElementsByType" );
    }

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementInterior ( lua_State* luaVM )
{
    // Check arg types
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab the interior and return it
            unsigned char ucInterior;
            if ( CStaticFunctionDefinitions::GetElementInterior ( *pEntity, ucInterior ) )
            {
                lua_pushnumber ( luaVM, ucInterior );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementInterior", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementInterior" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsElementWithinColShape ( lua_State* luaVM )
{
    // Check arg types
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) &&
        lua_istype ( luaVM, 2, LUA_TLIGHTUSERDATA ) )
    {
        // Grab elements
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        CClientColShape* pColShape = lua_tocolshape ( luaVM, 2 );

        // Valid element?
        if ( pEntity )
        {
            // Valid colshape?
            if ( pColShape )
            {
                // Check if it's within
                bool bWithin = pEntity->CollisionExists ( pColShape );
                lua_pushboolean ( luaVM, bWithin );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer ( luaVM, "isElementWithinColShape", "colshape", 2 );
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "isElementWithinColShape", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isElementWithinColShape" );

    // Failed
    lua_pushnil ( luaVM );
    return 1;
}


int CLuaFunctionDefs::IsElementWithinMarker( lua_State* luaVM )
{
    // Check arg types
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) &&
        lua_istype ( luaVM, 2, LUA_TLIGHTUSERDATA ) )
    {
        // Grab elements
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        CClientMarker* pMarker = lua_tomarker ( luaVM, 2 );

        // Valid element?
        if ( pEntity )
        {
            // Valid colshape?
            if ( pMarker )
            {
                // Check if it's within
                bool bWithin = pEntity->CollisionExists ( pMarker->GetColShape() );
                lua_pushboolean ( luaVM, bWithin );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer ( luaVM, "isElementWithinMarker", "marker", 2 );
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "isElementWithinMarker", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isElementWithinMarker" );

    // Failed
    lua_pushnil ( luaVM );
    return 1;
}


int CLuaFunctionDefs::GetElementsWithinColShape ( lua_State* luaVM )
{
    // Grab our VM
    CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
    if ( pLuaMain )
    {
        // Correct arg type?
        if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab optional type arg
            const char* szType = NULL;
            if ( lua_istype ( luaVM, 2, LUA_TSTRING ) )
            {
                // Is it zero length? Don't use it
                szType = lua_tostring ( luaVM, 2 );
                if ( szType [ 0 ] == 0 )
                    szType = NULL;
            }

            // Grab the colshape passed
            CClientColShape* pColShape = lua_tocolshape ( luaVM, 1 );
            if ( pColShape )
            {
                // Create a new table
                lua_newtable ( luaVM );

                // Add all the elements within the shape to it
                unsigned int uiIndex = 0;
                list < CClientEntity* > ::iterator iter = pColShape->CollidersBegin ();
                for ( ; iter != pColShape->CollidersEnd (); iter++ )
                {
                    if ( szType == NULL || strcmp ( (*iter)->GetTypeName (), szType ) == 0 )
                    {
                        lua_pushnumber ( luaVM, ++uiIndex );
                        lua_pushelement ( luaVM, *iter );
                        lua_settable ( luaVM, -3 );
                    }
                }

                // We're returning 1 table
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer ( luaVM, "getElementsWithinColShape", "colshape", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( luaVM, "getElementsWithinColShape" );
    }

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementDimension ( lua_State* luaVM )
{
    // Correct type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element to check
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab the dimension
            unsigned short usDimension = pEntity->GetDimension ();
            lua_pushnumber ( luaVM, usDimension );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementDimension", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementDimension" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementBoundingBox ( lua_State* luaVM )
{
    // Correct type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab the bounding box and return it
            CVector vecMin, vecMax;
            if ( CStaticFunctionDefinitions::GetElementBoundingBox ( *pEntity, vecMin, vecMax ) )
            {
                lua_pushnumber ( luaVM, vecMin.fX );
                lua_pushnumber ( luaVM, vecMin.fY );
                lua_pushnumber ( luaVM, vecMin.fZ );
                lua_pushnumber ( luaVM, vecMax.fX );
                lua_pushnumber ( luaVM, vecMax.fY );
                lua_pushnumber ( luaVM, vecMax.fZ );
                return 6;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementBoundingBox", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementBoundingBox" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementRadius ( lua_State* luaVM )
{
    // Correct type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab its radius and return it
            float fRadius = 0.0f;
            if ( CStaticFunctionDefinitions::GetElementRadius ( *pEntity, fRadius ) )
            {
                lua_pushnumber ( luaVM, fRadius );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementRadius", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementRadius" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsElementAttached ( lua_State* luaVM )
{
    // Correct type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            CClientEntity* pEntityAttachedTo = pEntity->GetAttachedTo();
            if ( pEntityAttachedTo )
            {
                if ( pEntityAttachedTo->IsEntityAttached ( pEntity ) )
                {
                    lua_pushboolean ( luaVM, true );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "isElementAttached", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isElementAttached" );

    // Failed
    lua_pushnil ( luaVM );
    return 1;
}


int CLuaFunctionDefs::GetElementAttachedTo ( lua_State* luaVM )
{
    // Check types
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element to check
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        CClientEntity* pEntityAttachedTo = NULL;

        // Valid?
        if ( pEntity )
        {
            // Grab the enity attached to it
            CClientEntity* pEntityAttachedTo = CStaticFunctionDefinitions::GetElementAttachedTo ( *pEntity );
            if ( pEntityAttachedTo )
            {
                lua_pushelement ( luaVM, pEntityAttachedTo );
                return 1;
            }
            else
            {
                lua_pushboolean ( luaVM, false );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementAttachedTo", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementAttachedTo" );

    // Failed
    lua_pushnil ( luaVM );
    return 1;
}


int CLuaFunctionDefs::GetAttachedElements ( lua_State* luaVM )
{
    // Grab our VM
    CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
    if ( pLuaMain )
    {
        // Correct type?
        if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
            if ( pEntity )
            {
                // Create a new table
                lua_newtable ( luaVM );

                // Add All Attached Elements
                unsigned int uiIndex = 0;
                list < CClientEntity* > ::const_iterator iter = pEntity->AttachedEntitiesBegin ();
                for ( ; iter != pEntity->AttachedEntitiesEnd () ; iter++ )
                {
                    CClientEntity * pAttached = *iter;
                    if ( pAttached->GetAttachedTo () == pEntity )
                    {
                        lua_pushnumber ( luaVM, ++uiIndex );
                        lua_pushelement ( luaVM, *iter );
                        lua_settable ( luaVM, -3 );
                    }
                }
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer ( luaVM, "getAttachedElements", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( luaVM, "getAttachedElements" );
    }

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementDistanceFromCentreOfMassToBaseOfModel ( lua_State* luaVM )
{
    // Valid types?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element to check
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab the distance and return it
            float fDistance;
            if ( CStaticFunctionDefinitions::GetElementDistanceFromCentreOfMassToBaseOfModel ( *pEntity, fDistance ) )
            {
                lua_pushnumber ( luaVM, fDistance );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementDistanceFromCentreOfMassToBaseOfModel", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementDistanceFromCentreOfMassToBaseOfModel" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsElementLocal ( lua_State* luaVM )
{
    // Check types
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Return whether it's local or not
            bool bLocal = pEntity->IsLocalEntity ();;
            lua_pushboolean ( luaVM, bLocal );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "isElementLocal", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isElementLocal" );

    // Failed
    lua_pushnil ( luaVM );
    return 1;
}


int CLuaFunctionDefs::GetElementAttachedOffsets ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA )
    {
        // Grab the attached element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        CVector vecPosition, vecRotation;

        // Valid element?
        if ( pEntity )
        {
            if ( CStaticFunctionDefinitions::GetElementAttachedOffsets ( *pEntity, vecPosition, vecRotation ) )
            {
                lua_pushnumber( luaVM, vecPosition.fX );
                lua_pushnumber( luaVM, vecPosition.fY );
                lua_pushnumber( luaVM, vecPosition.fZ );
                lua_pushnumber( luaVM, vecRotation.fX );
                lua_pushnumber( luaVM, vecRotation.fY );
                lua_pushnumber( luaVM, vecRotation.fZ );
                return 6;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementAttachedOffsets", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementAttachedOffsets" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementAlpha ( lua_State* luaVM )
{
    // Valid type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab its alpha level and return it
            unsigned char ucAlpha;
            if ( CStaticFunctionDefinitions::GetElementAlpha ( *pEntity, ucAlpha ) )
            {
                lua_pushnumber ( luaVM, ucAlpha );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementAlpha", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementAlpha" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementHealth ( lua_State* luaVM )
{
    // Same type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Get the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab the health and return it
            float fHealth;
            if ( CStaticFunctionDefinitions::GetElementHealth ( *pEntity, fHealth ) )
            {
                lua_pushnumber ( luaVM, fHealth );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementHealth", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementHealth" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementModel ( lua_State* luaVM )
{
    // Same type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Get the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            unsigned short usModel;
            if ( CStaticFunctionDefinitions::GetElementModel ( *pEntity, usModel ) )
            {
                lua_pushnumber ( luaVM, usModel );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementModel", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementModel" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetElementColShape ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA )
    {
        // Get the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            CClientEntity* pColShape = static_cast < CClientEntity* > ( CStaticFunctionDefinitions::GetElementColShape ( pEntity ) );
            if ( pColShape )
            {
                lua_pushelement ( luaVM, pColShape );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementColShape", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementColShape" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsElementInWater ( lua_State* luaVM )
{
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the entity and verify it.
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            bool bInWater = false;
            if ( CStaticFunctionDefinitions::IsElementInWater ( *pEntity, bInWater ) )
            {
                lua_pushboolean ( luaVM, bInWater );
                return 1;
            }
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isElementInWater" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsElementSyncer ( lua_State* luaVM )
{
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the entity and verify it.
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            bool bIsSyncer = false;
            if ( CStaticFunctionDefinitions::IsElementSyncer ( *pEntity, bIsSyncer ) )
            {
                lua_pushboolean ( luaVM, bIsSyncer );
                return 1;
            }
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isElementSyncer" );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::IsElementCollidableWith ( lua_State* luaVM )
{
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) &&
         lua_istype ( luaVM, 2, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the entity and verify it.
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        CClientEntity* pWithEntity = lua_toelement ( luaVM, 2 );
        if ( pEntity && pWithEntity )
        {
            bool bCanCollide;
            if ( CStaticFunctionDefinitions::IsElementCollidableWith ( *pEntity, *pWithEntity, bCanCollide ) )
            {
                lua_pushboolean ( luaVM, bCanCollide );
                return 1;
            }
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isElementCollidableWith" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsElementDoubleSided ( lua_State* luaVM )
{
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the entity and verify it.
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            lua_pushboolean ( luaVM, pEntity->IsDoubleSided () );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isElementDoubleSided" );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetElementCollisionsEnabled ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA )
    {
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            if ( CStaticFunctionDefinitions::GetElementCollisionsEnabled ( *pEntity ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }        
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getElementCollisionsEnabled", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getElementCollisionsEnabled" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsElementFrozen ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA )
    {
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            bool bFrozen;
            if ( CStaticFunctionDefinitions::IsElementFrozen ( *pEntity, bFrozen ) )
            {
                lua_pushboolean ( luaVM, bFrozen );
                return 1;
            }        
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "isElementFrozen", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isElementFrozen" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsElementStreamedIn ( lua_State* luaVM )
{
    // We have a correct parameter 1?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the entity and verify it.
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Is this a streaming compatible class?
            if ( pEntity->IsStreamingCompatibleClass () )
            {
                CClientStreamElement* pStreamElement = static_cast < CClientStreamElement* > ( pEntity );

                // Return whether or not this class is streamed in
                lua_pushboolean ( luaVM, pStreamElement->IsStreamedIn () );
                return 1;
            }
            else if ( pEntity->GetType() == CCLIENTSOUND )
            {
                CClientSound* pSound = static_cast < CClientSound* > ( pEntity );
                lua_pushboolean ( luaVM, pSound->IsSoundStopped() ? false : true );
                return 1;
            }
            else
                m_pScriptDebugging->LogWarning ( luaVM, "isElementStreamedIn; element is not streaming compatible\n" );
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "isElementStreamedIn", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isElementStreamedIn" );

    // We failed
    lua_pushnil ( luaVM );
    return 1;
}


int CLuaFunctionDefs::IsElementStreamable ( lua_State* luaVM )
{
    // We have a correct parameter 1?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the entity and verify it.
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Is this a streaming compatible class?
            if ( pEntity->IsStreamingCompatibleClass () )
            {
                CClientStreamElement* pStreamElement = static_cast < CClientStreamElement* > ( pEntity );

                bool bStreamable = ( pStreamElement->GetTotalStreamReferences () == 0 );
                // Return whether or not this element is set to never be streamed out
                lua_pushboolean ( luaVM, bStreamable );
                return 1;
            }
            else
                m_pScriptDebugging->LogWarning ( luaVM, "isElementStreamable; element is not streaming compatible\n" );

            // Return false, we're not streamable
            lua_pushboolean ( luaVM, false );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "isElementStreamable", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isElementStreamable" );

    // We failed
    lua_pushnil ( luaVM );
    return 1;
}


int CLuaFunctionDefs::IsElementOnScreen ( lua_State* luaVM )
{
    // Valid type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Return whether we're on the screen or not
            bool bOnScreen;
            if ( CStaticFunctionDefinitions::IsElementOnScreen ( *pEntity, bOnScreen ) )
            {
                lua_pushboolean ( luaVM, bOnScreen );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "isElementOnScreen", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isElementOnScreen" );

    lua_pushnil ( luaVM );
    return 1;
}


int CLuaFunctionDefs::CreateElement ( lua_State* luaVM )
{
    // Grab our VM
    CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
    if ( pLuaMain )
    {
        // Grab its resource
        CResource * pResource = pLuaMain->GetResource();
        if ( pResource )
        {
            // Grab the optional name argument
            char szDefaultID [] = "";
            const char* szID = szDefaultID;
            int iArgument2 = lua_type ( luaVM, 2 );
            if ( iArgument2 == LUA_TSTRING )
            {
                szID = lua_tostring ( luaVM, 2 );
            }
            else if ( iArgument2 != LUA_TNONE )
            {
                m_pScriptDebugging->LogBadType ( luaVM, "createElement" );
                lua_pushboolean ( luaVM, false );
                return 1;
            }

            // Verify argument types
            if ( lua_istype ( luaVM, 1, LUA_TSTRING ) )
            {
                // Grab the string
                const char* szTypeName = lua_tostring ( luaVM, 1 );

                // Try to create
                CClientDummy* pDummy = CStaticFunctionDefinitions::CreateElement ( *pResource, szTypeName, szID );
                if ( pDummy )
                {
                    // Add it to the element group
                    // TODO: Get rid of element groups
                    CElementGroup * pGroup = pResource->GetElementGroup();
                    if ( pGroup )
                    {
                        pGroup->Add ( ( CClientEntity* ) pDummy );
                    }

                    // Return it
                    lua_pushelement ( luaVM, pDummy );
                    return 1;
                }
                else
                    m_pScriptDebugging->LogError ( luaVM, "createElement; unable to create mor elements\n" );
            }
            else
                m_pScriptDebugging->LogBadType ( luaVM, "createElement" );
        }
    }

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::DestroyElement ( lua_State* luaVM )
{
    // Correct type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Destroy it
            if ( CStaticFunctionDefinitions::DestroyElement ( *pEntity ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "destroyElement", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "destroyElement" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementID ( lua_State* luaVM )
{
    // Correct type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) &&
         lua_istype ( luaVM, 2, LUA_TSTRING ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            const char* szID = lua_tostring ( luaVM, 2 );
            // It returns false if we tried to change ID of server-created element
            if ( CStaticFunctionDefinitions::SetElementID ( *pEntity, szID ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementID", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementID" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementData ( lua_State* luaVM )
{
//  bool setElementData ( element theElement, string key, var value, [bool synchronize = true] )
    CClientEntity* pEntity; SString strKey; CLuaArgument value; bool bSynchronize;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadString ( strKey );
    argStream.ReadLuaArgument ( value );
    argStream.ReadBool ( bSynchronize, true );

    if ( !argStream.HasErrors () )
    {
        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( pLuaMain )
        {
            if ( strKey.length () > MAX_CUSTOMDATA_NAME_LENGTH )
            {
                // Warn and truncate if key is too long
                m_pScriptDebugging->LogCustom ( luaVM, SString ( "Truncated argument @ '%s' [%s]", "setElementData", *SString ( "string length reduced to %d characters at argument 2", MAX_CUSTOMDATA_NAME_LENGTH ) ) );
                strKey = strKey.Left ( MAX_CUSTOMDATA_NAME_LENGTH );
            }

            if ( CStaticFunctionDefinitions::SetElementData ( *pEntity, strKey, value, *pLuaMain, bSynchronize ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", "setElementData", *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::RemoveElementData ( lua_State* luaVM )
{
//  bool removeElementData ( element theElement, string key )
    CClientEntity* pEntity; SString strKey;;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadString ( strKey );

    if ( !argStream.HasErrors () )
    {
        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( pLuaMain )
        {
            if ( strKey.length () > MAX_CUSTOMDATA_NAME_LENGTH )
            {
                // Warn and truncate if key is too long
                m_pScriptDebugging->LogCustom ( luaVM, SString ( "Truncated argument @ '%s' [%s]", "removeElementData", *SString ( "string length reduced to %d characters at argument 2", MAX_CUSTOMDATA_NAME_LENGTH ) ) );
                strKey = strKey.Left ( MAX_CUSTOMDATA_NAME_LENGTH );
            }

            if ( CStaticFunctionDefinitions::RemoveElementData ( *pEntity, strKey ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", "removeElementData", *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementPosition ( lua_State* luaVM )
{
    // Correct types?
    int iArgument2 = lua_type ( luaVM, 2 );
    int iArgument3 = lua_type ( luaVM, 3 );
    int iArgument4 = lua_type ( luaVM, 4 );
    if ( ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) ) &&
        ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
        ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
        ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
    {
        // Grab the element and the position to change to
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        CVector vecPosition ( static_cast < float > ( lua_tonumber ( luaVM, 2 ) ),
            static_cast < float > ( lua_tonumber ( luaVM, 3 ) ),
            static_cast < float > ( lua_tonumber ( luaVM, 4 ) ) );

        bool bWarp = true;
        if ( lua_type ( luaVM, 5 ) == LUA_TBOOLEAN )
            bWarp = lua_toboolean ( luaVM, 5 ) ? true : false;

        // Valid?
        if ( pEntity )
        {
            // Try to set the position
            if ( CStaticFunctionDefinitions::SetElementPosition ( *pEntity, vecPosition, bWarp ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementPosition", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementPosition" );

    // Error
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementRotation ( lua_State* luaVM )
{
    // Correct types?
    int iArgument2 = lua_type ( luaVM, 2 );
    int iArgument3 = lua_type ( luaVM, 3 );
    int iArgument4 = lua_type ( luaVM, 4 );
    if ( ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) ) &&
        ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
        ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
        ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
    {
        const char* szRotationOrder = "default";
        if ( lua_type ( luaVM, 5 ) == LUA_TSTRING ) 
        {
            szRotationOrder = lua_tostring ( luaVM, 5 );
        }

        // Grab the element and the position to change to
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        CVector vecRotation ( static_cast < float > ( lua_tonumber ( luaVM, 2 ) ),
            static_cast < float > ( lua_tonumber ( luaVM, 3 ) ),
            static_cast < float > ( lua_tonumber ( luaVM, 4 ) ) );

        // Valid?
        if ( pEntity )
        {
            // Try to set the position
            if ( CStaticFunctionDefinitions::SetElementRotation ( *pEntity, vecRotation, szRotationOrder ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementRotation", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementRotation" );

    // Error
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementVelocity ( lua_State* luaVM )
{
    // Verify the first argument
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Check rest of the arguments. Don't mind the last Z argument if the element is 2D
            int iArgument2 = lua_type ( luaVM, 2 );
            int iArgument3 = lua_type ( luaVM, 3 );
            int iArgument4 = lua_type ( luaVM, 4 );
            if ( ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
                ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
                ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING || pEntity->GetType () == CClientGame::RADAR_AREA ) )
            {
                // Grab the velocity
                CVector vecVelocity = CVector ( static_cast < float > ( lua_tonumber ( luaVM, 2 ) ),
                    static_cast < float > ( lua_tonumber ( luaVM, 3 ) ),
                    static_cast < float > ( lua_tonumber ( luaVM, 4 ) ) );
                // Set the velocity
                if ( CStaticFunctionDefinitions::SetElementVelocity ( *pEntity, vecVelocity ) )
                {
                    lua_pushboolean ( luaVM, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadType ( luaVM, "setElementVelocity" );
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementVelocity", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementVelocity" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementParent ( lua_State* luaVM )
{
    // Correct args?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) &&
        lua_istype ( luaVM, 2, LUA_TLIGHTUSERDATA ) )
    {
        // Grab entity 
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab new parent
            CClientEntity* pParent = lua_toelement ( luaVM, 2 );
            if ( pParent )
            {
                CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );

                // Change the parent
                if ( CStaticFunctionDefinitions::SetElementParent ( *pEntity, *pParent, pLuaMain ) )
                {
                    lua_pushboolean ( luaVM, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer ( luaVM, "setElementParent", "element", 2 );
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementParent", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementParent" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementInterior ( lua_State* luaVM )
{
    // Correct args?
    int iArgument2 = lua_type ( luaVM, 2 );
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) &&
        ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
    {
        // Grab the element and the interior
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        unsigned char ucInterior = static_cast < unsigned char > ( lua_tonumber ( luaVM, 2 ) );

        // Grab position if supplied
        bool bSetPosition = false;
        CVector vecPosition;

        int iArgument3 = lua_type ( luaVM, 3 );
        int iArgument4 = lua_type ( luaVM, 4 );
        int iArgument5 = lua_type ( luaVM, 5 );
        if ( ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) &&
            ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING ) )
        {
            vecPosition = CVector ( static_cast < float > ( lua_tonumber ( luaVM, 3 ) ),
                static_cast < float > ( lua_tonumber ( luaVM, 4 ) ),
                static_cast < float > ( lua_tonumber ( luaVM, 5 ) ) );
            bSetPosition = true;
        }

        // Valid element?
        if ( pEntity )
        {
            // Set the interior
            if ( CStaticFunctionDefinitions::SetElementInterior ( *pEntity, ucInterior, bSetPosition, vecPosition ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementInterior", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementInterior" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementDimension ( lua_State* luaVM )
{
    // Correct args?
    int iArgument2 = lua_type ( luaVM, 2 );
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) &&
        ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
    {
        // Grab the element and the dimension
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        unsigned short usDimension = static_cast < unsigned int > ( lua_tonumber ( luaVM, 2 ) );

        // Valid?
        if ( pEntity )
        {
            // Change the dimension
            if ( CStaticFunctionDefinitions::SetElementDimension ( *pEntity, usDimension ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementDimension", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementDimension" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::AttachElements ( lua_State* luaVM )
{
    // Valid arguments?
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA &&
        lua_type ( luaVM, 2 ) == LUA_TLIGHTUSERDATA )
    {
        // Grab the element to attach and the target
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        CClientEntity* pAttachedToEntity = lua_toelement ( luaVM, 2 );
        CVector vecPosition, vecRotation;

        // Grab the supplied arguments (pos: x y z, rot: x y z)
        int iArgument3 = lua_type ( luaVM, 3 );
        if ( iArgument3 == LUA_TSTRING || iArgument3 == LUA_TNUMBER )
        {
            vecPosition.fX = static_cast < float > ( lua_tonumber ( luaVM, 3 ) );

            int iArgument4 = lua_type ( luaVM, 4 );
            if ( iArgument4 == LUA_TSTRING || iArgument4 == LUA_TNUMBER )
            {
                vecPosition.fY = static_cast < float > ( lua_tonumber ( luaVM, 4 ) );

                int iArgument5 = lua_type ( luaVM, 5 );
                if ( iArgument5 == LUA_TSTRING || iArgument5 == LUA_TNUMBER )
                {
                    vecPosition.fZ = static_cast < float > ( lua_tonumber ( luaVM, 5 ) );

                    int iArgument6 = lua_type ( luaVM, 6 );
                    if ( iArgument6 == LUA_TSTRING || iArgument6 == LUA_TNUMBER )
                    {
                        vecRotation.fX = static_cast < float > ( lua_tonumber ( luaVM, 6 ) );

                        int iArgument7 = lua_type ( luaVM, 7 );
                        if ( iArgument7 == LUA_TSTRING || iArgument7 == LUA_TNUMBER )
                        {
                            vecRotation.fY = static_cast < float > ( lua_tonumber ( luaVM, 7 ) );

                            int iArgument8 = lua_type ( luaVM, 8 );
                            if ( iArgument8 == LUA_TSTRING || iArgument8 == LUA_TNUMBER )
                            {
                                vecRotation.fZ = static_cast < float > ( lua_tonumber ( luaVM, 8 ) );
                            }
                        }
                    }
                }
            }
        }

        // Valid element?
        if ( pEntity )
        {
            // Valid host element?
            if ( pAttachedToEntity )
            {
                // Try to attach them
                if ( CStaticFunctionDefinitions::AttachElements ( *pEntity, *pAttachedToEntity, vecPosition, vecRotation ) )
                {
                    lua_pushboolean ( luaVM, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer ( luaVM, "attachElements", "element", 2 );
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "attachElements", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "attachElements" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::DetachElements ( lua_State* luaVM )
{
    // Valid arg type?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) )
    {
        // Grab the element that's attached to something
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );

        // Grab the other element if supplied
        CClientEntity* pAttachedToEntity = NULL;
        if ( lua_istype ( luaVM, 2, LUA_TLIGHTUSERDATA ) )
            pAttachedToEntity = lua_toelement ( luaVM, 2 );

        // Valid?
        if ( pEntity )
        {
            // Try to detach
            if ( CStaticFunctionDefinitions::DetachElements ( *pEntity, pAttachedToEntity ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "detachElements", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "detachElements" );

    // Failure
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementAttachedOffsets ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA )
    {
        // Grab the element to attach and the target
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        CVector vecPosition, vecRotation;

        // Grab the supplied arguments (pos: x y z, rot: x y z)
        int iArgument2 = lua_type ( luaVM, 2 );
        if ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER )
        {
            vecPosition.fX = static_cast < float > ( lua_tonumber ( luaVM, 2 ) );

            int iArgument3 = lua_type ( luaVM, 3 );
            if ( iArgument3 == LUA_TSTRING || iArgument3 == LUA_TNUMBER )
            {
                vecPosition.fY = static_cast < float > ( lua_tonumber ( luaVM, 3 ) );

                int iArgument4 = lua_type ( luaVM, 4 );
                if ( iArgument4 == LUA_TSTRING || iArgument4 == LUA_TNUMBER )
                {
                    vecPosition.fZ = static_cast < float > ( lua_tonumber ( luaVM, 4 ) );

                    int iArgument5 = lua_type ( luaVM, 5 );
                    if ( iArgument5 == LUA_TSTRING || iArgument5 == LUA_TNUMBER )
                    {
                        vecRotation.fX = static_cast < float > ( lua_tonumber ( luaVM, 5 ) );

                        int iArgument6 = lua_type ( luaVM, 6 );
                        if ( iArgument6 == LUA_TSTRING || iArgument6 == LUA_TNUMBER )
                        {
                            vecRotation.fY = static_cast < float > ( lua_tonumber ( luaVM, 6 ) );

                            int iArgument7 = lua_type ( luaVM, 7 );
                            if ( iArgument7 == LUA_TSTRING || iArgument7 == LUA_TNUMBER )
                            {
                                vecRotation.fZ = static_cast < float > ( lua_tonumber ( luaVM, 7 ) );
                            }
                        }
                    }
                }
            }
        }

        // Valid element?
        if ( pEntity )
        {
            if ( CStaticFunctionDefinitions::SetElementAttachedOffsets ( *pEntity, vecPosition, vecRotation ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementAttachedOffsets", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementAttachedOffsets" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementCollisionsEnabled ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA &&
        lua_type ( luaVM, 2 ) == LUA_TBOOLEAN )
    {
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            bool bEnabled = ( lua_toboolean ( luaVM, 2 ) ) ? true:false;
            if ( CStaticFunctionDefinitions::SetElementCollisionsEnabled ( *pEntity, lua_toboolean ( luaVM, 2 ) ? true:false ) )
            {
                lua_pushboolean ( luaVM, true );    
                return 1;
            }        
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementCollisionsEnabled", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementCollisionsEnabled" );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetElementCollidableWith ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA &&
         lua_type ( luaVM, 2 ) == LUA_TLIGHTUSERDATA &&
         lua_type ( luaVM, 3 ) == LUA_TBOOLEAN )
    {
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        CClientEntity* pWithEntity = lua_toelement ( luaVM, 2 );
        if ( pEntity && pWithEntity )
        {
            bool bCanCollide = ( lua_toboolean ( luaVM, 3 ) ) ? true:false;
            if ( CStaticFunctionDefinitions::SetElementCollidableWith ( *pEntity, *pWithEntity, bCanCollide ) )
            {
                lua_pushboolean ( luaVM, true );    
                return 1;
            }        
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementCollidableWith", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementCollidableWith" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementDoubleSided ( lua_State* luaVM )
{
    // Valid args?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) &&
        lua_istype ( luaVM, 2, LUA_TBOOLEAN ) )
    {
        // Grab the element to change
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab the chosen value and set it
            pEntity->SetDoubleSided ( lua_toboolean ( luaVM, 2 ) ? true : false );
            lua_pushboolean ( luaVM, true );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementDoubleSided", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementDoubleSided" );

    // Failure
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementFrozen ( lua_State* luaVM )
{
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) && lua_istype ( luaVM, 2, LUA_TBOOLEAN ) )
    {
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            bool bFrozen = lua_toboolean ( luaVM, 2 ) ? true : false;

            if ( CStaticFunctionDefinitions::SetElementFrozen ( *pEntity, bFrozen ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementFrozen", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementFrozen" );

    // Failure
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementAlpha ( lua_State* luaVM )
{
    // Valid args?
    int iArgument2 = lua_type ( luaVM, 2 );
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) &&
        ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER ) )
    {
        // Grab the element to change the alpha of
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Grab the chosen alpha and set it
            unsigned char ucAlpha = static_cast < unsigned char > ( lua_tonumber ( luaVM, 2 ) );
            if ( CStaticFunctionDefinitions::SetElementAlpha ( *pEntity, ucAlpha ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementAlpha", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementAlpha" );

    // Failure
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementHealth ( lua_State* luaVM )
{
    // Check the args
    int iArgument2 = lua_type ( luaVM, 2 );
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) &&
        ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER ) )
    {
        // Grab the element to change the health of
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Try to change the health
            float fHealth = static_cast < float > ( lua_tonumber ( luaVM, 2 ) );
            if ( CStaticFunctionDefinitions::SetElementHealth ( *pEntity, fHealth ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementHealth", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementHealth" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementModel ( lua_State* luaVM )
{
    // Check the args
    int iArgument2 = lua_type ( luaVM, 2 );
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) &&
        ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER ) )
    {
        // Grab the element to change the health of
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Try to change the health
            unsigned short usModel = static_cast < unsigned short > ( lua_tonumber ( luaVM, 2 ) );
            if ( CStaticFunctionDefinitions::SetElementModel ( *pEntity, usModel ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementModel", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementModel" );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetElementStreamable ( lua_State* luaVM )
{
    // We have a correct parameter 1 and 2?
    if ( lua_istype ( luaVM, 1, LUA_TLIGHTUSERDATA ) &&
        lua_istype ( luaVM, 2, LUA_TBOOLEAN ) )
    {
        // Grab the entity and verify it.
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            // Is this a streaming compatible class?
            if ( pEntity->IsStreamingCompatibleClass () )
            {
                CClientStreamElement* pStreamElement = static_cast < CClientStreamElement* > ( pEntity );

                // Set this object's streamable state.
                bool bStreamable = lua_toboolean ( luaVM, 2 ) ? true : false;

                // TODO: maybe use a better VM-based reference system (rather than a boolean one)
                if ( bStreamable && ( pStreamElement->GetStreamReferences ( true ) > 0 ) )
                {
                    pStreamElement->RemoveStreamReference ( true );
                }
                else if ( !bStreamable && ( pStreamElement->GetStreamReferences ( true ) == 0 ) )
                {
                    pStreamElement->AddStreamReference ( true );
                }
                else
                {
                    lua_pushboolean ( luaVM, false );
                    return 1;
                }

                // Return success
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setElementStreamable", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setElementStreamable" );

    // We failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetLowLodElement ( lua_State* luaVM )
{
//  element getLowLODElement ( element theElement )
    CClientEntity* pEntity;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );

    if ( !argStream.HasErrors () )
    {
        CClientEntity* pLowLodEntity;
        if ( CStaticFunctionDefinitions::GetLowLodElement ( *pEntity, pLowLodEntity ) )
        {
            lua_pushelement ( luaVM, pLowLodEntity );
            return 1;
        }        
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", "getLowLODElement", *argStream.GetErrorMessage () ) );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetLowLodElement ( lua_State* luaVM )
{
//  bool setLowLODElement ( element theElement )
    CClientEntity* pEntity; CClientEntity* pLowLodEntity;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadUserData ( pLowLodEntity );

    if ( !argStream.HasErrors () )
    {
        if ( CStaticFunctionDefinitions::SetLowLodElement ( *pEntity, pLowLodEntity ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }        
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", "setLowLODElement", *argStream.GetErrorMessage () ) );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsElementLowLod ( lua_State* luaVM )
{
//  bool isElementLowLOD ( element theElement )
    CClientEntity* pEntity;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );

    if ( !argStream.HasErrors () )
    {
        bool bLowLod;
        if ( CStaticFunctionDefinitions::IsElementLowLod ( *pEntity, bLowLod ) )
        {
            lua_pushboolean ( luaVM, bLowLod );
            return 1;
        }        
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", "isElementLowLOD", *argStream.GetErrorMessage () ) );

    lua_pushboolean ( luaVM, false );
    return 1;
}
