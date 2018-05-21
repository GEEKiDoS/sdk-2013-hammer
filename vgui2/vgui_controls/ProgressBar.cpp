//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include <vgui_controls/ProgressBar.h>
#include <vgui_controls/Controls.h>

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY( ProgressBar );

BEGIN_PANEL_SETTINGS( ProgressBar )
	{ "progress", TYPE_STRING },
	{ "segment_gap", TYPE_INTEGER },
	{ "segment_width", TYPE_INTEGER },
	{ "variable", TYPE_STRING }
END_PANEL_SETTINGS( ProgressBar );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
ProgressBar::ProgressBar(Panel *parent, const char *panelName) : Panel(parent, panelName)
{
	_progress = 0.0f;
	m_pszDialogVar = NULL;
	SetSegmentInfo( 4, 8 );
	SetBarInset( 4 );
	SetMargin( 0 );
	m_iProgressDirection = PROGRESS_EAST;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
ProgressBar::~ProgressBar()
{
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void ProgressBar::SetSegmentInfo( int gap, int width )
{
	_segmentGap = gap;
	_segmentWide = width;
}

//-----------------------------------------------------------------------------
// Purpose: returns the number of segment blocks drawn
//-----------------------------------------------------------------------------
int ProgressBar::GetDrawnSegmentCount()
{
	int wide, tall;
	GetSize(wide, tall);
	int segmentTotal = wide / (_segmentGap + _segmentWide);
	return (int)(segmentTotal * _progress);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ProgressBar::PaintBackground()
{
	int wide, tall;
	GetSize(wide, tall);

	surface()->DrawSetColor(GetBgColor());
	surface()->DrawFilledRect(0, 0, wide, tall);
}

void ProgressBar::PaintSegment( int &x, int &y, int tall, int wide )
{
	switch( m_iProgressDirection )
	{
	case PROGRESS_EAST:
		x += _segmentGap;
		surface()->DrawFilledRect(x, y, x + _segmentWide, y + tall - (y * 2));
		x += _segmentWide;
		break;

	case PROGRESS_WEST:
		x -= _segmentGap + _segmentWide;
		surface()->DrawFilledRect(x, y, x + _segmentWide, y + tall - (y * 2));
		break;

	case PROGRESS_NORTH:
		y -= _segmentGap + _segmentWide;
		surface()->DrawFilledRect(x, y, x + wide - (x * 2), y + _segmentWide );
		break;

	case PROGRESS_SOUTH:
		y += _segmentGap;
		surface()->DrawFilledRect(x, y, x + wide - (x * 2), y + _segmentWide );
		y += _segmentWide;
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ProgressBar::Paint()
{
	int wide, tall;
	GetSize(wide, tall);

	// gaps
	int segmentTotal = 0, segmentsDrawn = 0;
	int x = 0, y = 0;

	switch( m_iProgressDirection )
	{
	case PROGRESS_WEST:
		wide -= 2 * m_iBarMargin;
		x = wide - m_iBarMargin;
		y = m_iBarInset;
		segmentTotal = wide / (_segmentGap + _segmentWide);
		segmentsDrawn = (int)(segmentTotal * _progress);
		break;

	case PROGRESS_EAST:
		wide -= 2 * m_iBarMargin;
		x = m_iBarMargin;
		y = m_iBarInset;
		segmentTotal = wide / (_segmentGap + _segmentWide);
		segmentsDrawn = (int)(segmentTotal * _progress);
		break;

	case PROGRESS_NORTH:
		tall -= 2 * m_iBarMargin;
		x = m_iBarInset;
		y = tall - m_iBarMargin;
		segmentTotal = tall / (_segmentGap + _segmentWide);
		segmentsDrawn = (int)(segmentTotal * _progress);
		break;

	case PROGRESS_SOUTH:
		tall -= 2 * m_iBarMargin;
		x = m_iBarInset;
		y = m_iBarMargin;
		segmentTotal = tall / (_segmentGap + _segmentWide);
		segmentsDrawn = (int)(segmentTotal * _progress);
		break;
	}

	surface()->DrawSetColor(GetFgColor());
	for (int i = 0; i < segmentsDrawn; i++)
	{
		PaintSegment( x, y, tall, wide );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ProgressBar::SetProgress(float progress)
{
	if (progress != _progress)
	{
		// clamp the progress value within the range
		if (progress < 0.0f)
		{
			progress = 0.0f;
		}
		else if (progress > 1.0f)
		{
			progress = 1.0f;
		}

		_progress = progress;
		Repaint();
	}
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
float ProgressBar::GetProgress()
{
	return _progress;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ProgressBar::ApplySchemeSettings(IScheme *pScheme)
{
	Panel::ApplySchemeSettings(pScheme);

	SetFgColor(GetSchemeColor("ProgressBar.FgColor", pScheme));
	SetBgColor(GetSchemeColor("ProgressBar.BgColor", pScheme));
	SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
}

//-----------------------------------------------------------------------------
// Purpose: utility function for calculating a time remaining string
//-----------------------------------------------------------------------------
bool ProgressBar::ConstructTimeRemainingString(wchar_t *output, int outputBufferSizeInBytes, float startTime, float currentTime, float currentProgress, float lastProgressUpdateTime, bool addRemainingSuffix)
{
	Assert(lastProgressUpdateTime <= currentTime);
	output[0] = 0;

	// calculate pre-extrapolation values
	float timeElapsed = lastProgressUpdateTime - startTime;
	float totalTime = timeElapsed / currentProgress;

	// calculate seconds
	int secondsRemaining = (int)(totalTime - timeElapsed);
	if (lastProgressUpdateTime < currentTime)
	{
		// old update, extrapolate
		float progressRate = currentProgress / timeElapsed;
		float extrapolatedProgress = progressRate * (currentTime - startTime);
		float extrapolatedTotalTime = (currentTime - startTime) / extrapolatedProgress;
		secondsRemaining = (int)(extrapolatedTotalTime - timeElapsed);
	}
	// if there's some time, make sure it's at least one second left
	if ( secondsRemaining == 0 && ( ( totalTime - timeElapsed ) > 0 ) )
	{
		secondsRemaining = 1;
	}

	// calculate minutes
	int minutesRemaining = 0;
	while (secondsRemaining >= 60)
	{
		minutesRemaining++;
		secondsRemaining -= 60;
	}

    char minutesBuf[16];
    Q_snprintf(minutesBuf, sizeof( minutesBuf ), "%d", minutesRemaining);
    char secondsBuf[16];
    Q_snprintf(secondsBuf, sizeof( secondsBuf ), "%d", secondsRemaining);

	if (minutesRemaining > 0)
	{
		wchar_t unicodeMinutes[16];
		g_pVGuiLocalize->ConvertANSIToUnicode(minutesBuf, unicodeMinutes, sizeof( unicodeMinutes ));
		wchar_t unicodeSeconds[16];
		g_pVGuiLocalize->ConvertANSIToUnicode(secondsBuf, unicodeSeconds, sizeof( unicodeSeconds ));

		const char *unlocalizedString = "#vgui_TimeLeftMinutesSeconds";
		if (minutesRemaining == 1 && secondsRemaining == 1)
		{
			unlocalizedString = "#vgui_TimeLeftMinuteSecond";
		}
		else if (minutesRemaining == 1)
		{
			unlocalizedString = "#vgui_TimeLeftMinuteSeconds";
		}
		else if (secondsRemaining == 1)
		{
			unlocalizedString = "#vgui_TimeLeftMinutesSecond";
		}

		char unlocString[64];
		Q_strncpy(unlocString, unlocalizedString,sizeof( unlocString ));
		if (addRemainingSuffix)
		{
			Q_strncat(unlocString, "Remaining", sizeof(unlocString ), COPY_ALL_CHARACTERS);
		}
		g_pVGuiLocalize->ConstructString(output, outputBufferSizeInBytes, g_pVGuiLocalize->Find(unlocString), 2, unicodeMinutes, unicodeSeconds);

	}
	else if (secondsRemaining > 0)
	{
		wchar_t unicodeSeconds[16];
		g_pVGuiLocalize->ConvertANSIToUnicode(secondsBuf, unicodeSeconds, sizeof( unicodeSeconds ));

		const char *unlocalizedString = "#vgui_TimeLeftSeconds";
		if (secondsRemaining == 1)
		{
			unlocalizedString = "#vgui_TimeLeftSecond";
		}
		char unlocString[64];
		Q_strncpy(unlocString, unlocalizedString,sizeof(unlocString));
		if (addRemainingSuffix)
		{
			Q_strncat(unlocString, "Remaining",sizeof(unlocString), COPY_ALL_CHARACTERS);
		}
		g_pVGuiLocalize->ConstructString(output, outputBufferSizeInBytes, g_pVGuiLocalize->Find(unlocString), 1, unicodeSeconds);
	}
	else
	{
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void ProgressBar::SetBarInset( int pixels )
{
	m_iBarInset = pixels;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
int ProgressBar::GetBarInset( void )
{
	return m_iBarInset;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void ProgressBar::SetMargin( int pixels )
{
	m_iBarMargin = pixels;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
int ProgressBar::GetMargin()
{
	return m_iBarMargin;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ProgressBar::ApplySettings(KeyValues *inResourceData)
{
	_progress = inResourceData->GetFloat("progress", 0.0f);
	_segmentGap = inResourceData->GetInt( "segment_gap", 4 );
	_segmentWide = inResourceData->GetInt( "segment_width", 8 );
	m_pszDialogVar = inResourceData->GetString("variable", NULL);

	BaseClass::ApplySettings(inResourceData);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ProgressBar::GetSettings(KeyValues *outResourceData)
{
	BaseClass::GetSettings(outResourceData);
	outResourceData->SetFloat("progress", _progress );
	outResourceData->SetInt( "segment_gap", _segmentGap );
	outResourceData->SetInt( "segment_width", _segmentWide );
	outResourceData->SetString( "variable", m_pszDialogVar );
}

//-----------------------------------------------------------------------------
// Purpose: Returns a string description of the panel fields for use in the UI
//-----------------------------------------------------------------------------
/*const char *ProgressBar::GetDescription( void )
{
	static char buf[1024];
	_snprintf(buf, sizeof(buf), "%s, string progress, string variable", BaseClass::GetDescription());
	return buf;
}*/

//-----------------------------------------------------------------------------
// Purpose: updates progress bar bases on values
//-----------------------------------------------------------------------------
void ProgressBar::OnDialogVariablesChanged(KeyValues *dialogVariables)
{
	if (!m_pszDialogVar.IsEmpty())
	{
		int val = dialogVariables->GetInt(m_pszDialogVar, -1);
		if (val >= 0.0f)
		{
			SetProgress(val / 100.0f);
		}
	}
}


DECLARE_BUILD_FACTORY( ContinuousProgressBar );

BEGIN_PANEL_SETTINGS( ContinuousProgressBar )
	{ "using_textures", TYPE_BOOL },
	{ "fg_texture", TYPE_STRING },
	{ "bg_texture", TYPE_STRING }
END_PANEL_SETTINGS( ContinuousProgressBar )

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
ContinuousProgressBar::ContinuousProgressBar(Panel *parent, const char *panelName) : ProgressBar(parent, panelName), m_bUsingTextures( false )
{
	for ( int i = 0; i < NUM_PROGRESS_TEXTURES; i++ )
	{
		m_nTextureId[i] = -1;
		m_pszImageName[i] = NULL;
	}
}

ContinuousProgressBar::~ContinuousProgressBar()
{
	for ( int i = 0; i < NUM_PROGRESS_TEXTURES; i++ )
	{
		if ( vgui::surface() && vgui::surface()->IsTextureIDValid( m_nTextureId[i] ) )
		{
			vgui::surface()->DeleteTextureByID( m_nTextureId[i] );
			m_nTextureId[i] = -1;
		}
	}
}

void ContinuousProgressBar::SetImage( const char* imageName, progress_textures_t iPos )
{
	if ( !imageName )
	{
		if ( vgui::surface() && vgui::surface()->IsTextureIDValid( m_nTextureId[iPos] ) )
		{
			vgui::surface()->DeleteTextureByID( m_nTextureId[iPos] );
			m_nTextureId[iPos] = -1;
		}

		m_bUsingTextures = false;
		SetPaintBorderEnabled( true );

		return;
	}

	m_pszImageName[iPos].Format( "vgui/%s", imageName );

	m_bUsingTextures = true;
	SetPaintBorderEnabled( false );

	InvalidateLayout( false, true ); // force applyschemesettings to run
}

void ContinuousProgressBar::ApplySchemeSettings( IScheme* pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	for ( int i = 0; i < NUM_PROGRESS_TEXTURES; i++ )
	{
		if ( !m_pszImageName[i].IsEmpty() )
		{
			if ( m_nTextureId[i] == -1 )
			{
				m_nTextureId[i] = surface()->CreateNewTextureID();
			}

			surface()->DrawSetTextureFile( m_nTextureId[i], m_pszImageName[i], true, false );
		}
	}
}

void ContinuousProgressBar::ApplySettings( KeyValues* inResourceData )
{
	BaseClass::ApplySettings( inResourceData );
	bool bNotFound;
	const bool bUsingTextures = inResourceData->GetBool( "using_textures", false, &bNotFound );
	if ( !bNotFound )
		m_bUsingTextures = bUsingTextures;
	if ( const char* fg_texture = inResourceData->GetString( "fg_texture", NULL ) )
		SetImage( fg_texture, PROGRESS_TEXTURE_FG );
	if ( const char* bg_texture = inResourceData->GetString( "bg_texture", NULL ) )
		SetImage( bg_texture, PROGRESS_TEXTURE_BG );

	InvalidateLayout( true );
}

void ContinuousProgressBar::GetSettings( KeyValues* outResourceData )
{
	BaseClass::GetSettings( outResourceData );
	outResourceData->SetBool( "using_textures", m_bUsingTextures );
	outResourceData->SetString( "fg_texture", m_pszImageName[PROGRESS_TEXTURE_FG] );
	outResourceData->SetString( "bg_texture", m_pszImageName[PROGRESS_TEXTURE_BG] );
}


void ContinuousProgressBar::PaintBackground()
{
	if ( !m_bUsingTextures )
		return BaseClass::PaintBackground();

	// If we don't have a Bg image, use the foreground
	int iTextureID = m_nTextureId[PROGRESS_TEXTURE_BG] != -1 ? m_nTextureId[PROGRESS_TEXTURE_BG] : m_nTextureId[PROGRESS_TEXTURE_FG];
	vgui::surface()->DrawSetTexture( iTextureID );
	vgui::surface()->DrawSetColor( GetBgColor() );

	int wide, tall;
	GetSize( wide, tall );

	vgui::surface()->DrawTexturedRect( 0, 0, wide, tall );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ContinuousProgressBar::Paint()
{
	int x = 0, y = 0;
	int wide, tall;
	GetSize(wide, tall);

	surface()->DrawSetColor(GetFgColor());

	void( ISurface::*func )( int, int, int, int ) = &ISurface::DrawFilledRect;
	if ( m_bUsingTextures )
	{
		func = &ISurface::DrawTexturedRect;

		vgui::surface()->DrawSetTexture( m_nTextureId[PROGRESS_TEXTURE_FG] );
	}

	switch( m_iProgressDirection )
	{
	case PROGRESS_EAST:
		( surface()->*func )( x, y, x + (int)( wide * _progress ), y + tall );
		break;

	case PROGRESS_WEST:
		( surface()->*func )( x + (int)( wide * ( 1.0f - _progress ) ), y, x + wide, y + tall );
		break;

	case PROGRESS_NORTH:
		( surface()->*func )( x, y + (int)( tall * ( 1.0f - _progress ) ), x + wide, y + tall );
		break;

	case PROGRESS_SOUTH:
		( surface()->*func )( x, y, x + wide, y + (int)( tall * _progress ) );
		break;
	}
}