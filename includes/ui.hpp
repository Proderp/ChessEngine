#include "includes.hpp"
#include "ui-elements.hpp"

class UI {
private:
    sf::RenderWindow& window;
    const sf::Vector2f& windowSize;

    sf::View boardView;
	sf::View uiView;
	sf::View historyView;

	const UIConsts uiConsts;

    Button resignationButton;
    
    PromoMenuLayout promoMenuLayout;	
	ResignationConfirmationLayout resignationConfirmationLayout;
	
    MainMenuLayout mainMenuLayout;
	PrePlayLayout prePlayLayout;
	SettingsLayout settingsLayout;
	GameOverLayout gameOverLayout;

	sf::FloatRect historyViewport;
    HistoryViewportMetrics historyViewportMetrics;
	BoardMetrics boardMetrics;

	void recalibrateViews();

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

	void setBoardSizes();
	void setOffsets();
	void setBoardEdges();
	void setHistoryViewport();

public:
    UI(sf::RenderWindow& window, const sf::Vector2f& windowSize);

    void updateUI();

	const float getMargin() const;

	void setUIView();
	void setHistoryView();
	void setBoardView();

	const sf::View& getUIView();	
	const sf::View& getHistoryView();
	const sf::View& getBoardView();

	void moveHistoryView(const sf::Vector2f position);
	void setHistoryViewCenter(const sf::Vector2f center);

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