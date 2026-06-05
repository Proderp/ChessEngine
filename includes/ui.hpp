#include "includes.hpp"
#include "ui-elements.hpp"

class UI {
private:


public:
    Button resignationButton;
    
    PromoMenuLayout promoMenuLayout;	
	ResignationConfirmationLayout resignationConfirmationLayout;
	
    MainMenuLayout mainMenuLayout;
	PrePlayLayout prePlayLayout;
	SettingsLayout settingsLayout;
	GameOverLayout gameOverLayout;

    HistoryViewportMetrics historyViewportMetrics;
	BoardMetrics boardMetrics;
};