//========= Cascade (hx) — shared definitions ==================================
//
// Constants shared by client and server. Keep this header free of engine
// includes so it can be pulled into any translation unit.
//
//=============================================================================
#ifndef HX_SHAREDDEFS_H
#define HX_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#define HX_GAME_NAME		"Cascade"
#define HX_GAME_VERSION		"0.0.1"

// Round loop state, replicated to the client through CHxGameRules.
// See docs/GAMEPLAY.md for the state diagram.
enum HxRoundState_t
{
	HX_ROUND_INIT = 0,		// map loaded, player not yet spawned
	HX_ROUND_PLAYING,		// player alive, run in progress
	HX_ROUND_PLAYER_DEAD,	// player dead, waiting for checkpoint restore (mode 2) or engine reload (mode 1)
	HX_ROUND_WON,			// hx_logic_run Win fired
	HX_ROUND_LOST,			// hx_logic_run Lose fired

	HX_ROUND_STATE_COUNT
};

// Payload of the HxCheckpoint user message (one byte).
enum HxCheckpointEvent_t
{
	HX_CHECKPOINT_SET = 0,
	HX_CHECKPOINT_RESTORED = 1,
};

#define HX_USERMSG_CHECKPOINT	"HxCheckpoint"

#endif // HX_SHAREDDEFS_H
