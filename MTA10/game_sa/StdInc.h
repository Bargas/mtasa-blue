#pragma message("Compiling precompiled header.\n")

// Pragmas
#pragma warning (disable:4409)
#pragma warning (disable:4250)

// To make the code know we compile for MTA:BLUE Eir fork.
#define _MTA_BLUE

#define NOMINMAX
#include <windows.h>
#define MTA_CLIENT
#define SHARED_UTIL_WITH_HASH_MAP
#define SHARED_UTIL_WITH_FAST_HASH_MAP
#include "SharedUtil.h"
#include "SharedUtil.MemAccess.h"
#include <stdio.h>

// Screw the windows header.
#undef GetObject

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

// Define these if you are experiencing heap trouble.
//#define USE_HEAP_DEBUGGING
//#define USE_FULL_PAGE_HEAP
//#define PAGE_HEAP_INTEGRITY_CHECK
//#define PAGE_HEAP_MEMORY_STATS
#include <debugsdk/dbgheap.h>

// SDK includes
#include <core/CCoreInterface.h>
#include <net/CNet.h>
#include <game/CGame.h>
#include <game/CWanted.h>
#include <../version.h>
#include <ijsify.h>

// Game includes
#include "RenderWare.h"
#include "CFileUtilsSA.h"
#include "CQuadTreeSA.h"
#include "CExecutiveManagerSA.h"
#include "CRecordingsSA.h"
#include "CTransformationSA.h"
#include "CModelInfoSA.h"
#include "CEntitySA.h"
#include "gamesa_init.h"
#include "Common.h"
#include "CWorldSA.h"
#include "CIMGManagerSA.h"
#include "CPoolsSA.h"
#include "CRenderWareSA.h"
#include "CModelManagerSA.h"
#include "CTextureManagerSA.h"
#include "CClockSA.h"
#include "CFontSA.h"
#include "CRadarSA.h"
#include "CMenuManagerSA.h"
#include "CCameraSA.h"
#include "CCheckpointsSA.h"
#include "CCoronasSA.h"
#include "CPickupsSA.h"
#include "CPathFindSA.h"
#include "CWeaponInfoSA.h"
#include "CExplosionManagerSA.h"
#include "CFireManagerSA.h"
#include "CHandlingManagerSA.h"
#include "CHudSA.h"
#include "C3DMarkersSA.h"
#include "CStatsSA.h"
#include "CTheCarGeneratorsSA.h"
#include "CPadSA.h"
#include "CAERadioTrackManagerSA.h"
#include "CWeatherSA.h"
#include "CTextSA.h"
#include "CPedSA.h"
#include "CPedSoundSA.h"
#include "CAudioEngineSA.h"
#include "CAudioContainerSA.h"
#include "CPlayerInfoSA.h"
#include "CPopulationSA.h"
#include "CSettingsSA.h"
#include "CCarEnterExitSA.h"
#include "COffsets.h"
#include "CControllerConfigManagerSA.h"
#include "CProjectileInfoSA.h"
#include "CEventListSA.h"
#include "CGaragesSA.h"
#include "CTasksSA.h"
#include "CEventDamageSA.h"
#include "CEventGunShotSA.h"
#include "CAnimManagerSA.h"
#include "CVisibilityPluginsSA.h"
#include "CKeyGenSA.h"
#include "CRopesSA.h"
#include "CFxSA.h"
#include "CFxSystemBPSA.h"
#include "CFxSystemSA.h"
#include "CFxManagerSA.h"
#include "HookSystem.h"
#include "CAtomicModelInfoSA.h"
#include "CClumpModelInfoSA.h"
#include "CPedModelInfoSA.h"
#include "CVehicleModelInfoSA.h"
#include "CColPointSA.h"
#include "CCivilianPedSA.h"
#include "CAnimBlendAssociationSA.h"
#include "CAnimBlendAssocGroupSA.h"
#include "CAnimBlendHierarchySA.h"
#include "CAnimBlendSequenceSA.h"
#include "CAnimBlendStaticAssociationSA.h"
#include "CAnimBlockSA.h"
#include "CAutomobileSA.h"
#include "CBikeSA.h"
#include "CBoatSA.h"
#include "CBmxSA.h"
#include "CQuadBikeSA.h"
#include "CMonsterTruckSA.h"
#include "CPlaneSA.h"
#include "CTrailerSA.h"
#include "CPlayerPedSA.h"
#include "CCivilianPedSA.h"
#include "CObjectSA.h"
#include "CBuildingSA.h"
#include "CRestartSA.h"
#include "CWaterManagerSA.h"
#include "CPedDamageResponseSA.h"
#include "CPedDamageResponseCalculatorSA.h"
#include "CPointLightsSA.h"
#include "CWeaponStatSA.h"
#include "CWeaponStatManagerSA.h"
#include "CShadowDataSA.h"
#include "CBuoyancySA.h"
#include "CCacheSA.h"
#include "CStreamingSA.h"
#include "CStreamerSA.h"
#include "CGameSA.h"