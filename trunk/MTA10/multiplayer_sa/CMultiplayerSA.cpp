/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        multiplayer_sa/CMultiplayerSA.cpp
*  PURPOSE:     Multiplayer module class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*               Peter <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include <stdio.h>

// These includes have to be fixed!
#include "..\game_sa\CCameraSA.h"
#include "..\game_sa\CEntitySA.h"
#include "..\game_sa\CPedSA.h"
#include "..\game_sa\common.h"

extern CGame * pGameInterface;

unsigned long CMultiplayerSA::HOOKPOS_FindPlayerCoors;
unsigned long CMultiplayerSA::HOOKPOS_FindPlayerCentreOfWorld;
unsigned long CMultiplayerSA::HOOKPOS_FindPlayerHeading;
unsigned long CMultiplayerSA::HOOKPOS_CStreaming_Update_Caller;
unsigned long CMultiplayerSA::HOOKPOS_CHud_Draw_Caller;
unsigned long CMultiplayerSA::HOOKPOS_CRunningScript_Process;
unsigned long CMultiplayerSA::HOOKPOS_CExplosion_AddExplosion;
unsigned long CMultiplayerSA::HOOKPOS_CRealTimeShadowManager__ReturnRealTimeShadow;
unsigned long CMultiplayerSA::HOOKPOS_CCustomRoadsignMgr__RenderRoadsignAtomic;
unsigned long CMultiplayerSA::HOOKPOS_Trailer_BreakTowLink;
unsigned long CMultiplayerSA::HOOKPOS_CRadar__DrawRadarGangOverlay;
unsigned long CMultiplayerSA::HOOKPOS_CTaskComplexJump__CreateSubTask;
unsigned long CMultiplayerSA::HOOKPOS_CTrain_ProcessControl_Derail;
unsigned long CMultiplayerSA::HOOKPOS_CVehicle_SetupRender;
unsigned long CMultiplayerSA::HOOKPOS_CVehicle_ResetAfterRender;
unsigned long CMultiplayerSA::HOOKPOS_CObject_Render;
unsigned long CMultiplayerSA::HOOKPOS_EndWorldColors;

unsigned long CMultiplayerSA::FUNC_CStreaming_Update;
unsigned long CMultiplayerSA::FUNC_CAudioEngine__DisplayRadioStationName;
unsigned long CMultiplayerSA::FUNC_CHud_Draw;

unsigned long CMultiplayerSA::ADDR_CursorHiding;
unsigned long CMultiplayerSA::ADDR_GotFocus;

unsigned long CMultiplayerSA::FUNC_CPlayerInfoBase;

#define HOOKPOS_FxManager_CreateFxSystem 0x4A9BE0
#define HOOKPOS_FxManager_DestroyFxSystem 0x4A9810

DWORD RETURN_FxManager_CreateFxSystem = 0x4A9BE8;
DWORD RETURN_FxManager_DestroyFxSystem = 0x4A9817;

#define HOOKPOS_CCam_ProcessFixed 0x051D470
#define HOOKPOS_CTaskSimplePlayerOnFoot_ProcessPlayerWeapon 0x6859a0
#define HOOKPOS_CPed_IsPlayer 0x5DF8F0

DWORD RETURN_CTaskSimplePlayerOnFoot_ProcessPlayerWeapon = 0x6859A7;
DWORD RETURN_CPed_IsPlayer = 0x5DF8F6;

CPed* pContextSwitchedPed = 0;
CVector vecCenterOfWorld;
FLOAT fFalseHeading;
bool bSetCenterOfWorld;
DWORD dwVectorPointer;
bool bInStreamingUpdate;
bool bHideRadar;
bool bHasProcessedScript;
float fX, fY, fZ;
DWORD RoadSignFixTemp;
DWORD dwEAEG = 0;
bool m_bExplosionsDisabled;
float fGlobalGravity = 0.008f;
float fLocalPlayerGravity = 0.008f;
float fLocalPlayerCameraRotation = 0.0f;
bool bCustomCameraRotation = false;

bool bUsingCustomSkyGradient = false;
BYTE ucSkyGradientTopR = 0;
BYTE ucSkyGradientTopG = 0;
BYTE ucSkyGradientTopB = 0;
BYTE ucSkyGradientBottomR = 0;
BYTE ucSkyGradientBottomG = 0;
BYTE ucSkyGradientBottomB = 0;
bool bUsingCustomWaterColor = false;
float fWaterColorR = 0.0F;
float fWaterColorG = 0.0F;
float fWaterColorB = 0.0F;
float fWaterColorA = 0.0F;

CStatsData localStatsData;
bool bLocalStatsStatic = true;
extern bool bWeaponFire;

PreContextSwitchHandler* m_pPreContextSwitchHandler = NULL;
PostContextSwitchHandler* m_pPostContextSwitchHandler = NULL;
PreWeaponFireHandler* m_pPreWeaponFireHandler = NULL;
PostWeaponFireHandler* m_pPostWeaponFireHandler = NULL;
DamageHandler* m_pDamageHandler = NULL;
FireHandler* m_pFireHandler = NULL;
ProjectileHandler* m_pProjectileHandler = NULL;
ProjectileStopHandler* m_pProjectileStopHandler = NULL;
ProcessCamHandler* m_pProcessCamHandler = NULL;
GameProcessHandler* m_pGameProcessHandler = NULL;

ExplosionHandler * m_pExplosionHandler; // stores our handler
BreakTowLinkHandler * m_pBreakTowLinkHandler = NULL;
DrawRadarAreasHandler * m_pDrawRadarAreasHandler = NULL;
Render3DStuffHandler * m_pRender3DStuffHandler = NULL;


VOID HOOK_FindPlayerCoors();
VOID HOOK_FindPlayerCentreOfWorld();
VOID HOOK_FindPlayerHeading();
VOID HOOK_CStreaming_Update_Caller();
VOID HOOK_CHud_Draw_Caller();
VOID HOOK_CRunningScript_Process();
VOID HOOK_CExplosion_AddExplosion();
VOID HOOK_CRealTimeShadowManager__ReturnRealTimeShadow();
VOID HOOK_CCustomRoadsignMgr__RenderRoadsignAtomic();
VOID HOOK_Trailer_BreakTowLink();
VOID HOOK_CRadar__DrawRadarGangOverlay();
VOID HOOK_CTaskComplexJump__CreateSubTask();
VOID HOOK_CWeapon_FireAreaEffect();
VOID HOOK_CBike_ProcessRiderAnims();
VOID HOOK_FxManager_CreateFxSystem ();
VOID HOOK_FxManager_DestroyFxSystem ();
VOID HOOK_CCam_ProcessFixed ();
VOID HOOK_Render3DStuff ();
VOID HOOK_CTaskSimplePlayerOnFoot_ProcessPlayerWeapon ();
VOID HOOK_CPed_IsPlayer ();
VOID HOOK_CTrain_ProcessControl_Derail ();
VOID HOOK_CVehicle_SetupRender ();
VOID HOOK_CVehicle_ResetAfterRender();
VOID HOOK_CObject_Render ();
VOID HOOK_EndWorldColors ();
VOID HOOK_CGame_Process ();

CEntitySAInterface * dwSavedPlayerPointer = 0;
CEntitySAInterface * activeEntityForStreaming = 0; // the entity that the streaming system considers active

#define CALL_Render3DStuff 0x53EABF
#define FUNC_Render3DStuff 0x53DF40

#define CALL_CRenderer_Render 0x53EA12
#define FUNC_CRenderer_Render 0x727140

#define CALL_CWeapon_FireAreaEffect 0x73EBFE
#define FUNC_CWeapon_FireAreaEffect 0x53A450
#define CALL_CBike_ProcessRiderAnims 0x6BF425   // @ CBike::ProcessDrivingAnims

#define CALL_CGame_Process 0x53E981

DWORD FUNC_CBike_ProcessRiderAnims = 0x6B7280;
DWORD FUNC_CEntity_Render = 0x534310;


CMultiplayerSA::CMultiplayerSA()
{
    // Unprotect all of the GTASA code at once and leave it that way
    DWORD oldProt;
    VirtualProtect((LPVOID)0x401000, 0x4A3000, PAGE_EXECUTE_READWRITE, &oldProt);

    // Initialize the offsets
    eGameVersion version = pGameInterface->GetGameVersion ();
    switch ( version )
    {
        case VERSION_EU_10: COffsetsMP::Initialize10EU (); break;
        case VERSION_US_10: COffsetsMP::Initialize10US (); break;
        case VERSION_11:    COffsetsMP::Initialize11 (); break;
        case VERSION_20:    COffsetsMP::Initialize20 (); break;
    }

	Population = new CPopulationSA;
    
    CRemoteDataSA::Init();

    m_bExplosionsDisabled = false;
	m_pExplosionHandler = NULL;
    m_pBreakTowLinkHandler = NULL;
    m_pDrawRadarAreasHandler = NULL;
    m_pDamageHandler = NULL;
    m_pFireHandler = NULL;
    m_pProjectileHandler = NULL;
    m_pProjectileStopHandler = NULL;

    memset ( &localStatsData, 0, sizeof ( CStatsData ) );
    localStatsData.StatTypesFloat [ 24 ] = 569.0f; // Max Health
}

void CMultiplayerSA::InitHooks()
{
	InitKeysyncHooks();
    InitShotsyncHooks();
	bSetCenterOfWorld = false;
	bHasProcessedScript = false;

	//00442DC6  |. 0F86 31090000  JBE gta_sa_u.004436FD
	//00442DC6     E9 32090000    JMP gta_sa_u.004436FD

    // increase the number of vehicles types (not actual vehicles) that can be loaded at once
    *(int *)0x8a5a84 = 127;

    // DISABLE CGameLogic::Update
	memset((void *)0x442AD0, 0xC3, 1);

    // STOP IT TRYING TO LOAD THE SCM
	*(BYTE *)0x468EB5 = 0xEB;
	*(BYTE *)0x468EB6 = 0x32;

	HookInstall(HOOKPOS_FindPlayerCoors, (DWORD)HOOK_FindPlayerCoors, 6);
	HookInstall(HOOKPOS_FindPlayerCentreOfWorld, (DWORD)HOOK_FindPlayerCentreOfWorld, 6);
	HookInstall(HOOKPOS_FindPlayerHeading, (DWORD)HOOK_FindPlayerHeading, 6);
	HookInstall(HOOKPOS_CStreaming_Update_Caller, (DWORD)HOOK_CStreaming_Update_Caller, 7);
	HookInstall(HOOKPOS_CHud_Draw_Caller, (DWORD)HOOK_CHud_Draw_Caller, 10);
	HookInstall(HOOKPOS_CRunningScript_Process, (DWORD)HOOK_CRunningScript_Process, 6);
	HookInstall(HOOKPOS_CExplosion_AddExplosion, (DWORD)HOOK_CExplosion_AddExplosion, 6);
    HookInstall(HOOKPOS_CRealTimeShadowManager__ReturnRealTimeShadow, (DWORD)HOOK_CRealTimeShadowManager__ReturnRealTimeShadow, 6);
	HookInstall(HOOKPOS_CCustomRoadsignMgr__RenderRoadsignAtomic, (DWORD)HOOK_CCustomRoadsignMgr__RenderRoadsignAtomic, 6);
    HookInstall(HOOKPOS_Trailer_BreakTowLink, (DWORD)HOOK_Trailer_BreakTowLink, 6);
    HookInstall(HOOKPOS_CRadar__DrawRadarGangOverlay, (DWORD)HOOK_CRadar__DrawRadarGangOverlay, 6);
    HookInstall(HOOKPOS_CTaskComplexJump__CreateSubTask, (DWORD)HOOK_CTaskComplexJump__CreateSubTask, 6);
    HookInstall(HOOKPOS_FxManager_CreateFxSystem, (DWORD)HOOK_FxManager_CreateFxSystem, 8);
    HookInstall(HOOKPOS_FxManager_DestroyFxSystem, (DWORD)HOOK_FxManager_DestroyFxSystem, 7);
    HookInstall(HOOKPOS_CCam_ProcessFixed, (DWORD)HOOK_CCam_ProcessFixed, 7);
    HookInstall(HOOKPOS_CTaskSimplePlayerOnFoot_ProcessPlayerWeapon, (DWORD)HOOK_CTaskSimplePlayerOnFoot_ProcessPlayerWeapon, 7);
    HookInstall(HOOKPOS_CPed_IsPlayer, (DWORD)HOOK_CPed_IsPlayer, 6);
    HookInstall(HOOKPOS_CTrain_ProcessControl_Derail, (DWORD)HOOK_CTrain_ProcessControl_Derail, 6);
    HookInstall(HOOKPOS_CVehicle_SetupRender, (DWORD)HOOK_CVehicle_SetupRender, 5);
    HookInstall(HOOKPOS_CVehicle_ResetAfterRender, (DWORD)HOOK_CVehicle_ResetAfterRender, 5);
    HookInstall(HOOKPOS_CObject_Render, (DWORD)HOOK_CObject_Render, 5);
	HookInstall(HOOKPOS_EndWorldColors, (DWORD)HOOK_EndWorldColors, 5);

    HookInstallCall ( CALL_CGame_Process, (DWORD)HOOK_CGame_Process );
    HookInstallCall ( CALL_CBike_ProcessRiderAnims, (DWORD)HOOK_CBike_ProcessRiderAnims );
    HookInstallCall ( CALL_Render3DStuff, (DWORD)HOOK_Render3DStuff );
	HookInstallCall ( CALL_CWeapon_FireAreaEffect, (DWORD)HOOK_CWeapon_FireAreaEffect);

    // Disable GTA setting g_bGotFocus to false when we minimize
    memset ( (void *)ADDR_GotFocus, 0x90, pGameInterface->GetGameVersion () == VERSION_EU_10 ? 6 : 10 );

    // Increase double link limit from 3200 ro 4000
    *(int*)0x00550F82 = 4000;


    // Disable GTA being able to call CAudio::StopRadio ()
    // Well this isn't really CAudio::StopRadio, it's some global class
    // func that StopRadio just jumps to.
	*(BYTE *)0x4E9820 = 0xC2;
    *(BYTE *)0x4E9821 = 0x08;
    *(BYTE *)0x4E9822 = 0x00;

    // Disable GTA being able to call CAudio::StartRadio ()
	*(BYTE *)0x4DBEC0 = 0xC2;
    *(BYTE *)0x4DBEC1 = 0x00;
    *(BYTE *)0x4DBEC2 = 0x00;

	*(BYTE *)0x4EB3C0 = 0xC2;
    *(BYTE *)0x4EB3C1 = 0x10;
    *(BYTE *)0x4EB3C2 = 0x00;
    
    // DISABLE cinematic camera for trains
    *(BYTE *)0x52A535 = 0;

    // DISABLE wanted levels for military zones
	*(BYTE *)0x72DF0D = 0xEB;

    // THROWN projectiles throw more accurately
	*(BYTE *)0x742685 = 0x90;
    *(BYTE *)0x742686 = 0xE9;

    // DISABLE CProjectileInfo::RemoveAllProjectiles
	*(BYTE *)0x7399B0 = 0xC3;

    // DISABLE CRoadBlocks::GenerateRoadblocks
    *(BYTE *)0x4629E0 = 0xC3;


    // Temporary hack for disabling hand up
    /*
	*(BYTE *)0x62AEE7 = 0x90;
    *(BYTE *)0x62AEE8 = 0x90;
    *(BYTE *)0x62AEE9 = 0x90;
    *(BYTE *)0x62AEEA = 0x90;
    *(BYTE *)0x62AEEB = 0x90;
    *(BYTE *)0x62AEEC = 0x90;
    */

    // DISABLE CAERadioTrackManager::CheckForMissionStatsChanges(void) (special DJ banter)
	*(BYTE *)0x4E8410 = 0xC3; // retn

    // DISABLE CPopulation__AddToPopulation
	*(BYTE *)0x614720 = 0x32; // xor al, al
    *(BYTE *)0x614721 = 0xC0;
    *(BYTE *)0x614722 = 0xC3; // retn

    // Disables deletion of RenderWare objects during unloading of ModelInfo
    // This is used so we can circumvent the limit of ~21 different vehicles by managing the RwObject ourselves
    //*(BYTE *)0x4C9890 = 0xC3;

    //memset ( (void*)0x408A1B, 0x90, 5 );

    // Hack to make the choke task use 0 time left remaining when he starts t
    // just stand there looking. So he won't do that.
    *(unsigned char *)0x620607 = 0x33;
    *(unsigned char *)0x620608 = 0xC0;

    *(unsigned char *)0x620618 = 0x33;
    *(unsigned char *)0x620619 = 0xC0;
    *(unsigned char *)0x62061A = 0x90;
    *(unsigned char *)0x62061B = 0x90;
    *(unsigned char *)0x62061C = 0x90;

    // Hack to make non-local players always update their aim on akimbo weapons using camera
    // so they don't freeze when local player doesn't aim.
    *(BYTE *)0x61EFFE = 0xEB;   // JMP imm8 (was JZ imm8)
    

    // DISABLE CGameLogic__SetPlayerWantedLevelForForbiddenTerritories
	*(BYTE *)0x441770 = 0xC3;

    // DISABLE CCrime__ReportCrime
	*(BYTE *)0x532010 = 0xC3;
    
	// Disables deletion of RenderWare objects during unloading of ModelInfo
	// This is used so we can circumvent the limit of ~21 different vehicles by managing the RwObject ourselves
	//*(BYTE *)0x4C9890 = 0xC3;

    /*
    004C021D   B0 00            MOV AL,0
    004C021F   90               NOP
    004C0220   90               NOP
    004C0221   90               NOP
    */
	*(BYTE *)0x4C01F0 = 0xB0;
	*(BYTE *)0x4C01F1 = 0x00;
    *(BYTE *)0x4C01F2 = 0x90;
    *(BYTE *)0x4C01F3 = 0x90;
    *(BYTE *)0x4C01F4 = 0x90;

    // Disable MakePlayerSafe
	*(BYTE *)0x56E870 = 0xC2;
	*(BYTE *)0x56E871 = 0x08;
	*(BYTE *)0x56E872 = 0x00;

    // Disable call to FxSystem_c__GetCompositeMatrix in CAEFireAudioEntity::UpdateParameters 
    // that was causing a crash - spent ages debugging, the crash happens if you create 40 or 
    // so vehicles that catch fire (upside down) then delete them, repeating a few times.
    memset((void*)0x4DCF87,0x90,6);
    
    /*
    // DISABLE CPed__RemoveBodyPart
	*(BYTE *)0x5F0140 = 0xC2;
    *(BYTE *)0x5F0141 = 0x08;
    *(BYTE *)0x5F0142 = 0x00;
    */

    // ALLOW picking up of all vehicles (GTA doesn't allow picking up of 'locked' script created vehicles)
	*(BYTE *)0x6A436C = 0x90;
    *(BYTE *)0x6A436D = 0x90;

    // MAKE CEntity::GetIsOnScreen always return true, experimental
   /*
	*(BYTE *)0x534540 = 0xB0;
    *(BYTE *)0x534541 = 0x01;
    *(BYTE *)0x534542 = 0xC3;
    */

    //DISABLE CPad::ReconcileTwoControllersInput
	/*
	*(BYTE *)0x53F530 = 0xC2;
    *(BYTE *)0x53F531 = 0x0C;
    *(BYTE *)0x53F532 = 0x00;

	*(BYTE *)0x53EF80 = 0xC3;

	*(BYTE *)0x541DDC = 0xEB;
    *(BYTE *)0x541DDD = 0x60;
*/
    // DISABLE big buildings (test)
    /*
	*(char*)0x533150 = 0xC3;
    */
    
	// PREVENT THE RADIO OR ENGINE STOPPING WHEN PLAYER LEAVES VEHICLE
	// THIS ON ITS OWN will cause sounds to be left behind and other artifacts
	/*
	*(char *)0x4FB8C0 = 0xC3;
	*/


/*	
	memset((void *)0x4FBA3E, 0x90, 5);
	*/
	

	// DISABLE REPLAYS
/*	
	memset((void *)0x460500, 0xC3, 1);
*/
    // PREVENT the game from making dummy objects (may fix a crash, guesswork really)
    // This seems to work, but doesn't actually fix anything. Maybe a reason to do it in the future.
    //00615FE3     EB 09          JMP SHORT gta_sa_u.00615FEE
	/*
	memset((void *)0x615FE3, 0xEB, 1);
	*/

    // Make combines eat players *untested*
	//memset ( (LPVOID)0x6A9739, 0x90, 6 );
    
    // Players always lean out whatever the camera mode
    // 00621983     EB 13          JMP SHORT hacked_g.00621998
	*(BYTE *)0x621983 = 0xEB;

    
    // Players can fire drivebys whatever camera mode
    // 627E01 - 6 bytes
	memset ( (LPVOID)0x627E01, 0x90, 6 );

	memset ( (LPVOID)0x62840D, 0x90, 6 );

	// Satchel crash fix
	// C89110: satchel (bomb) positions pointer?
	// C891A8+4: satchel (model) positions pointer? gets set to NULL on player death, causing an access violation
	// C891A8+12: satchel (model) disappear time (in SystemTime format). 738F99 clears the satchel when VAR_SystemTime is larger.
	memset ( (LPVOID)0x738F3A, 0x90, 83 );

    // Prevent gta stopping driveby players from falling off
	memset ( (LPVOID)0x6B5B17, 0x90, 6 );

    // Increase VehicleStruct pool size
	*(BYTE *)0x5B8FE4 = 0x7F; // its signed so the higest you can go with this is 0x7F before it goes negative = crash
    
	/*
    // CTaskSimpleCarDrive: Swaps driveby for gang-driveby for drivers
	memset ( (LPVOID)0x6446A7, 0x90, 6 );
    
    // CTaskSimpleCarDrive: Swaps driveby for gang-driveby for passengers
	memset ( (LPVOID)0x6446BD, 0x90, 6 );
	*/
    

	// DISABLE PLAYING REPLAYS
	memset((void *)0x460390, 0xC3, 1);

	memset((void *)0x4600F0, 0xC3, 1);

	memset((void *)0x45F050, 0xC3, 1);

	// DISABLE CHEATS
	memset((void *)0x439AF0, 0xC3, 1);
		
	memset((void *)0x438370, 0xC3, 1);


    // DISABLE GARAGES
    *(BYTE *)(0x44AA89 + 0) = 0xE9;
    *(BYTE *)(0x44AA89 + 1) = 0x28;
    *(BYTE *)(0x44AA89 + 2) = 0x01;
    *(BYTE *)(0x44AA89 + 3) = 0x00;
    *(BYTE *)(0x44AA89 + 4) = 0x00;
    *(BYTE *)(0x44AA89 + 5) = 0x90;

    *(DWORD *)0x44C7E0 = 0x44C7C4;
    *(DWORD *)0x44C7E4 = 0x44C7C4;
    *(DWORD *)0x44C7F8 = 0x44C7C4;
    *(DWORD *)0x44C7FC = 0x44C7C4;
    *(DWORD *)0x44C804 = 0x44C7C4;
    *(DWORD *)0x44C808 = 0x44C7C4;
    *(DWORD *)0x44C83C = 0x44C7C4;
    *(DWORD *)0x44C840 = 0x44C7C4;
    *(DWORD *)0x44C850 = 0x44C7C4;
    *(DWORD *)0x44C854 = 0x44C7C4;
    *(DWORD *)0x44C864 = 0x44C7C4;
    *(DWORD *)0x44C868 = 0x44C7C4;
    *(DWORD *)0x44C874 = 0x44C7C4;
    *(DWORD *)0x44C878 = 0x44C7C4;
    *(DWORD *)0x44C88C = 0x44C7C4;
    *(DWORD *)0x44C890 = 0x44C7C4;
    *(DWORD *)0x44C89C = 0x44C7C4;
    *(DWORD *)0x44C8A0 = 0x44C7C4;
    *(DWORD *)0x44C8AC = 0x44C7C4;
    *(DWORD *)0x44C8B0 = 0x44C7C4;

    *(BYTE *)(0x44C39A + 0) = 0x0F;
    *(BYTE *)(0x44C39A + 1) = 0x84;
    *(BYTE *)(0x44C39A + 2) = 0x24;
    *(BYTE *)(0x44C39A + 3) = 0x04;
    *(BYTE *)(0x44C39A + 4) = 0x00;
    *(BYTE *)(0x44C39A + 5) = 0x00;

    // Avoid garage doors closing when you change your model
    memset((LPVOID)0x4486F7, 0x90, 4);
    

    // Disable CStats::IncrementStat (returns at start of function)
	*(BYTE *)0x55C180 = 0xC3;
	/*
	memset((void *)0x55C1A9, 0x90, 14 );
	memset((void *)0x55C1DD, 0x90, 7 );
	*/

	// DISABLE STATS DECREMENTING
	memset((void *)0x559FD5, 0x90, 7 );
	memset((void *)0x559FEB, 0x90, 7 );

	// DISABLE STATS MESSAGES
	memset((void *)0x55B980, 0xC3, 1);

	memset((void *)0x559760, 0xC3, 1);

    // ALLOW more than 8 players (crash with more if this isn't done)
    //0060D64D   90               NOP
    //0060D64E   E9 9C000000      JMP gta_sa.0060D6EF
	*(BYTE *)0x60D64D = 0x90;
    *(BYTE *)0x60D64E = 0xE9;

    // PREVENT CJ smoking and drinking like an addict
    //005FBA26   EB 29            JMP SHORT gta_sa.005FBA51
	*(BYTE *)0x5FBA26 = 0xEB;

    // PREVENT the camera from messing up for drivebys for vehicle drivers
	*(BYTE *)0x522423 = 0x90;
    *(BYTE *)0x522424 = 0x90;
    
    LPVOID patchAddress = NULL;
    // ALLOW ALT+TABBING WITHOUT PAUSING
    //if ( pGameInterface->GetGameVersion() == GAME_VERSION_US ) // won't work as pGameInterface isn't inited
    if ( *(BYTE *)0x748ADD == 0xFF && *(BYTE *)0x748ADE == 0x53 )
        patchAddress = (LPVOID)0x748A8D;
    else
        patchAddress = (LPVOID)0x748ADD;

    memset(patchAddress, 0x90, 6);

    // CENTER VEHICLE NAME and ZONE NAME messages
    // 0058B0AD   6A 02            PUSH 2 // orientation
    // VEHICLE
	*(BYTE *)0x58B0AE = 0x00;

    // ZONE
	*(BYTE *)0x58AD56 = 0x00;

    // 85953C needs to equal 320.0 to center the text (640.0 being the base width)
	*(float *)0x85953C = 320.0f;

    // 0058B147   D80D 0C958500    FMUL DWORD PTR DS:[85950C] // the text needs to be moved to the left
    //VEHICLE
	*(BYTE *)0x58B149 = 0x3C;

    //ZONE
	*(BYTE *)0x58AE52 = 0x3C;

	// DISABLE SAM SITES
	*(BYTE *)0x5A07D0 = 0xC3;

	// DISABLE TRAINS (AUTO GENERATED ONES)
	*(BYTE *)0x6F7900 = 0xC3;
    
    // Prevent TRAINS spawning with PEDs
	*(BYTE *)0x6F7865 = 0xEB;

	// DISABLE PLANES
	*(BYTE *)0x6CD2F0 = 0xC3;
	
	// DISABLE EMERGENCY VEHICLES
	*(BYTE *)0x42B7D0 = 0xC3;

	// DISABLE CAR GENERATORS
	*(BYTE *)0x6F3F40 = 0xC3;

    // DISABLE CEntryExitManager::Update (they crash when you enter anyway)
	*(BYTE *)0x440D10 = 0xC3;

	// Disable MENU AFTER alt + tab
	//0053BC72   C605 7B67BA00 01 MOV BYTE PTR DS:[BA677B],1	
	*(BYTE *)0x53BC78 = 0x00;

	// DISABLE HUNGER MESSAGES
	memset ( (LPVOID)0x56E740, 0x90, 5 );

	// DISABLE RANDOM VEHICLE UPGRADES
	memset ( (LPVOID)0x6B0BC2, 0xEB, 1 );

	// DISABLE CPOPULATION::UPDATE - DOES NOT prevent vehicles - only on-foot peds
	/*	
	*(BYTE *)0x616650 = 0xC3;
    *(BYTE *)0xA43088 = 1;
	*/

	// SORT OF HACK to make peds always walk around, even when in free-camera mode (in the editor)
	*(BYTE *)0x53C017 = 0x90;
	*(BYTE *)0x53C018 = 0x90;

	// DISABLE random cars
	//*(BYTE *)0x4341C0 = 0xC3;
	
	// DISABLE heat flashes
	/*
	*(BYTE *)0x6E3521 = 0x90;
	*(BYTE *)0x6E3522 = 0xE9;
	*/

	// DECREASE ROF for missiles from hydra
	// 006D462C     81E1 E8030000  AND ECX,3E8
	// 006D4632     81C1 E8030000  ADD ECX,3E8
    /*	
	*(BYTE *)0x6D462E = 0xE8;
	*(BYTE *)0x6D462F = 0x03;
	*(BYTE *)0x6D4634 = 0xE8;
	*(BYTE *)0x6D4635 = 0x03;
    */

	// HACK to allow boats to be rotated
	/*
	006F2089   58               POP EAX
	006F208A   90               NOP
	006F208B   90               NOP
	006F208C   90               NOP
	006F208D   90               NOP
	*/
	*(BYTE *)0x6F2089 = 0x58;
	memset((void *)0x6F208A,0x90,4);

    // Prevent the game deleting _any_ far away vehicles - will cause issues for population vehicles in the future
	*(BYTE *)0x42CD10 = 0xC3;

    // DISABLE real-time shadows for peds
    *(BYTE *)0x5E68A0 = 0xEB;

    // and some more, just to be safe
    //00542483   EB 0B            JMP SHORT gta_sa.00542490
    *(BYTE *)0x542483 = 0xEB;

    // DISABLE weapon pickups
    *(BYTE *)0x5B47B0 = 0xC3;

    // INCREASE CEntyInfoNode pool size
    //00550FB9   68 F4010000      PUSH 1F4
    /*
    *(BYTE *)0x550FBA = 0xE8;
    *(BYTE *)0x550FBB = 0x03;
    */
    *(BYTE *)0x550FBA = 0x00;
    *(BYTE *)0x550FBB = 0x10;

    
	/*
	*(BYTE *)0x469F00 = 0xC3;
	*/

	// CCAM::PROCESSFIXED remover
/*
	*(BYTE *)0x51D470 = 0xC2;
	*(BYTE *)0x51D471 = 0x10;
	*(BYTE *)0x51D472 = 0x00;
*/

	// HACK to prevent RealTimeShadowManager crash
	// 00542483     EB 0B          JMP SHORT gta_sa_u.00542490
	/*
	*(BYTE *)0x542483 = 0xEB;
*/
	
	//InitShotsyncHooks();

    //DISABLE CPad::ReconcileTwoControllersInput
    *(BYTE *)0x53F530 = 0xC2;
    *(BYTE *)0x53F531 = 0x0C;
    *(BYTE *)0x53F532 = 0x00;

    *(BYTE *)0x53EF80 = 0xC3;

    *(BYTE *)0x541DDC = 0xEB;
    *(BYTE *)0x541DDD = 0x60;

    // DISABLE CWanted Helis (always return 0 from CWanted::NumOfHelisRequired)
    *(BYTE *)0x561FA4 = 0x90;
    *(BYTE *)0x561FA5 = 0x90;

    // DISABLE  CWanted__UpdateEachFrame
    memset( (void*)0x53BFF6, 0x90, 5 );

    // DISABLE CWanted__Update
    memset( (void*)0x60EBCC, 0x90, 5 );

	// Disable armour-increase upon entering an enforcer
	*(BYTE *)0x6D189B = 0x06;

	// Removes the last weapon pickups from interiors as well
	*(BYTE *)0x591F90 = 0xC3;

	// Trains may infact go further than Los Santos
	*(BYTE *)0x4418E0 = 0xC3;

    // EXPERIMENTAL - disable unloading of cols
   // memset( (void*)0x4C4EDA, 0x90, 10 );

    // Make CTaskComplexSunbathe::CanSunbathe always return true
	*(BYTE *)0x632140 = 0xB0;
    *(BYTE *)0x632141 = 0x01;
    *(BYTE *)0x632142 = 0xC3;
    
    // Stop CTaskSimpleCarDrive::ProcessPed from exiting passengers with CTaskComplexSequence (some timer check)
    *(BYTE *)0x644C18 = 0x90;
	*(BYTE *)0x644C19 = 0xE9;

    // Stop CPlayerPed::ProcessControl from calling CVisibilityPlugins::SetClumpAlpha
    memset ( (void*)0x5E8E84, 0x90, 5 );

    // Stop CVehicle::UpdateClumpAlpha from calling CVisibilityPlugins::SetClumpAlpha
    memset ( (void*)0x6D29CB, 0x90, 5 );

    // Disable CVehicle::DoDriveByShootings
    memset ( (void*)0x741FD0, 0x90, 3 );
    *(BYTE *)0x741FD0 = 0xC3;

    // Disable CTaskSimplePlayerOnFoot::PlayIdleAnimations (ret 4)
    *(BYTE *)0x6872C0 = 0xC2;
    *(BYTE *)0x6872C1 = 0x04;
    *(BYTE *)0x6872C2 = 0x00;

    /*
    // Disable forcing of ped animations to the player one in CPlayerPed::ProcessAnimGroups
    memset ( (LPVOID)0x609A44, 0x90, 21 );
    */

    // Let us sprint everywhere (always return 0 from CSurfaceData::isSprint)
    *(DWORD *)0x55E870 = 0xC2C03366;
    *(WORD *)0x55E874  = 0x0004;

    // Create pickup objects in interior 0 instead of 13
    *(BYTE *)0x59FAA3 = 0x00;

    // Don't get shotguns from police cars
    *(BYTE *)0x6D19CD = 0xEB;

    // Don't get golf clubs from caddies
    *(BYTE *)0x6D1A1A = 0xEB;

	// Don't get 20 health from ambulances
	*(BYTE *)0x6D1762 = 0x00;

    // Prevent CVehicle::RecalcTrainRailPosition from changing train speed
    memset((void *)0x6F701D, 0x90, 6);
    *(BYTE *)0x6F7069 = 0xEB;

    // The instanthit function for bullets ignores the first few bullets shot by
    // remote players after reloading because some flag isn't set (no bullet impact
    // graphics, no damage). Makes e.g. sawnoffs completely ineffective.
    // Remove this check so that no bullets are ignored.
    *(BYTE *)0x73FDF9 = 0xEB;

    // Make sure water is always drawn after trees and LOD instead of before
    // Disabled for now... Works fine for trees/LOD but *always* draws in front of
    // high grass which is annoying
    //*(WORD *)0x53DF55 = 0x9090;

    // Disallow spraying gang tags
    memset ( (void *)0x565C5C, 0x90, 10 );

    // Allow turning on vehicle lights even if the engine is off
    memset ( (void *)0x6E1DBC, 0x90, 8 );

    // Fix vehicle back lights both using light state 3 (SA bug)
    *(BYTE *)0x6E1D4F = 2;

    // Fix for sliding over objects and vehicles (ice floor)
    *(BYTE *)0x5E1E72 = 0xE9;
    *(BYTE *)0x5E1E73 = 0xB9;
    *(BYTE *)0x5E1E74 = 0x00;
    *(BYTE *)0x5E1E77 = 0x90;
}


// Used to store copied pointers for explosions in the FxSystem

std::list < DWORD* > Pointers_FxSystem;

void AddFxSystemPointer ( DWORD* pPointer )
{
    Pointers_FxSystem.push_front ( pPointer );
}


void RemoveFxSystemPointer ( DWORD* pPointer )
{
    // Look through our list for the pointer
    std::list < DWORD* > ::iterator iter = Pointers_FxSystem.begin ();
    for ( ; iter != Pointers_FxSystem.end (); iter++ )
    {
        // It exists in our list?
        if ( *iter == pPointer )
        {
            // Remove it from the list over our copied matrices
            Pointers_FxSystem.erase ( iter );

            // Delete the pointer itself
            free ( pPointer );
            return;
        }
    }
}

CRemoteDataStorage * CMultiplayerSA::CreateRemoteDataStorage ()
{
    return new CRemoteDataStorageSA ();
}

void CMultiplayerSA::DestroyRemoteDataStorage ( CRemoteDataStorage* pData )
{
    delete (CRemoteDataStorageSA *)pData;
}

void CMultiplayerSA::AddRemoteDataStorage ( CPlayerPed* pPed, CRemoteDataStorage* pData )
{
    CRemoteDataSA::AddRemoteDataStorage ( pPed, pData );
}

void CMultiplayerSA::RemoveRemoteDataStorage ( CPlayerPed* pPed )
{
    CRemoteDataSA::RemoveRemoteDataStorage ( pPed );
}

CPed * CMultiplayerSA::GetContextSwitchedPed ( void )
{
    return pContextSwitchedPed;
}

void CMultiplayerSA::AllowWindowsCursorShowing ( bool bAllow )
{
    /*
    0074821D   6A 00            PUSH 0
    0074821F   FF15 EC828500    CALL DWORD PTR DS:[<&USER32.ShowCursor>] ; USER32.ShowCursor
    00748225   6A 00            PUSH 0
    00748227   FF15 9C828500    CALL DWORD PTR DS:[<&USER32.SetCursor>]  ; USER32.SetCursor
    */
    BYTE originalCode[16] = {0x6A, 0x00, 0xFF, 0x15, 0xEC, 0x82, 0x85, 0x00, 0x6A, 0x00, 0xFF, 0x15, 0x9C, 0x82, 0x85, 0x00};

    if ( bAllow )
    {
	    memset ( (LPVOID)ADDR_CursorHiding, 0x90, 16 );
    }
    else
    {
        memcpy ( (LPVOID)ADDR_CursorHiding, &originalCode, 16 );
    }
}


CShotSyncData * CMultiplayerSA::GetLocalShotSyncData ( )
{
    return GetLocalPedShotSyncData();
}

void CMultiplayerSA::DisablePadHandler( bool bDisabled )
{
	// DISABLE GAMEPADS (testing)
	if ( bDisabled )
		*(BYTE *)0x7449F0 = 0xC3;
	else
		*(BYTE *)0x7449F0 = 0x8B;
}

void CMultiplayerSA::DisableHeatHazeEffect ( bool bDisable )
{
	*(bool *)0xC402BA = bDisable;
}

void CMultiplayerSA::DisableAllVehicleWeapons ( bool bDisable )
{
	if ( bDisable )
		*(BYTE *)0x6E3950 = 0xC3;
	else
		*(BYTE *)0x6E3950 = 0x83;
}

void CMultiplayerSA::DisableZoneNames ( bool bDisabled )
{
    // Please use CHud::DisableAreaName instead
    pGameInterface->GetHud()->DisableAreaName ( bDisabled );
}

void CMultiplayerSA::DisableBirds ( bool bDisabled )
{
	if ( bDisabled )
		*(BYTE *)0x712330 = 0xC3;
	else
		*(BYTE *)0x712330 = 0xA1;
}

void CMultiplayerSA::DisableClouds ( bool bDisabled )
{
    //volumetric clouds
	if ( bDisabled )
		*(BYTE *)0x716380 = 0xC3;
	else
		*(BYTE *)0x716380 = 0xA1;

    // normal clouds
    //0071395A     90             NOP
	if ( bDisabled )
		*(BYTE *)0x713950 = 0xC3;
	else
		*(BYTE *)0x713950 = 0x83;

    // plane trails (not really clouds, but they're sort of vapour)
	if ( bDisabled )
    {
		*(BYTE *)0x717180 = 0xC2;
        *(BYTE *)0x717181 = 0x04;
        *(BYTE *)0x717182 = 0x00;
    }
	else
    {
		*(BYTE *)0x717180 = 0x83;
        *(BYTE *)0x717181 = 0xEC;
        *(BYTE *)0x717182 = 0x08;
    }
}

void CMultiplayerSA::SetSkyColor ( unsigned char TopRed, unsigned char TopGreen, unsigned char TopBlue, unsigned char BottomRed, unsigned char BottomGreen, unsigned char BottomBlue )
{
	bUsingCustomSkyGradient = true;
	ucSkyGradientTopR = TopRed;
	ucSkyGradientTopG = TopGreen;
	ucSkyGradientTopB = TopBlue;
	ucSkyGradientBottomR = BottomRed;
	ucSkyGradientBottomG = BottomGreen;
	ucSkyGradientBottomB = BottomBlue;
}

void CMultiplayerSA::ResetSky ( void )
{
	bUsingCustomSkyGradient = false;
}

void CMultiplayerSA::SetWaterColor ( float fWaterRed, float fWaterGreen, float fWaterBlue, float fWaterAlpha )
{
	bUsingCustomWaterColor = true;
    // Water surface
	fWaterColorR = fWaterRed;
	fWaterColorG = fWaterGreen;
	fWaterColorB = fWaterBlue;
	fWaterColorA = fWaterAlpha;
    // Underwater
    *(BYTE *)0x8D5140 = (BYTE)fWaterRed;
    *(BYTE *)0x8D5141 = (BYTE)fWaterGreen;
    *(BYTE *)0x8D5142 = (BYTE)fWaterBlue;
    *(BYTE *)0x8D5143 = (BYTE)fWaterAlpha;
    *(BYTE *)0x7051A7 = 255-(BYTE)fWaterAlpha;          // These numbers are added to r,g,b
    *(float *)0x872660 = 255-fWaterAlpha;
    *(BYTE *)0x7051D7 = 255-(BYTE)fWaterAlpha;
}

void CMultiplayerSA::ResetWater ( void )
{
	bUsingCustomWaterColor = false;
    *(DWORD *)0x8D5140 = 0x40404040;
    *(BYTE *)0x7051A7 = 184;
    *(float *)0x872660 = 184.0f;
    *(BYTE *)0x7051D7 = 184;
}

bool CMultiplayerSA::GetExplosionsDisabled ( void )
{
    return m_bExplosionsDisabled;
}


void CMultiplayerSA::DisableExplosions ( bool bDisabled )
{
    m_bExplosionsDisabled = bDisabled;
}



void CMultiplayerSA::SetExplosionHandler ( ExplosionHandler * pExplosionHandler )
{
	m_pExplosionHandler = pExplosionHandler;
}

void CMultiplayerSA::SetProjectileHandler ( ProjectileHandler * pProjectileHandler )
{
    m_pProjectileHandler = pProjectileHandler;
}

void CMultiplayerSA::SetProjectileStopHandler ( ProjectileStopHandler * pProjectileHandler )
{
    m_pProjectileStopHandler = pProjectileHandler;
}

void CMultiplayerSA::SetBreakTowLinkHandler ( BreakTowLinkHandler * pBreakTowLinkHandler )
{
    m_pBreakTowLinkHandler = pBreakTowLinkHandler;
}

void CMultiplayerSA::SetDrawRadarAreasHandler ( DrawRadarAreasHandler * pRadarAreasHandler )
{
    m_pDrawRadarAreasHandler = pRadarAreasHandler;
}

void CMultiplayerSA::SetRender3DStuffHandler ( Render3DStuffHandler * pHandler )
{
    m_pRender3DStuffHandler = pHandler;
}

void CMultiplayerSA::SetDamageHandler ( DamageHandler * pDamageHandler )
{
    m_pDamageHandler = pDamageHandler;
}

void CMultiplayerSA::SetFireHandler ( FireHandler * pFireHandler )
{
    m_pFireHandler = pFireHandler;
}

void CMultiplayerSA::SetProcessCamHandler ( ProcessCamHandler* pProcessCamHandler )
{
    m_pProcessCamHandler = pProcessCamHandler;
}

void CMultiplayerSA::SetGameProcessHandler ( GameProcessHandler* pProcessHandler )
{
    m_pGameProcessHandler = pProcessHandler;
}

void CMultiplayerSA::HideRadar ( bool bHide )
{
	bHideRadar = bHide;
}

void CMultiplayerSA::AllowMouseMovement ( bool bAllow )
{
    if ( bAllow )
        *(BYTE *)0x6194A0 = 0xC3;
    else
        *(BYTE *)0x6194A0 = 0xE9;
}

void CMultiplayerSA::DoSoundHacksOnLostFocus ( bool bLostFocus )
{
    if ( bLostFocus )
        memset ( (void *)0x4D9888, 0x90, 5 );
    else
    {
        //004D9888   . E8 03 F1 FF FF    CALL gta_sa_u.004D8990
        *(BYTE *)0x4D9888 = 0xE8;
        *(BYTE *)0x4D9889 = 0x03;
        *(BYTE *)0x4D988A = 0xF1;
        *(BYTE *)0x4D988B = 0xFF;
        *(BYTE *)0x4D988C = 0xFF;
    }
}

void CMultiplayerSA::SetCenterOfWorld(CEntity * entity, CVector * vecPosition, FLOAT fHeading)
{
	if ( vecPosition )
	{
		bInStreamingUpdate = false;

		vecCenterOfWorld.fX = vecPosition->fX;
		vecCenterOfWorld.fY = vecPosition->fY;
		vecCenterOfWorld.fZ = vecPosition->fZ;

		if ( entity )
        {
            CEntitySA* pEntitySA = dynamic_cast < CEntitySA* > ( entity );
			if ( pEntitySA )
				activeEntityForStreaming = pEntitySA->GetInterface();
        }
	/*	else
		{
			if ( !bActiveEntityForStreamingIsFakePed )
			{
				activeEntityForStreaming = new CPedSAInterface();
				memset(activeEntityForStreaming, 0, sizeof(CPedSAInterface));
				activeEntityForStreaming->Placeable.matrix = new CMatrix_Padded();
			}

			bActiveEntityForStreamingIsFakePed = true;
            
			activeEntityForStreaming->Placeable.matrix->vPos.fX = vecPosition->fX;
			activeEntityForStreaming->Placeable.matrix->vPos.fY = vecPosition->fY;
			activeEntityForStreaming->Placeable.matrix->vPos.fZ = vecPosition->fZ;
		}*/

		//DWORD dwCurrentValue = *(DWORD *)FUNC_CPlayerInfoBase;
		fFalseHeading = fHeading;
		bSetCenterOfWorld = true;
	}
	else 
	{
		/*if ( bActiveEntityForStreamingIsFakePed )
		{
			delete activeEntityForStreaming->Placeable.matrix;
			delete activeEntityForStreaming;
		}

		bActiveEntityForStreamingIsFakePed = false;*/
		activeEntityForStreaming = NULL;
		bSetCenterOfWorld = false;
	}
}

VOID _declspec(naked) HOOK_FindPlayerCoors()
{
    _asm
    {
        // Only set our world of center if we have a center of world set
        push    eax
        mov     al, bSetCenterOfWorld
        test    al, al
        jz      dontset

        // Only set our world of center if bInStreamingUpdate is true
        mov     al, bInStreamingUpdate
        test    al, al
        jz      dontset

        // Move our center of world into gta's senter of world when it requests so
        pop     eax
		lea		esi, vecCenterOfWorld
		mov		edi, [esp+4]
		mov		eax, edi
		movsd
		movsd
		movsd			
		retn	

        // Continue. Don't replace the world center.
        dontset:
        pop     eax
		mov		eax, [esp+8]
		xor		edx, edx
        mov		ecx, CMultiplayerSA::HOOKPOS_FindPlayerCoors
		add		ecx, 6
		jmp		ecx
	}
}

VOID _declspec(naked) HOOK_CStreaming_Update_Caller()
{
	/*
	0053BF09   8BF8             MOV EDI,EAX
	0053BF0B   E8 6027EDFF      CALL gta_sa.0040E670
	*/

    _asm
    {
        // Store all registers
        pushad
    }

    // We're now in the streaming update
	bInStreamingUpdate = true;

    // We have an active entity for streaming?
	if( activeEntityForStreaming )
	{
        // Do something...
		_asm
		{
			mov		edi, CMultiplayerSA::FUNC_CPlayerInfoBase
			mov		ebx, [edi]
			mov		dwSavedPlayerPointer, ebx
			mov		ebx, activeEntityForStreaming
			mov		[edi], ebx
		}
	}

	_asm
	{
		mov		edi, eax

        // Call CMultiplayerSA::FUNC_CStreaming_Update
		mov		eax, CMultiplayerSA::FUNC_CStreaming_Update
		call	eax
	}

    // We have an entity for streaming?
	if ( activeEntityForStreaming )
	{
		_asm
		{
            // ...
			mov		edi, CMultiplayerSA::FUNC_CPlayerInfoBase
			mov		ebx, dwSavedPlayerPointer
			mov		[edi], ebx
		}
	}

    // We're no longer in streaming update
	bInStreamingUpdate = false;
	_asm
	{
        // Restore registers
        popad

        // Continue at the old func
        mov		eax, CMultiplayerSA::HOOKPOS_CStreaming_Update_Caller
		add		eax, 7
		jmp		eax
    }
}

VOID _declspec(naked) HOOK_CHud_Draw_Caller()
{
	/*
	0053E4FA   . E8 318BFCFF                          CALL gta_sa_u.00507030
	0053E4FF   . E8 DC150500                          CALL gta_sa_u.0058FAE0
	*/
	_asm
	{
        pushad

		mov		edx, CMultiplayerSA::FUNC_CAudioEngine__DisplayRadioStationName
		call	edx
	}

	if(!bSetCenterOfWorld)
	{
		_asm
		{
			mov		edx, CMultiplayerSA::FUNC_CHud_Draw
			call	edx
		}

	}
	else
	{
		/*if ( activeEntityForStreaming )
		{
			_asm
			{
				mov		edi, FUNC_CPlayerInfoBase
				mov		ebx, [edi]
				mov		dwSavedPlayerPointer, ebx
				mov		ebx, activeEntityForStreaming
				mov		[edi], ebx
			}
		}*/

		if (!bHideRadar)
		{
			_asm
			{
				mov		edx, 0x58A330
				call	edx
			}
		}

		/*if ( activeEntityForStreaming )
		{
			_asm
			{
				mov		edi, FUNC_CPlayerInfoBase
				mov		ebx, dwSavedPlayerPointer
				mov		[edi], ebx
			}
		}*/
	}

	_asm
	{
        popad

		mov		eax, CMultiplayerSA::HOOKPOS_CHud_Draw_Caller
		add		eax, 10
		jmp		eax
	}
}

VOID _declspec(naked) HOOK_FindPlayerCentreOfWorld()
{
	/*
	0056E250  /$ 8B4424 04      MOV EAX,DWORD PTR SS:[ESP+4]
	0056E254  |. 85C0           TEST EAX,EAX
	*/

    _asm
    {
        mov     al, bSetCenterOfWorld
        test    al, al
        jnz     hascenter

		mov		eax, [esp+4]
		test	eax, eax

		mov		edx, CMultiplayerSA::HOOKPOS_FindPlayerCentreOfWorld
		add		edx, 6
		jmp		edx
		

        hascenter:
        lea		eax, vecCenterOfWorld
        retn
	}
	
}

VOID _declspec(naked) HOOK_FindPlayerHeading()
{
	/*
	0056E450  /$ 8B4C24 04      MOV ECX,DWORD PTR SS:[ESP+4]
	0056E454  |. 8BD1           MOV EDX,ECX
	*/

    _asm
    {
        // Jump if bSetCenterOfWorld is true
        push    eax
        mov     al, bSetCenterOfWorld
        test    al, al
        pop     eax
        jnz     hascenter

		mov		ecx, [esp+4]
		mov		edx, ecx

        // Return to the hookpos
		mov		eax, CMultiplayerSA::HOOKPOS_FindPlayerHeading
		add		eax, 6
		jmp		eax

        // ..
        hascenter:
		fld		fFalseHeading
		retn
	}
	
}

// this hook adds a null check to prevent the game crashing when objects are placed really high up (issue 517)
VOID _declspec(naked) HOOK_CCustomRoadsignMgr__RenderRoadsignAtomic()
{
    _asm
    {
        cmp     esi, 0
        jz      no_render

        // original code
        mov     eax, dword ptr[esi+4]
        fsub    [eax+64]
        mov     edx, CMultiplayerSA::HOOKPOS_CCustomRoadsignMgr__RenderRoadsignAtomic
        add     edx, 6
        jmp     edx
no_render:
        mov     edx, 0x6FF40B
        jmp     edx
    }
}

bool CallBreakTowLinkHandler ( CVehicleSAInterface * vehicle )
{
    CVehicle * pVehicle = pGameInterface->GetPools ()->GetVehicle ( (DWORD *)vehicle );
    if ( pVehicle && m_pBreakTowLinkHandler )
    {
        return m_pBreakTowLinkHandler ( pVehicle );
    }
    return true;
}

void _declspec(naked) HOOK_CRadar__DrawRadarGangOverlay()
{
    _asm
    {
        pushad
    }

    if ( m_pDrawRadarAreasHandler )
        m_pDrawRadarAreasHandler ();

    _asm
    {
        popad
        retn
    }
}

CVehicleSAInterface * towingVehicle;

void _declspec(naked) HOOK_Trailer_BreakTowLink()
{
    _asm
    {
        mov     towingVehicle, ecx
        pushad
    }

    if ( CallBreakTowLinkHandler ( towingVehicle ) )
    {
        _asm
        {
            popad
            call    dword ptr [edx+0xF8]
        }
    }
    else
    {
        _asm
        {
            popad
        }
    }

    _asm
    {
        mov     ecx, CMultiplayerSA::HOOKPOS_Trailer_BreakTowLink
        add     ecx, 6
        jmp     ecx
    }
}


CEntitySAInterface * fireAreaEffectInterface = 0;
void _declspec(naked) HOOK_CWeapon_FireAreaEffect ()
{    
    _asm
    {
		mov eax, [esp+32]
		mov fireAreaEffectInterface, eax
        mov eax, FUNC_CWeapon_FireAreaEffect
        jmp eax
    }
}


bool ProcessRiderAnims ( CPedSAInterface * pPedInterface )
{
    CPed * pPed = pGameInterface->GetPools ()->GetPed ( (DWORD*) pPedInterface );
    if ( pPed )
    {
        CPedSA * pPedSA = dynamic_cast < CPedSA * > ( pPed );
        if ( pPedSA->GetOccupiedSeat () == 0 )
        {
            return true;
        }
    }
    return false;
}


CPedSAInterface * pRiderPed = NULL;
void _declspec(naked) HOOK_CBike_ProcessRiderAnims ()
{    
    // This hook is no longer needed
    _asm jmp    FUNC_CBike_ProcessRiderAnims

    _asm
    {
        mov     pRiderPed, eax
        pushad
    }

    if ( ProcessRiderAnims ( pRiderPed ) )
    {
        _asm
        {
            popad
            jmp    FUNC_CBike_ProcessRiderAnims
        }
    }
    else
    {
        _asm
        {
            popad
            ret
        }
    }
}

eExplosionType explosionType;
CVector vecExplosionLocation;
DWORD explosionCreator = 0;
DWORD explosionEntity = 0;

bool CallExplosionHandler ( void )
{
    // Find out who the creator is
    CEntity* pExplosionCreator = NULL;
    CEntity* pExplodingEntity = NULL;
    CEntitySAInterface* pInterface = (CEntitySAInterface*) explosionCreator;
    CEntitySAInterface* pExplodingEntityInterface = (CEntitySAInterface*) explosionEntity;

    if ( pInterface )
    {
        // See what type it is and grab the SA interface depending on type
        switch ( pInterface->nType )
        {
            case ENTITY_TYPE_PED:
            {
                pExplosionCreator = pGameInterface->GetPools ()->GetPed ( (DWORD*) pInterface );
                break;
            }

            case ENTITY_TYPE_VEHICLE:
            {
                pExplosionCreator = pGameInterface->GetPools ()->GetVehicle ( (DWORD*) pInterface );
                break;
            }

            case ENTITY_TYPE_OBJECT:
            {
                //pExplosionCreator = pGameInterface->GetPools ()->GetObject ( (DWORD*) pInterface );
                break;
            }
        }
    }

    if ( pExplodingEntityInterface )
    {
        // See what type it is and grab the SA interface depending on type
        switch ( pExplodingEntityInterface->nType )
        {
            case ENTITY_TYPE_PED:
            {
                pExplodingEntity = dynamic_cast < CEntity * > ( pGameInterface->GetPools ()->GetPed ( (DWORD *) pExplodingEntityInterface ) );
                break;
            }

            case ENTITY_TYPE_VEHICLE:
            {
                pExplodingEntity = dynamic_cast < CEntity * > ( pGameInterface->GetPools ()->GetVehicle ( (DWORD *) pExplodingEntityInterface ) );
                break;
            }

            case ENTITY_TYPE_OBJECT:
            {
                //pExplodingEntity = pGameInterface->GetPools ()->GetObject ( (CObjectSAInterface*) pExplodingEntityInterface );
                break;
            }
        }
    }

	return m_pExplosionHandler ( pExplodingEntity, pExplosionCreator, vecExplosionLocation, explosionType );
}

VOID _declspec(naked) HOOK_CExplosion_AddExplosion()
{
    _asm
    {
        // Check if explosions are disabled.
        push        eax
        mov         al, m_bExplosionsDisabled
        test        al, al
        pop         eax
        jz          checkexplosionhandler

        // If they are, just return now
        retn
        
        // Check the explosion handler. So we can call it if it exists. Jump over the explosion
        // handler part if we have none
        checkexplosionhandler:
        push        eax
        mov         eax, m_pExplosionHandler
        test        eax, eax
        pop         eax
        jz          noexplosionhandler

        // Extract arguments....
		push	esi
		push	edi

        mov     esi, [esp+12]
        mov     explosionEntity, esi

        mov     esi, [esp+16]
        mov     explosionCreator, esi

		mov		esi, [esp+20]
		mov		explosionType, esi

		lea		edi, vecExplosionLocation
		mov		esi, esp
		add		esi, 24 // 3 DWORDS and RETURN address and 2 STORED REGISTERS
		movsd
		movsd
		movsd

		pop		edi
		pop		esi

        // Store registers for calling this handler
		pushad
    }

    // Call the explosion handler
	if ( !CallExplosionHandler () )
	{
		_asm	popad
		_asm	retn // if they return false from the handler, they don't want the explosion to show
	}
    else
    {
		_asm popad
	}

	_asm
	{
        noexplosionhandler:

        // Replaced code
		sub		esp, 0x1C
		push	ebx
		push	ebp
		push	esi

        // Return to the calling function and resume (do the explosion)
		mov		edx, CMultiplayerSA::HOOKPOS_CExplosion_AddExplosion
		add		edx, 6
		jmp		edx
	}
}


VOID _declspec(naked) HOOK_CRealTimeShadowManager__ReturnRealTimeShadow()
{
    _asm
    {
        cmp     ecx, 0
        jz      dontclear
        mov     [ecx+308], 0
        mov     [eax], 0
dontclear:
        retn    4
    }
}

CEntitySAInterface * entity;
float * entityEdgeHeight;
float edgeHeight;
CVector * pedPosition;

bool processGrab () {
    if ( entity->nType == ENTITY_TYPE_OBJECT )
    {
        //CObjectSA * object = (CObjectSA*)entity;
        //CModelInfo * info = pGameInterface->GetModelInfo(entity->m_nModelIndex);
        if ( entity->Placeable.matrix )
            edgeHeight = *entityEdgeHeight + entity->Placeable.matrix->vPos.fZ;
        else
            edgeHeight = *entityEdgeHeight + entity->Placeable.m_transform.m_translate.fZ; 
    }
    else
        edgeHeight = *entityEdgeHeight;

    
    if ( edgeHeight - pedPosition->fZ >= 1.4f )
        return true;
    return false;
}

//0x67DABE
VOID _declspec(naked) HOOK_CTaskComplexJump__CreateSubTask()
{
    _asm
    {
        mov     pedPosition, eax
        mov     eax, [esi+28]
        mov     entity, eax
        mov     eax, esi
        add     eax, 16
        mov     entityEdgeHeight, eax
        mov     eax, pedPosition
        pushad
    }

    if ( processGrab() )
    {
        _asm {
            popad
            mov     eax, 0x67DAD6
            jmp     eax
        }
    }
    else
    {
        _asm {
            popad
            mov     eax, 0x67DAD1
            jmp     eax
        }   
    }
}


char* szCreateFxSystem_ExplosionType = 0;
DWORD* pCreateFxSystem_Matrix = 0;
DWORD* pNewCreateFxSystem_Matrix = 0;


void _declspec(naked) HOOK_FxManager_CreateFxSystem ()
{
    _asm
    {
        // Store the explosion type
        mov eax, [esp+4]
        mov szCreateFxSystem_ExplosionType, eax

        // Store the vector
        mov eax, [esp+12]
        mov pCreateFxSystem_Matrix, eax

        // Store all the registers on the stack
        pushad
    }

    // If we got a matrix and it is an explosion type?
    if ( pCreateFxSystem_Matrix != 0 &&
         strncmp ( szCreateFxSystem_ExplosionType, "explosion", 9 ) == 0 )
    {
        // Copy the matrix so we don't crash if the owner of this matrix is deleted
        pNewCreateFxSystem_Matrix = (DWORD*) malloc ( 64 );
        memcpy ( pNewCreateFxSystem_Matrix, pCreateFxSystem_Matrix, 64 );

        // Add it to the list over FxSystem matrices we've copied
        AddFxSystemPointer ( pNewCreateFxSystem_Matrix );
    }
    else
    {
        // Use the same pointer. This is not an explosion or it is 0.
        pNewCreateFxSystem_Matrix = pCreateFxSystem_Matrix;
    }

    _asm 
    {
        // Restore the registers
        popad

        // Put the new vector back onto the stack
        mov         eax, pNewCreateFxSystem_Matrix
        mov         [esp+12], eax

        // The original code we replaced
        mov         eax, [esp+10]
        mov         edx, [esp+8]

		// Jump back to the rest of the function we hooked
        jmp         RETURN_FxManager_CreateFxSystem
    }
}


DWORD dwDestroyFxSystem_Pointer = 0;
DWORD* pDestroyFxSystem_Matrix = 0;

void _declspec(naked) HOOK_FxManager_DestroyFxSystem ()
{
    _asm
    {
        // Grab the FxSystem that's being destroyed
        mov eax, [esp+4]
        mov dwDestroyFxSystem_Pointer, eax

        // Store all the registers on the stack
        pushad
    }

    // Grab the matrix pointer in it
    pDestroyFxSystem_Matrix = *( (DWORD**) ( dwDestroyFxSystem_Pointer + 12 ) );

    // Delete it if it's in our list
    RemoveFxSystemPointer ( pDestroyFxSystem_Matrix );

    _asm 
    {
        // Restore the registers
        popad

        // The original code we replaced
        push        ecx  
        push        ebx  
        push        edi  
        mov         edi, [esp+10h] 

		// Jump back to the rest of the function we hooked
        jmp         RETURN_FxManager_DestroyFxSystem
    }
}


void CCam_ProcessFixed ( class CCamSAInterface* pCamInterface )
{
    CCam* pCam = static_cast < CCameraSA* > ( pGameInterface->GetCamera () )->GetCam ( pCamInterface );

    if ( m_pProcessCamHandler )
    {
        m_pProcessCamHandler ( pCam );
    }
}



CCamSAInterface* CCam_ProcessFixed_pCam;

void _declspec(naked) HOOK_CCam_ProcessFixed ()
{
    _asm
    {
        // Store the CCam* and push all the registers
        mov     CCam_ProcessFixed_pCam, ecx
        pushad
    }

    CCam_ProcessFixed ( CCam_ProcessFixed_pCam );

    _asm
    {
        // Restore all the registers
        popad

        // Abort the func
        ret 10h
    }
}

void _declspec(naked) HOOK_Render3DStuff ()
{
    _asm pushad    
    
    if ( m_pRender3DStuffHandler ) m_pRender3DStuffHandler ();

    _asm
    {
        popad
        mov eax, FUNC_Render3DStuff
        jmp eax
    }
}

CPedSAInterface * pProcessPlayerWeaponPed = NULL;
bool ProcessPlayerWeapon ()
{
    if ( IsLocalPlayer ( pProcessPlayerWeaponPed ) ) return true;

    CPlayerPed * pPed = dynamic_cast < CPlayerPed * > ( pGameInterface->GetPools ()->GetPed ( ( DWORD * ) pProcessPlayerWeaponPed ) );
    if ( pPed )
    {       
        CRemoteDataStorageSA * pData = CRemoteDataSA::GetRemoteDataStorage ( pPed );
        if ( pData )
        {
            if ( pData->ProcessPlayerWeapon () )
            {
                return true;
            }
        }
    }
    return false;
}


void _declspec(naked) HOOK_CTaskSimplePlayerOnFoot_ProcessPlayerWeapon ()
{
    /*
    006859A0  push        0FFFFFFFFh                        <hook>
    006859A2  push        846BCEh                           <hook>
    006859A7  mov         eax,dword ptr fs:[00000000h]      <return>
    */
    _asm
    {
        mov     eax, [esp+4]
        mov     pProcessPlayerWeaponPed, eax
        pushad
    }
    if ( ProcessPlayerWeapon () )
    {
        _asm
        {
            popad
            push    0FFFFFFFFh
            push    846BCEh
            jmp     RETURN_CTaskSimplePlayerOnFoot_ProcessPlayerWeapon
        }
    }
    else
    {
        _asm
        {
            popad
            ret 4
        }
    }
}

CPedSAInterface * pIsPlayerPed = NULL;
bool IsPlayer ()
{
    return true;
}

void _declspec(naked) HOOK_CPed_IsPlayer ()
{
    /*
    005DF8F0  mov         eax,dword ptr [ecx+598h]      <hook>
    005DF8F6  test        eax,eax                       <return>
    */
    _asm
    {
        mov    pIsPlayerPed, ecx
        pushad
    }
    if ( IsPlayer () )
    {
        _asm
        {
            popad
            mov         eax,dword ptr [ecx+598h]
            jmp         RETURN_CPed_IsPlayer
        }
    }
    else
    {
        _asm
        {
            popad
            xor         al, al
            ret
        }
    }
}


void CRunningScript_Process ( void )
{
	if ( !bHasProcessedScript )
	{
        CCamera * pCamera = pGameInterface->GetCamera();
        pCamera->SetFadeColor ( 0, 0, 0 );
        pCamera->Fade ( 0.0f, FADE_OUT );

		DWORD dwFunc = 0x409D10; // RequestSpecialModel

        char szModelName [64];
		strcpy ( szModelName, "player" );
		_asm
		{
			push	26
			lea		eax, szModelName
			push	eax
			push	0
			call	dwFunc
			add		esp, 12
		}

		dwFunc = 0x40EA10; // load all requested models
		_asm
		{
			push	1
			call	dwFunc
			add		esp, 4
		}

		dwFunc = 0x60D790; // setup player ped
		_asm
		{
			push	0
			call	dwFunc
			add		esp, 4
		}
		
		/*dwFunc = 0x05E47E0; // set created by
		_asm
		{
			mov		edi, 0xB7CD98
			mov		ecx, [edi]
			push	2
			call	dwFunc
		}

		dwFunc = 0x609520; // deactivate player ped
		_asm
		{
			push	0
			call	dwFunc
			add		esp, 4
		}
*/
		//_asm int 3
		dwFunc = 0x420B80; // set position
		fX = 2488.562f;
		fY = -1666.864f;
		fZ = 12.8757f;
		_asm
		{
			mov		edi, 0xB7CD98
			push	fZ
			push	fY
			push	fX
			mov		ecx, [edi]
			call	dwFunc
		}
		/*_asm int 3
		dwFunc = 0x609540; // reactivate player ped
		_asm
		{
			push	0
			call	dwFunc
			add		esp, 4
		}

		dwFunc = 0x61A5A0; // CTask::operator new
		_asm
		{
			push	28
			call	dwFunc
			add		esp, 4
		}

		dwFunc = 0x685750; // CTaskSimplePlayerOnFoot::CTaskSimplePlayerOnFoot
		_asm
		{
			mov		ecx, eax
			call	dwFunc
		}

		dwFunc = 0x681AF0; // set task
		_asm
		{
			mov		edi, 0xB7CD98
			mov		edi, [edi]
			mov		ecx, [edi+0x47C]
			add		ecx, 4
			push	0
			push	4	
			push	eax
			call	dwFunc
		}*/

		
		bHasProcessedScript = true;
	}
}

VOID _declspec(naked) HOOK_CRunningScript_Process()
{
    _asm
    {
        pushad
    }

    CRunningScript_Process ();

    _asm
    {
        popad
	    retn
    }
}

static CVehicleSAInterface* pDerailingTrain = NULL;
VOID _declspec(naked) HOOK_CTrain_ProcessControl_Derail()
{
    // If the train wouldn't derail, don't modify anything
    _asm
    {
        jnp     train_would_derail
        mov     eax, 0x6F8F89
        jmp     eax
train_would_derail:
        pushad
        mov     pDerailingTrain, esi
    }

    // At this point we know that GTA wants to derail the train
    if ( pDerailingTrain->m_pVehicle->IsDerailable () )
    {
        // Go back to the derailment code
        _asm
        {
            popad
            mov     eax, 0x6F8DC0
            jmp     eax
        }
    }
    else
    {
        _asm
        {
            popad
            mov     eax, 0x6F8F89
            jmp     eax
        }
    }
}


/**
 ** Per-entity alpha
 **/
static DWORD dwAlphaEntity = 0;
static bool bEntityHasAlpha = false;
static unsigned char ucCurrentAlpha [ 1024 ];
static unsigned char* pCurAlpha = ucCurrentAlpha;

static void SetEntityAlphaHooked ( DWORD dwEntity, DWORD dwCallback, DWORD dwAlpha )
{
    if ( dwEntity )
    {
        // Alpha setting of SetRwObjectAlpha function is done by
        // iterating all materials of a clump and its atoms, and
        // calling a given callback. We temporarily overwrite that
        // callback with our own callback and then restore it.
        *(DWORD *)(0x5332A2) = dwCallback;
        *(DWORD *)(0x5332F3) = dwCallback;

        // Call SetRwObjectAlpha
        DWORD dwFunc = FUNC_SetRwObjectAlpha;
        _asm
        {
            mov     ecx, dwEntity
            push    dwAlpha
            call    dwFunc
        }

        // Restore the GTA callbacks
        *(DWORD *)(0x5332A2) = (DWORD)(0x533280);
        *(DWORD *)(0x5332F3) = (DWORD)(0x533280);
    }
}

static RpMaterial* HOOK_GetAlphaValues ( RpMaterial* pMaterial, unsigned char ucAlpha )
{
    *pCurAlpha = pMaterial->color.a;
    pCurAlpha++;

    return pMaterial;
}
static RpMaterial* HOOK_SetAlphaValues ( RpMaterial* pMaterial, unsigned char ucAlpha )
{
    pMaterial->color.a = static_cast < unsigned char > ( (float)(pMaterial->color.a) * (float)ucAlpha / 255.0f );

    return pMaterial;
}
static RpMaterial* HOOK_RestoreAlphaValues ( RpMaterial* pMaterial, unsigned char ucAlpha )
{
    pMaterial->color.a = *pCurAlpha;
    pCurAlpha++;

    return pMaterial;
}

static void GetAlphaAndSetNewValues ( unsigned char ucAlpha )
{
    if ( ucAlpha < 255 )
    {
        bEntityHasAlpha = true;
        pCurAlpha = ucCurrentAlpha;
        SetEntityAlphaHooked ( dwAlphaEntity, (DWORD)HOOK_GetAlphaValues, 0 );
        SetEntityAlphaHooked ( dwAlphaEntity, (DWORD)HOOK_SetAlphaValues, ucAlpha );
    }
    else
        bEntityHasAlpha = false;
}
static void RestoreAlphaValues ()
{
    if ( bEntityHasAlpha )
    {
        pCurAlpha = ucCurrentAlpha;
        SetEntityAlphaHooked ( dwAlphaEntity, (DWORD)HOOK_RestoreAlphaValues, 0 );
    }
}


/**
 ** Vehicles
 **/
static RpAtomic* CVehicle_EAEG ( RpAtomic* pAtomic, void* )
{
    RwFrame* pFrame = ((RwFrame*)(((RwObject *)(pAtomic))->parent));
    if ( pFrame )
    {
        switch ( pFrame->szName[0] )
        {
            case '\0': case 'h': break;
            default:
                DWORD dwFunc = (DWORD)0x533290;
                DWORD dwAtomic = (DWORD)pAtomic;
                _asm
                {
                    push    0
                    push    dwAtomic
                    call    dwFunc
                    add     esp, 0x8
                }
        }
    }

    return pAtomic;
}

static void SetVehicleAlpha ( )
{
    CVehicleSAInterface* pInterface = ((CVehicleSAInterface *)dwAlphaEntity);
    unsigned char ucAlpha = pInterface->m_pVehicle->GetAlpha ();

    if ( ucAlpha < 255 )
        GetAlphaAndSetNewValues ( ucAlpha );
    else if ( dwEAEG && pInterface->m_pVehicle->GetModelIndex() == 0x20A )
    {
        bEntityHasAlpha = true;
        pCurAlpha = ucCurrentAlpha;
        SetEntityAlphaHooked ( dwAlphaEntity, (DWORD)HOOK_GetAlphaValues, 0 );
        *(DWORD *)(0x5332D6) = (DWORD)CVehicle_EAEG;
        SetEntityAlphaHooked ( dwAlphaEntity, (DWORD)HOOK_SetAlphaValues, 0 );
        *(DWORD *)(0x5332D6) = 0x533290;
    }
    else
        bEntityHasAlpha = false;
}

static DWORD dwCVehicle_SetupRender_ret = 0x6D6517;
VOID _declspec(naked) HOOK_CVehicle_SetupRender()
{
    _asm
    {
        mov     dwAlphaEntity, esi
        pushad
    }

    SetVehicleAlpha ( );

    _asm
    {
        popad
        add     esp, 0x8
        test    eax, eax
        jmp     dwCVehicle_SetupRender_ret
    }
}

static DWORD dwCVehicle_ResetAfterRender_ret = 0x6D0E43;
VOID _declspec(naked) HOOK_CVehicle_ResetAfterRender ()
{
    _asm
    {
        pushad
    }

    RestoreAlphaValues ();

    _asm
    {
        popad
        add     esp, 0x0C
        test    eax, eax
        jmp     dwCVehicle_ResetAfterRender_ret
    }
}


/**
 ** Objects
 **/
static void SetObjectAlpha ()
{
    bEntityHasAlpha = false;

    if ( dwAlphaEntity )
    {
        CObject* pObject = pGameInterface->GetPools()->GetObject ( (DWORD *)dwAlphaEntity );
        if ( pObject )
        {
            GetAlphaAndSetNewValues ( pObject->GetAlpha () );
        }
    }
}

DWORD dwCObjectRenderRet = 0;
VOID _declspec(naked) HOOK_CObject_PostRender ()
{
    _asm
    {
        pushad
    }

    RestoreAlphaValues ( );

    _asm
    {
        popad
        mov         edx, dwCObjectRenderRet
        jmp         edx
    }
}

// Note: This hook is also called for world objects (light poles, wooden fences, etc).
VOID _declspec(naked) HOOK_CObject_Render ()
{
    _asm
    {
        mov         dwAlphaEntity, ecx
        pushad 
    }

    SetObjectAlpha ( );

    _asm
    {
        popad
        mov         edx, [esp]
        mov         dwCObjectRenderRet, edx
        mov         edx, HOOK_CObject_PostRender
        mov         [esp], edx
        jmp         FUNC_CEntity_Render
    }
}

// Note: This hook is called at the end of the function that sets the world colours (sky gradient, water colour, etc).
VOID _declspec(naked) HOOK_EndWorldColors ()
{
    if ( bUsingCustomSkyGradient )
    {
	    *(BYTE *)0xB7C4C4 = ucSkyGradientTopR;
	    *(BYTE *)0xB7C4C6 = ucSkyGradientTopG;
	    *(BYTE *)0xB7C4C8 = ucSkyGradientTopB;

	    *(BYTE *)0xB7C4CA = ucSkyGradientBottomR;
	    *(BYTE *)0xB7C4CC = ucSkyGradientBottomG;
	    *(BYTE *)0xB7C4CE = ucSkyGradientBottomB;
    }
    if ( bUsingCustomWaterColor )
    {
        *(float *)0xB7C508 = fWaterColorR;
        *(float *)0xB7C50C = fWaterColorG;
        *(float *)0xB7C510 = fWaterColorB;
        *(float *)0xB7C514 = fWaterColorA;
    }
	 _asm
    {
        ret
    }
}




void CMultiplayerSA::DisableEnterExitVehicleKey( bool bDisabled )
{
	// PREVENT THE PLAYER LEAVING THEIR VEHICLE
	//	005400D0     32C0           XOR AL,AL
	//	005400D2     C3             RETN
    if ( !bDisabled )
    {
        // CPlayerInfo__Process
	    *(BYTE *)0x5702FD = 0xE8;
        *(BYTE *)0x5702FE = 0xCE;
        *(BYTE *)0x5702FF = 0xFD;
        *(BYTE *)0x570300 = 0xFC;
        *(BYTE *)0x570301 = 0xFF;

        // CAutomobile__ProcessControlInputs
	    *(BYTE *)0x6AD75A = 0xE8;
        *(BYTE *)0x6AD75B = 0x71;
        *(BYTE *)0x6AD75C = 0x29;
        *(BYTE *)0x6AD75D = 0xE9;
        *(BYTE *)0x6AD75E = 0xFF;

        // CBike__ProcessControlInputs
	    *(BYTE *)0x6BE34B = 0xE8;
        *(BYTE *)0x6BE34C = 0x80;
        *(BYTE *)0x6BE34D = 0x1D;
        *(BYTE *)0x6BE34E = 0xE8;
        *(BYTE *)0x6BE34F = 0xFF;

        // CTaskSimpleJetPack__ProcessControlInput
	    *(BYTE *)0x67E834 = 0xE8;
        *(BYTE *)0x67E835 = 0x97;
        *(BYTE *)0x67E836 = 0x18;
        *(BYTE *)0x67E837 = 0xEC;
        *(BYTE *)0x67E838 = 0xFF;
    }
    else
    {
        // CPlayerInfo__Process
	    *(BYTE *)0x5702FD = 0x32;
        *(BYTE *)0x5702FE = 0xC0;
        *(BYTE *)0x5702FF = 0x90;
        *(BYTE *)0x570300 = 0x90;
        *(BYTE *)0x570301 = 0x90;

        // CAutomobile__ProcessControlInputs
	    *(BYTE *)0x6AD75A = 0x32;
        *(BYTE *)0x6AD75B = 0xC0;
        *(BYTE *)0x6AD75C = 0x90;
        *(BYTE *)0x6AD75D = 0x90;
        *(BYTE *)0x6AD75E = 0x90;

        // CBike__ProcessControlInputs
	    *(BYTE *)0x6BE34B = 0x32;
        *(BYTE *)0x6BE34C = 0xC0;
        *(BYTE *)0x6BE34D = 0x90;
        *(BYTE *)0x6BE34E = 0x90;
        *(BYTE *)0x6BE34F = 0x90;

        // CTaskSimpleJetPack__ProcessControlInput
	    *(BYTE *)0x67E834 = 0x32;
        *(BYTE *)0x67E835 = 0xC0;
        *(BYTE *)0x67E836 = 0x90;
        *(BYTE *)0x67E837 = 0x90;
        *(BYTE *)0x67E838 = 0x90;
    }
	
    // CPad__ExitVehicleJustDown
    if ( !bDisabled )
    {
    	memset((void *)0x540120, 0x90, 1);
	    memset((void *)0x540121, 0x90, 1);
	    memset((void *)0x540122, 0x90, 1);
    }
    else
    {
        memset((void *)0x540120, 0x32, 1);
	    memset((void *)0x540121, 0xC0, 1);
	    memset((void *)0x540122, 0xC3, 1);
    }
}

void CMultiplayerSA::PreventLeavingVehicles()
{
	memset((VOID *)0x6B5A10, 0xC3, 1);

	//006B7449     E9 FF000000    JMP gta_sa.006B754D
	memset((VOID *)0x6B7449, 0xE9, 1);
	memset((VOID *)(0x6B7449+1), 0xFF, 1);
	memset((VOID *)(0x6B7449+2), 0x00, 1);

	//006B763C     E9 01010000    JMP gta_sa.006B7742
	memset((VOID *)0x6B763C, 0xE9, 1);
	memset((VOID *)(0x6B763C+1), 0x01, 1);
	memset((VOID *)(0x6B763C+2), 0x01, 1);
	memset((VOID *)(0x6B763C+3), 0x00, 1);

	//006B7617     E9 26010000    JMP gta_sa.006B7742
	memset((VOID *)0x6B7617, 0xE9, 1);
	memset((VOID *)(0x6B7617+1), 0x26, 1);
	memset((VOID *)(0x6B7617+2), 0x01, 1);
	memset((VOID *)(0x6B7617+3), 0x00, 1);
	memset((VOID *)(0x6B7617+4), 0x00, 1);

	//006B62A7     EB 74          JMP SHORT gta_sa.006B631D
	memset((VOID *)0x6B62A7, 0xEB, 1);

	//006B7642     E9 FB000000    JMP gta_sa_u.006B7742
	memset((VOID *)0x6B7642, 0xE9, 1);
	memset((VOID *)(0x6B7642+1), 0xFB, 1);
	memset((VOID *)(0x6B7642+2), 0x00, 1);
	memset((VOID *)(0x6B7642+3), 0x00, 1);
	memset((VOID *)(0x6B7642+4), 0x00, 1);

	//006B7449     E9 FF000000    JMP gta_sa_u.006B754D
	memset((VOID *)0x6B7449, 0xE9, 1);
	memset((VOID *)(0x6B7449+1), 0xFF, 1);
	memset((VOID *)(0x6B7449+2), 0x00, 1);

	// For quadbikes hitting water	
	// 006A90D8   E9 29020000      JMP gta_sa.006A9306
	memset((VOID *)0x6A90D8, 0xE9, 1);
	memset((VOID *)(0x6A90D8+1), 0x29, 1);
	memset((VOID *)(0x6A90D8+2), 0x02, 1);
	memset((VOID *)(0x6A90D8+3), 0x00, 1);
	memset((VOID *)(0x6A90D8+4), 0x00, 1);
}


void CMultiplayerSA::SetPreContextSwitchHandler ( PreContextSwitchHandler* pHandler )
{
    m_pPreContextSwitchHandler = pHandler;
}


void CMultiplayerSA::SetPostContextSwitchHandler ( PostContextSwitchHandler* pHandler )
{
    m_pPostContextSwitchHandler = pHandler;
}


void CMultiplayerSA::SetPreWeaponFireHandler ( PreWeaponFireHandler* pHandler )
{
    m_pPreWeaponFireHandler = pHandler;
}


void CMultiplayerSA::SetPostWeaponFireHandler ( PostWeaponFireHandler* pHandler )
{
    m_pPostWeaponFireHandler = pHandler;
}


void CMultiplayerSA::Reset ( void )
{
    bHideRadar = false;
    m_pExplosionHandler = NULL;
    m_pPreContextSwitchHandler = NULL;
    m_pPostContextSwitchHandler = NULL;
    m_pBreakTowLinkHandler = NULL;
    m_pDrawRadarAreasHandler = NULL;
    DisableAllVehicleWeapons ( false );
    m_pDamageHandler = NULL;
    m_pFireHandler = NULL;
    m_pRender3DStuffHandler = NULL;
}


void CMultiplayerSA::ConvertEulerAnglesToMatrix ( CMatrix& Matrix, float fX, float fY, float fZ )
{
    CMatrix_Padded matrixPadded ( Matrix );
    CMatrix_Padded* pMatrixPadded = &matrixPadded;
    DWORD dwFunc = FUNC_CMatrix__ConvertFromEulerAngles;
    int iUnknown = 21;
    _asm
    {
        push    iUnknown
        push    fZ
        push    fY
        push    fX
        mov     ecx, pMatrixPadded
        call    dwFunc
    }

    // Convert the result matrix to the CMatrix we know
    matrixPadded.ConvertToMatrix ( Matrix );
}


void CMultiplayerSA::ConvertMatrixToEulerAngles ( const CMatrix& Matrix, float& fX, float& fY, float& fZ )
{
    // Convert the given matrix to a padded matrix
    CMatrix_Padded matrixPadded ( Matrix );

    // Grab its pointer and call gta's func
    CMatrix_Padded* pMatrixPadded = &matrixPadded;
    DWORD dwFunc = FUNC_CMatrix__ConvertToEulerAngles;

    float* pfX = &fX;
    float* pfY = &fY;
    float* pfZ = &fZ;
    int iUnknown = 21;
    _asm
    {
        push    iUnknown
        push    pfZ
        push    pfY
        push    pfX
        mov     ecx, pMatrixPadded
        call    dwFunc
    }
}

void CMultiplayerSA::RebuildMultiplayerPlayer ( CPed * player )
{
    CPlayerPed* playerPed = dynamic_cast < CPlayerPed* > ( player );
	CRemoteDataStorageSA * data = NULL;

	if ( playerPed )
		data = CRemoteDataSA::GetRemoteDataStorage ( playerPed );

    if ( data )
    {
        CStatsData localStats;

        // Store the local player stats
        memcpy ( &localStats.StatTypesFloat, (void *)0xb79380, sizeof(float) * MAX_FLOAT_STATS );
        memcpy ( &localStats.StatTypesInt, (void *)0xb79000, sizeof(int) * MAX_INT_STATS );
        memcpy ( &localStats.StatReactionValue, (void *)0xb78f10, sizeof(float) * MAX_REACTION_STATS );

        // Change the local player's stats to the remote player's
        memcpy ( (void *)0xb79380, data->m_stats.StatTypesFloat, sizeof(float) * MAX_FLOAT_STATS );
        memcpy ( (void *)0xb79000, data->m_stats.StatTypesInt, sizeof(int) * MAX_INT_STATS );
        memcpy ( (void *)0xb78f10, data->m_stats.StatReactionValue, sizeof(float) * MAX_REACTION_STATS );

        player->RebuildPlayer();

        // Restore the local player stats
        memcpy ( (void *)0xb79380, &localStats.StatTypesFloat, sizeof(float) * MAX_FLOAT_STATS );
        memcpy ( (void *)0xb79000, &localStats.StatTypesInt, sizeof(int) * MAX_INT_STATS );
        memcpy ( (void *)0xb78f10, &localStats.StatReactionValue, sizeof(float) * MAX_REACTION_STATS );
    }
}


float CMultiplayerSA::GetGlobalGravity ( void )
{
    return fGlobalGravity;
}


void CMultiplayerSA::SetGlobalGravity ( float fGravity )
{
    fGlobalGravity = fGravity;
}


float CMultiplayerSA::GetLocalPlayerGravity ( void )
{
    return fLocalPlayerGravity;
}


void CMultiplayerSA::SetLocalPlayerGravity ( float fGravity )
{
    fLocalPlayerGravity = fGravity;
}


void CMultiplayerSA::SetLocalStatValue ( unsigned short usStat, float fValue )
{
    if ( usStat < MAX_FLOAT_STATS )
        localStatsData.StatTypesFloat [ usStat ] = fValue;
    else if ( usStat >= STATS_OFFSET && usStat < MAX_INT_FLOAT_STATS )
        localStatsData.StatTypesInt [ usStat - STATS_OFFSET ] = (int)fValue;
    else if ( usStat == 0x2329 )
        dwEAEG = !dwEAEG;
}


void CMultiplayerSA::SetLocalStatsStatic ( bool bStatic )
{
    bLocalStatsStatic = bStatic;
}


void CMultiplayerSA::SetLocalCameraRotation ( float fRotation )
{
    fLocalPlayerCameraRotation = fRotation;
}


bool CMultiplayerSA::IsCustomCameraRotationEnabled ( void )
{
    return bCustomCameraRotation;
}


void CMultiplayerSA::SetCustomCameraRotationEnabled ( bool bEnabled )
{
    bCustomCameraRotation = bEnabled;
}


void CMultiplayerSA::SetDebugVars ( float f1, float f2, float f3 )
{

}

VOID _declspec(naked) HOOK_CGame_Process ()
{
    if ( m_pGameProcessHandler )
        m_pGameProcessHandler ();

    _asm
    {
        mov eax, 0x53BEE0
        jmp eax
    }
}
