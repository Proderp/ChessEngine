#pragma once
#include "game.hpp"
#include "render.hpp"
#include "includes.hpp"

class App {
private:
	sf::RenderWindow window;
	GameState currentState = GameState::MainMenu;

	Game game;
	Render renderer;
	
	sf::Clock frameClock;

	PlayerChoices playerChoices;
	Settings settings;

	bool isDragging = false;
	sf::Vector2f currentMousePosition;

	bool isAnimating = false;
	AnimationState currentAnimation;
	sf::Vector2f currentAnimatedPosition;
	bool wasJustDragged = false;

	bool hasSecondaryAnimation = false;
	AnimationState secondaryAnimation;
	sf::Vector2f secondaryAnimatedPosition;

	sf::Image windowIcon;
	
	void handleMouseClick(const sf::Event::MouseButtonPressed& mouseEvent);

	void handleLeftClick(const sf::Event::MouseButtonPressed& mouseEvent);
	void handleLeftClickMainMenu(const sf::Event::MouseButtonPressed& mouseEvent);
	void handleLeftClickPrePlay(const sf::Event::MouseButtonPressed& mouseEvent);
	void handleLeftClickSettings(const sf::Event::MouseButtonPressed& mouseEvent);
	void handleLeftClickPlaying(const sf::Event::MouseButtonPressed& mouseEvent);
	void handleLeftClickGameOver(const sf::Event::MouseButtonPressed& mouseEvent);
	void handleClickOnBoard(const sf::Vector2i& mousePosition);
	void startAnimation(const Move& lastMove);

	void handleResignation(const sf::Vector2f uiPos);
	std::optional<PieceType> promoMenuPick(const sf::Vector2f& uiPos, const std::optional<Side>& promotionSide) const;
	void handlePromotion(const sf::Vector2f uiPos);

	void handleScroll(const sf::Event::MouseWheelScrolled& scrollEvent);
	void limitScroll();
	void scrollToBottom();

	void handleMouseRelease(const sf::Event::MouseButtonReleased& mouseReleased);

	void handleEscapeButton();
	void fullScreenToggle();

	void renderMainMenu();

	void renderPrePlay();

	void renderSettings();

	void renderPlayingState();
	void renderBoardView();
	void renderHistoryView();
	void renderUIView();

	void renderGameOverState();
public:

	App();

	void run();
	
	void handleEvents();
	
	void updateTimers(const float deltaTime);
	void handleAnimation(const float deltaTime);

	void render();
};