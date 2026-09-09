//========= Cascade (hx) — checkpoint store and logic entities =================
#include "cbase.h"
#include "player.h"
#include "basecombatweapon.h"
#include "ammodef.h"
#include "hx/hx_checkpoint.h"
#include "hx_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CHxCheckpointStore g_HxCheckpointStore;

//-----------------------------------------------------------------------------
void CHxCheckpointStore::Clear( void )
{
	m_bValid = false;
	m_szMapName[0] = 0;
	m_vecOrigin = vec3_origin;
	m_angles = vec3_angle;
	m_nHealth = 100;
	m_nArmor = 0;
	m_bHasSuit = false;
	m_szActiveWeapon[0] = 0;
	m_Weapons.RemoveAll();
	m_Ammo.RemoveAll();
}

bool CHxCheckpointStore::IsValidForMap( const char *pszMapName ) const
{
	return m_bValid && pszMapName && Q_stricmp( m_szMapName, pszMapName ) == 0;
}

//-----------------------------------------------------------------------------
void CHxCheckpointStore::Capture( CBasePlayer *pPlayer, const char *pszMapName, const Vector &vecOrigin, const QAngle &angles )
{
	Clear();
	if ( !pPlayer )
		return;

	Q_strncpy( m_szMapName, pszMapName ? pszMapName : "", sizeof( m_szMapName ) );
	m_vecOrigin = vecOrigin;
	m_angles = angles;
	m_nHealth = MAX( pPlayer->GetHealth(), 1 );
	m_nArmor = pPlayer->ArmorValue();
	m_bHasSuit = pPlayer->IsSuitEquipped();

	CBaseCombatWeapon *pActive = pPlayer->GetActiveWeapon();
	if ( pActive )
		Q_strncpy( m_szActiveWeapon, pActive->GetClassname(), sizeof( m_szActiveWeapon ) );

	for ( int i = 0; i < pPlayer->WeaponCount(); i++ )
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetWeapon( i );
		if ( !pWeapon )
			continue;
		HxCheckpointWeapon_t w;
		Q_strncpy( w.szClassname, pWeapon->GetClassname(), sizeof( w.szClassname ) );
		w.nClip1 = pWeapon->Clip1();
		w.nClip2 = pWeapon->Clip2();
		m_Weapons.AddToTail( w );
	}

	CAmmoDef *pAmmoDef = GetAmmoDef();
	for ( int i = 1; i < pAmmoDef->m_nAmmoIndex; i++ )
	{
		int nCount = pPlayer->GetAmmoCount( i );
		if ( nCount <= 0 )
			continue;
		Ammo_t *pAmmo = pAmmoDef->GetAmmoOfIndex( i );
		if ( !pAmmo || !pAmmo->pName )
			continue;
		HxCheckpointAmmo_t a;
		Q_strncpy( a.szAmmoName, pAmmo->pName, sizeof( a.szAmmoName ) );
		a.nCount = nCount;
		m_Ammo.AddToTail( a );
	}

	m_bValid = true;
	DevMsg( "[hx] checkpoint captured on %s at (%.0f %.0f %.0f): hp %d armor %d, %d weapons, %d ammo types\n",
		m_szMapName, m_vecOrigin.x, m_vecOrigin.y, m_vecOrigin.z, m_nHealth, m_nArmor, m_Weapons.Count(), m_Ammo.Count() );
}

//-----------------------------------------------------------------------------
void CHxCheckpointStore::ApplyTo( CBasePlayer *pPlayer )
{
	if ( !pPlayer || !m_bValid )
		return;

	pPlayer->RemoveAllItems( true );
	pPlayer->RemoveAllAmmo();

	if ( m_bHasSuit )
		pPlayer->EquipSuit( false );

	pPlayer->SetHealth( m_nHealth );
	pPlayer->SetArmorValue( m_nArmor );

	for ( int i = 0; i < m_Weapons.Count(); i++ )
	{
		CBaseEntity *pEnt = pPlayer->GiveNamedItem( m_Weapons[i].szClassname );
		CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon *>( pEnt );
		if ( !pWeapon )
			pWeapon = pPlayer->Weapon_OwnsThisType( m_Weapons[i].szClassname );
		if ( pWeapon )
		{
			if ( m_Weapons[i].nClip1 >= 0 )
				pWeapon->SetClip1( m_Weapons[i].nClip1 );
			if ( m_Weapons[i].nClip2 >= 0 )
				pWeapon->SetClip2( m_Weapons[i].nClip2 );
		}
	}

	CAmmoDef *pAmmoDef = GetAmmoDef();
	for ( int i = 0; i < m_Ammo.Count(); i++ )
	{
		int nIndex = pAmmoDef->Index( m_Ammo[i].szAmmoName );
		if ( nIndex > 0 )
			pPlayer->SetAmmoCount( m_Ammo[i].nCount, nIndex );
	}

	if ( m_szActiveWeapon[0] )
	{
		CBaseCombatWeapon *pActive = pPlayer->Weapon_OwnsThisType( m_szActiveWeapon );
		if ( pActive )
			pPlayer->Weapon_Switch( pActive );
	}

	pPlayer->Teleport( &m_vecOrigin, &m_angles, &vec3_origin );
	pPlayer->SnapEyeAngles( m_angles );

	DevMsg( "[hx] checkpoint applied\n" );
}

//=============================================================================
// hx_logic_checkpoint — mappers fire Activate (e.g. from a trigger_once) to
// record a checkpoint. Spawn point = this entity's origin/angles, or the
// player's own position with spawnflag 1.
//=============================================================================
#define SF_HX_CHECKPOINT_USE_PLAYER_POS	0x0001

class CHxLogicCheckpoint : public CLogicalEntity
{
	DECLARE_CLASS( CHxLogicCheckpoint, CLogicalEntity );
public:
	void InputActivate( inputdata_t &inputdata );

	DECLARE_DATADESC();

private:
	COutputEvent m_OnActivated;
};

LINK_ENTITY_TO_CLASS( hx_logic_checkpoint, CHxLogicCheckpoint );

BEGIN_DATADESC( CHxLogicCheckpoint )
	DEFINE_INPUTFUNC( FIELD_VOID, "Activate", InputActivate ),
	DEFINE_OUTPUT( m_OnActivated, "OnActivated" ),
END_DATADESC()

void CHxLogicCheckpoint::InputActivate( inputdata_t &inputdata )
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if ( !pPlayer || !HxGameRules() )
		return;

	Vector vecOrigin = GetAbsOrigin();
	QAngle angles = GetAbsAngles();
	if ( HasSpawnFlags( SF_HX_CHECKPOINT_USE_PLAYER_POS ) )
	{
		vecOrigin = pPlayer->GetAbsOrigin();
		angles = pPlayer->EyeAngles();
	}

	HxGameRules()->OnCheckpointSet( pPlayer, vecOrigin, angles );
	m_OnActivated.FireOutput( inputdata.pActivator, this );
}

//=============================================================================
// hx_logic_run — one per map. Win/Lose end the run; OnWin/OnLose let the map
// react (fade, changelevel, credits).
//=============================================================================
class CHxLogicRun : public CLogicalEntity
{
	DECLARE_CLASS( CHxLogicRun, CLogicalEntity );
public:
	void InputWin( inputdata_t &inputdata );
	void InputLose( inputdata_t &inputdata );

	DECLARE_DATADESC();

private:
	COutputEvent m_OnWin;
	COutputEvent m_OnLose;
};

LINK_ENTITY_TO_CLASS( hx_logic_run, CHxLogicRun );

BEGIN_DATADESC( CHxLogicRun )
	DEFINE_INPUTFUNC( FIELD_VOID, "Win", InputWin ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Lose", InputLose ),
	DEFINE_OUTPUT( m_OnWin, "OnWin" ),
	DEFINE_OUTPUT( m_OnLose, "OnLose" ),
END_DATADESC()

void CHxLogicRun::InputWin( inputdata_t &inputdata )
{
	if ( HxGameRules() )
		HxGameRules()->OnRunWon();
	m_OnWin.FireOutput( inputdata.pActivator, this );
}

void CHxLogicRun::InputLose( inputdata_t &inputdata )
{
	if ( HxGameRules() )
		HxGameRules()->OnRunLost();
	m_OnLose.FireOutput( inputdata.pActivator, this );
}
