#pragma once
#include "includes.hpp"
#include "game.hpp"

class Render {
private:
	const ColourContainer colours;
		
	sf::Font font;
	std::array<sf::Texture, 3> sideChoiceTextures;
	std::array<sf::Texture, 2> opponentChoiceTextures;
	std::array<sf::Texture, 3> timeControlChoiceTextures;
	std::array<std::array<sf::Texture, 6>, 2> pieceTextures;

	unsigned int titleCharSize;
	unsigned int menuButtonCharSize;
	sf::Text mainMenuTitleText{ font, "Chess" };
	sf::RectangleShape mainMenuButtonRect;
	sf::Text playButtonText{ font, "Play" };
	sf::Text settingsButtonText{ font, "Settings" };
	sf::Text quitButtonText{ font, "Quit" };

	float prePlayTextXPosition;
	unsigned int prePlayAndSettingsCharSize;
	sf::RectangleShape prePlayButtonRect;
	sf::Text prePlayText{ font, "" };
	sf::Text incrementText{ font, "" };

	float settingsTextXPosition;
	sf::RectangleShape settingsButtonRect;
	sf::Text settingsText{font, ""};
	sf::Text toggleText{font, ""};

	sf::RectangleShape tintedGlassRect;
	sf::RectangleShape gameOverLayoutBackground;
	sf::RectangleShape gameOverButtonRect;
	sf::Text gameOverText{ font, "" };

	unsigned int rankAndFileCharSize;

	std::stringstream timeFormatter;

	sf::Text whiteTimerText{ font, "" };
	sf::Text blackTimerText{ font, "" };

	sf::RectangleShape whiteTimerBackground;
	sf::RectangleShape blackTimerBackground;

	sf::RectangleShape boardSquareRect;
	sf::CircleShape moveCircle;

	sf::Text stamp{ font, "" };

	unsigned int areYouSureCharSize = static_cast<unsigned int>(historyViewportMetrics.historyViewWidth * 0.1f);
	unsigned int buttonCharSize = areYouSureCharSize / 2;
	sf::Text areYouSureText{ font, "Are you sure you\nwant to resign?", areYouSureCharSize };
	sf::Text cancelText{ font, "Cancel", buttonCharSize };
	sf::Text confirmText{ font, "Confirm", buttonCharSize };

	float panelWidth{};
	float panelHeight{};

	sf::RectangleShape promoMenuBackground;
	sf::RectangleShape slotRect;

	std::array<sf::Text, RANKS> rankText = {
		sf::Text{font, "8"},
		sf::Text{font, "7"},
		sf::Text{font, "6"},
		sf::Text{font, "5"},
		sf::Text{font, "4"},
		sf::Text{font, "3"},
		sf::Text{font, "2"},
		sf::Text{font, "1"}
	};

	std::array<sf::Text, FILES> fileText = {
		sf::Text{font, "a"},
		sf::Text{font, "b"},
		sf::Text{font, "c"},
		sf::Text{font, "d"},
		sf::Text{font, "e"},
		sf::Text{font, "f"},
		sf::Text{font, "g"},
		sf::Text{font, "h"}
	};

	float scaleValue;

	sf::RenderWindow& window;

	std::unordered_map<PieceType, int> whiteGraveyardCount;
	std::unordered_map<PieceType, int> blackGraveyardCount;

	void loadTextures();

	void loadFont();

	void setWindowInfo(const unsigned int windowWidth, const unsigned int windowHeight);
	void setBoardSizes();
	void setOffsets();
	void setBoardEdges();
	void setHistoryViewport();
	void setTimerText();
	void setTimerBackgrounds();
	void setMoveCircle();
	void setMoveHistoryText();
	void setPromoMenuBackground();
	void setPromoMenuSlots();
	void setRankAndFiles();
	void setResignationConfirmationText();
	void setMainMenuText();
	void setPrePlayPage();
	void setSettingsPage();
	void setGameOverLayout();

	int calculateMaterialAdvantage(const BoardMaterial& boardMaterial);

	void setTextOrigin(sf::Text& text);

	float windowWidth = DEFAULT_WINDOW_WIDTH;
	float windowHeight = DEFAULT_WINDOW_HEIGHT;
	
	float moveHistorySize{};
	float rowDistance{};

	sf::FloatRect historyViewport;
public:
	Side playerSide;

	const BoardMetrics& getBoardMetrics() const;
	const HistoryViewportMetrics& getHistoryViewportMetrics() const;

	const sf::FloatRect& getHistoryViewport() const;

	float getWindowWidth() const;
	float getWindowHeight() const;
	float getMargin() const;
	float getPadding() const;
	float getSpacing() const;

	float getMoveHistorySize() const;
	void setMoveHistorySize(const float newSize);

	float getRowDistance() const;
	void setRowDistance(const float newDistance);

	Render(sf::RenderWindow& window, const Side playerSide);

	void setInfo(const unsigned int windowWidth, const unsigned int windowHeight);

	void drawBoard(const std::array<std::array<Square, 8>, 8>& board, const int selectedRank, const int selectedFile, const std::optional<Move>& lastMove);

	void drawSinglePiece(const Piece& piece, const int rank, const int file, const sf::Vector2f mousePosition = sf::Vector2f{INVALID_COORD, INVALID_COORD});

	void drawPieces(
		const std::array<std::array<Square, 8>, 8>& board, 
		const bool isDragging, const sf::Vector2f mousePosition, 
		const int selectedRank, const int selectedFile, 
		const bool isAnimating, 
		const int animationRank, const int animationFile,
		const sf::Vector2f animationPosition,
		const bool hasSecondaryAnimation,
		const int secAnimRank, const int secAnimFile,
		const sf::Vector2f secAnimPosition);

	void drawLegalMoves(const std::vector<Move>& currentMoves);

	void drawPromoMenu(const std::optional<Side>& promotionSide);

	void drawRanksAndFiles(const Side currentTurn);

	void drawTimers(const float whiteTimeInSeconds, const float blackTimeInSeconds);

	void drawGraveyards(const BoardMaterial& boardMaterial);

	void updateGraveyardCounts(const std::vector<Piece>& whiteGraveyard, const std::vector<Piece>& blackGraveyard);

	void drawMoveHistory(const std::vector<Turn>& turns);

	void drawResignationButton(const Button& resignationButton);

	void drawResignationConfirmation(const ResignationConfirmationLayout& resignationConfirmationLayout);

	void drawMainMenu(const MainMenuLayout& mainMenuLayout);

	void drawPrePlayPage(const PrePlayLayout& prePlayLayout, const PlayerChoices& choices);

	void drawSettingsPage(const SettingsLayout& settingsLayout, const Settings& settings);

	void drawGameOverLayout(const GameOverLayout& gameOverLayout, const std::optional<GameOverType> reason, const std::optional<Side> winner);
};