#include "includes.hpp"
#include "ui-elements.hpp"

class UI {
private:
	void resizeResignationButton();
	
	void resizeResignationConfirmationLayout();
	void resizeLayoutButtons();
	void resizeLayoutButtonBounds();

	void resizePromoLayout();
	void recalculateMoveHistorySize();

	void resizeMainMenuLayout();
	void resizeButtonBounds(Button& button);

	void resizePrePlayLayout();
	void resizeSettingsLayout();

	void resizeGameOverLayout();

public:
    const sf::Vector2f& windowSize;

    UI(const sf::Vector2f& windowSize);

    void setUIElements();

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