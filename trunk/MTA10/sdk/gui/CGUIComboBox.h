/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        gui/CGUIComboBox.h
*  PURPOSE:     Combobox widget class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Stanislav Bobrov <lil_Toady@hotmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGUICOMBOBOX_H
#define __CGUICOMBOBOX_H

#include "CGUIElement.h"
#include "CGUICallback.h"

class CGUIComboBox : public CGUIElement
{
public:
    virtual                     ~CGUIComboBox       ( void ) {};

    virtual void                SetReadOnly         ( bool bRead ) = 0;
    virtual CGUIListItem*       AddItem             ( const char* szText ) = 0;
    virtual CGUIListItem*       GetSelectedItem     ( void ) = 0;
    virtual void                Clear               ( void ) = 0;
};

#endif
