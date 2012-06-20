/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CWaterManager.h
*  PURPOSE:     Water entity manager class
*  DEVELOPERS:  arc_
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CWaterManager;

#ifndef __CWaterManager_H
#define __CWaterManager_H

class CWaterManager
{
    friend class CWater;

public:
                                    CWaterManager               ();
                                    ~CWaterManager              ();

    CWater*                         Create                      ( CWater::EWaterType waterType, CElement* pParent, CXMLNode* Node = NULL );
    CWater*                         CreateFromXML               ( CElement* pParent, CXMLNode& Node, CLuaMain* pLuaMain, CEvents* pEvents );
    void                            DeleteAll                   ();

    inline unsigned int             Count                       ()          { return static_cast < unsigned int > ( m_List.size () ); };
    bool                            Exists                      ( CWater* pWater );

    float                           GetGlobalWaveHeight         () const            { return m_fGlobalWaveHeight; }
    void                            SetGlobalWaveHeight         ( float fHeight )   { m_fGlobalWaveHeight = fHeight; }

    const SWorldWaterLevelInfo&     GetWorldWaterLevelInfo      () const            { return m_WorldWaterLevelInfo; }
    void                            SetWorldWaterLevel          ( float fLevel, bool bIncludeWorldNonSeaLevel );
    void                            ResetWorldWaterLevel        ( void );
    void                            SetElementWaterLevel        ( CWater* pWater, float fLevel );
    void                            SetAllElementWaterLevel     ( float fLevel );


    std::list < CWater* > ::const_iterator IterBegin            ()          { return m_List.begin (); }
    std::list < CWater* > ::const_iterator IterEnd              ()          { return m_List.end (); }

protected:
    inline void                     AddToList                   ( CWater* pWater )              { m_List.push_back ( pWater ); }
    void                            RemoveFromList              ( CWater* pWater );

    bool                            m_bDontRemoveFromList;
    std::list < CWater* >           m_List;

    SWorldWaterLevelInfo            m_WorldWaterLevelInfo;
    float                           m_fGlobalWaveHeight;
};

#endif
