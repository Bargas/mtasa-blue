/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/lua/CLuaMain.cpp
*  PURPOSE:     Lua virtual machine container class
*  DEVELOPERS:  Oliver Brown <>
*               Christian Myhre Lundheim <>
*               Ed Lyons <>
*               Cecill Etheredge <>
*               Jax <>
*               Kevin Whiteside <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

#include "CLuaFunctionDefinitions.h"

#include <clocale>

static CLuaManager* m_pLuaManager;

#define HOOK_INSTRUCTION_COUNT 1000000
#define HOOK_MAXIMUM_TIME 5000

extern CGame* g_pGame;
extern CNetServer* g_pRealNetServer;

// This script is loaded into all VM's created.
const char szPreloadedScript [] = ""\

    // Code for allowing this syntax:   exports.resourceName:exportedFunctionName (...)
    //                                  exports["resourceName"]:exportedFunctionName (...)
    //                                  exports[resourcePointer]:exportedFunctionName (...)
    // Aswell as the old:               call ( getResourceFromName ( "resourceName" ), "exportedFunctionName", ... )
    "local rescallMT = {}\n" \
    "function rescallMT:__index(k)\n" \
    "        if type(k) ~= 'string' then k = tostring(k) end\n" \
    "        self[k] = function(resExportTable, ...)\n" \
    "                if type(self.res) == 'userdata' and getResourceRootElement(self.res) then\n" \
    "                        return call(self.res, k, ...)\n" \
    "                else\n" \
    "                        return nil\n" \
    "                end\n" \
    "        end\n" \
    "        return self[k]\n" \
    "end\n" \
    "local exportsMT = {}\n" \
    "function exportsMT:__index(k)\n" \
    "        if type(k) == 'userdata' and getResourceRootElement(k) then\n" \
    "                return setmetatable({ res = k }, rescallMT)\n" \
    "        elseif type(k) ~= 'string' then\n" \
    "                k = tostring(k)\n" \
    "        end\n" \
    "        local res = getResourceFromName(k)\n" \
    "        if res and getResourceRootElement(res) then\n" \
    "                return setmetatable({ res = res }, rescallMT)\n" \
    "        else\n" \
    "                outputDebugString('exports: Call to non-running server resource (' .. k .. ')', 1)\n" \
    "                return setmetatable({}, rescallMT)\n" \
    "        end\n" \
    "end\n" \
    "exports = setmetatable({}, exportsMT)\n";

CLuaMain::CLuaMain ( CLuaManager* pLuaManager,
                     CObjectManager* pObjectManager,
                     CPlayerManager* pPlayerManager,
                     CVehicleManager* pVehicleManager,
                     CBlipManager* pBlipManager,
                     CRadarAreaManager* pRadarAreaManager,
                     CMapManager* pMapManager,
                     CResource* pResourceOwner, bool bEnableOOP  )
{
    // Initialise everything to be setup in the Start function
    m_pLuaManager = pLuaManager;
    m_luaVM = NULL;
    m_pResource = pResourceOwner;
    m_pResourceFile = NULL;
    m_bBeingDeleted = false;
    m_pLuaTimerManager = new CLuaTimerManager;
    m_FunctionEnterTimer.SetMaxIncrement ( 500 );
    m_WarningTimer.SetMaxIncrement ( 1000 );
    m_uiOpenFileCountWarnThresh = 10;
    m_uiOpenXMLFileCountWarnThresh = 20;

    m_pObjectManager = pObjectManager;
    m_pPlayerManager = pPlayerManager;
    m_pRadarAreaManager = pRadarAreaManager;
    m_pVehicleManager = pVehicleManager;
    m_pBlipManager = pBlipManager;
    m_pMapManager = pMapManager;

    m_bEnableOOP = bEnableOOP;


    CPerfStatLuaMemory::GetSingleton ()->OnLuaMainCreate ( this );
    CPerfStatLuaTiming::GetSingleton ()->OnLuaMainCreate ( this );
}


CLuaMain::~CLuaMain ( void )
{
    // remove all current remote calls originating from this VM
    g_pGame->GetRemoteCalls()->Remove ( this );
    g_pGame->GetLuaCallbackManager ()->OnLuaMainDestroy ( this );
    g_pGame->GetLatentTransferManager ()->OnLuaMainDestroy ( this );
    g_pGame->GetDebugHookManager()->OnLuaMainDestroy ( this );
    g_pGame->GetScriptDebugging()->OnLuaMainDestroy ( this );

    // Unload the current script
    UnloadScript ();
    m_bBeingDeleted = true;

    // Delete the timer manager
    delete m_pLuaTimerManager;

    // Eventually delete the XML files the LUA script didn't
    list<CXMLFile *>::iterator iterXML = m_XMLFiles.begin ();
    for ( ; iterXML != m_XMLFiles.end (); ++iterXML )
    {
        delete *iterXML;
    }

    // Eventually delete the text displays the LUA script didn't
    list<CTextDisplay *>::iterator iterDisplays = m_Displays.begin ();
    for ( ; iterDisplays != m_Displays.end (); ++iterDisplays )
    {
        delete *iterDisplays;
    }

    // Eventually delete the text items the LUA script didn't
    list<CTextItem *>::iterator iterItems = m_TextItems.begin ();
    for ( ; iterItems != m_TextItems.end (); ++iterItems )
    {
        delete *iterItems;
    }

    CPerfStatLuaMemory::GetSingleton ()->OnLuaMainDestroy ( this );
    CPerfStatLuaTiming::GetSingleton ()->OnLuaMainDestroy ( this );
}

bool CLuaMain::BeingDeleted ( void )
{
    return m_bBeingDeleted;
}


void CLuaMain::ResetInstructionCount ( void )
{
    m_FunctionEnterTimer.Reset ();
}


void CLuaMain::InitSecurity ( void )
{
    lua_register ( m_luaVM, "dofile", CLuaFunctionDefinitions::DisabledFunction );
    lua_register ( m_luaVM, "loadfile", CLuaFunctionDefinitions::DisabledFunction );
    lua_register ( m_luaVM, "require", CLuaFunctionDefinitions::DisabledFunction );
    lua_register ( m_luaVM, "loadlib", CLuaFunctionDefinitions::DisabledFunction );
    lua_register ( m_luaVM, "getfenv", CLuaFunctionDefinitions::DisabledFunction );
    lua_register ( m_luaVM, "newproxy", CLuaFunctionDefinitions::DisabledFunction );
}



void CLuaMain::AddVector3DClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classmetamethod ( luaVM, "__tostring", CLuaVectorDefs::ToString );
    lua_classmetamethod ( luaVM, "__gc", CLuaVectorDefs::Destroy );
    
    lua_classmetamethod ( luaVM, "__add", CLuaVectorDefs::Add );
    lua_classmetamethod ( luaVM, "__sub", CLuaVectorDefs::Sub );
    lua_classmetamethod ( luaVM, "__mul", CLuaVectorDefs::Mul );
    lua_classmetamethod ( luaVM, "__div", CLuaVectorDefs::Div );
    lua_classmetamethod ( luaVM, "__pow", CLuaVectorDefs::Pow );
    lua_classmetamethod ( luaVM, "__unm", CLuaVectorDefs::Unm );
    lua_classmetamethod ( luaVM, "__eq", CLuaVectorDefs::Eq );
    
    lua_classfunction ( luaVM, "create", "", CLuaVectorDefs::Create );
    lua_classfunction ( luaVM, "normalize", "", CLuaVectorDefs::Normalize );
    lua_classfunction ( luaVM, "cross", "", CLuaVectorDefs::Cross );
    lua_classfunction ( luaVM, "dot", "", CLuaVectorDefs::Dot );
    
    lua_classfunction ( luaVM, "getLength", "", CLuaVectorDefs::GetLength );
    lua_classfunction ( luaVM, "getSquaredLength", "", CLuaVectorDefs::GetLengthSquared );
    lua_classfunction ( luaVM, "getNormalized", "", CLuaVectorDefs::GetNormalized );
    lua_classfunction ( luaVM, "getX", "", CLuaVectorDefs::GetX );
    lua_classfunction ( luaVM, "getY", "", CLuaVectorDefs::GetY );
    lua_classfunction ( luaVM, "getZ", "", CLuaVectorDefs::GetZ );
    
    lua_classfunction ( luaVM, "setX", "", CLuaVectorDefs::SetX );
    lua_classfunction ( luaVM, "setY", "", CLuaVectorDefs::SetY );
    lua_classfunction ( luaVM, "setZ", "", CLuaVectorDefs::SetZ );
    
    lua_classvariable ( luaVM, "x", "", "", CLuaVectorDefs::SetX, CLuaVectorDefs::GetX );
    lua_classvariable ( luaVM, "y", "", "", CLuaVectorDefs::SetY, CLuaVectorDefs::GetY );
    lua_classvariable ( luaVM, "z", "", "", CLuaVectorDefs::SetZ, CLuaVectorDefs::GetZ );
    lua_classvariable ( luaVM, "length", "", "", NULL, CLuaVectorDefs::GetLength );
    lua_classvariable ( luaVM, "squaredLength", "", "", NULL, CLuaVectorDefs::GetLengthSquared );
    lua_classvariable ( luaVM, "normalized", "", "", NULL, CLuaVectorDefs::GetNormalized );
    
    lua_registerclass ( luaVM, "Vector3" );
}


void CLuaMain::AddVector2DClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_registerclass ( luaVM, "Vector2" );
}


void CLuaMain::AddMatrixClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classmetamethod ( luaVM, "__tostring", CLuaMatrixDefs::ToString );
    lua_classmetamethod ( luaVM, "__gc", CLuaMatrixDefs::Destroy );
    
    lua_classmetamethod ( luaVM, "__add", CLuaMatrixDefs::Add );
    lua_classmetamethod ( luaVM, "__sub", CLuaMatrixDefs::Sub );
    lua_classmetamethod ( luaVM, "__mul", CLuaMatrixDefs::Mul );
    lua_classmetamethod ( luaVM, "__div", CLuaMatrixDefs::Div );
    
    lua_classfunction ( luaVM, "create", "", CLuaMatrixDefs::Create );
    

    lua_classfunction ( luaVM, "transformPosition", "", CLuaMatrixDefs::TransformPosition );
    lua_classfunction ( luaVM, "transformDirection", "", CLuaMatrixDefs::TransformDirection );
    lua_classfunction ( luaVM, "inverse", "", CLuaMatrixDefs::Inverse );

    lua_classfunction ( luaVM, "getPosition", "", CLuaMatrixDefs::GetPosition );
    lua_classfunction ( luaVM, "getRotation", "", CLuaMatrixDefs::GetRotation );
    lua_classfunction ( luaVM, "getForward", "", CLuaMatrixDefs::GetForward );
    lua_classfunction ( luaVM, "getRight", "", CLuaMatrixDefs::GetRight );
    lua_classfunction ( luaVM, "getUp", "", CLuaMatrixDefs::GetUp );
    
    lua_classfunction ( luaVM, "setPosition", "", CLuaMatrixDefs::SetPosition );
    lua_classfunction ( luaVM, "setForward", "", CLuaMatrixDefs::SetForward );
    lua_classfunction ( luaVM, "setRight", "", CLuaMatrixDefs::SetRight );
    lua_classfunction ( luaVM, "setUp", "", CLuaMatrixDefs::SetUp );
    
    lua_classvariable ( luaVM, "position", "", "", CLuaMatrixDefs::SetPosition, CLuaMatrixDefs::GetPosition );
    lua_classvariable ( luaVM, "rotation", "", "", NULL, CLuaMatrixDefs::GetRotation );
    lua_classvariable ( luaVM, "forward", "", "", CLuaMatrixDefs::SetForward, CLuaMatrixDefs::GetForward );
    lua_classvariable ( luaVM, "right", "", "", CLuaMatrixDefs::SetRight, CLuaMatrixDefs::GetRight );
    lua_classvariable ( luaVM, "up", "", "", CLuaMatrixDefs::SetUp, CLuaMatrixDefs::GetUp );
    
    lua_registerclass ( luaVM, "Matrix" );
}


// TODO: position, rotation and velocity classes, data specials
void CLuaMain::AddElementClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "getByID", "getElementByID" );
    lua_classfunction ( luaVM, "getAllByType", "getElementsByType" );
    lua_classfunction ( luaVM, "getByIndex", "getElementByIndex" );
    
    lua_classfunction ( luaVM, "create", "createElement" );
    lua_classfunction ( luaVM, "clone", "cloneElement" );
    lua_classfunction ( luaVM, "destroy", "destroyElement" );
    lua_classfunction ( luaVM, "clearVisibility", "clearElementVisibleTo" );
    lua_classfunction ( luaVM, "attach", "attachElements" );
    lua_classfunction ( luaVM, "detach", "detachElements" );
    lua_classfunction ( luaVM, "removeData", "removeElementData" );
    
    lua_classfunction ( luaVM, "setParent", "setElementParent" );
    lua_classfunction ( luaVM, "setVelocity", "setElementVelocity" );
    lua_classfunction ( luaVM, "setFrozen", "setElementFrozen" );
    lua_classfunction ( luaVM, "setHealth", "setElementHealth" );
    lua_classfunction ( luaVM, "setModel", "setElementModel" );
    lua_classfunction ( luaVM, "setSyncer", "setElementSyncer" );
    lua_classfunction ( luaVM, "setAlpha", "setElementAlpha" );
    lua_classfunction ( luaVM, "setDoubleSided", "setElementDoubleSided" );
    lua_classfunction ( luaVM, "setCollisionsEnabled", "setElementCollisionsEnabled" );
    lua_classfunction ( luaVM, "setData", "setElementData" );
    lua_classfunction ( luaVM, "setPosition", "setElementPosition" );
    lua_classfunction ( luaVM, "setRotation", "setElementRotation" );
    lua_classfunction ( luaVM, "setVelocity", "setElementVelocity" );
    lua_classfunction ( luaVM, "setID", "setElementID" );
    lua_classfunction ( luaVM, "setInterior", "setElementInterior" );
    lua_classfunction ( luaVM, "setDimension", "setElementDimension" );
    lua_classfunction ( luaVM, "setAttachedOffsets", "setElementAttachedOffsets" );
    
    lua_classfunction ( luaVM, "getAttachedOffsets", "getElementAttachedOffsets" );
    lua_classfunction ( luaVM, "getChild", "getElementChild" );
    lua_classfunction ( luaVM, "getChildren", "getElementChildren" );
    lua_classfunction ( luaVM, "getParent", "getElementParent" );
    lua_classfunction ( luaVM, "getAttachedElements", "getAttachedElements" );
    lua_classfunction ( luaVM, "getAttachedTo", "getAttachedTo" );
    lua_classfunction ( luaVM, "getVelocity", "getElementVelocity" );
    lua_classfunction ( luaVM, "getID", "getElementID" );
    lua_classfunction ( luaVM, "getZoneName", "getElementZoneName" );
    lua_classfunction ( luaVM, "getAlpha", "getElementAlpha" );
    lua_classfunction ( luaVM, "getHealth", "getElementHealth" );
    lua_classfunction ( luaVM, "getModel", "getElementModel" );
    lua_classfunction ( luaVM, "getChildrenCount", "getElementChildrenCount" );
    lua_classfunction ( luaVM, "getSyncer", "getElementSyncer" );
    lua_classfunction ( luaVM, "getAllData", "getAllElementData" );
    lua_classfunction ( luaVM, "getColShape", "getElementColShape" );
    lua_classfunction ( luaVM, "getData", "getElementData" );
    //lua_classfunction ( luaVM, "getPosition", "getElementPosition", CLuaOOPDefs::GetElementPosition );
    //lua_classfunction ( luaVM, "getRotation", "getElementRotation", CLuaOOPDefs::GetElementRotation );
    lua_classfunction ( luaVM, "getPosition", "getElementPosition" );
    lua_classfunction ( luaVM, "getRotation", "getElementRotation" );
    lua_classfunction ( luaVM, "getType", "getElementType" );
    lua_classfunction ( luaVM, "getInterior", "getElementInterior" );
    lua_classfunction ( luaVM, "getDimension", "getElementDimension" );
    lua_classfunction ( luaVM, "getLowLOD", "getLowLODElement" );
    
    lua_classfunction ( luaVM, "areCollisionsEnabled", "areElementCollisionsEnabled" );
    lua_classfunction ( luaVM, "isWithinMarker", "isElementWithinMarker" );
    lua_classfunction ( luaVM, "isWithinColShape", "isElementWithinColShape" );
    lua_classfunction ( luaVM, "isFrozen", "isElementFrozen" );
    lua_classfunction ( luaVM, "isInWater", "isElementInWater" );
    lua_classfunction ( luaVM, "isDoubleSided", "isElementDoubleSided" );
    lua_classfunction ( luaVM, "isVisibleTo", "isElementVisibleTo" );
    lua_classfunction ( luaVM, "isLowLOD", "isElementLowLOD" );
    lua_classfunction ( luaVM, "isAttached", "isElementAttached" );

    lua_classvariable ( luaVM, "id", "setElementID", "getElementID" );
    lua_classvariable ( luaVM, "parent", "setElementParent", "getElementParent" );
    lua_classvariable ( luaVM, "zoneName", NULL, "getElementZoneName" );
    lua_classvariable ( luaVM, "attachedTo", "attachElements", "getElementAttachedTo" );
    lua_classvariable ( luaVM, "children", NULL, "getElementChildren" ); // should this be a special table using getElementChild instead?
    lua_classvariable ( luaVM, "frozen", "setElementFrozen", "isElementFrozen" );
    lua_classvariable ( luaVM, "attachedElements", NULL, "getAttachedElements" );
    lua_classvariable ( luaVM, "inWater", NULL, "isElementInWater" );
    lua_classvariable ( luaVM, "health", "setElementHealth", "getElementHealth" );
    lua_classvariable ( luaVM, "alpha", "setElementAlpha", "getElementAlpha" );
    lua_classvariable ( luaVM, "type", NULL, "getElementType" );
    lua_classvariable ( luaVM, "dimension", "setElementDimension", "getElementDimension" );
    lua_classvariable ( luaVM, "doubleSided", "setElementDoubleSided", "isElementDoubleSided" );
    lua_classvariable ( luaVM, "model", "setElementModel", "getElementModel" );
    lua_classvariable ( luaVM, "lowLOD", "setLowLODElement", "getLowLODElement" );
    lua_classvariable ( luaVM, "syncer", NULL, "isElementSyncer" );
    lua_classvariable ( luaVM, "childrenCount", NULL, "getElementChildrenCount" );
    lua_classvariable ( luaVM, "interior", "setElementInterior", "getElementInterior" );
    lua_classvariable ( luaVM, "colShape", NULL, "getElementColShape" );
    lua_classvariable ( luaVM, "collisions", "setElementCollisionsEnabled", "getElementCollisionsEnabled" );
    //lua_classvariable ( luaVM, "position", CLuaFunctionDefs::SetElementPosition, CLuaOOPDefs::GetElementPosition );
    //lua_classvariable ( luaVM, "rotation", CLuaFunctionDefs::SetElementRotation, CLuaOOPDefs::GetElementRotation );
    //lua_classvariable ( luaVM, "velocity", "setElementVelocity", "getElementVelocity", CLuaFunctionDefs::SetElementVelocity, CLuaOOPDefs::GetElementVelocity );
    //lua_classvariable ( luaVM, "data", "setElementData", "getElementData", CLuaOOPDefs::SetElementData, CLuaOOPDefs::GetElementData );
    //lua_classvariable ( luaVM, "visibility", "setElementVisibleTo", "isElementVisibleTo", CLuaOOPDefs::SetElementVisibleTo, CLuaOOPDefs::IsElementVisibleTo ); // .visibility[john]=false
    
    lua_registerclass ( luaVM, "Element" );
}


void CLuaMain::AddACLClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );

    lua_registerclass ( luaVM, "ACL" );
}


void CLuaMain::AddACLGroupClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );

    lua_registerclass ( luaVM, "ACLGroup" );
}


void CLuaMain::AddAccountClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );

    lua_registerclass ( luaVM, "Account" );
    
}


// TODO: The "set" attribute of .admin, .reason and .unbanTime needs to be checked for syntax
void CLuaMain::AddBanClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "create", "addBan" );
    lua_classfunction ( luaVM, "remove", "removeBan" );
    lua_classfunction ( luaVM, "getList", "getBans" );
    lua_classvariable ( luaVM, "list", NULL, "getBans" );
    
    lua_classfunction ( luaVM, "getAdmin", "getBanAdmin" );
    lua_classfunction ( luaVM, "getIP", "getBanIP" );
    lua_classfunction ( luaVM, "getNick", "getBanNick" );
    lua_classfunction ( luaVM, "getReason", "getBanReason" );
    lua_classfunction ( luaVM, "getSerial", "getBanSerial" );
    lua_classfunction ( luaVM, "getTime", "getBanTime" );
    lua_classfunction ( luaVM, "getUnbanTime", "getUnbanTime" );
    
    lua_classvariable ( luaVM, "admin", "setBanAdmin", "getBanAdmin" );
    lua_classvariable ( luaVM, "IP", NULL, "getBanIP" );
    lua_classvariable ( luaVM, "nick", NULL, "getBanNick" );
    lua_classvariable ( luaVM, "serial", NULL, "getBanSerial" );
    lua_classvariable ( luaVM, "time", NULL, "getBanTime" );
    lua_classvariable ( luaVM, "unbanTime", NULL, "getUnbanTime" );
    lua_classvariable ( luaVM, "reason", "setBanReason", "getBanReason" );
    
    lua_registerclass ( luaVM, "Ban" );
}


void CLuaMain::AddBlipClass( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "create", "createBlip" );
    lua_classfunction ( luaVM, "createAttachedTo", "createBlipAttachedTo" );
    
    lua_classfunction ( luaVM, "getColor", "getBlipColor" );
    lua_classfunction ( luaVM, "getVisibleDistance", "getBlipVisibleDistance" );
    lua_classfunction ( luaVM, "getOrdering", "getBlipOrdering" );
    lua_classfunction ( luaVM, "getSize", "getBlipSize" );
    lua_classfunction ( luaVM, "getIcon", "getBlipIcon" );
    
    lua_classfunction ( luaVM, "setColor", "setBlipColor" );
    lua_classfunction ( luaVM, "setVisibleDistance", "setBlipVisibleDistance" );
    lua_classfunction ( luaVM, "setOrdering", "setBlipOrdering" );
    lua_classfunction ( luaVM, "setSize", "setBlipSize" );
    lua_classfunction ( luaVM, "setIcon", "setBlipIcon" );
    
    lua_classvariable ( luaVM, "icon", "setBlipIcon", "getBlipIcon" );
    lua_classvariable ( luaVM, "size", "setBlipSize", "getBlipSize" );
    lua_classvariable ( luaVM, "ordering", "setBlipOrdering", "getBlipOrdering" );
    lua_classvariable ( luaVM, "visibleDistance", "setBlipVisibleDistance", "getBlipVisibleDistance" );
    //lua_classvariable ( luaVM, "color", "setBlipColor", "getBlipColor", "" ); color
    
    lua_registerclass ( luaVM, "Blip", "Element" );
    
}


void CLuaMain::AddColShapeClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "Circle", "createColCircle" );
    lua_classfunction ( luaVM, "Cuboid", "createColCuboid" );
    lua_classfunction ( luaVM, "Rectangle", "createColRectangle" );
    lua_classfunction ( luaVM, "Sphere", "createColSphere" );
    lua_classfunction ( luaVM, "Tube", "createColTube" );
    lua_classfunction ( luaVM, "Polygon", "createColPolygon" );

    lua_classfunction ( luaVM, "getElementsWithin", "getElementsWithinColShape" );
    //lua_classfunction ( luaVM, "isElementWithin", "isElementWithinColShape" ); TODO: swap args around.
    
    lua_registerclass ( luaVM, "ColShape", "Element" );
}


void CLuaMain::AddFileClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "create", "fileOpen" );
    lua_classfunction ( luaVM, "destroy", "fileClose" );
    lua_classfunction ( luaVM, "close", "fileClose" );
    lua_classfunction ( luaVM, "new", "fileCreate" );
    
    lua_classfunction ( luaVM, "delete", "fileDelete" );
    lua_classfunction ( luaVM, "exists", "fileExists" );
    lua_classfunction ( luaVM, "flush", "fileFlush" );
    lua_classfunction ( luaVM, "getPos", "fileGetPos" );
    lua_classfunction ( luaVM, "getSize", "fileGetSize" );
    lua_classfunction ( luaVM, "isEOF", "fileIsEOF" );
    lua_classfunction ( luaVM, "read", "fileRead" );
    lua_classfunction ( luaVM, "rename", "fileRename" );
    lua_classfunction ( luaVM, "setPos", "fileSetPos" );
    lua_classfunction ( luaVM, "write", "fileWrite" );
    lua_classfunction ( luaVM, "copy", "fileCopy" );
    
    lua_classvariable ( luaVM, "pos", "fileSetPos", "fileGetPos" );
    lua_classvariable ( luaVM, "size", NULL, "fileGetSize" );
    lua_classvariable ( luaVM, "isEOF", NULL, "fileIsEOF" );

    lua_registerclass ( luaVM, "File" );
}


void CLuaMain::AddMarkerClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "create", "createMarker" );
    lua_classfunction ( luaVM, "getCount", "getMarkerCount" );
    //lua_classfunction ( luaVM, "isElementWithin", "isElementWithinMarker", CLuaOOPDefs::isElementWithinMarker ); TODO: swap args
    
    lua_classfunction ( luaVM, "getType", "getMarkerType" );
    lua_classfunction ( luaVM, "getIcon", "getMarkerIcon" );
    lua_classfunction ( luaVM, "getSize", "getMarkerSize" );
    lua_classfunction ( luaVM, "getTarget", "getMarkerTarget" ); // vector
    lua_classfunction ( luaVM, "getColor", "getMarkerColor" ); // color
    
    lua_classfunction ( luaVM, "setType", "setMarkerType" );
    lua_classfunction ( luaVM, "setIcon", "setMarkerIcon" );
    lua_classfunction ( luaVM, "setSize", "setMarkerSize" );
    lua_classfunction ( luaVM, "setTarget", "setMarkerTarget" ); // vector
    lua_classfunction ( luaVM, "setColor", "setMarkerColor" ); // color
    
    lua_classvariable ( luaVM, "type", "setMarkerType", "getMarkerType" );
    lua_classvariable ( luaVM, "icon", "setMarkerIcon", "getMarkerIcon" );
    lua_classvariable ( luaVM, "size", "setMarkerSize", "getMarkerSize" );
    
    //lua_classvariable ( luaVM, "target", CLuaOOPDefs::SetMarkerTarget, CLuaOOPDefs::GetMarkerTarget ); vector
    //lua_classvariable ( luaVM, "color", CLuaOOPDefs::SetMarkerColor, CLuaOOPDefs::GetMarkerColor ); color

    lua_registerclass ( luaVM, "Marker", "Element" );
}


void CLuaMain::AddObjectClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "create", "createObject" );
    lua_classfunction ( luaVM, "move", "moveObject" );
    lua_classfunction ( luaVM, "stop", "stopObject" );
    
    lua_classfunction ( luaVM, "getScale", "getObjectScale" );
    lua_classfunction ( luaVM, "setScale", "setObjectScale" );
    
    lua_classvariable ( luaVM, "scale", "setObjectScale", "getObjectScale" );

    lua_registerclass ( luaVM, "Object", "Element" );
}


void CLuaMain::AddPedClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );

    lua_registerclass ( luaVM, "Ped", "Element" );
}


void CLuaMain::AddPickupClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "create", "createPickup" );
    lua_classfunction ( luaVM, "use", "usePickup" );
    
    lua_classfunction ( luaVM, "getAmmo", "getPickupAmmo" );
    lua_classfunction ( luaVM, "getAmount", "getPickupAmount" );
    lua_classfunction ( luaVM, "getWeapon", "getPickupWeapon" );
    lua_classfunction ( luaVM, "getRespawnInterval", "getPickupRespawnInterval" );
    lua_classfunction ( luaVM, "getType", "getPickupType" );
    lua_classfunction ( luaVM, "setType", "setPickupType" );
    lua_classfunction ( luaVM, "setRespawnInterval", "setPickupRespawnInterval" );
    
    lua_classvariable ( luaVM, "ammo", NULL, "getPickupAmmo" );
    lua_classvariable ( luaVM, "amount", NULL, "getPickupAmount" );
    lua_classvariable ( luaVM, "spawned", NULL, "isPickupSpawned" );
    lua_classvariable ( luaVM, "weapon", NULL, "getPickupWeapon" );
    lua_classvariable ( luaVM, "type", "setPickupType", "getPickupType" );
    lua_classvariable ( luaVM, "respawnInterval", "setPickupRespawnInterval", "getPickupRespawnInterval" );
    
    lua_registerclass ( luaVM, "Pickup", "Element" );
}


// TODO: color class, table specials
void CLuaMain::AddPlayerClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );

    lua_classfunction ( luaVM, "getAllAlive", "getAlivePlayers" );
    lua_classfunction ( luaVM, "getAllDead", "getDeadPlayers" );
    lua_classfunction ( luaVM, "getRandom", "getRandomPlayer" );
    lua_classfunction ( luaVM, "getCount", "getPlayerCount" );
    lua_classfunction ( luaVM, "isVoiceEnabled", "isVoiceEnabled" );
    
    lua_classfunction ( luaVM, "create", "getPlayerFromName" );
    lua_classfunction ( luaVM, "ban", "banPlayer" );
    lua_classfunction ( luaVM, "kick", "kickPlayer" );
    lua_classfunction ( luaVM, "redirect", "redirectPlayer" );
    lua_classfunction ( luaVM, "resendModInfo", "resendPlayerModInfo" );
    lua_classfunction ( luaVM, "spawn", "spawnPlayer" );
    lua_classfunction ( luaVM, "takeMoney", "takePlayerMoney" );
    lua_classfunction ( luaVM, "takeScreenshot", "takePlayerScreenShot" );
    lua_classfunction ( luaVM, "giveMoney", "givePlayerMoney" );
    lua_classfunction ( luaVM, "showHudComponent", "showPlayerHudComponent" );
    
    lua_classfunction ( luaVM, "forceMap", "forcePlayerMap" );
    lua_classfunction ( luaVM, "setTeam", "setPlayerTeam" );
    lua_classfunction ( luaVM, "setMuted", "setPlayerMuted" );
    lua_classfunction ( luaVM, "setName", "setPlayerName" );
    lua_classfunction ( luaVM, "setBlurLevel", "setPlayerBlurLevel" );
    lua_classfunction ( luaVM, "setWantedLevel", "setPlayerWantedLevel" );
    lua_classfunction ( luaVM, "setMoney", "setPlayerMoney" );
    lua_classfunction ( luaVM, "setNametagText", "setPlayerNametagText" );
    lua_classfunction ( luaVM, "setNametagShowing", "setPlayerNametagShowing" );
    lua_classfunction ( luaVM, "setNametagColor", "setPlayerNametagColor" ); // color
    lua_classfunction ( luaVM, "setAnnounceValue", "setPlayerAnnounceValue" );
    lua_classfunction ( luaVM, "setVoiceBroadcastTo", "setPlayerVoiceBroadcastTo" );
    lua_classfunction ( luaVM, "setVoiceIgnoreFrom", "setPlayerVoiceIgnoreFrom" );
    lua_classfunction ( luaVM, "setHudComponentVisible", "setPlayerHudComponentVisible" );
    
    lua_classfunction ( luaVM, "isMapForced", "isPlayerMapForced" );
    lua_classfunction ( luaVM, "isMuted", "isPlayerMuted" );
    lua_classfunction ( luaVM, "isNametagShowing", "isPlayerNametagShowing" );
    lua_classfunction ( luaVM, "getNametagText", "getPlayerNametagText" );
    lua_classfunction ( luaVM, "getNametagColor", "getPlayerNametagColor" ); // color
    lua_classfunction ( luaVM, "getName", "getPlayerName" );
    lua_classfunction ( luaVM, "getIdleTime", "getPlayerIdleTime" );
    lua_classfunction ( luaVM, "getPing", "getPlayerPing" );
    lua_classfunction ( luaVM, "getWantedLevel", "getPlayerWantedLevel" );
    lua_classfunction ( luaVM, "getSerial", "getPlayerSerial" );
    lua_classfunction ( luaVM, "getIP", "getPlayerIP" );
    lua_classfunction ( luaVM, "getTeam", "getPlayerTeam" );
    lua_classfunction ( luaVM, "getBlurLevel", "getPlayerBlurLevel" );
    lua_classfunction ( luaVM, "getVersion", "getPlayerVersion" );
    lua_classfunction ( luaVM, "getMoney", "getPlayerMoney" );
    lua_classfunction ( luaVM, "getAnnounceValue", "getPlayerAnnounceValue" );
    lua_classfunction ( luaVM, "getACInfo", "getPlayerACInfo" );

    lua_classvariable ( luaVM, "ACInfo", NULL, "getPlayerACInfo" );
    lua_classvariable ( luaVM, "voiceBroadcastTo", "setPlayerVoiceBroadcastTo", NULL );
    lua_classvariable ( luaVM, "voiceIgnoreFrom", "setPlayerVoiceIgnoreFrom", NULL );
    lua_classvariable ( luaVM, "money", "setPlayerMoney", "getPlayerMoney" );
    lua_classvariable ( luaVM, "version", NULL, "getPlayerVersion" );
    lua_classvariable ( luaVM, "wantedLevel", "setPlayerWantedLevel", "getPlayerWantedLevel" );
    lua_classvariable ( luaVM, "blurLevel", "setPlayerBlurLevel", "getPlayerBlurLevel" );
    lua_classvariable ( luaVM, "name", "setPlayerName", "getPlayerName" );
    lua_classvariable ( luaVM, "muted", "setPlayerMuted", "isPlayerMuted" );
    lua_classvariable ( luaVM, "idleTime", NULL, "getPlayerIdleTime" );
    lua_classvariable ( luaVM, "ping", NULL, "getPlayerPing" );
    lua_classvariable ( luaVM, "serial", NULL, "getPlayerSerial" );
    lua_classvariable ( luaVM, "ip", NULL, "getPlayerIP" );
    lua_classvariable ( luaVM, "team", "setPlayerTeam", "getPlayerTeam" );
    lua_classvariable ( luaVM, "mapForced", "forcePlayerMap", "isPlayerMapForced" );
    lua_classvariable ( luaVM, "nametagText", "setPlayerNametagText", "getPlayerNametagText" );
    lua_classvariable ( luaVM, "nametagShowing", "setPlayerNametagShowing", "isPlayerNametagShowing" );
    //lua_classvariable ( luaVM, "nametagColor", "setPlayerNametagColor", "getPlayerNametagColor", CLuaFunctionDefs::SetPlayerNametagColor, CLuaOOPDefs::GetPlayerNametagColor ); // color class
    //lua_classvariable ( luaVM, "announceValue", "setPlayerAnnounceValue", "getPlayerAnnounceValue", CLuaFunctionDefs::SetPlayerAnnounceValue, CLuaOOPDefs::GetPlayerAnnounceValue ); // .announceValue[key]=value
    //lua_classvariable ( luaVM, "hudComponent", "setHudComponentVisible", "", CLuaOOPDefs::SetHudComponentVisible, NULL ); .hudComponent["radar"]=true (a get function to needs to be implemented too)
    lua_classvariable ( luaVM, "random", NULL, "getRandomPlayer" );
    lua_classvariable ( luaVM, "count", NULL, "getPlayerCount" );
    lua_classvariable ( luaVM, "voiceEnabled", NULL, "isVoiceEnabled" );
    
    lua_registerclass ( luaVM, "Player", "Ped" );
}


void CLuaMain::AddRadarAreaClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "create", "createRadarArea" );
    lua_classfunction ( luaVM, "isInside", "isInsideRadarArea" ); // vector2
    
    lua_classfunction ( luaVM, "isFlashing", "isRadarAreaFlashing" );
    lua_classfunction ( luaVM, "getSize", "getRadarAreaSize" );
    lua_classfunction ( luaVM, "getColor", "getRadarAreaColor" );
    
    lua_classfunction ( luaVM, "setSize", "setRadarAreaSize" );
    lua_classfunction ( luaVM, "setFlashing", "setRadarAreaFlashing" );
    lua_classfunction ( luaVM, "setColor", "setRadarAreaColor" );
    
    lua_classvariable ( luaVM, "flashing", "isRadarAreaFlashing", "setRadarAreaFlashing" );
    //lua_classvariable ( luaVM, "color", "getRadarAreaColor", "setRadarAreaColor", "" ); color
    //lua_classvariable ( luaVM, "size", "getRadarAreaSize", "setRadarAreaSize", "" ); vector2

    lua_registerclass ( luaVM, "RadarArea", "Element" );
}


void CLuaMain::AddResourceClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );

    lua_registerclass ( luaVM, "Resource" );
}


void CLuaMain::AddConnectionClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "create", "dbConnect" );
    lua_classfunction ( luaVM, "exec", "dbExec" );
    lua_classfunction ( luaVM, "query", "dbQuery" ); // TODO: Accommodate for callbacks and arguments

    lua_registerclass ( luaVM, "Connection", "Element" );
}


// TODO: We need code to integrate this class into the handles returned by the db functions
void CLuaMain::AddQueryHandleClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "poll", "dbPoll" );
    lua_classfunction ( luaVM, "free", "dbFree" );

    lua_registerclass ( luaVM, "QueryHandle" );
}


void CLuaMain::AddTeamClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "create", "createTeam" );
    lua_classfunction ( luaVM, "getFromName", "getTeamFromName" );
    lua_classfunction ( luaVM, "countPlayers", "countPlayersInTeam" );
    lua_classfunction ( luaVM, "getPlayers", "getPlayersInTeam" );
    
    lua_classfunction ( luaVM, "getFriendlyFire", "getTeamFriendlyFire" );
    lua_classfunction ( luaVM, "getName", "getTeamName" );
    lua_classfunction ( luaVM, "getColor", "getTeamColor" ); // color
    
    lua_classfunction ( luaVM, "setName", "setTeamName" );
    lua_classfunction ( luaVM, "setColor", "setTeamColor" ); // color
    lua_classfunction ( luaVM, "setFriendlyFire", "setTeamFriendlyFire" );
    
    lua_classvariable ( luaVM, "playerCount", NULL, "countPlayersInTeam" );
    lua_classvariable ( luaVM, "friendlyFire", "setTeamFriendlyFire", "getTeamFriendlyFire" );
    lua_classvariable ( luaVM, "players", NULL, "getPlayersInTeam" ); // todo: perhaps table.insert/nilvaluing?
    lua_classvariable ( luaVM, "name", "setTeamName", "getTeamName" );
    //lua_classvariable ( luaVM, "color", "setTeamColor", "getTeamColor", , ); //color

    lua_registerclass ( luaVM, "Team", "Element" );
}


void CLuaMain::AddTextDisplayClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );

    lua_registerclass ( luaVM, "TextDisplay" );
}


void CLuaMain::AddTextItemClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );

    lua_registerclass ( luaVM, "TextItem" );
}


void CLuaMain::AddVehicleClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );

    lua_registerclass ( luaVM, "Vehicle", "Element" );
}


void CLuaMain::AddWaterClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "create", "createWater" );

    lua_classfunction ( luaVM, "getVertexPosition", "getWaterVertexPosition" ); // vector3
    lua_classfunction ( luaVM, "getWaveHeight", "getWaveHeight" );
    lua_classfunction ( luaVM, "getColor", "getWaterColor" ); // color
    
    lua_classfunction ( luaVM, "setWaveHeight", "setWaveHeight" );
    lua_classfunction ( luaVM, "setColor", "setWaterColor" ); // color
    lua_classfunction ( luaVM, "setVertexPosition", "setWaterVertexPosition" ); // vector3
    lua_classfunction ( luaVM, "setLevel", "setWaterLevel" ); // vector3d
    
    lua_classfunction ( luaVM, "resetColor", "resetWaterColor" );
    lua_classfunction ( luaVM, "resetLevel", "resetWaterLevel" );
    
    lua_classvariable ( luaVM, "level", "setWaterLevel", NULL );
    lua_classvariable ( luaVM, "height", "setWaveHeight", "getWaveHeight" );
    //lua_classvariable ( luaVM, "color", "setWaterColor", "getWaterColor", "" ); // color

    lua_registerclass ( luaVM, "Water", "Element" );
}


void CLuaMain::AddXMLClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "load", "xmlLoadFile" );
    lua_classfunction ( luaVM, "destroy", "xmlUnloadFile" );
    lua_classfunction ( luaVM, "copy", "xmlCopyFile" );
    lua_classfunction ( luaVM, "create", "xmlCreateFile" );
    lua_classfunction ( luaVM, "destroy", "xmlDestroyNode" );
    lua_classfunction ( luaVM, "loadMapData", "loadMapData" );
    lua_classfunction ( luaVM, "saveMapData", "saveMapData" );
    
    lua_classfunction ( luaVM, "setValue", "xmlNodeGetValue" );
    lua_classfunction ( luaVM, "setAttribute", "xmlNodeSetAttribute" );
    lua_classfunction ( luaVM, "setValue", "xmlNodeSetValue" );
    lua_classfunction ( luaVM, "saveFile", "xmlSaveFile" );
    lua_classfunction ( luaVM, "createChild", "xmlCreateChild" );
    lua_classfunction ( luaVM, "findChild", "xmlFindChild" );
    lua_classfunction ( luaVM, "setName", "xmlNodeSetName" );
    
    lua_classfunction ( luaVM, "getAttributes", "xmlNodeGetAttributes" );
    lua_classfunction ( luaVM, "getChildren", "xmlNodeGetChildren" );
    lua_classfunction ( luaVM, "getName", "xmlNodeGetName" );
    lua_classfunction ( luaVM, "getParent", "xmlNodeGetParent" );
    lua_classfunction ( luaVM, "getAttribute", "xmlNodeGetAttribute" ); // table classvar?
    
    lua_classvariable ( luaVM, "value", "xmlNodeSetValue", "xmlNodeGetValue" );
    lua_classvariable ( luaVM, "name", "xmlNodeSetName", "xmlNodeGetName" );
    lua_classvariable ( luaVM, "attributes", NULL, "xmlNodeGetAttributes" );
    lua_classvariable ( luaVM, "children", NULL, "xmlNodeGetChildren" );
    lua_classvariable ( luaVM, "parent", NULL, "xmlNodeGetParent" );

    lua_registerclass ( luaVM, "XML" );
}


void CLuaMain::AddTimerClass ( lua_State* luaVM )
{
    lua_newclass ( luaVM );
    
    lua_classfunction ( luaVM, "create", "setTimer" );
    lua_classfunction ( luaVM, "destroy", "killTimer" );
    lua_classfunction ( luaVM, "reset", "resetTimer" );
    lua_classfunction ( luaVM, "isValid", "isTimer" );
    
    lua_classfunction ( luaVM, "getDetails", "getTimerDetails" );
    
    lua_classvariable ( luaVM, "valid", NULL, "isTimer" );
    
    lua_registerclass ( luaVM, "Timer" );
}


void CLuaMain::InitClasses ( lua_State* luaVM )
{
    lua_initclasses             ( luaVM );


    // Element
    lua_newclass ( luaVM );

    AddVector3DClass            ( luaVM );
    //AddVector2DClass          ( luaVM );
    AddMatrixClass              ( luaVM );

    if ( !m_bEnableOOP )
        return;

    AddElementClass             ( luaVM );
    AddACLClass                 ( luaVM );
    AddACLGroupClass            ( luaVM );
    AddAccountClass             ( luaVM );
    AddBanClass                 ( luaVM );
    AddBlipClass                ( luaVM );
    AddColShapeClass            ( luaVM );
    AddFileClass                ( luaVM );
    AddMarkerClass              ( luaVM );
    AddObjectClass              ( luaVM );
    AddPedClass                 ( luaVM );
    AddPickupClass              ( luaVM );
    AddPlayerClass              ( luaVM );
    AddRadarAreaClass           ( luaVM );
    AddResourceClass            ( luaVM );
    AddConnectionClass          ( luaVM );
    AddQueryHandleClass         ( luaVM );
    AddTeamClass                ( luaVM );
    AddTextDisplayClass         ( luaVM );
    AddTextItemClass            ( luaVM );
    AddVehicleClass             ( luaVM );
    AddWaterClass               ( luaVM );
    AddXMLClass                 ( luaVM );
    AddTimerClass               ( luaVM );
}


void CLuaMain::InitVM ( void )
{
    assert( !m_luaVM );

    // Create a new VM
    m_luaVM = lua_open ();
    m_pLuaManager->OnLuaMainOpenVM( this, m_luaVM );

    // Set the instruction count hook
    lua_sethook ( m_luaVM, InstructionCountHook, LUA_MASKCOUNT, HOOK_INSTRUCTION_COUNT );

    // Load LUA libraries
    luaopen_base ( m_luaVM );
    luaopen_math ( m_luaVM );
    luaopen_string ( m_luaVM );
    luaopen_table ( m_luaVM );
    luaopen_debug ( m_luaVM );

    // Initialize security restrictions. Very important to prevent lua trojans and viruses!
    InitSecurity ();

    // Registering C functions
    CLuaCFunctions::RegisterFunctionsWithVM ( m_luaVM );

    // Create class metatables
    InitClasses ( m_luaVM );

    // Oli: Don't forget to add new ones to CLuaManager::LoadCFunctions. Thanks!

    // create global vars
    lua_pushelement ( m_luaVM, g_pGame->GetMapManager()->GetRootElement() );
    lua_setglobal ( m_luaVM, "root" );

    lua_pushresource ( m_luaVM, m_pResource );
    lua_setglobal ( m_luaVM, "resource" );

    lua_pushelement ( m_luaVM, m_pResource->GetResourceRootElement () );
    lua_setglobal ( m_luaVM, "resourceRoot" );

    // Load pre-loaded lua code
    LoadScript ( szPreloadedScript );
}


// Special function(s) that are only visible to HTMLD scripts
void CLuaMain::RegisterHTMLDFunctions ( void )
{
    CLuaHTTPDefs::LoadFunctions ( m_luaVM );
}


void CLuaMain::InstructionCountHook ( lua_State* luaVM, lua_Debug* pDebug )
{
    // Grab our lua VM
    CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
    if ( pLuaMain )
    {
        // Above max time?
        if ( pLuaMain->m_FunctionEnterTimer.Get () > HOOK_MAXIMUM_TIME )
        {
            // Print it in the console
            CLogger::ErrorPrintf ( "Infinite/too long execution (%s)\n", pLuaMain->GetScriptName () );
            
            SString strAbortInf = "Aborting; infinite running script in ";
            strAbortInf += pLuaMain->GetScriptName ();
            
            // Error out
            lua_pushstring ( luaVM, strAbortInf );
            lua_error ( luaVM );
        }
    }
}


bool CLuaMain::LoadScriptFromBuffer ( const char* cpInBuffer, unsigned int uiInSize, const char* szFileName )
{
    SString strNiceFilename = ConformResourcePath( szFileName );

    // Decrypt if required
    const char* cpBuffer;
    uint uiSize;
    if ( !g_pRealNetServer->DecryptScript( cpInBuffer, uiInSize, &cpBuffer, &uiSize, strNiceFilename ) )
    {
        // Problems problems
        if ( GetTimeString( true ) <= INVALID_COMPILED_SCRIPT_CUTOFF_DATE )
        {
            SString strMessage( "%s is invalid and will not work after %s. Please re-compile at http://luac.mtasa.com/", *strNiceFilename, INVALID_COMPILED_SCRIPT_CUTOFF_DATE ); 
            g_pGame->GetScriptDebugging()->LogWarning ( m_luaVM, "Script warning: %s", *strMessage );
            // cpBuffer is always valid after call to DecryptScript
        }
        else
        {
            SString strMessage( "%s is invalid. Please re-compile at http://luac.mtasa.com/", *strNiceFilename ); 
            g_pGame->GetScriptDebugging()->LogError ( m_luaVM, "Loading script failed: %s", *strMessage );
            return false;
        }
    }

    bool bUTF8;

    // UTF-8 BOM?  Compare by checking the standard UTF-8 BOM
    if ( IsUTF8BOM( cpBuffer, uiSize ) == false )
    {
        // Maybe not UTF-8, if we have a >80% heuristic detection confidence, assume it is
        bUTF8 = ( GetUTF8Confidence ( (const unsigned char*)cpBuffer, uiSize ) >= 80 );
    }
    else
    {
        // If there's a BOM, load ignoring the first 3 bytes
        bUTF8 = true;
        cpBuffer += 3;
        uiSize -= 3;
    }

    // If compiled script, make sure correct chunkname is embedded
    EmbedChunkName( strNiceFilename, &cpBuffer, &uiSize );

    if ( m_luaVM )
    {
        // Are we not marked as UTF-8 already, and not precompiled?
        std::string strUTFScript;
        if ( !bUTF8 && !IsLuaCompiledScript( cpBuffer, uiSize ) )
        {
            std::string strBuffer = std::string(cpBuffer, uiSize);
#ifdef WIN32
            std::setlocale(LC_CTYPE,""); // Temporarilly use locales to read the script
            strUTFScript = UTF16ToMbUTF8(ANSIToUTF16( strBuffer ));
            std::setlocale(LC_CTYPE,"C");
#else
            strUTFScript = UTF16ToMbUTF8(ANSIToUTF16( strBuffer ));
#endif

            if ( uiSize != strUTFScript.size() )
            {
                uiSize = strUTFScript.size();
                g_pGame->GetScriptDebugging()->LogWarning ( m_luaVM, "Script '%s' is not encoded in UTF-8.  Loading as ANSI...", strNiceFilename.c_str() );
            }
        }
        else
            strUTFScript = std::string(cpBuffer, uiSize);

        // Run the script
        if ( luaL_loadbuffer ( m_luaVM, bUTF8 ? cpBuffer : strUTFScript.c_str(), uiSize, SString ( "@%s", *strNiceFilename ) ) )
        {
            // Print the error
            std::string strRes = lua_tostring( m_luaVM, -1 );
            if ( strRes.length () )
            {
                CLogger::LogPrintf ( "SCRIPT ERROR: %s\n", strRes.c_str () );
                g_pGame->GetScriptDebugging()->LogError ( m_luaVM, "Loading script failed: %s", strRes.c_str () );
            }
            else
            {
                CLogger::LogPrint ( "SCRIPT ERROR: Unknown\n" );
                g_pGame->GetScriptDebugging()->LogError ( m_luaVM, "Loading script failed for unknown reason" );
            }
        }
        else
        {
            ResetInstructionCount ();
            int luaSavedTop = lua_gettop ( m_luaVM );
            int iret = this->PCall ( m_luaVM, 0, LUA_MULTRET, 0 ) ;
            if ( iret == LUA_ERRRUN || iret == LUA_ERRMEM )
            {
                SString strRes = lua_tostring( m_luaVM, -1 );
        
                std::vector <SString> vecSplit;
                strRes.Split ( ":", vecSplit );
                
                if ( vecSplit.size ( ) >= 3 )
                {
                    SString strFile = vecSplit[0];
                    int     iLine   = atoi ( vecSplit[1].c_str ( ) );
                    SString strMsg  = vecSplit[2].substr ( 1 );
                    
                    g_pGame->GetScriptDebugging()->LogError ( strFile, iLine, strMsg );
                }
                else
                {
                    if ( !strRes.ContainsI ( ExtractFilename ( strNiceFilename ) ) )
                    {
                        // Add filename to error message, if not already present
                        strRes = SString ( "%s (global scope) - %s", *strNiceFilename, *strRes );
                    }
                    g_pGame->GetScriptDebugging()->LogError ( m_luaVM, "%s", strRes.c_str () );
                }
            }
            // Cleanup any return values
            if ( lua_gettop ( m_luaVM ) > luaSavedTop )
                lua_settop( m_luaVM, luaSavedTop );
            return true;
        }
    }

    return false;
}


bool CLuaMain::LoadScript ( const char* szLUAScript )
{
    if ( m_luaVM )
    {
        // Run the script
        if ( !luaL_loadbuffer ( m_luaVM, szLUAScript, strlen(szLUAScript), NULL ) )
        {
            ResetInstructionCount ();
            int luaSavedTop = lua_gettop ( m_luaVM );
            int iret = this->PCall ( m_luaVM, 0, LUA_MULTRET, 0 ) ;
            if ( iret == LUA_ERRRUN || iret == LUA_ERRMEM )
            {
                std::string strRes = ConformResourcePath ( lua_tostring( m_luaVM, -1 ) );
                g_pGame->GetScriptDebugging()->LogError ( m_luaVM, "Executing in-line script failed: %s", strRes.c_str () );
            }
            // Cleanup any return values
            if ( lua_gettop ( m_luaVM ) > luaSavedTop )
                lua_settop( m_luaVM, luaSavedTop );
        }
        else
        {
            std::string strRes = ConformResourcePath ( lua_tostring( m_luaVM, -1 ) );
            g_pGame->GetScriptDebugging()->LogError ( m_luaVM, "Loading in-line script failed: %s", strRes.c_str () );
        }
    }
    else
        return false;

    return true;
}


void CLuaMain::Start ( void )
{

}


void CLuaMain::UnloadScript ( void )
{
    // Delete all timers and events
    m_pLuaTimerManager->RemoveAllTimers ();

    // Delete all keybinds
    list < CPlayer* > ::const_iterator iter = m_pPlayerManager->IterBegin ();
    for ( ; iter != m_pPlayerManager->IterEnd (); ++iter )
    {
        if ( (*iter)->IsJoined () )
            (*iter)->GetKeyBinds ()->RemoveAllKeys ( this );
    }

    // End the lua vm
    if ( m_luaVM )
    {
        m_pLuaManager->OnLuaMainCloseVM( this, m_luaVM );
        CLuaFunctionRef::RemoveLuaFunctionRefsForVM( m_luaVM );
        lua_close( m_luaVM );
        m_luaVM = NULL;
    }
}


void CLuaMain::DoPulse ( void )
{
    m_pLuaTimerManager->DoPulse ( this );
}


// Keep count of the number of open files in this resource and issue a warning if too high
void CLuaMain::OnOpenFile( const SString& strFilename )
{
    m_OpenFilenameList.push_back( strFilename );
    if ( m_OpenFilenameList.size() >= m_uiOpenFileCountWarnThresh )
    {
        m_uiOpenFileCountWarnThresh = m_OpenFilenameList.size() * 2;
        CLogger::LogPrintf ( "Notice: There are now %d open files in resource '%s'\n", m_OpenFilenameList.size(), GetScriptName() );
    }
}


void CLuaMain::OnCloseFile( const SString& strFilename )
{
    ListRemoveFirst( m_OpenFilenameList, strFilename );
}


CXMLFile * CLuaMain::CreateXML ( const char * szFilename )
{
    CXMLFile * pFile = g_pServerInterface->GetXML ()->CreateXML ( szFilename, true );
    if ( pFile )
    {
        m_XMLFiles.push_back ( pFile );
        if ( m_XMLFiles.size() >= m_uiOpenXMLFileCountWarnThresh )
        {
            m_uiOpenXMLFileCountWarnThresh = m_XMLFiles.size() * 2;
            CLogger::LogPrintf ( "Notice: There are now %d open XML files in resource '%s'\n", m_XMLFiles.size(), GetScriptName() );
        }
    }
    return pFile;
}


void CLuaMain::DestroyXML ( CXMLFile * pFile )
{
    m_XMLFiles.remove ( pFile );
    delete pFile;
}


void CLuaMain::DestroyXML ( CXMLNode * pRootNode )
{
    list<CXMLFile *>::iterator iter;
    for ( iter = m_XMLFiles.begin(); iter != m_XMLFiles.end(); ++iter )
    {
        CXMLFile * file = (*iter);
        if ( file )
        {
            if ( file->GetRootNode() == pRootNode )
            {
                m_XMLFiles.erase ( iter );
                delete file;
                break;
            }
        }
    }
}


void CLuaMain::SaveXML ( CXMLNode * pRootNode )
{
    list<CXMLFile *>::iterator iter;
    for ( iter = m_XMLFiles.begin(); iter != m_XMLFiles.end(); ++iter )
    {
        CXMLFile * file = (*iter);
        if ( file )
        {
            if ( file->GetRootNode() == pRootNode )
            {
                file->Write();
                break;
            }
        }
    }
    if ( m_pResource )
    {
        list < CResourceFile* > ::iterator iter = m_pResource->IterBegin ();
        for ( ; iter != m_pResource->IterEnd () ; ++iter )
        {
            CResourceFile* pResourceFile = *iter;
            if ( pResourceFile->GetType () == CResourceFile::RESOURCE_FILE_TYPE_CONFIG )
            {
                CResourceConfigItem* pConfigItem = static_cast < CResourceConfigItem* > ( pResourceFile );
                if ( pConfigItem->GetRoot () == pRootNode )
                {
                    CXMLFile* pFile = pConfigItem->GetFile ();
                    if ( pFile )
                    {
                        pFile->Write ();
                    }
                    break;
                }
            }
        }
    }
}


CTextDisplay * CLuaMain::CreateDisplay ( )
{
    CTextDisplay * pDisplay = new CTextDisplay;
    m_Displays.push_back ( pDisplay );
    return pDisplay;
}


void CLuaMain::DestroyDisplay ( CTextDisplay * pDisplay )
{
    m_Displays.remove ( pDisplay );
    delete pDisplay;
}


CTextItem * CLuaMain::CreateTextItem ( const char* szText, float fX, float fY, eTextPriority priority, const SColor color, float fScale, unsigned char format, unsigned char ucShadowAlpha )
{
    CTextItem * pTextItem = new CTextItem( szText, CVector2D ( fX, fY ), priority, color, fScale, format, ucShadowAlpha );
    m_TextItems.push_back ( pTextItem );
    return pTextItem;
}


void CLuaMain::DestroyTextItem ( CTextItem * pTextItem )
{
    m_TextItems.remove ( pTextItem );
    delete pTextItem;
}


CTextDisplay* CLuaMain::GetTextDisplayFromScriptID ( uint uiScriptID )
{
    CTextDisplay* pTextDisplay = (CTextDisplay*) CIdArray::FindEntry ( uiScriptID, EIdClass::TEXT_DISPLAY );
    dassert ( !pTextDisplay || ListContains ( m_Displays, pTextDisplay ) );
    return pTextDisplay;
}


CTextItem* CLuaMain::GetTextItemFromScriptID ( uint uiScriptID )
{
    CTextItem* pTextItem = (CTextItem*) CIdArray::FindEntry ( uiScriptID, EIdClass::TEXT_ITEM );
    dassert ( !pTextItem || ListContains ( m_TextItems, pTextItem ) );
    return pTextItem;
}


///////////////////////////////////////////////////////////////
//
// CLuaMain::GetFunctionTag
//
// Turn iFunctionNumber into something human readable
//
///////////////////////////////////////////////////////////////
const SString& CLuaMain::GetFunctionTag ( int iLuaFunction )
{
    // Find existing
    SString* pTag = MapFind ( m_FunctionTagMap, iLuaFunction );
#ifndef CHECK_FUNCTION_TAG
    if ( !pTag )
#endif
    {
        // Create if required
        SString strText;

        lua_Debug debugInfo;
        lua_getref ( m_luaVM, iLuaFunction );
        if ( lua_getinfo( m_luaVM, ">nlS", &debugInfo ) )
        {
            // Make sure this function isn't defined in a string
            if ( debugInfo.source[0] == '@' )
            {
                //std::string strFilename2 = ConformResourcePath ( debugInfo.source );
                SString strFilename = debugInfo.source;

                int iPos = strFilename.find_last_of ( "/\\" );
                if ( iPos >= 0 )
                    strFilename = strFilename.substr ( iPos + 1 );

                strText = SString ( "@%s:%d", strFilename.c_str (), debugInfo.currentline != -1 ? debugInfo.currentline : debugInfo.linedefined, iLuaFunction );
            }
            else
            {
                strText = SString ( "@func_%d %s", iLuaFunction, debugInfo.short_src );
            }
        }
        else
        {
            strText = SString ( "@func_%d NULL", iLuaFunction );
        }

    #ifdef CHECK_FUNCTION_TAG
        if ( pTag )
        {
            // Check tag remains unchanged
            assert ( strText == *pTag );
            return *pTag;
        }
    #endif

        MapSet ( m_FunctionTagMap, iLuaFunction, strText );
        pTag = MapFind ( m_FunctionTagMap, iLuaFunction );
    }
    return *pTag;
}


///////////////////////////////////////////////////////////////
//
// CLuaMain::PCall
//
// lua_pcall call wrapper
//
///////////////////////////////////////////////////////////////
int CLuaMain::PCall ( lua_State *L, int nargs, int nresults, int errfunc )
{
    if ( m_uiPCallDepth++ == 0 )
        m_WarningTimer.Reset();   // Only restart timer if initial call

    g_pGame->GetScriptDebugging()->PushLuaMain ( this );
    int iret = lua_pcall ( L, nargs, nresults, errfunc );
    g_pGame->GetScriptDebugging()->PopLuaMain ( this );

    --m_uiPCallDepth;
    return iret;
}


///////////////////////////////////////////////////////////////
//
// CLuaMain::CheckExecutionTime
//
// Issue warning if execution time is too long
//
///////////////////////////////////////////////////////////////
void CLuaMain::CheckExecutionTime( void )
{
    // Do time check
    if ( m_WarningTimer.Get() < 5000 )
        return;
    m_WarningTimer.Reset();

    // No warning if no players
    if ( g_pGame->GetPlayerManager()->Count() == 0 )
        return;

    // Issue warning about script execution time
    CLogger::LogPrintf ( "WARNING: Long execution (%s)\n", GetScriptName () );
}
