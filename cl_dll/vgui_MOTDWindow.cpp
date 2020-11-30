#include "VGUI_Font.h"
#include "VGUI_ScrollPanel.h"
#include "VGUI_TextImage.h"

#include<VGUI_StackLayout.h>

#include "hud.h"
#include "cl_util.h"
#include "camera.h"
#include "kbutton.h"
#include "const.h"

#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_ServerBrowser.h"

#include "buymenu.h" // HLWE-styled MOTD...

#define MOTD_TITLE_X		XRES(16)
#define MOTD_TITLE_Y		YRES(16)

#define MOTD_WINDOW_X				XRES(112)
#define MOTD_WINDOW_Y				YRES(80)
#define MOTD_WINDOW_SIZE_X			XRES(424)
#define MOTD_WINDOW_SIZE_Y			YRES(312)

//-----------------------------------------------------------------------------
// Purpose: Displays the MOTD and basic server information
//-----------------------------------------------------------------------------
class CMessageWindowPanel : public CMenuPanel
{
public:
	CMessageWindowPanel( const char *szMOTD, const char *szTitle, int iShadeFullScreen, int iRemoveMe, int x, int y, int wide, int tall );
private:
	CTransparentPanel *m_pBackgroundPanel;
	void paint();
};

//-----------------------------------------------------------------------------
// Purpose: Creates a new CMessageWindowPanel
// Output : CMenuPanel - interface to the panel
//-----------------------------------------------------------------------------
CMenuPanel *CMessageWindowPanel_Create( const char *szMOTD, const char *szTitle, int iShadeFullscreen, int iRemoveMe, int x, int y, int wide, int tall )
{
	return new CMessageWindowPanel( szMOTD, szTitle, iShadeFullscreen, iRemoveMe, x, y, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Constructs a message panel
//-----------------------------------------------------------------------------
CMessageWindowPanel::CMessageWindowPanel( const char *szMOTD, const char *szTitle, int iShadeFullscreen, int iRemoveMe, int x, int y, int wide, int tall ) : CMenuPanel( iShadeFullscreen ? 100 : 255, iRemoveMe, x, y, wide, tall )
{
	// Get the scheme used for the Titles
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();

	// schemes
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
	SchemeHandle_t hMOTDText = pSchemes->getSchemeHandle( "Briefing Text" );
	setPaintBackgroundEnabled(false);


	// Create the window
	m_pBackgroundPanel = new CTransparentPanel( iShadeFullscreen ? 255 : 100, MOTD_WINDOW_X, MOTD_WINDOW_Y, MOTD_WINDOW_SIZE_X, MOTD_WINDOW_SIZE_Y );
	m_pBackgroundPanel->setParent( this );
	m_pBackgroundPanel->setVisible( true );

	int iXSize,iYSize;
	getSize( iXSize,iYSize );

	// Create the title
	Label *pLabel = new Label( "", MOTD_TITLE_X, MOTD_TITLE_Y );
	pLabel->setParent( this );
	pLabel->setFont( pSchemes->getFont(hTitleScheme) );


	pLabel->setFgColor( 255, 255, 255, 0 );
	pLabel->setContentAlignment( vgui::Label::a_west );
	pLabel->setText(szTitle);
	pLabel->setPaintBackgroundEnabled(false);

	// Create the Scroll panel
	ScrollPanel *pScrollPanel = new CHLWEScrollPanel( XRES(16), MOTD_TITLE_Y*2 + YRES(16), iXSize - XRES(32), iYSize - (YRES(48) + BUTTON_SIZE_Y*2) );
	pScrollPanel->setParent(this);
	
	//force the scrollbars on so clientClip will take them in account after the validate
	pScrollPanel->setScrollBarAutoVisible(false, false);
	pScrollPanel->setScrollBarVisible(true, true);
	pScrollPanel->validate();

	// Create the text panel
	TextPanel *pText = new TextPanel( "", 0,0, 64,64);
	pText->setParent( pScrollPanel->getClient() );

	// get the font and colors from the scheme
	pText->setFont( pSchemes->getFont(hMOTDText) );
	pText->setFgColor( 255, 170, 0, 0 );
	pText->setText(szMOTD);
	pText->setPaintBackgroundEnabled(false);

	// Get the total size of the MOTD text and resize the text panel
	int iScrollSizeX, iScrollSizeY;

	// First, set the size so that the client's wdith is correct at least because the
	//  width is critical for getting the "wrapped" size right.
	// You'll see a horizontal scroll bar if there is a single word that won't wrap in the
	//  specified width.
	pText->getTextImage()->setSize(pScrollPanel->getClientClip()->getWide(), pScrollPanel->getClientClip()->getTall());
	pText->getTextImage()->getTextSizeWrapped( iScrollSizeX, iScrollSizeY );
	
	// Now resize the textpanel to fit the scrolled size
	pText->setSize( iScrollSizeX , iScrollSizeY );

	//turn the scrollbars back into automode
	pScrollPanel->setScrollBarAutoVisible(true, true);
	pScrollPanel->setScrollBarVisible(false, false);

	pScrollPanel->setBgColor(0, 0, 0, 55);
	pScrollPanel->setPaintBackgroundEnabled(true);
	pScrollPanel->setBorder( new LineBorder( Color(236,236,236,0) ) );

	pScrollPanel->validate();

	CHLWEButton *pButton = new CHLWEButton( CHudTextMessage::BufferedLocaliseTextString( "  #Menu_OK" ), XRES(16), iYSize - YRES(16) - BUTTON_SIZE_Y/*, CMENU_SIZE_X, BUTTON_SIZE_Y*/);
	pButton->addActionSignal(new CMenuHandler_TextWindow(HIDE_TEXTWINDOW));
	pButton->setParent(this);
}

void CMessageWindowPanel::paint ()
{
	//Background
    drawSetColor(0, 0, 0, 127);
    drawFilledRect(0, 0, getWide(), getTall());

	//Bright borderlines
	drawSetColor(236, 236, 236, 0);
	drawFilledRect(0, 0, XRES(1), getTall());
	drawFilledRect(0, 0, getWide(), YRES(1));

	//Dark borderlines
	drawSetColor(112, 112, 112, 0);
	drawFilledRect(0, getTall() - YRES(1), getWide(), getTall());
	drawFilledRect(getWide() - XRES(1), 0, getWide(), getTall());
}