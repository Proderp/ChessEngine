#include "render.hpp"

Render::Render(const BoardMetrics& boardMetrics, sf::RenderWindow& window, const Side playerSide) : 
	boardMetrics(boardMetrics),
	window(window), 
	playerSide(playerSide)
{
	loadTextures();
	loadFont();
	setInfo(window.getSize().x, window.getSize().y);
}

void Render::loadTextures() {
	const std::array<std::string, 2> sideNames = { "white", "black" };
	const std::array<std::string, 6> typeNames = { "pawn", "knight", "bishop", "rook", "queen", "king" };

	for (std::size_t side = 0; side < sideNames.size(); ++side) {
		for (std::size_t type = 0; type < typeNames.size(); ++type) {

			const std::string fileName = "assets/images/" + sideNames[side] + "-" + typeNames[type] + ".png";

			if (!pieceTextures[side][type].loadFromFile(fileName)) {
				std::cerr << "Error: Couldn't load " << fileName << std::endl;
				continue;
			}

			pieceTextures[side][type].setSmooth(true);
		}
	}

	const std::array<std::string, 3> sideIconNames = { "white-king", "black-king", "random-icon"};
	for (std::size_t i = 0; i < sideIconNames.size(); i++) {
		const std::string fileName = "assets/images/" + sideIconNames[i] + ".png";

		if (!sideChoiceTextures[i].loadFromFile(fileName)) {
			std::cerr << "Error: Couldn't load " << fileName << std::endl;
			continue;
		}
		
		sideChoiceTextures[i].setSmooth(true);
	}

	if (!opponentChoiceTextures[0].loadFromFile("assets/images/engine-icon.png")) {
		std::cerr << "Error: Couldn't load icon" << std::endl;
		opponentChoiceTextures[0].setSmooth(true);
	}
	if (!opponentChoiceTextures[1].loadFromFile("assets/images/human-icon.png")) {
		std::cerr << "Error: Couldn't load icon" << std::endl;
		opponentChoiceTextures[1].setSmooth(true);
	}

	const std::array<std::string, 3> timeControlNames = { "bullet", "blitz", "rapid" };
	for (std::size_t i = 0; i < timeControlNames.size(); i++) {
		const std::string fileName = "assets/images/" + timeControlNames[i] + "-icon.png";

		if (!timeControlChoiceTextures[i].loadFromFile(fileName)) {
			std::cerr << "Error: Couldn't load " << fileName << std::endl;
			continue;
		}

		timeControlChoiceTextures[i].setSmooth(true);
	}
}

void Render::loadFont() {
	if (!font.openFromFile("assets/fonts/IosevkaCharonMono-Medium.ttf")) {
		std::cerr << "Error: Couldn't load font!" << std::endl;
	}
}

void Render::setInfo(const unsigned int windowWidth, const unsigned int windowHeight) {
	setWindowInfo(windowWidth, windowHeight);
	setBoardSizes();
	setOffsets();
	setBoardEdges();
	setHistoryViewport();
	setTimerText();
	setTimerBackgrounds();
	setMoveCircle();
	setMoveHistoryText();
	setPromoMenuBackground();
	setPromoMenuSlots();
	setRankAndFiles();
	setResignationConfirmationText();
	setMainMenuText();
	setPrePlayPage();
	setSettingsPage();
	setGameOverLayout();

	boardSquareRect.setSize({ boardMetrics.tileSize, boardMetrics.tileSize });
}

void Render::setWindowInfo(const unsigned int windowWidth, const unsigned int windowHeight) {
	this->windowWidth = static_cast<float>(windowWidth);
	this->windowHeight = static_cast<float>(windowHeight);
}

void Render::setTimerText() {
	whiteTimerText.setFillColor(sf::Color::Black);
	blackTimerText.setFillColor(sf::Color::Black);

	const unsigned int timerFontSize = static_cast<unsigned int>(boardMetrics.tileSize * 0.8f);
	whiteTimerText.setCharacterSize(timerFontSize);
	blackTimerText.setCharacterSize(timerFontSize);
}
void Render::setTimerBackgrounds() {
	const sf::Vector2f backgroundSize = { boardMetrics.tileSize * 2.5f, boardMetrics.tileSize };

	whiteTimerBackground.setSize(backgroundSize);
	whiteTimerBackground.setFillColor(colours.lightColour);
	whiteTimerBackground.setOrigin({ 0.f, backgroundSize.y / 2.f });

	blackTimerBackground.setSize(backgroundSize);
	blackTimerBackground.setFillColor(colours.darkColour);
	blackTimerBackground.setOrigin({ 0.f, backgroundSize.y / 2.f });
}
void Render::setMoveCircle() {
	moveCircle.setRadius(boardMetrics.circleSize);
	moveCircle.setOrigin({ boardMetrics.circleSize, boardMetrics.circleSize });
	moveCircle.setFillColor(colours.legalCirclesColour);
}
void Render::setMoveHistoryText() {
	const unsigned int moveHistoryCharSize = static_cast<unsigned int>(boardMetrics.tileSize / 2.f);
	stamp.setCharacterSize(moveHistoryCharSize);
}
void Render::setPromoMenuBackground() {
	panelWidth = boardMetrics.tileSize + (padding * 2.f);
	panelHeight = (static_cast<float>(promotionPieceTypes.size()) * boardMetrics.tileSize) + (padding * 2.f) + ((static_cast<float>(promotionPieceTypes.size()) - 1.f) * spacing);

	promoMenuBackground.setSize({ panelWidth, panelHeight });
	promoMenuBackground.setFillColor(colours.promoWindowBGColour);
	promoMenuBackground.setOutlineColor(colours.promoWindowOutlineColour);
	promoMenuBackground.setOutlineThickness(2.f);
}
void Render::setPromoMenuSlots() {
	slotRect.setSize({ boardMetrics.tileSize, boardMetrics.tileSize });
	slotRect.setFillColor(colours.transparentWhite);
	slotRect.setOrigin({ boardMetrics.tileSize / 2.f, boardMetrics.tileSize / 2.f });
	slotRect.setOutlineColor(colours.slotOutlineColour);
	slotRect.setOutlineThickness(1.f);
}
void Render::setRankAndFiles() {
	rankAndFileCharSize = static_cast<unsigned int>(boardMetrics.tileSize) / 3;

	auto formatText = [&](sf::Text& text, const float xPosition, const float yPosition) {
		text.setCharacterSize(rankAndFileCharSize);
		setTextOrigin(text);
		text.setPosition({ xPosition, yPosition });
	};

	for (int i = 0; i < RANKS; i++) {
		formatText(rankText[i], boardMetrics.offsetX - boardMetrics.tileSize, boardMetrics.offsetY + (i * boardMetrics.tileSize));
		formatText(fileText[i], boardMetrics.offsetX + (i * boardMetrics.tileSize), boardMetrics.offsetY + boardMetrics.boardSize);
	}
}
void Render::setResignationConfirmationText() {
	areYouSureCharSize = static_cast<unsigned int>(historyViewportMetrics.historyViewWidth * 0.1f);
	buttonCharSize = areYouSureCharSize / 2;

	areYouSureText.setCharacterSize(areYouSureCharSize);
	cancelText.setCharacterSize(buttonCharSize);
	confirmText.setCharacterSize(buttonCharSize);
}
void Render::setMainMenuText() {
	titleCharSize = static_cast<unsigned int>(boardMetrics.tileSize * 2.5f);
	mainMenuTitleText.setCharacterSize(titleCharSize);

	setTextOrigin(mainMenuTitleText);

	const sf::Vector2f titlePosition = { windowWidth / 2.f, windowHeight / 4.f };
	mainMenuTitleText.setPosition(titlePosition);

	menuButtonCharSize = static_cast<unsigned int>(boardMetrics.tileSize / 2.f);
	playButtonText.setCharacterSize(menuButtonCharSize);
	setTextOrigin(playButtonText);

	settingsButtonText.setCharacterSize(menuButtonCharSize);
	setTextOrigin(settingsButtonText);

	quitButtonText.setCharacterSize(menuButtonCharSize);
	setTextOrigin(quitButtonText);
}

void Render::setPrePlayPage() {
	prePlayTextXPosition = windowWidth * 0.33f;
	prePlayAndSettingsCharSize = static_cast<unsigned int>(boardMetrics.tileSize / 2.f);
}

void Render::setSettingsPage() {
	settingsTextXPosition = windowWidth * 0.45f;
}

void Render::setGameOverLayout() {
	tintedGlassRect.setSize({ windowWidth, windowHeight });
	tintedGlassRect.setFillColor(sf::Color(0, 0, 0, 150));
}

void Render::drawBoard(const std::array<std::array<Square, 8>, 8> &board, const int selectedRank, const int selectedFile, const std::optional<Move>& lastMove) {
	for (int rank = 0; rank < RANKS; rank++) {
		for (int file = 0; file < FILES; file++) {

			const Square &square = board[rank][file];

			const int drawnRank = (playerSide == Side::Black) ? 7 - rank : rank;
			const int drawnFile = (playerSide == Side::Black) ? 7 - file : file;

			boardSquareRect.setFillColor((square.colour == Colour::Light) ? colours.lightColour : colours.darkColour);
			if (rank == selectedRank and file == selectedFile)
				boardSquareRect.setFillColor(colours.highlightColour);

			if (lastMove.has_value()) {
				if (rank == lastMove->fromRank and file == lastMove->fromFile)
					boardSquareRect.setFillColor(colours.lastMoveFromColour);

				if (rank == lastMove->toRank and file == lastMove->toFile)
					boardSquareRect.setFillColor(colours.lastMoveToColour);
			}

			boardSquareRect.setOrigin({ boardMetrics.tileSize / 2.f, boardMetrics.tileSize / 2.f });
			boardSquareRect.setPosition({ boardMetrics.offsetX + (drawnFile * boardMetrics.tileSize), boardMetrics.offsetY + (drawnRank * boardMetrics.tileSize) });
			window.draw(boardSquareRect);
		}
	}
}

void Render::drawSinglePiece(const Piece& piece, const int rank, const int file, const sf::Vector2f mousePosition) {

	const int sideIndex = static_cast<int>(piece.side);
	const int typeIndex = static_cast<int>(piece.type);

	sf::Sprite pieceSprite(pieceTextures[sideIndex][typeIndex]);

	float rawSpriteWidth = pieceSprite.getLocalBounds().size.x;

	scaleValue = (boardMetrics.tileSize * 0.9f / rawSpriteWidth);
	pieceSprite.setScale({ scaleValue, scaleValue });

	float spriteWidth = pieceSprite.getLocalBounds().size.x;
	float spriteHeight = pieceSprite.getLocalBounds().size.y;
	pieceSprite.setOrigin({ spriteWidth / 2.f, spriteHeight / 2.f });

	const int drawnRank = (playerSide == Side::Black) ? 7 - rank : rank;
	const int drawnFile = (playerSide == Side::Black) ? 7 - file : file;

	if (mousePosition.x == INVALID_COORD) {
		pieceSprite.setPosition({ boardMetrics.offsetX + (drawnFile * boardMetrics.tileSize), boardMetrics.offsetY + (drawnRank * boardMetrics.tileSize)});
	}
	else {
		pieceSprite.setPosition(mousePosition);
	}

	window.draw(pieceSprite);
}

void Render::drawPieces(
	const std::array<std::array<Square, 8>, 8>& board,
	const bool isDragging, const sf::Vector2f mousePosition,
	const int selectedRank, const int selectedFile,
	const bool isAnimating,
	const int animationRank, const int animationFile,
	const sf::Vector2f animationPosition,
	const bool hasSecondaryAnimation,
	const int secAnimRank, const int secAnimFile,
	const sf::Vector2f secAnimPosition)
{
	for (int rank = 0; rank < RANKS; rank++) {
		for (int file = 0; file < FILES; file++) {
			const auto& piece = board[rank][file].piece;
			if (piece.has_value()) {
				if (isAnimating and rank == animationRank and file == animationFile) {
					drawSinglePiece(piece.value(), rank, file, animationPosition);
				}
				else if (hasSecondaryAnimation and rank == secAnimRank and file == secAnimFile) {
					drawSinglePiece(piece.value(), rank, file, secAnimPosition);
				}
				else if (isDragging and rank == selectedRank and file == selectedFile) {
					drawSinglePiece(piece.value(), rank, file, mousePosition);
				}
				else {
					drawSinglePiece(piece.value(), rank, file);
				}
			}
		}
	}
}

void Render::drawLegalMoves(const std::vector<Move>& currentMoves) {
	for (const Move& move : currentMoves) {
		const int drawnRank = (playerSide == Side::Black) ? 7 - move.toRank : move.toRank;
		const int drawnFile = (playerSide == Side::Black) ? 7 - move.toFile : move.toFile;
		moveCircle.setPosition({ boardMetrics.offsetX + (drawnFile * boardMetrics.tileSize), boardMetrics.offsetY + (drawnRank * boardMetrics.tileSize) });
		window.draw(moveCircle);
	}
}

void Render::drawPromoMenu(const std::optional<Side>& promoSide) {
	if (!promoSide.has_value()) 
		return;

	const int sideIndex = static_cast<int>((promoSide.value() == Side::White) ? Side::White : Side::Black);

	promoMenuBackground.setPosition({ boardMetrics.boardRightEdge + margin, boardMetrics.boardTopEdge });
	window.draw(promoMenuBackground);

	const float panelX = promoMenuBackground.getPosition().x;
	const float panelY = promoMenuBackground.getPosition().y;
	const float centerX = panelX + (panelWidth / 2.f);

	for (std::size_t i{ 0 }; i < promotionPieceTypes.size(); ++i) {
		const float centerY = panelY + padding + (boardMetrics.tileSize / 2.f) + (static_cast<float>(i) * (boardMetrics.tileSize + spacing));

		slotRect.setPosition({ centerX, centerY });
		window.draw(slotRect);

		const int typeIndex = static_cast<int>(promotionPieceTypes[i]);
		sf::Sprite pieceSprite(pieceTextures[sideIndex][typeIndex]);

		const sf::FloatRect spriteBounds = pieceSprite.getLocalBounds();
		const float maxDimension = std::max(spriteBounds.size.x, spriteBounds.size.y);
		const float scale = (boardMetrics.tileSize * 0.85f) / maxDimension;

		pieceSprite.setScale({scale, scale});
		pieceSprite.setOrigin({ spriteBounds.size.x / 2.f, spriteBounds.size.y / 2.f });
		pieceSprite.setPosition({ centerX, centerY });
		window.draw(pieceSprite);
	}
}

void Render::setTextOrigin(sf::Text& text) {
	sf::FloatRect bounds = text.getLocalBounds();

	text.setOrigin({
		std::round(bounds.position.x + bounds.size.x / 2.f),
		std::round(bounds.position.y + bounds.size.y / 2.f)
	});
}

void Render::drawRanksAndFiles(const Side currentTurn) {
	for (int i = 0; i < RANKS; i++) {
		std::string rankString(1, (playerSide == Side::White) ? ('8' - i) : ('1' + i));
		std::string fileString(1, (playerSide == Side::White) ? ('a' + i) : ('h' - i));

		rankText[i].setString(rankString);
		fileText[i].setString(fileString);

		window.draw(rankText[i]);
		window.draw(fileText[i]);
	}
}

void Render::drawTimers(const float whiteTimeInSeconds, const float blackTimeInSeconds) {
	
	auto formatTime = [&](const float timeInSeconds) -> std::string {
		timeFormatter.str("");
		if (timeInSeconds <= 0.f) return "0.0";

		if (timeInSeconds >= 10.f) {
			const int minutes = static_cast<int>(timeInSeconds) / 60;
			const int seconds = static_cast<int>(timeInSeconds) % 60;

			timeFormatter << ((minutes < 10) ? "0" : "") << minutes << ":" << ((seconds < 10) ? "0" : "") << seconds;
			return timeFormatter.str();
		}
		
		const int seconds = static_cast<int>(timeInSeconds);
		const int tenths = static_cast<int>((timeInSeconds - seconds) * 10.f);
		timeFormatter << "0:" << seconds << "." << tenths;
		return timeFormatter.str();
	};

	whiteTimerText.setString(formatTime(whiteTimeInSeconds));
	blackTimerText.setString(formatTime(blackTimeInSeconds));

	setTextOrigin(whiteTimerText);
	setTextOrigin(blackTimerText);

	const float xPosition = boardMetrics.boardRightEdge + boardMetrics.tileSize / 2.f;
	const sf::Vector2f topPosition = { xPosition, boardMetrics.boardTopEdge + (boardMetrics.tileSize / 2.f) };
	const sf::Vector2f bottomPosition = { xPosition, boardMetrics.boardTopEdge + boardMetrics.boardSize - (boardMetrics.tileSize / 2.f) };

	const float leftAnchor = whiteTimerBackground.getSize().x - boardMetrics.tileSize * 1.25f;

	if (playerSide == Side::White) {
		whiteTimerBackground.setPosition(bottomPosition);
		whiteTimerText.setPosition({ bottomPosition.x + leftAnchor, bottomPosition.y });

		blackTimerBackground.setPosition(topPosition);
		blackTimerText.setPosition({ topPosition.x + leftAnchor, topPosition.y });
	}
	else {
		whiteTimerBackground.setPosition(topPosition);
		whiteTimerText.setPosition({ topPosition.x + leftAnchor, topPosition.y });

		blackTimerBackground.setPosition(bottomPosition);
		blackTimerText.setPosition({ bottomPosition.x + leftAnchor, bottomPosition.y });
	}

	window.draw(whiteTimerBackground);
	window.draw(whiteTimerText);
	window.draw(blackTimerBackground);
	window.draw(blackTimerText);
}

void Render::drawGraveyards(const BoardMaterial& boardMaterial) {
	int numOfPiecesWhiteGraveyard = 0;
	int numOfPiecesBlackGraveyard = 0;
	const int materialAdvantage = calculateMaterialAdvantage(boardMaterial);

	for (const auto& [pieceType, count] : whiteGraveyardCount) {
		numOfPiecesWhiteGraveyard += count;
	}

	for (const auto& [pieceType, count] : blackGraveyardCount) {
		numOfPiecesBlackGraveyard += count;
	}

	const float pieceSpacing = boardMetrics.tileSize * 0.2f;
	const float requiredSpaceForWhite = pieceSpacing * numOfPiecesWhiteGraveyard;
	const float requiredSpaceForBlack = pieceSpacing * numOfPiecesBlackGraveyard;

	float currentXBlackGraveyard = boardMetrics.boardRightEdge + (boardMetrics.tileSize * 3.5f) + requiredSpaceForBlack;
	float currentXWhiteGraveyard = boardMetrics.boardRightEdge + (boardMetrics.tileSize * 3.5f) + requiredSpaceForWhite;

	const float topY = boardMetrics.boardTopEdge + (boardMetrics.tileSize / 2.f);
	const float bottomY = boardMetrics.boardTopEdge + boardMetrics.boardSize - (boardMetrics.tileSize / 2.f);

	const float blackGraveyardY = (playerSide == Side::White) ? bottomY : topY;
	const float whiteGraveyardY = (playerSide == Side::White) ? topY : bottomY;

	sf::Text materialAdvantageText(font, "", static_cast<unsigned int>(boardMetrics.tileSize * 0.45f));
	if (materialAdvantage > 0) {
		materialAdvantageText.setString(" + " + std::to_string(materialAdvantage));
		materialAdvantageText.setPosition({ currentXBlackGraveyard + boardMetrics.tileSize * 0.6f, blackGraveyardY });
	}
	else if (materialAdvantage < 0) {
		materialAdvantageText.setString(" + " + std::to_string(std::abs(materialAdvantage)));
		materialAdvantageText.setPosition({ currentXWhiteGraveyard + boardMetrics.tileSize * 0.6f, whiteGraveyardY });
	}

	setTextOrigin(materialAdvantageText);
	window.draw(materialAdvantageText);

	auto drawPiecesInGraveyard = [&](const int sideIndex, const int typeIndex, const int count, float& currentX, const float yPos) {
		for (int i = 0; i < count; i++) {
			sf::Sprite miniPieceSprite(pieceTextures[sideIndex][typeIndex]);

			const sf::FloatRect bounds = miniPieceSprite.getLocalBounds();
			const float maxDimension = std::max(bounds.size.x, bounds.size.y);
			const float scale = (boardMetrics.tileSize * 0.45f) / maxDimension;

			miniPieceSprite.setScale({ scale, scale });
			miniPieceSprite.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });

			miniPieceSprite.setPosition({ currentX, yPos });
			window.draw(miniPieceSprite);

			currentX -= pieceSpacing;
		}
	};
	
	const std::array<PieceType, 5> pieceTypes = { PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight, PieceType::Pawn };
	const int blackSideIndex = static_cast<int>(Side::Black);
	const int whiteSideIndex = static_cast<int>(Side::White);

	for (const PieceType& pieceType : pieceTypes) {
		const int typeIndex = static_cast<int>(pieceType);
		drawPiecesInGraveyard(blackSideIndex, typeIndex, blackGraveyardCount[pieceType], currentXBlackGraveyard, blackGraveyardY);
	}

	for (const PieceType& pieceType : pieceTypes) {
		const int typeIndex = static_cast<int>(pieceType);
		drawPiecesInGraveyard(whiteSideIndex, typeIndex, whiteGraveyardCount[pieceType], currentXWhiteGraveyard, whiteGraveyardY);
	}
}

int Render::calculateMaterialAdvantage(const BoardMaterial& boardMaterial) {

	auto calculateAdvantage = [](const Material& materialSide) -> int {
		const int total = (materialSide.pawns * pawnValue) +
			(materialSide.knights * knightValue) +
			((materialSide.lightSquaredBishops + materialSide.darkSquaredBishops) * bishopValue) +
			(materialSide.rooks * rookValue) +
			(materialSide.queens * queenValue);

		return total;
	};

	const int whiteTotal = calculateAdvantage(boardMaterial.white) / 100;

	const int blackTotal = calculateAdvantage(boardMaterial.black) / 100;

	const int advantage = whiteTotal - blackTotal;
	return advantage;
}

void Render::updateGraveyardCounts(const std::vector<Piece>& whiteGraveyard, const std::vector<Piece>& blackGraveyard) {
	whiteGraveyardCount.clear();
	blackGraveyardCount.clear();

	auto updateSingleGraveyardCount = [](const std::vector<Piece>& graveyard, std::unordered_map<PieceType, int>& countMap) {
		for (const Piece& piece : graveyard) {
			switch (piece.type) {
			case PieceType::Pawn:
				countMap[PieceType::Pawn]++;
				break;
			case PieceType::Knight:
				countMap[PieceType::Knight]++;
				break;
			case PieceType::Bishop:
				countMap[PieceType::Bishop]++;
				break;
			case PieceType::Rook:
				countMap[PieceType::Rook]++;
				break;
			case PieceType::Queen:
				countMap[PieceType::Queen]++;
				break;
			}
		}
	};

	updateSingleGraveyardCount(whiteGraveyard, whiteGraveyardCount);
	updateSingleGraveyardCount(blackGraveyard, blackGraveyardCount);
}

void Render::drawMoveHistory(const std::vector<Turn>& turns) {
	const float startingXPosition = margin;
	float currentXPosition = startingXPosition;
	float currentYPosition = margin;

	const float whiteColumn = historyViewportMetrics.historyViewWidth * 0.3f;
	const float blackColumn = whiteColumn * 2.f;

	moveHistorySize = turns.size() * rowDistance + margin;

	auto drawText = [&](const std::string& text, const float xPosition) {
		stamp.setString(text);
		stamp.setPosition({xPosition, currentYPosition});
		window.draw(stamp);
	};

	for (const Turn& turn : turns) {
		drawText(std::to_string(turn.moveNumber) + ".", startingXPosition);

		drawText(turn.whiteMove, whiteColumn);

		if (!turn.blackMove.empty()) {
			drawText(turn.blackMove, blackColumn);
		}

		currentYPosition += rowDistance;
	}
}

void Render::drawResignationButton(const Button& resignationButton) {
	sf::RectangleShape buttonShape{resignationButton.size};
	buttonShape.setFillColor(sf::Color::Green);
	buttonShape.setOutlineColor(colours.slotOutlineColour);
	buttonShape.setOutlineThickness(4.f);
	buttonShape.setOrigin(buttonShape.getGeometricCenter());
	buttonShape.setPosition(resignationButton.position);
	window.draw(buttonShape);
}

void Render::drawResignationConfirmation(const ResignationConfirmationLayout& resignationConfirmationLayout) {
	auto formatRect = [&](const sf::Vector2f size, const sf::Color backgroundColour, const sf::Vector2f position) {
		sf::RectangleShape rect{ size };
		rect.setFillColor(backgroundColour);
		rect.setOutlineColor(colours.promoWindowOutlineColour);
		rect.setOutlineThickness(4.f);
		rect.setOrigin(rect.getGeometricCenter());
		rect.setPosition(position);
		window.draw(rect);
	};

	formatRect(resignationConfirmationLayout.size, colours.promoWindowBGColour, resignationConfirmationLayout.position);

	formatRect(resignationConfirmationLayout.cancelButton.size, sf::Color::Yellow, resignationConfirmationLayout.cancelButton.position);

	formatRect(resignationConfirmationLayout.confirmButton.size, sf::Color::Red, resignationConfirmationLayout.confirmButton.position);

	auto drawText = [&](sf::Text& text, const sf::Vector2f position) {
		setTextOrigin(text);
		text.setPosition(position);
		window.draw(text);
	};
	
	const sf::Vector2f areYouSurePosition = { boardMetrics.offsetX + boardMetrics.boardSize + historyViewportMetrics.historyViewWidth / 2.f, boardMetrics.offsetY + boardMetrics.boardSize / 2.f - boardMetrics.tileSize * 1.5f };
	drawText(areYouSureText, areYouSurePosition);

	drawText(cancelText, resignationConfirmationLayout.cancelButton.position);
	drawText(confirmText, resignationConfirmationLayout.confirmButton.position);
}

void Render::drawMainMenu(const MainMenuLayout& mainMenuLayout) {
	window.draw(mainMenuTitleText);

	auto formatRect = [&](const sf::Vector2f position, const sf::Color colour) {
		mainMenuButtonRect.setSize(mainMenuLayout.playButton.size);
		mainMenuButtonRect.setPosition(position);
		mainMenuButtonRect.setOrigin(mainMenuButtonRect.getGeometricCenter());
		mainMenuButtonRect.setFillColor(colour);
		mainMenuButtonRect.setOutlineThickness(4.f);
		window.draw(mainMenuButtonRect);
	};
	
	formatRect(mainMenuLayout.playButton.position, colours.greenColour);
	formatRect(mainMenuLayout.settingsButton.position, colours.grayButtonColour);
	formatRect(mainMenuLayout.quitButton.position, colours.redColour);

	playButtonText.setPosition(mainMenuLayout.playButton.position);
	settingsButtonText.setPosition(mainMenuLayout.settingsButton.position);
	quitButtonText.setPosition(mainMenuLayout.quitButton.position);

	window.draw(playButtonText);
	window.draw(settingsButtonText);
	window.draw(quitButtonText);
}

void Render::drawPrePlayPage(const PrePlayLayout& prePlayLayout, const PlayerChoices& choices) {
	const sf::Vector2f buttonSize = prePlayLayout.sideButtons[0]->size;
	sf::Sprite iconSprite{ sideChoiceTextures[0] };

	float currentXPosition = windowWidth * 0.4f;
	float currentYPosition = boardMetrics.tileSize / 2.f;

	auto drawPrePlayText = [&](const std::string string) {
		prePlayText.setString(string);
		prePlayText.setCharacterSize(prePlayAndSettingsCharSize);
		prePlayText.setFillColor(sf::Color::Black);
		
		const sf::FloatRect& bounds = prePlayText.getLocalBounds();
		prePlayText.setOrigin({
			std::round(bounds.size.x),
			std::round(bounds.size.y / 2.f)
		});

		prePlayText.setPosition({ prePlayTextXPosition, currentYPosition + buttonSize.y / 2.f });
		window.draw(prePlayText);
	};

	auto drawButtonRect = [&](const Button* button, const bool isSelected) {
		prePlayButtonRect.setSize(button->size);
		prePlayButtonRect.setPosition(button->position);
		prePlayButtonRect.setOrigin(prePlayButtonRect.getGeometricCenter());
		
		if (isSelected) {
			prePlayButtonRect.setFillColor(colours.greenColour);
			prePlayButtonRect.setOutlineColor(colours.lightColour);
			prePlayButtonRect.setOutlineThickness(4.f);
		}
		else {
			prePlayButtonRect.setFillColor(colours.grayButtonColour);
			prePlayButtonRect.setOutlineColor(sf::Color::Black);
			prePlayButtonRect.setOutlineThickness(2.f);
		}
		
		window.draw(prePlayButtonRect);
		
		currentXPosition += buttonSize.y * 1.5f;
	};

	auto drawIcon = [&](const sf::Vector2f position) {
		const sf::FloatRect bounds = iconSprite.getLocalBounds();
		float scale = (buttonSize.y * 0.7f) / std::max(bounds.size.x, bounds.size.y);
		iconSprite.setScale({ scale, scale });
		iconSprite.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
		iconSprite.setPosition(position);

		window.draw(iconSprite);
	};

	auto changePositions = [&]() {
		currentXPosition = windowWidth * 0.4f;
		currentYPosition += buttonSize.y * 1.5f;
	};

	drawPrePlayText("Side");
	for (std::size_t sideIndex  = 0; sideIndex < prePlayLayout.sideButtons.size(); sideIndex++) {
		const Button* currentButton = prePlayLayout.sideButtons[sideIndex];
		const bool isSelected = (static_cast<int>(choices.sideChoice) == sideIndex);
		drawButtonRect(currentButton, isSelected);

		iconSprite.setTexture(sideChoiceTextures[sideIndex], true);
		drawIcon(currentButton->position);
	}
	changePositions();

	drawPrePlayText("Opponent Type");
	for (std::size_t opponentIndex = 0; opponentIndex < prePlayLayout.opponentButtons.size(); opponentIndex++) {
		const Button* currentButton = prePlayLayout.opponentButtons[opponentIndex];
		const bool isSelected = (static_cast<int>(choices.opponentChoice) == opponentIndex);
		drawButtonRect(currentButton, isSelected);

		iconSprite.setTexture(opponentChoiceTextures[opponentIndex], true);
		drawIcon(currentButton->position);
	}
	changePositions();

	drawPrePlayText("Time Control");
	for (std::size_t timeIndex = 0; timeIndex < prePlayLayout.timeControlButtons.size(); timeIndex++) {
		const Button* currentButton = prePlayLayout.timeControlButtons[timeIndex];
		const bool isSelected = (static_cast<int>(choices.timeControlChoice) == timeIndex);
		drawButtonRect(currentButton, isSelected);

		iconSprite.setTexture(timeControlChoiceTextures[timeIndex], true);
		drawIcon(currentButton->position);
	}
	changePositions();

	auto drawIncrementText = [&](const sf::Vector2f position, const std::string incrementString) {
		incrementText.setString(incrementString);
		incrementText.setCharacterSize(prePlayAndSettingsCharSize);
		incrementText.setFillColor(sf::Color::White);

		const sf::FloatRect& bounds = incrementText.getLocalBounds();
		setTextOrigin(incrementText);

		incrementText.setPosition(position);
		window.draw(incrementText);
	};

	drawPrePlayText("Increment");
	for (std::size_t incrementIndex = 0; incrementIndex < prePlayLayout.incrementButtons.size(); incrementIndex++) {
		const Button* currentButton = prePlayLayout.incrementButtons[incrementIndex];
		const bool isSelected = choices.timeIncrement == timeIncrementChoices[incrementIndex];
		drawButtonRect(currentButton, isSelected);
		drawIncrementText(currentButton->position, std::to_string(timeIncrementChoices[incrementIndex]) + " s");
	}

	prePlayText.setString("Start");
	prePlayText.setCharacterSize(prePlayAndSettingsCharSize);
	setTextOrigin(prePlayText);
	prePlayText.setFillColor(colours.lastMoveFromColour);
	prePlayText.setPosition(prePlayLayout.startGameButton.position);

	drawButtonRect(&prePlayLayout.startGameButton, false);
	window.draw(prePlayText);

	drawButtonRect(&prePlayLayout.backButton, false);

	prePlayText.setString("Back");
	prePlayText.setCharacterSize(static_cast<unsigned int>(boardMetrics.tileSize * 0.4f));
	prePlayText.setFillColor(colours.redColour);

	setTextOrigin(prePlayText);
	prePlayText.setPosition(prePlayLayout.backButton.position);
	window.draw(prePlayText);
}

void Render::drawSettingsPage(const SettingsLayout& settingsLayout, const Settings& settings) {
	const sf::Vector2f toggleButtonSize = settingsLayout.toggleButtons[0]->size;
	const std::array<bool, 3> toggleStates = { settings.fullScreen, settings.showLegalMoves, settings.autoPromoteToQueen };
	const std::array<std::string, 3> toggleLabels = { "Full Screen", "Show Legal Moves", "Auto-Promote to Queen" };

	auto drawButtonRect = [&](const Button& button, const std::size_t index) {
		settingsButtonRect.setSize(button.size);
		settingsButtonRect.setPosition(button.position);
		settingsButtonRect.setOrigin(settingsButtonRect.getGeometricCenter());
		settingsButtonRect.setOutlineColor(sf::Color::Black);
		settingsButtonRect.setOutlineThickness(4.f);

		if (toggleStates[index]) {
			settingsButtonRect.setFillColor(colours.greenColour);
			toggleText.setString("ON");
		}
		else {
			settingsButtonRect.setFillColor(colours.redColour);
			toggleText.setString("OFF");
		}

		toggleText.setCharacterSize(prePlayAndSettingsCharSize);
		setTextOrigin(toggleText);
		toggleText.setPosition(button.position);

		window.draw(settingsButtonRect);
		window.draw(toggleText);
	};


	for (std::size_t toggleIndex = 0; toggleIndex < settingsLayout.toggleButtons.size(); toggleIndex++) {
		const Button& currentButton = *settingsLayout.toggleButtons[toggleIndex];
		drawButtonRect(currentButton, toggleIndex);

		settingsText.setString(toggleLabels[toggleIndex]);
		settingsText.setCharacterSize(prePlayAndSettingsCharSize);
		settingsText.setFillColor(sf::Color::Black);

		sf::FloatRect labelBounds = settingsText.getLocalBounds();
		settingsText.setOrigin({ labelBounds.size.x, std::round(labelBounds.size.y / 2.f) });

		settingsText.setPosition({ currentButton.position.x - currentButton.size.x / 2.f - boardMetrics.tileSize * 0.5f, currentButton.position.y });
		window.draw(settingsText);
	}

	settingsButtonRect.setSize(settingsLayout.backButton.size);
	settingsButtonRect.setPosition(settingsLayout.backButton.position);
	settingsButtonRect.setOrigin(settingsButtonRect.getGeometricCenter());
	settingsButtonRect.setFillColor(colours.grayButtonColour);
	settingsButtonRect.setOutlineColor(sf::Color::Black);
	settingsButtonRect.setOutlineThickness(2.f);

	settingsText.setString("Back");
	settingsText.setCharacterSize(static_cast<unsigned int>(boardMetrics.tileSize * 0.4f));
	settingsText.setFillColor(sf::Color(150, 50, 50));

	setTextOrigin(settingsText);
	settingsText.setPosition(settingsLayout.backButton.position);

	window.draw(settingsButtonRect);
	window.draw(settingsText);
}

void Render::drawGameOverLayout(const GameOverLayout& gameOverLayout, const std::optional<GameOverType> reason, const std::optional<Side> winner) {
	window.draw(tintedGlassRect);

	gameOverLayoutBackground.setSize(gameOverLayout.size);
	gameOverLayoutBackground.setPosition(gameOverLayout.position);
	gameOverLayoutBackground.setOrigin(gameOverLayoutBackground.getGeometricCenter());
	gameOverLayoutBackground.setFillColor(colours.promoWindowOutlineColour);
	gameOverLayoutBackground.setOutlineColor(sf::Color(40, 40, 40));
	gameOverLayoutBackground.setOutlineThickness(3.f);
	window.draw(gameOverLayoutBackground);

	gameOverButtonRect.setSize(gameOverLayout.mainMenuButton.size);
	gameOverButtonRect.setPosition(gameOverLayout.mainMenuButton.position);
	gameOverButtonRect.setOrigin(gameOverButtonRect.getGeometricCenter());
	gameOverButtonRect.setFillColor(colours.redColour);
	gameOverButtonRect.setOutlineColor(sf::Color::White);
	gameOverButtonRect.setOutlineThickness(3.f);
	window.draw(gameOverButtonRect);

	gameOverButtonRect.setSize(gameOverLayout.playAgainButton.size);
	gameOverButtonRect.setPosition(gameOverLayout.playAgainButton.position);
	gameOverButtonRect.setOrigin(gameOverButtonRect.getGeometricCenter());
	gameOverButtonRect.setFillColor(colours.greenColour);
	gameOverButtonRect.setOutlineColor(sf::Color::White);
	gameOverButtonRect.setOutlineThickness(3.f);
	window.draw(gameOverButtonRect);

	gameOverText.setFillColor(sf::Color::White);

	unsigned int buttonCharSize = static_cast<unsigned int>(gameOverLayout.size.y * 0.075f);
	gameOverText.setCharacterSize(buttonCharSize);

	gameOverText.setString("Main Menu");
	setTextOrigin(gameOverText);
	gameOverText.setPosition(gameOverLayout.mainMenuButton.position);
	window.draw(gameOverText);

	gameOverText.setString("Play Again");
	setTextOrigin(gameOverText);
	gameOverText.setPosition(gameOverLayout.playAgainButton.position);
	window.draw(gameOverText);

	std::string titleString = "Game Over";
	if (winner.has_value()) {
		if (winner.value() == Side::White) titleString = "White Wins!";
		else if (winner.value() == Side::Black) titleString = "Black Wins!";
		else titleString = "It's a Draw!";
	}

	gameOverText.setString(titleString);
	unsigned int titleCharSize = static_cast<unsigned int>(gameOverLayout.size.y * 0.15f);
	gameOverText.setCharacterSize(titleCharSize);
	setTextOrigin(gameOverText);
	gameOverText.setFillColor(colours.lightColour);
	gameOverText.setPosition({ gameOverLayout.position.x, gameOverLayout.position.y - (gameOverLayout.size.y * 0.25f) });
	window.draw(gameOverText);

	std::string reasonString;
	if (reason.has_value()) {
		switch (reason.value()) {
		case GameOverType::Checkmate: 
			reasonString += "by Checkmate"; 
			break;
		case GameOverType::Stalemate: 
			reasonString += "by Stalemate"; 
			break;
		case GameOverType::Resignation: 
			reasonString += "by Resignation"; 
			break;
		case GameOverType::Timeout: 
			reasonString += "on Time"; 
			break;
		case GameOverType::ThreefoldRepetition: 
			reasonString += "by Repetition"; 
			break;
		case GameOverType::InsufficientMaterial: 
			reasonString += "Insufficient Material"; 
			break;
		}
	}

	unsigned int reasonCharSize = static_cast<unsigned int>(gameOverLayout.size.y * 0.10f);
	gameOverText.setCharacterSize(reasonCharSize);

	gameOverText.setString(reasonString);
	setTextOrigin(gameOverText);
	gameOverText.setPosition({ gameOverLayout.position.x, gameOverLayout.position.y - (gameOverLayout.size.y * 0.08f) });
	window.draw(gameOverText);
}

const sf::FloatRect& Render::getHistoryViewport() const {
	return historyViewport;
}

float Render::getWindowWidth() const {
	return windowWidth;
}

float Render::getWindowHeight() const {
	return windowHeight;
}

float Render::getMoveHistorySize() const {
	return moveHistorySize;
}

void Render::setMoveHistorySize(const float newSize) {
	moveHistorySize = newSize;
}

float Render::getRowDistance() const {
	return rowDistance;
}

void Render::setRowDistance(const float newDistance) {
	rowDistance = newDistance;
}