//========= Cascade (hx) — game rules ==========================================
#include "cbase.h"
#include "hx_gamerules.h"
#include "hx_shareddefs.h"

#ifdef CLIENT_DLL

#else
	#include "player.h"
	#include "ai_basenpc.h"
	#include "basecombatweapon.h"
	#include "entitylist.h"
	#include "hx/hx_checkpoint.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar hx_checkpoint_mode;
extern ConVar hx_checkpoint_respawn_delay;
extern ConVar hx_weapon_roster;
extern ConVar hx_npc_roster;

REGISTER_GAMERULES_CLASS( CHxGameRules );

BEGIN_NETWORK_TABLE_NOBASE( CHxGameRules, DT_HxGameRules )
	#ifdef CLIENT_DLL
		RecvPropInt( RECVINFO( m_nRoundState ) ),
		RecvPropInt( RECVINFO( m_nCheckpointCount ) ),
	#else
		SendPropInt( SENDINFO( m_nRoundState ), 4, SPROP_UNSIGNED ),
		SendPropInt( SENDINFO( m_nCheckpointCount ), 16, SPROP_UNSIGNED ),
	#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( hx_gamerules, CHxGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( HxGameRulesProxy, DT_HxGameRulesProxy )

#ifdef CLIENT_DLL
	void RecvProxy_HxGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CHxGameRules *pRules = HxGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CHxGameRulesProxy, DT_HxGameRulesProxy )
		RecvPropDataTable( "hx_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_HxGameRules ), RecvProxy_HxGameRules )
	END_RECV_TABLE()
#else
	void *SendProxy_HxGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CHxGameRules *pRules = HxGameRules();
		Assert( pRules );
		pRecipients->SetAllRecipients();
		return pRules;
	}

	BEGIN_SEND_TABLE( CHxGameRulesProxy, DT_HxGameRulesProxy )
		SendPropDataTable( "hx_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_HxGameRules ), SendProxy_HxGameRules )
	END_SEND_TABLE()
#endif

//-----------------------------------------------------------------------------
// Returns true when pszClassname appears in the comma-separated roster, or the
// roster is empty (= everything allowed).
//-----------------------------------------------------------------------------
static bool RosterAllows( const ConVar &roster, const char *pszClassname )
{
	const char *pszList = roster.GetString();
	if ( !pszList || !pszList[0] || !pszClassname )
		return true;

	const int nLen = Q_strlen( pszClassname );
	const char *p = pszList;
	while ( *p )
	{
		while ( *p == ',' || *p == ' ' )
			p++;
		const char *pEnd = p;
		while ( *pEnd && *pEnd != ',' && *pEnd != ' ' )
			pEnd++;
		if ( ( pEnd - p ) == nLen && Q_strnicmp( p, pszClassname, nLen ) == 0 )
			return true;
		p = pEnd;
	}
	return false;
}

CHxGameRules::CHxGameRules()
{
	m_nRoundState = HX_ROUND_INIT;
	m_nCheckpointCount = 0;
#ifndef CLIENT_DLL
	m_flRestoreTime = 0.0f;
#endif
}

#ifndef CLIENT_DLL

//-----------------------------------------------------------------------------
bool CHxGameRules::IsWeaponAllowed( const char *pszClassname ) const
{
	return RosterAllows( hx_weapon_roster, pszClassname );
}

bool CHxGameRules::IsNPCAllowed( const char *pszClassname ) const
{
	return RosterAllows( hx_npc_roster, pszClassname );
}

//-----------------------------------------------------------------------------
void CHxGameRules::SetRoundState( HxRoundState_t state )
{
	if ( m_nRoundState == state )
		return;
	DevMsg( "[hx] round state %d -> %d\n", m_nRoundState.Get(), state );
	m_nRoundState = state;
}

//-----------------------------------------------------------------------------
// Called once all map entities have spawned (CServerGameDLL::LevelInit_ParseAllEntities path).
//-----------------------------------------------------------------------------
void CHxGameRules::LevelInitPostEntity( void )
{
	BaseClass::LevelInitPostEntity();

	m_flRestoreTime = 0.0f;
	SetRoundState( HX_ROUND_INIT );

	if ( !g_HxCheckpointStore.IsValidForMap( STRING( gpGlobals->mapname ) ) )
		g_HxCheckpointStore.Clear();

	RemoveDisallowedNPCs();
}

//-----------------------------------------------------------------------------
// hx_npc_roster: map-placed NPCs not on the roster are removed. NPCs created
// later by npc_maker/npc_template_maker are not filtered (documented in
// docs/GAMEPLAY.md).
//-----------------------------------------------------------------------------
void CHxGameRules::RemoveDisallowedNPCs( void )
{
	if ( !hx_npc_roster.GetString()[0] )
		return;

	int nRemoved = 0;
	CBaseEntity *pEnt = gEntList.FirstEnt();
	while ( pEnt )
	{
		CBaseEntity *pNext = gEntList.NextEnt( pEnt );
		if ( pEnt->IsNPC() && !IsNPCAllowed( pEnt->GetClassname() ) )
		{
			UTIL_Remove( pEnt );
			nRemoved++;
		}
		pEnt = pNext;
	}
	if ( nRemoved )
		Msg( "[hx] hx_npc_roster removed %d NPC(s)\n", nRemoved );
}

//-----------------------------------------------------------------------------
void CHxGameRules::Think( void )
{
	BaseClass::Think();

	if ( GetRoundState() == HX_ROUND_PLAYER_DEAD && m_flRestoreTime > 0.0f && gpGlobals->curtime >= m_flRestoreTime )
	{
		m_flRestoreTime = 0.0f;
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		if ( !pPlayer || pPlayer->IsAlive() )
			return;

		if ( hx_checkpoint_mode.GetInt() == 2 )
		{
			// CBasePlayer::ForceRespawn strips the inventory and calls Spawn(); PlayerSpawn() below
			// reapplies the checkpoint.
			pPlayer->ForceRespawn();
		}
		else if ( hx_checkpoint_mode.GetInt() == 1 )
		{
			// The 64-bit SDK Base engine does not reload on death by itself (verified: 13 s after
			// `kill` the state was still PLAYER_DEAD). Same command player_loadsaved uses
			// (game/server/player.cpp CRevertSaved::LoadThink).
			engine->ServerCommand( "reload\n" );
		}
	}
}

//-----------------------------------------------------------------------------
void CHxGameRules::PlayerSpawn( CBasePlayer *pPlayer )
{
	BaseClass::PlayerSpawn( pPlayer );

	if ( GetRoundState() == HX_ROUND_PLAYER_DEAD && hx_checkpoint_mode.GetInt() == 2 &&
		 g_HxCheckpointStore.IsValidForMap( STRING( gpGlobals->mapname ) ) )
	{
		g_HxCheckpointStore.ApplyTo( pPlayer );
		SendCheckpointEvent( pPlayer, HX_CHECKPOINT_RESTORED );
	}

	if ( GetRoundState() == HX_ROUND_INIT || GetRoundState() == HX_ROUND_PLAYER_DEAD )
		SetRoundState( HX_ROUND_PLAYING );
}

//-----------------------------------------------------------------------------
void CHxGameRules::PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
	BaseClass::PlayerKilled( pVictim, info );

	if ( GetRoundState() != HX_ROUND_PLAYING )
		return;

	SetRoundState( HX_ROUND_PLAYER_DEAD );

	// Mode 0 leaves stock HL2 behaviour alone. Mode 1 reloads the autosave, mode 2 soft-respawns;
	// both after hx_checkpoint_respawn_delay (see Think).
	const int nMode = hx_checkpoint_mode.GetInt();
	if ( nMode == 1 || ( nMode == 2 && g_HxCheckpointStore.IsValidForMap( STRING( gpGlobals->mapname ) ) ) )
	{
		m_flRestoreTime = gpGlobals->curtime + hx_checkpoint_respawn_delay.GetFloat();
	}
}

//-----------------------------------------------------------------------------
bool CHxGameRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon )
{
	if ( pWeapon && !IsWeaponAllowed( pWeapon->GetClassname() ) )
		return false;
	return BaseClass::CanHavePlayerItem( pPlayer, pWeapon );
}

//-----------------------------------------------------------------------------
// Called for items/weapons about to be created (item_world.cpp, ai_basenpc.cpp weapon drops).
//-----------------------------------------------------------------------------
bool CHxGameRules::IsAllowedToSpawn( CBaseEntity *pEntity )
{
	if ( pEntity && dynamic_cast<CBaseCombatWeapon *>( pEntity ) && !IsWeaponAllowed( pEntity->GetClassname() ) )
		return false;
	return BaseClass::IsAllowedToSpawn( pEntity );
}

//-----------------------------------------------------------------------------
void CHxGameRules::OnCheckpointSet( CBasePlayer *pPlayer, const Vector &vecOrigin, const QAngle &angles )
{
	if ( !pPlayer || hx_checkpoint_mode.GetInt() == 0 )
		return;

	g_HxCheckpointStore.Capture( pPlayer, STRING( gpGlobals->mapname ), vecOrigin, angles );
	m_nCheckpointCount = m_nCheckpointCount + 1;

	if ( hx_checkpoint_mode.GetInt() == 1 )
	{
		// Same path as logic_autosave (game/server/logicentities.cpp CLogicAutosave::InputSave).
		engine->ServerCommand( "autosave\n" );
	}

	SendCheckpointEvent( pPlayer, HX_CHECKPOINT_SET );
}

//-----------------------------------------------------------------------------
void CHxGameRules::OnRunWon( void )
{
	if ( GetRoundState() == HX_ROUND_PLAYING )
		SetRoundState( HX_ROUND_WON );
}

void CHxGameRules::OnRunLost( void )
{
	if ( GetRoundState() == HX_ROUND_PLAYING || GetRoundState() == HX_ROUND_PLAYER_DEAD )
	{
		m_flRestoreTime = 0.0f;
		SetRoundState( HX_ROUND_LOST );
	}
}

//-----------------------------------------------------------------------------
void CHxGameRules::SendCheckpointEvent( CBasePlayer *pPlayer, HxCheckpointEvent_t event )
{
	CSingleUserRecipientFilter filter( pPlayer );
	filter.MakeReliable();
	UserMessageBegin( filter, HX_USERMSG_CHECKPOINT );
		WRITE_BYTE( (int)event );
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Developer commands (need sv_cheats 1).
//-----------------------------------------------------------------------------
CON_COMMAND_F( hx_checkpoint, "Set a checkpoint at the player's current position.", FCVAR_CHEAT )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient() ? UTIL_GetCommandClient() : UTIL_GetLocalPlayer();
	if ( pPlayer && HxGameRules() )
		HxGameRules()->OnCheckpointSet( pPlayer, pPlayer->GetAbsOrigin(), pPlayer->EyeAngles() );
}

CON_COMMAND_F( hx_run_win, "Force the run into the WON state.", FCVAR_CHEAT )
{
	if ( HxGameRules() )
		HxGameRules()->OnRunWon();
}

CON_COMMAND_F( hx_run_lose, "Force the run into the LOST state.", FCVAR_CHEAT )
{
	if ( HxGameRules() )
		HxGameRules()->OnRunLost();
}

CON_COMMAND_F( hx_round_state, "Print the current Cascade round state.", FCVAR_NONE )
{
	if ( HxGameRules() )
		Msg( "[hx] round state %d, checkpoints %d, checkpoint store %s, curtime %.2f\n", HxGameRules()->GetRoundState(),
			HxGameRules()->GetCheckpointCount(), g_HxCheckpointStore.IsValidForMap( STRING( gpGlobals->mapname ) ) ? "valid" : "empty",
			gpGlobals->curtime );
}

#endif // !CLIENT_DLL
