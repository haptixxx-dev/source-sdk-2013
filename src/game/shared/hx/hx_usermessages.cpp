//========= Cascade (hx) — user messages =======================================
#include "cbase.h"
#include "usermessages.h"
#include "hx_shareddefs.h"
#include "hx_usermessages.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void HX_RegisterUserMessages( void )
{
	// One byte: HxCheckpointEvent_t
	usermessages->Register( HX_USERMSG_CHECKPOINT, 1 );
}
