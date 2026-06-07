#include "includes.hpp"

struct UIConsts {
	const float margin{16.f};
	const float padding{10.f};
	const float spacing{10.f};
};

struct Button {
	Button() {};
	sf::Vector2f position;
	sf::Vector2f size;
	sf::FloatRect bounds;
};

struct PromoMenuLayout {
	sf::Vector2f panelPosition;
	sf::Vector2f panelSize;
	std::array<sf::FloatRect, 4> slotRects;
};

struct ResignationConfirmationLayout {
	sf::Vector2f position;
	sf::Vector2f size;
	Button cancelButton;
	Button confirmButton;
};

struct MainMenuLayout {
	Button playButton;
	Button settingsButton;
	Button quitButton;
};

struct PrePlayLayout {
private:
	Button whiteSideButton;
	Button blackSideButton;
	Button randomSideButton;

	Button aiButton;
	Button humanButton;

	Button bulletButton;
	Button blitzButton;
	Button rapidButton;

	Button zeroButton;
	Button oneButton;
	Button twoButton;
	Button fiveButton;
	Button tenButton;

public:
	std::array<Button*, 3> sideButtons = { &whiteSideButton, &blackSideButton, &randomSideButton };

	std::array<Button*, 2> opponentButtons = { &aiButton, &humanButton };

	std::array<Button*, 3> timeControlButtons = { &bulletButton, &blitzButton, &rapidButton };

	std::array<Button*, 5> incrementButtons = { &zeroButton, &oneButton, &twoButton, &fiveButton, &tenButton };
	
	Button startGameButton;
	Button backButton;
};

struct SettingsLayout {
private:
	Button fullScreenToggle;
	Button legalMovesToggle;
	Button autoPromoteToggle;

public:
	std::array<Button*, 3> toggleButtons = { &fullScreenToggle, &legalMovesToggle, &autoPromoteToggle };

	Button backButton;
};

struct GameOverLayout {
	sf::Vector2f position;
	sf::Vector2f size;

	Button mainMenuButton;
	Button playAgainButton;
};

struct ColourContainer {
	const sf::Color lightColour = sf::Color(255, 255, 255);
	const sf::Color darkColour = sf::Color(251, 194, 115);
	const sf::Color highlightColour = sf::Color(240, 217, 135, 100);
	const sf::Color legalCirclesColour = sf::Color(100, 149, 237, 100);
	const sf::Color lastMoveFromColour = sf::Color(100, 150, 100);
	const sf::Color lastMoveToColour = sf::Color(100, 160, 100);
	const sf::Color transparentWhite = sf::Color(255, 255, 255, 18);
	const sf::Color promoWindowBGColour = sf::Color(50, 50, 50, 200);
	const sf::Color promoWindowOutlineColour = sf::Color(0, 0, 0, 120);
	const sf::Color slotOutlineColour = sf::Color(0, 100, 0, 120);
	const sf::Color grayButtonColour = sf::Color(60, 60, 60);
	const sf::Color greenColour = sf::Color(50, 150, 50);
	const sf::Color redColour = sf::Color(150, 50, 50);
};

struct BoardMetrics {
	float boardSize{};
	float offsetX{};
	float offsetY{};

	float boardRightEdge{};
	float boardTopEdge{};
	float tileSize{};

	float circleSize{};
};

struct HistoryViewportMetrics {
	float historyViewStartX{};
	float historyViewStartY{};

	float historyViewWidth{};
	float historyViewHeight{};

	float viewportXPercentage{};
	float viewportYPercentage{};

	float viewportWidthPercentage{};
	float viewportHeightPercentage{};
};