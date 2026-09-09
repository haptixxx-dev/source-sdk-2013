//========= Cascade (hx) — game rules ==========================================
//
// CHxGameRules is the single override point for everything Cascade changes
// about Half-Life 2 (constraint C5: disable through rules and ConVars, never by
// deleting upstream code). It derives from CHalfLife2 so every HL2 rule still
// applies unless overridden here.
//
//=============================================================================
#ifndef HX_GAMERULES_H
#define HX_GAMERULES_H
#ifdef _WIN32
#pragma once
#endif

#include "hl2_gamerules.h"
#include "hx_shareddefs.h"

#ifdef CLIENT_DLL
	#define CHxGameRules C_HxGameRules
	#define CHxGameRulesProxy C_HxGameRulesProxy
#endif

class CHxGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CHxGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};

class CHxGameRules : public CHalfLife2
{
public:
	DECLARE_CLASS( CHxGameRules, CHalfLife2 );

#ifdef CLIENT_DLL
	DECLARE_CLIENTCLASS_NOBASE();
#else
	DECLARE_SERVERCLASS_NOBASE();
#endif

	CHxGameRules();
	virtual ~CHxGameRules() {}

	virtual const char *GetGameDescription( void ) { return HX_GAME_NAME; }

	HxRoundState_t	GetRoundState( void ) const { return (HxRoundState_t)m_nRoundState.Get(); }
	int				GetCheckpointCount( void ) const { return m_nCheckpointCount; }

#ifndef CLIENT_DLL
	virtual void	Think( void );
	virtual void	LevelInitPostEntity( void );
	virtual void	PlayerSpawn( CBasePlayer *pPlayer );
	virtual void	PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	virtual bool	CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon );
	virtual bool	IsAllowedToSpawn( CBaseEntity *pEntity );

	// Round loop (docs/GAMEPLAY.md)
	void			SetRoundState( HxRoundState_t state );
	void			OnCheckpointSet( CBasePlayer *pPlayer, const Vector &vecOrigin, const QAngle &angles );
	void			OnRunWon( void );
	void			OnRunLost( void );

	// Rosters (hx_weapon_roster / hx_npc_roster)
	bool			IsWeaponAllowed( const char *pszClassname ) const;
	bool			IsNPCAllowed( const char *pszClassname ) const;

private:
	void			SendCheckpointEvent( CBasePlayer *pPlayer, HxCheckpointEvent_t event );
	void			RemoveDisallowedNPCs( void );

	float			m_flRestoreTime;	// gpGlobals->curtime at which a soft respawn happens (mode 2)
#endif

	CNetworkVar( int, m_nRoundState );
	CNetworkVar( int, m_nCheckpointCount );
};

//-----------------------------------------------------------------------------
// Gets us at the Cascade game rules
//-----------------------------------------------------------------------------
inline CHxGameRules *HxGameRules( void )
{
	return static_cast<CHxGameRules *>( g_pGameRules );
}

#endif // HX_GAMERULES_H
