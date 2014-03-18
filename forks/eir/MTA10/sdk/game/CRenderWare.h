/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CRenderWare.h
*  PURPOSE:     RenderWare engine interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CRENDERWARE
#define __CRENDERWARE

#include "RenderWare_shared.h"

class CD3DDUMMY;
class CClientEntityBase;
class CShaderItem;
typedef CShaderItem CSHADERDUMMY;
struct RwTexture;
struct RpClump;
struct RwTexDictionary;
class CColModel;
struct RwFrame;

// A list of custom textures to add to a model's txd
struct SReplacementTextures
{
    struct SPerTxd
    {
        std::vector < RwTexture* >  usingTextures;
        ushort                      usTxdId;
        bool                        bTexturesAreCopies;
    };

    std::vector < RwTexture* >      textures;       // List of textures we want to inject into TXD's
    std::vector < SPerTxd >         perTxdList;     // TXD's which have been modified
    std::vector < ushort >          usedInTxdIds;
    std::vector < ushort >          usedInModelIds;
};

// Shader layers to render
struct SShaderItemLayers
{
    SShaderItemLayers ( void ) : pBase ( NULL ), bUsesVertexShader ( false ) {}
    CShaderItem*                 pBase;
    std::vector < CShaderItem* > layerList;
    bool                         bUsesVertexShader;
};

enum EEntityTypeMask
{
    TYPE_MASK_NONE    = 0,
    TYPE_MASK_WORLD   = 1,
    TYPE_MASK_PED     = 2,
    TYPE_MASK_VEHICLE = 4,
    TYPE_MASK_OBJECT  = 8,
    TYPE_MASK_OTHER   = 16,
    TYPE_MASK_ALL     = 127,
};

enum eWorldRenderMode
{
    WORLD_RENDER_ORIGINAL,
    WORLD_RENDER_MESHLOCAL_ALPHAFIX,
    WORLD_RENDER_SCENE_ALPHAFIX
};

enum eShaderLightingMode
{
    SHADER_LIGHTING_SINGULAR,
    SHADER_LIGHTING_MULTI
};

typedef void (*PFN_WATCH_CALLBACK) ( CSHADERDUMMY* pContext, CD3DDUMMY* pD3DDataNew, CD3DDUMMY* pD3DDataOld );

#define MAX_ATOMICS_PER_CLUMP   128

class CRenderWare
{
public:
    virtual void                    EnableEnvMapRendering               ( bool enabled ) = 0;
    virtual bool                    IsEnvMapRenderingEnabled            ( void ) const = 0;

    // Lighting utilities.
    virtual void                    SetGlobalLightingAlwaysEnabled      ( bool enabled ) = 0;
    virtual bool                    IsGlobalLightingAlwaysEnabled       ( void ) const = 0;

    virtual void                    SetLocalLightingAlwaysEnabled       ( bool enabled ) = 0;
    virtual bool                    IsLocalLightingAlwaysEnabled        ( void ) const = 0;

    // Shader lighting management.
    virtual void                    SetShaderLightingMode               ( eShaderLightingMode mode ) = 0;
    virtual eShaderLightingMode     GetShaderLightingMode               ( void ) const = 0;

    // Rendering modes.
    virtual void                    SetWorldRenderMode                  ( eWorldRenderMode mode ) = 0;
    virtual eWorldRenderMode        GetWorldRenderMode                  ( void ) const = 0;

    virtual bool                ModelInfoTXDLoadTextures    ( SReplacementTextures* pReplacementTextures, const SString& szFilename, bool bFilteringEnabled ) = 0;
    virtual bool                ModelInfoTXDAddTextures     ( SReplacementTextures* pReplacementTextures, ushort usModelId ) = 0;
    virtual void                ModelInfoTXDRemoveTextures  ( SReplacementTextures* pReplacementTextures ) = 0;
    virtual void                ClothesAddReplacementTxd    ( char* pFileData, ushort usFileId ) = 0;
    virtual void                ClothesRemoveReplacementTxd ( char* pFileData ) = 0;
    virtual bool                HasClothesReplacementChanged( void ) = 0;
    virtual RwTexDictionary *   ReadTXD                     ( const char *szTXD ) = 0;
    virtual RpClump *           ReadDFF                     ( const char *szDFF, unsigned short usModelID, bool bLoadEmbeddedCollisions, CColModel*& colOut ) = 0;
    virtual CColModel *         ReadCOL                     ( const char * szCOLFile ) = 0;
    virtual void                DestroyDFF                  ( RpClump * pClump ) = 0;
    virtual void                DestroyTXD                  ( RwTexDictionary * pTXD ) = 0;
    virtual void                DestroyTexture              ( RwTexture * pTex ) = 0;
    virtual void                ReplaceCollisions           ( CColModel * pColModel, unsigned short usModelID ) = 0;
    virtual bool                PositionFrontSeat           ( RpClump *pClump, unsigned short usModelID ) = 0;
    virtual void                ReplaceAllAtomicsInModel    ( RpClump * pSrc, unsigned short usModelID ) = 0;
    virtual void                RepositionAtomic            ( RpClump * pDst, RpClump * pSrc, const char * szName ) = 0;
    virtual void                AddAllAtomics               ( RpClump * pDst, RpClump * pSrc ) = 0;
    virtual void                ReplaceModel                ( RpClump * pNew, unsigned short usModelID ) = 0;
    virtual void                PulseWorldTextureWatch      ( void ) = 0;
    virtual void                GetModelTextureNames        ( std::vector < SString >& outNameList, ushort usModelID ) = 0;
    virtual const SString&      GetTextureName              ( CD3DDUMMY* pD3DData ) = 0;

    virtual void                SetRenderingClientEntity    ( CClientEntityBase* pClientEntity, ushort usModelId, int iTypeMask ) = 0;
    virtual SShaderItemLayers*  GetAppliedShaderForD3DData  ( CD3DDUMMY* pD3DData ) = 0;
    virtual void                AppendAdditiveMatch         ( CSHADERDUMMY* pShaderData, CClientEntityBase* pClientEntity, const char* strTextureNameMatch, float fShaderPriority, bool bShaderLayered, int iTypeMask, uint uiShaderCreateTime, bool bShaderUsesVertexShader, bool bAppendLayers ) = 0;
    virtual void                AppendSubtractiveMatch      ( CSHADERDUMMY* pShaderData, CClientEntityBase* pClientEntity, const char* strTextureNameMatch ) = 0;
    virtual void                RemoveClientEntityRefs      ( CClientEntityBase* pClientEntity ) = 0;
    virtual void                RemoveShaderRefs            ( CSHADERDUMMY* pShaderItem ) = 0;
    virtual RwFrame *           GetFrameFromName            ( RpClump * pRoot, SString strName ) = 0;
};


#endif
