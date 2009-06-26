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

// These includes have to be fixed!
#include "..\game_sa\CCameraSA.h"
#include "..\game_sa\CEntitySA.h"
#include "..\game_sa\CPedSA.h"
#include "..\game_sa\common.h"

using namespace std;

extern CGame * pGameInterface;

unsigned long CMultiplayerSA::HOOKPOS_FindPlayerCoors;
unsigned long CMultiplayerSA::HOOKPOS_FindPlayerCentreOfWorld;
unsigned long CMultiplayerSA::HOOKPOS_FindPlayerHeading;
unsigned long CMultiplayerSA::HOOKPOS_CStreaming_Update_Caller;
unsigned long CMultiplayerSA::HOOKPOS_CHud_Draw_Caller;
unsigned long CMultiplayerSA::HOOKPOS_CRealTimeShadowManager__ReturnRealTimeShadow;
unsigned long CMultiplayerSA::HOOKPOS_CCustomRoadsignMgr__RenderRoadsignAtomic;
unsigned long CMultiplayerSA::HOOKPOS_Trailer_BreakTowLink;
unsigned long CMultiplayerSA::HOOKPOS_CRadar__DrawRadarGangOverlay;
unsigned long CMultiplayerSA::HOOKPOS_CTaskComplexJump__CreateSubTask;
unsigned long CMultiplayerSA::HOOKPOS_CTrain_ProcessControl_Derail;
unsigned long CMultiplayerSA::HOOKPOS_EndWorldColors;
unsigned long CMultiplayerSA::HOOKPOS_CWorld_ProcessVerticalLineSectorList;
unsigned long CMultiplayerSA::HOOKPOS_ComputeDamageResponse_StartChoking;

unsigned long CMultiplayerSA::FUNC_CStreaming_Update;
unsigned long CMultiplayerSA::FUNC_CAudioEngine__DisplayRadioStationName;
unsigned long CMultiplayerSA::FUNC_CHud_Draw;

unsigned long CMultiplayerSA::ADDR_CursorHiding;
unsigned long CMultiplayerSA::ADDR_GotFocus;

unsigned long CMultiplayerSA::FUNC_CPlayerInfoBase;


#define HOOKPOS_CCam_ProcessFixed                           0x51D470
#define HOOKPOS_CTaskSimplePlayerOnFoot_ProcessPlayerWeapon 0x6859a0
#define HOOKPOS_CPed_IsPlayer                               0x5DF8F0

DWORD RETURN_CCam_ProcessFixed =                            0x51D475;
DWORD RETURN_CTaskSimplePlayerOnFoot_ProcessPlayerWeapon =  0x6859A7;
DWORD RETURN_CPed_IsPlayer =                                0x5DF8F6;

#define VAR_CollisionStreamRead_ModelInfo                   0x9689E0
#define HOOKPOS_CollisionStreamRead                         0x41B1D0
DWORD RETURN_CollisionStreamRead =                          0x41B1D6;

#define CALL_Render3DStuff                                  0x53EABF
#define FUNC_Render3DStuff                                  0x53DF40

#define CALL_CRenderer_Render                               0x53EA12
#define FUNC_CRenderer_Render                               0x727140

#define CALL_CBike_ProcessRiderAnims                        0x6BF425   // @ CBike::ProcessDrivingAnims

#define HOOKPOS_CGame_Process                               0x53C095
DWORD RETURN_CGame_Process =                                0x53C09F;

#define HOOKPOS_Idle                                        0x53E981
DWORD RETURN_Idle =                                         0x53E98B;

DWORD FUNC_CBike_ProcessRiderAnims =                        0x6B7280;

CPed* pContextSwitchedPed = 0;
CVector vecCenterOfWorld;
FLOAT fFalseHeading;
bool bSetCenterOfWorld;
DWORD dwVectorPointer;
bool bInStreamingUpdate;
bool bHideRadar;
DWORD RoadSignFixTemp;
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
FireHandler* m_pFireHandler = NULL;
ProjectileHandler* m_pProjectileHandler = NULL;
ProjectileStopHandler* m_pProjectileStopHandler = NULL;
ProcessCamHandler* m_pProcessCamHandler = NULL;
ChokingHandler* m_pChokingHandler = NULL;
PostWorldProcessHandler * m_pPostWorldProcessHandler = NULL;
IdleHandler * m_pIdleHandler = NULL;

BreakTowLinkHandler * m_pBreakTowLinkHandler = NULL;
DrawRadarAreasHandler * m_pDrawRadarAreasHandler = NULL;
Render3DStuffHandler * m_pRender3DStuffHandler = NULL;
extern DamageHandler* m_pDamageHandler;
extern FireDamageHandler* m_pFireDamageHandler;

CEntitySAInterface * dwSavedPlayerPointer = 0;
CEntitySAInterface * activeEntityForStreaming = 0; // the entity that the streaming system considers active

void HOOK_FindPlayerCoors();
void HOOK_FindPlayerCentreOfWorld();
void HOOK_FindPlayerHeading();
void HOOK_CStreaming_Update_Caller();
void HOOK_CHud_Draw_Caller();
void HOOK_CRealTimeShadowManager__ReturnRealTimeShadow();
void HOOK_CCustomRoadsignMgr__RenderRoadsignAtomic();
void HOOK_Trailer_BreakTowLink();
void HOOK_CRadar__DrawRadarGangOverlay();
void HOOK_CTaskComplexJump__CreateSubTask();
void HOOK_CBike_ProcessRiderAnims();
void HOOK_CCam_ProcessFixed ();
void HOOK_Render3DStuff ();
void HOOK_CTaskSimplePlayerOnFoot_ProcessPlayerWeapon ();
void HOOK_CPed_IsPlayer ();
void HOOK_CTrain_ProcessControl_Derail ();
void HOOK_EndWorldColors ();
void HOOK_CWorld_ProcessVerticalLineSectorList ();
void HOOK_ComputeDamageResponse_StartChoking ();
void HOOK_CollisionStreamRead ();
void HOOK_CGame_Process ();
void HOOK_Idle ();

void vehicle_lights_init ();
void vehicle_gravity_init ();
void entity_alpha_init ();
void fx_manager_init ();
void explosion_init ();
void running_script_init ();
void damage_events_init ();
void animation_init ();

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
    
    m_pBreakTowLinkHandler = NULL;
    m_pDrawRadarAreasHandler = NULL;
    m_pDamageHandler = NULL;
    m_pFireDamageHandler = NULL;
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
    vehicle_lights_init ();
    vehicle_gravity_init ();
    entity_alpha_init ();
    fx_manager_init ();
    explosion_init ();
    running_script_init ();
    damage_events_init ();
    animation_init ();

    eGameVersion version = pGameInterface->GetGameVersion ();

	bSetCenterOfWorld = false;	

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
    HookInstall(HOOKPOS_CRealTimeShadowManager__ReturnRealTimeShadow, (DWORD)HOOK_CRealTimeShadowManager__ReturnRealTimeShadow, 6);
	HookInstall(HOOKPOS_CCustomRoadsignMgr__RenderRoadsignAtomic, (DWORD)HOOK_CCustomRoadsignMgr__RenderRoadsignAtomic, 6);
    HookInstall(HOOKPOS_Trailer_BreakTowLink, (DWORD)HOOK_Trailer_BreakTowLink, 6);
    HookInstall(HOOKPOS_CRadar__DrawRadarGangOverlay, (DWORD)HOOK_CRadar__DrawRadarGangOverlay, 6);
    HookInstall(HOOKPOS_CTaskComplexJump__CreateSubTask, (DWORD)HOOK_CTaskComplexJump__CreateSubTask, 6);
    HookInstall(HOOKPOS_CCam_ProcessFixed, (DWORD)HOOK_CCam_ProcessFixed, 5);
    HookInstall(HOOKPOS_CTaskSimplePlayerOnFoot_ProcessPlayerWeapon, (DWORD)HOOK_CTaskSimplePlayerOnFoot_ProcessPlayerWeapon, 7);
    HookInstall(HOOKPOS_CPed_IsPlayer, (DWORD)HOOK_CPed_IsPlayer, 6);
    HookInstall(HOOKPOS_CTrain_ProcessControl_Derail, (DWORD)HOOK_CTrain_ProcessControl_Derail, 6);
    HookInstall(HOOKPOS_EndWorldColors, (DWORD)HOOK_EndWorldColors, 5);
    HookInstall(HOOKPOS_CWorld_ProcessVerticalLineSectorList, (DWORD)HOOK_CWorld_ProcessVerticalLineSectorList, 8);
    HookInstall(HOOKPOS_ComputeDamageResponse_StartChoking, (DWORD)HOOK_ComputeDamageResponse_StartChoking, 7);
    HookInstall(HOOKPOS_CollisionStreamRead, (DWORD)HOOK_CollisionStreamRead, 6);    
    HookInstall(HOOKPOS_CGame_Process, (DWORD)HOOK_CGame_Process, 10 );
    HookInstall(HOOKPOS_Idle, (DWORD)HOOK_Idle, 10 );

    HookInstallCall ( CALL_CBike_ProcessRiderAnims, (DWORD)HOOK_CBike_ProcessRiderAnims );
    HookInstallCall ( CALL_Render3DStuff, (DWORD)HOOK_Render3DStuff );

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
    // Nop the whole CTagManager::IsTag function and replace its body with:
    // xor eax, eax
    // ret
    // to make it always return false
    memset ( (void *)0x49CCE0, 0x90, 74 );
    *(DWORD *)(0x49CCE0) = 0x90C3C033;
    // Remove also some hardcoded and inlined checks for if it's a tag
    memset ( (void *)0x53374A, 0x90, 56 );
    *(BYTE *)(0x4C4403) = 0xEB;        

    // Fix for sliding over objects and vehicles (ice floor)
    *(BYTE *)0x5E1E72 = 0xE9;
    *(BYTE *)0x5E1E73 = 0xB9;
    *(BYTE *)0x5E1E74 = 0x00;
    *(BYTE *)0x5E1E77 = 0x90;

    // Avoid GTA setting vehicle first color to white after changing the paintjob
    memset ( (void *)0x6D65C5, 0x90, 11 );

    // Disable idle cam
    *(BYTE *)0x522C80 = 0xC3;

    // Disable radar map hiding when pressing TAB (action key) while on foot
    memset ( (void *)0x58FC3E, 0x90, 14 );

    // No intro movies kthx
    if ( version == VERSION_US_10 )
    {
        *(DWORD *)0x748EF8 = 0x748AE7;
        *(DWORD *)0x748EFC = 0x748B08;
        *(BYTE *)0x748B0E = 5;
    }
    else if ( version == VERSION_EU_10 )
    {
        *(DWORD *)0x748F48 = 0x748B37;
        *(DWORD *)0x748F4C = 0x748B58;
        *(BYTE *)0x748B5E = 5;
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

void CMultiplayerSA::SetCloudsEnabled ( bool bDisabled )
{
    //volumetric clouds
	if ( bDisabled )
        *(BYTE *)0x716380 = 0xA1;
	else
		*(BYTE *)0x716380 = 0xC3;

    // normal clouds
    //0071395A     90             NOP
    if ( bDisabled )
        *(BYTE *)0x713950 = 0x83;
    else
        *(BYTE *)0x713950 = 0xC3;

    // plane trails (not really clouds, but they're sort of vapour)

    if ( bDisabled )
    {
		*(BYTE *)0x717180 = 0x83;
        *(BYTE *)0x717181 = 0xEC;
        *(BYTE *)0x717182 = 0x08;
    }
	else
    {
		*(BYTE *)0x717180 = 0xC2;
        *(BYTE *)0x717181 = 0x04;
        *(BYTE *)0x717182 = 0x00;
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

extern bool m_bExplosionsDisabled;
bool CMultiplayerSA::GetExplosionsDisabled ( void )
{
    return m_bExplosionsDisabled;
}


void CMultiplayerSA::DisableExplosions ( bool bDisabled )
{
    m_bExplosionsDisabled = bDisabled;
}


extern ExplosionHandler * m_pExplosionHandler;
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

void CMultiplayerSA::SetFireDamageHandler ( FireDamageHandler * pFireDamageHandler )
{
    m_pFireDamageHandler = pFireDamageHandler;
}

void CMultiplayerSA::SetFireHandler ( FireHandler * pFireHandler )
{
    m_pFireHandler = pFireHandler;
}

void CMultiplayerSA::SetProcessCamHandler ( ProcessCamHandler* pProcessCamHandler )
{
    m_pProcessCamHandler = pProcessCamHandler;
}

void CMultiplayerSA::SetChokingHandler ( ChokingHandler* pChokingHandler )
{
    m_pChokingHandler = pChokingHandler;
}

extern AddAnimationHandler * m_pAddAnimationHandler;
void CMultiplayerSA::SetAddAnimationHandler ( AddAnimationHandler * pHandler )
{
    m_pAddAnimationHandler = pHandler;
}

extern BlendAnimationHandler * m_pBlendAnimationHandler;
void CMultiplayerSA::SetBlendAnimationHandler ( BlendAnimationHandler * pHandler )
{
    m_pBlendAnimationHandler = pHandler;
}

void CMultiplayerSA::SetPostWorldProcessHandler ( PostWorldProcessHandler * pHandler )
{
    m_pPostWorldProcessHandler = pHandler;
}

void CMultiplayerSA::SetIdleHandler ( IdleHandler * pHandler )
{
    m_pIdleHandler = pHandler;
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

void _declspec(naked) HOOK_FindPlayerCoors()
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

void _declspec(naked) HOOK_CStreaming_Update_Caller()
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

void _declspec(naked) HOOK_CHud_Draw_Caller()
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

void _declspec(naked) HOOK_FindPlayerCentreOfWorld()
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

void _declspec(naked) HOOK_FindPlayerHeading()
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
void _declspec(naked) HOOK_CCustomRoadsignMgr__RenderRoadsignAtomic()
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


void _declspec(naked) HOOK_CRealTimeShadowManager__ReturnRealTimeShadow()
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
void _declspec(naked) HOOK_CTaskComplexJump__CreateSubTask()
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


bool CCam_ProcessFixed ( class CCamSAInterface* pCamInterface )
{
    CCam* pCam = static_cast < CCameraSA* > ( pGameInterface->GetCamera () )->GetCam ( pCamInterface );

    if ( m_pProcessCamHandler )
    {
        return m_pProcessCamHandler ( pCam );
    }
    return false;
}



CCamSAInterface* CCam_ProcessFixed_pCam;

void _declspec(naked) HOOK_CCam_ProcessFixed ()
{
    _asm
    {
        mov CCam_ProcessFixed_pCam, ecx
    }

    if ( CCam_ProcessFixed ( CCam_ProcessFixed_pCam ) )
    {
        _asm
        {
            ret 10h
        }
    }
    else
    {
        _asm
        {
            mov ecx, CCam_ProcessFixed_pCam
            sub esp, 28h
            push ebx
            push ebp
            jmp RETURN_CCam_ProcessFixed
        }
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

static CVehicleSAInterface* pDerailingTrain = NULL;
void _declspec(naked) HOOK_CTrain_ProcessControl_Derail()
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


// Note: This hook is called at the end of the function that sets the world colours (sky gradient, water colour, etc).
void _declspec(naked) HOOK_EndWorldColors ()
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


// This hook modifies the code in CWorld::ProcessVerticalLineSectorList to
// force it to also check the world objects, so we can get a reliable ground
// position on custom object maps. This will make getGroundPosition, jetpacks
// and molotovs to work.
static DWORD dwObjectsChecked = 0;
static DWORD dwProcessVerticalKeepLooping = 0x5632D1;
static DWORD dwProcessVerticalEndLooping = 0x56335F;
static DWORD dwGlobalListOfObjects = 0xB9ACCC;
void _declspec(naked) HOOK_CWorld_ProcessVerticalLineSectorList ( )
{
    _asm
    {
        test    ebp, ebp
        jz      end_of_entities_list
        jmp     dwProcessVerticalKeepLooping

end_of_entities_list:
        mov     eax, dwObjectsChecked
        test    eax, eax
        jnz     stop_looping
        mov     dwObjectsChecked, 1
        mov     ebp, dwGlobalListOfObjects
        mov     ebp, [ebp]
        test    ebp, ebp
        jz      stop_looping
        jmp     dwProcessVerticalKeepLooping

stop_looping:
        mov     dwObjectsChecked, 0
        jmp     dwProcessVerticalEndLooping
    }
}



// Hook to detect when a player is choking
DWORD dwChokingChoke = 0x4C05C1;
DWORD dwChokingDontchoke = 0x4C0620;
unsigned char ucChokingWeaponType = 0;
CPedSAInterface * pChokingPedInterface = NULL;
CPlayerPed * pChokingPed = NULL;
CPedSAInterface * pResponsiblePedInterface = NULL;
CPlayerPed * pResponsiblePed = NULL;
void _declspec(naked) HOOK_ComputeDamageResponse_StartChoking ()
{
    _asm
    {
        pushad
        mov     al, [esp+0x8C]
        mov     ucChokingWeaponType, al
        mov     eax, dword ptr [edi]
        mov     pChokingPedInterface, eax
        mov     pResponsiblePedInterface, ebp
    }

    pChokingPed = dynamic_cast < CPlayerPed * > ( pGameInterface->GetPools ()->GetPed ( ( DWORD * ) pChokingPedInterface ) );
    pResponsiblePed = dynamic_cast < CPlayerPed * > ( pGameInterface->GetPools ()->GetPed ( ( DWORD * ) pResponsiblePedInterface ) );

    if ( pChokingPed )
    {
        if ( m_pChokingHandler )
        {
            if ( !m_pChokingHandler ( pChokingPed, pResponsiblePed, ucChokingWeaponType ) ) goto dont_choke;
        }
    }

    _asm
    {
        popad
        mov     ecx, [edi]
        mov     eax, [ecx+0x47C]
        jmp     dwChokingChoke
dont_choke:
        popad
        jmp     dwChokingDontchoke
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
	memset((void *)0x6B5A10, 0xC3, 1);

	//006B7449     E9 FF000000    JMP gta_sa.006B754D
	memset((void *)0x6B7449, 0xE9, 1);
	memset((void *)(0x6B7449+1), 0xFF, 1);
	memset((void *)(0x6B7449+2), 0x00, 1);

	//006B763C     E9 01010000    JMP gta_sa.006B7742
	memset((void *)0x6B763C, 0xE9, 1);
	memset((void *)(0x6B763C+1), 0x01, 1);
	memset((void *)(0x6B763C+2), 0x01, 1);
	memset((void *)(0x6B763C+3), 0x00, 1);

	//006B7617     E9 26010000    JMP gta_sa.006B7742
	memset((void *)0x6B7617, 0xE9, 1);
	memset((void *)(0x6B7617+1), 0x26, 1);
	memset((void *)(0x6B7617+2), 0x01, 1);
	memset((void *)(0x6B7617+3), 0x00, 1);
	memset((void *)(0x6B7617+4), 0x00, 1);

	//006B62A7     EB 74          JMP SHORT gta_sa.006B631D
	memset((void *)0x6B62A7, 0xEB, 1);

	//006B7642     E9 FB000000    JMP gta_sa_u.006B7742
	memset((void *)0x6B7642, 0xE9, 1);
	memset((void *)(0x6B7642+1), 0xFB, 1);
	memset((void *)(0x6B7642+2), 0x00, 1);
	memset((void *)(0x6B7642+3), 0x00, 1);
	memset((void *)(0x6B7642+4), 0x00, 1);

	//006B7449     E9 FF000000    JMP gta_sa_u.006B754D
	memset((void *)0x6B7449, 0xE9, 1);
	memset((void *)(0x6B7449+1), 0xFF, 1);
	memset((void *)(0x6B7449+2), 0x00, 1);

	// For quadbikes hitting water	
	// 006A90D8   E9 29020000      JMP gta_sa.006A9306
	memset((void *)0x6A90D8, 0xE9, 1);
	memset((void *)(0x6A90D8+1), 0x29, 1);
	memset((void *)(0x6A90D8+2), 0x02, 1);
	memset((void *)(0x6A90D8+3), 0x00, 1);
	memset((void *)(0x6A90D8+4), 0x00, 1);
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

extern DWORD dwEAEG;
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

void _declspec(naked) HOOK_CollisionStreamRead ()
{
    if ( *(DWORD *)VAR_CollisionStreamRead_ModelInfo )
    {
        _asm
        {
            mov eax, dword ptr fs:[0]
            jmp RETURN_CollisionStreamRead
        }
    }
    else
    {
        _asm
        {
            ret
        }
    }
}

DWORD dwCWorld_Process = 0x5684a0;
void _declspec(naked) HOOK_CGame_Process ()
{
    _asm
    {
        call    dwCWorld_Process
        mov     ecx, 0B72978h
        pushad
    }

    if ( m_pPostWorldProcessHandler ) m_pPostWorldProcessHandler ();

    _asm
    {
        popad
        jmp     RETURN_CGame_Process;
    }
}


DWORD dwCGame_Process = 0x53BEE0;
void _declspec(naked) HOOK_Idle ()
{
    _asm
    {
        call    dwCGame_Process
        pushad
    }

    if ( m_pIdleHandler ) m_pIdleHandler ();

    _asm
    {
        popad
        mov     ecx,0B6BC90h
        jmp     RETURN_Idle;
    }
}


// Allowing a created object into the vertical line test makes getGroundPosition, jetpacks and molotovs to work.
// Not allowing a created object into the vertical line test makes the breakable animation work.
void CMultiplayerSA::AllowCreatedObjectsInVerticalLineTest ( bool bOn )
{
    static BYTE bufOriginalData[90] = {0};
    static bool bState = false;

    // Change required?
    if ( bState != bOn )
    {
        // Done initialization?
        if ( bufOriginalData[0] == 0 )
            memcpy ( bufOriginalData, (void *)0x59FABC, 90 );

        bState = bOn;
        if ( bOn )
        {
            // Make created objects to have a control code, so they can be checked for vertical line test HOOK
            memset ( (void *)0x59FABC, 0x90, 90 );
        }
        else
        {
            // Make created objects not be checked for vertical line test HOOK
            memcpy ( (void *)0x59FABC, bufOriginalData, 90 );
        }
    }
}