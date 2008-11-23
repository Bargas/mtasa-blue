/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/Packets.h
*  PURPOSE:     Packet enumeration
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Oliver Brown <>
*               Kent Simon <>
*               Jax <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __PACKETS_H
#define __PACKETS_H

#include <net/net_packetenums.h>

enum
{
    // Connection packets
    PACKET_ID_SERVER_JOINEDGAME = PACKET_ID_END_OF_INTERNAL_PACKETS,
    PACKET_ID_SERVER_DISCONNECTED,

    // All our outgoing only packets use this
    PACKET_ID_RPC,

    // Player packets
    PACKET_ID_PLAYER_LIST,
    PACKET_ID_PLAYER_KEYSYNC,
    PACKET_ID_PLAYER_PURESYNC,
    PACKET_ID_PLAYER_VEHICLE_PURESYNC,
    PACKET_ID_RETURN_SYNC,
    PACKET_ID_PLAYER_SPAWN,
    PACKET_ID_PLAYER_WASTED,
    PACKET_ID_PLAYER_CHANGE_NICK,
    PACKET_ID_PLAYER_PINGS,
    PACKET_ID_PLAYER_STATS,
    PACKET_ID_PLAYER_CLOTHES,

    PACKET_ID_EXPLOSION,
	PACKET_ID_FIRE,
    PACKET_ID_PROJECTILE,
    PACKET_ID_DETONATE_SATCHELS,

    // Console, chat and command packets
    PACKET_ID_COMMAND,
    PACKET_ID_CHAT_ECHO,
    PACKET_ID_CONSOLE_ECHO,
    PACKET_ID_DEBUG_ECHO,

    // Map related packets
    PACKET_ID_MAP_INFO,
    PACKET_ID_MAP_START,
    PACKET_ID_MAP_RESTART,
    PACKET_ID_MAP_STOP,

    // Entity related packets
    PACKET_ID_ENTITY_ADD,
    PACKET_ID_ENTITY_REMOVE,
    PACKET_ID_PICKUP_HIDESHOW,
    PACKET_ID_PICKUP_HIT_CONFIRM,

    // Vehicle related packets
    PACKET_ID_UNOCCUPIED_VEHICLE_STARTSYNC,
    PACKET_ID_UNOCCUPIED_VEHICLE_STOPSYNC,
    PACKET_ID_UNOCCUPIED_VEHICLE_SYNC,
    PACKET_ID_VEHICLE_SPAWN,
    PACKET_ID_VEHICLE_INOUT,
    PACKET_ID_VEHICLE_DAMAGE_SYNC,
    PACKET_ID_VEHICLE_TRAILER,

    // Ped sync
    PACKET_ID_PED_STARTSYNC,
    PACKET_ID_PED_STOPSYNC,
    PACKET_ID_PED_SYNC,

    // Rcon related
    PACKET_ID_PLAYER_RCON,
    PACKET_ID_PLAYER_RCON_LOGIN,
	PACKET_ID_PLAYER_RCON_KICK,
	PACKET_ID_PLAYER_RCON_BAN,
	PACKET_ID_PLAYER_RCON_MUTE,
    PACKET_ID_PLAYER_RCON_FREEZE,

    // Voice
    PACKET_ID_VOICE_DATA,

    // Anticheat memory challenges
    PACKET_ID_CHEAT_CHALLENGEMEMORY,
    PACKET_ID_CHEAT_RETURN,

    // Map related packets
    PACKET_ID_MAP_LIST,
    PACKET_ID_LUA,

    // GUI related packets
    PACKET_ID_TEXT_ITEM,

    // Score table updates
    PACKET_ID_SCORETABLE,

    // Team related packets
    PACKET_ID_TEAMS,

    // Lua related packets
    PACKET_ID_LUA_EVENT,

    // Resource related packets
    PACKET_ID_RESOURCE_START,
    PACKET_ID_RESOURCE_STOP,

    // Custom data related packets
    PACKET_ID_CUSTOM_DATA,

    // Camera related packets
    PACKET_ID_CAMERA_SYNC,
};

#endif
