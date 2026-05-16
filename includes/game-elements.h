#pragma once
#include "includes.h"

struct Piece {
	Side side;
	PieceType type;
	bool hasMoved;

	Piece();

	Piece(Side side, PieceType type, bool hasMoved);
};
 
struct Square {
	Colour colour;
	std::optional<Piece> piece;

	Square();

	Square(Colour colour);
};

struct Move {
	int fromRank, fromFile;
	int toRank, toFile;

	std::optional<PieceType> promoteInto;

	int score;
	bool isLoudMove{ false };
};

struct Material {
	int pawns{};
	int knights{};
	int lightSquaredBishops{};
	int darkSquaredBishops{};
	int rooks{};
	int queens{};
};

struct BoardMaterial {
	Material white;
	Material black;
};

struct PromoMenuLayout {
	sf::Vector2f panelPosition;
	sf::Vector2f panelSize;
	std::array<sf::FloatRect, 4> slotRects;
};

struct Turn {
	int moveNumber{};
	std::string whiteMove;
	std::string blackMove;
};

struct Button {
	Button();
	sf::Vector2f position;
	sf::Vector2f size;
	sf::FloatRect bounds;
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

struct PlayerChoices {
	Side sideChoice{ Side::White };
	OpponentType opponentChoice{ OpponentType::Engine };
	TimeControl timeControlChoice{ TimeControl::Blitz };
	int timeIncrement{ timeIncrementChoices.at(0) };
};

struct Settings {
	bool fullScreen{ false };
	bool showLegalMoves{ true };
	bool autoPromoteToQueen{ false };
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

struct UIContainer {
	PromoMenuLayout promoMenuLayout;	
	Button resignationButton;
	ResignationConfirmationLayout resignationConfirmationLayout;
	MainMenuLayout mainMenuLayout;
	PrePlayLayout prePlayLayout;
	SettingsLayout settingsLayout;
	GameOverLayout gameOverLayout;
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

struct MoveSnapshot {
	int previousEnPassantRank;
	int previousEnPassantFile;
	bool pieceHasAlreadyMoved;
	std::size_t previousHalfmoveCounter;
	int previousEvaluation;
};

struct OpeningBookMove {
	std::string uciMove;
	int weight{};
};

struct SyzygyBoard {
	uint64_t white = 0, black = 0, kings = 0, queens = 0, rooks = 0, bishops = 0, knights = 0, pawns = 0;
};

struct AnimationState {
	sf::Vector2f startPixel;
	sf::Vector2f endPixel;

	float elapsedSeconds = 0.f;
	float durationSeconds = animationDuration;

	Piece movingPiece = Piece(Side::White, PieceType::Pawn, false);

	int toRank = INVALID_COORD;
	int toFile = INVALID_COORD;
};