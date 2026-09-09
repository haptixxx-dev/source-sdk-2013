//========= Cascade (hx) — ConVars =============================================
//
// Every Cascade tunable lives here so `find hx_` in the console lists them all.
// Compiled into both client.so and server.so; FCVAR_REPLICATED keeps the two
// in sync, FCVAR_NOTIFY announces changes to players.
//
//=============================================================================
#include "cbase.h"
#include "hx_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar hx_version( "hx_version", HX_GAME_VERSION, FCVAR_REPLICATED | FCVAR_NOTIFY,
	"Cascade game version (read-only by convention)." );

ConVar hx_checkpoint_mode( "hx_checkpoint_mode", "1", FCVAR_REPLICATED | FCVAR_NOTIFY,
	"0 = off (stock HL2 behaviour). "
	"1 = hx_logic_checkpoint issues an engine autosave; death reloads it. "
	"2 = soft respawn: the player is restored at the last checkpoint without reloading the map.",
	true, 0.0f, true, 2.0f );

ConVar hx_checkpoint_respawn_delay( "hx_checkpoint_respawn_delay", "2.5", FCVAR_REPLICATED,
	"Seconds between the player's death and the soft respawn (hx_checkpoint_mode 2).",
	true, 0.0f, true, 30.0f );

ConVar hx_weapon_roster( "hx_weapon_roster", "", FCVAR_REPLICATED | FCVAR_NOTIFY,
	"Comma-separated weapon classnames the player may pick up (e.g. weapon_pistol,weapon_smg1). "
	"Empty = every HL2 weapon." );

ConVar hx_npc_roster( "hx_npc_roster", "", FCVAR_REPLICATED | FCVAR_NOTIFY,
	"Comma-separated NPC classnames allowed in a map; others are removed after the map spawns. "
	"Empty = every HL2 NPC." );
