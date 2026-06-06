#include "includes.hpp"
#include "ui-elements.hpp"

class UI {
private:
    sf::RenderWindow& window;
    const sf::Vector2f& windowSize;

    sf::View boardView;
	sf::View uiView;
	sf::View historyView;

    Button resignationButton;
    
    PromoMenuLayout promoMenuLayout;	
	ResignationConfirmationLayout resignationConfirmationLayout;
	
    MainMenuLayout mainMenuLayout;
	PrePlayLayout prePlayLayout;
	SettingsLayout settingsLayout;
	GameOverLayout gameOverLayout;

    HistoryViewportMetrics historyViewportMetrics;
	BoardMetrics boardMetrics;

	const float margin{16.f};
	const float padding{10.f};
	const float spacing{10.f};

    void handleResize(const unsigned int windowWidth, const unsigned int windowHeight);
	void recalibrateViews(const unsigned int windowWidth, const unsigned int windowHeight);

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
    UI(sf::RenderWindow& window, const sf::Vector2f& windowSize);

    void updateUI();
};