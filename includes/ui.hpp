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

	void recalibrateViews(const sf::Vector2f newWindowSize);

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

	void setUIView();
	void setHistoryView();
	void setBoardView();

	const sf::View& getUIView();	
	const sf::View& getHistoryView();
	const sf::View& getBoardView();

	const Button& getResignationButton();

	const PromoMenuLayout& getPromoMenuLayout();
	const ResignationConfirmationLayout& getResignationConfirmationLayout();

	const MainMenuLayout& getMainMenuLayout();
	const PrePlayLayout& getPrePlayLayout();
	const SettingsLayout& getSettingsLayout();
	const GameOverLayout& getGameOverLayout();

	const HistoryViewportMetrics& getHistoryViewportMetrics();
	const BoardMetrics& getBoardMetrics();
};