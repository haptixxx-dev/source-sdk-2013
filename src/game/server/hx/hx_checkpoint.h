//========= Cascade (hx) — checkpoint store ====================================
//
// Server-only snapshot of the player's state at the last hx_logic_checkpoint.
// Lives outside the gamerules object because CreateGameRulesObject() replaces
// the rules on every map load (game/shared/gamerules_register.cpp:106) and a
// soft respawn (hx_checkpoint_mode 2) must survive that.
//
//=============================================================================
#ifndef HX_CHECKPOINT_H
#define HX_CHECKPOINT_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"

class CBasePlayer;

struct HxCheckpointWeapon_t
{
	char	szClassname[64];
	int		nClip1;
	int		nClip2;
};

struct HxCheckpointAmmo_t
{
	char	szAmmoName[32];
	int		nCount;
};

class CHxCheckpointStore
{
public:
	CHxCheckpointStore() { Clear(); }

	void	Clear( void );
	bool	IsValidForMap( const char *pszMapName ) const;

	// Snapshot pPlayer's inventory and vitals; the respawn point is vecOrigin/angles.
	void	Capture( CBasePlayer *pPlayer, const char *pszMapName, const Vector &vecOrigin, const QAngle &angles );

	// Rebuild pPlayer from the snapshot: teleport, vitals, suit, weapons, clips, ammo, active weapon.
	void	ApplyTo( CBasePlayer *pPlayer );

private:
	bool		m_bValid;
	char		m_szMapName[64];
	Vector		m_vecOrigin;
	QAngle		m_angles;
	int			m_nHealth;
	int			m_nArmor;
	bool		m_bHasSuit;
	char		m_szActiveWeapon[64];
	CUtlVector<HxCheckpointWeapon_t>	m_Weapons;
	CUtlVector<HxCheckpointAmmo_t>		m_Ammo;
};

extern CHxCheckpointStore g_HxCheckpointStore;

#endif // HX_CHECKPOINT_H
