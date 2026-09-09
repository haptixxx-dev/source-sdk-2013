//========= Cascade (hx) — checkpoint / round-state HUD element ================
//
// Flashes "CHECKPOINT" / "CHECKPOINT RESTORED" on the HxCheckpoint user
// message and shows the terminal round state (won/lost) from CHxGameRules.
// Layout comes from scripts/HudLayout.res "HudHxCheckpoint".
//
//=============================================================================
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "hx_gamerules.h"
#include "hx_shareddefs.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Panel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

class CHxHudCheckpoint : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE( CHxHudCheckpoint, Panel );
public:
	CHxHudCheckpoint( const char *pElementName );

	virtual void Init( void );
	virtual void Reset( void );
	virtual bool ShouldDraw( void );
	virtual void Paint( void );

	void MsgFunc_HxCheckpoint( bf_read &msg );

private:
	void DrawCentered( const wchar_t *pwsz, int y, Color color );
	const wchar_t *Localize( const char *pszToken, const wchar_t *pwszFallback );

	CPanelAnimationVar( HFont, m_hFont, "TextFont", "HudHintTextLarge" );
	CPanelAnimationVar( Color, m_TextColor, "TextColor", "255 220 0 255" );
	CPanelAnimationVar( Color, m_WonColor, "WonColor", "56 182 255 255" );
	CPanelAnimationVar( Color, m_LostColor, "LostColor", "255 138 61 255" );
	CPanelAnimationVar( float, m_flFlashSeconds, "FlashSeconds", "2.5" );

	float	m_flFlashUntil;
	int		m_nLastEvent;
};

DECLARE_HUDELEMENT( CHxHudCheckpoint );
DECLARE_HUD_MESSAGE( CHxHudCheckpoint, HxCheckpoint );

CHxHudCheckpoint::CHxHudCheckpoint( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudHxCheckpoint" )
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_MISCSTATUS );
	m_flFlashUntil = 0.0f;
	m_nLastEvent = -1;
}

void CHxHudCheckpoint::Init( void )
{
	HOOK_HUD_MESSAGE( CHxHudCheckpoint, HxCheckpoint );
	Reset();
}

void CHxHudCheckpoint::Reset( void )
{
	m_flFlashUntil = 0.0f;
	m_nLastEvent = -1;
}

void CHxHudCheckpoint::MsgFunc_HxCheckpoint( bf_read &msg )
{
	m_nLastEvent = msg.ReadByte();
	m_flFlashUntil = gpGlobals->curtime + m_flFlashSeconds;
}

bool CHxHudCheckpoint::ShouldDraw( void )
{
	if ( !CHudElement::ShouldDraw() )
		return false;
	if ( gpGlobals->curtime < m_flFlashUntil )
		return true;
	CHxGameRules *pRules = HxGameRules();
	if ( !pRules )
		return false;
	return pRules->GetRoundState() == HX_ROUND_WON || pRules->GetRoundState() == HX_ROUND_LOST;
}

const wchar_t *CHxHudCheckpoint::Localize( const char *pszToken, const wchar_t *pwszFallback )
{
	const wchar_t *pwsz = g_pVGuiLocalize->Find( pszToken );
	return pwsz ? pwsz : pwszFallback;
}

void CHxHudCheckpoint::DrawCentered( const wchar_t *pwsz, int y, Color color )
{
	int wide, tall;
	GetSize( wide, tall );
	int textWide, textTall;
	surface()->GetTextSize( m_hFont, pwsz, textWide, textTall );
	surface()->DrawSetTextFont( m_hFont );
	surface()->DrawSetTextColor( color );
	surface()->DrawSetTextPos( ( wide - textWide ) / 2, y );
	surface()->DrawUnicodeString( pwsz );
}

void CHxHudCheckpoint::Paint( void )
{
	int y = 0;

	CHxGameRules *pRules = HxGameRules();
	if ( pRules && pRules->GetRoundState() == HX_ROUND_WON )
	{
		DrawCentered( Localize( "#hx_RunWon", L"RUN COMPLETE" ), y, m_WonColor );
		y += surface()->GetFontTall( m_hFont );
	}
	else if ( pRules && pRules->GetRoundState() == HX_ROUND_LOST )
	{
		DrawCentered( Localize( "#hx_RunLost", L"RUN FAILED" ), y, m_LostColor );
		y += surface()->GetFontTall( m_hFont );
	}

	if ( gpGlobals->curtime < m_flFlashUntil )
	{
		float flAlpha = clamp( ( m_flFlashUntil - gpGlobals->curtime ) / MAX( m_flFlashSeconds, 0.01f ), 0.0f, 1.0f );
		Color c = m_TextColor;
		c[3] = (int)( 255.0f * flAlpha );
		const wchar_t *pwsz = ( m_nLastEvent == HX_CHECKPOINT_RESTORED )
			? Localize( "#hx_CheckpointRestored", L"CHECKPOINT RESTORED" )
			: Localize( "#hx_Checkpoint", L"CHECKPOINT" );
		DrawCentered( pwsz, y, c );
	}
}
