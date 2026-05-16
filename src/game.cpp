#include "game.h"

Game::Game() {}

Game::Game(const std::string& fen, const PlayerChoices& playerChoices) {
	
	setUpGame(playerChoices);

	createBoard();
	BOARD.loadFen(fen);
	
	BOARD.loadOpeningBook();

	BOARD.countMaterial();
	checkWinner();
	
	makeNextMove();
}

void Game::setUpGame(const PlayerChoices& playerChoices) {
	switch (playerChoices.sideChoice) {
	case Side::White:
		playerSide = Side::White;
		break;
	case Side::Black:
		playerSide = Side::Black;
		break;
	case Side::Random:
		std::random_device rd;
		std::mt19937 engine(rd());
		std::uniform_int_distribution<> range(0, 1);
		const int choice = range(engine);
		playerSide = (choice == 0) ? Side::White : Side::Black;
		break;
	}

	if (playerChoices.opponentChoice == OpponentType::Human) {
		opponentType = OpponentType::Human;
	}

	switch (playerChoices.timeControlChoice) {
	case TimeControl::Bullet:
		whiteTimeInSeconds = 60;
		blackTimeInSeconds = 60;
		break;
	case TimeControl::Blitz:
		whiteTimeInSeconds = 180;
		blackTimeInSeconds = 180;
		break;
	case TimeControl::Rapid:
		whiteTimeInSeconds = 600;
		blackTimeInSeconds = 600;
		break;
	}

	timeIncrement = playerChoices.timeIncrement;
}

void Game::createBoard() {
	for (int rank = 0; rank < RANKS; rank++) {
		for (int file = 0; file < FILES; file++) {
			Square newSquare{BOARD.determineSquareColour(rank, file)};
			BOARD.board[rank][file] = newSquare;
		}
	}
}

void Game::processBoardClick(const int rank, const int file) {
	if (isAIThinking)
		return;

	if (rank >= 0 and rank < RANKS and
		file >= 0 and file < FILES) {
		checkValidSquare(rank, file);
	}
	else resetSelectedSquare();
}

void Game::selectPiece(const int rank, const int file) {
	selectedRank = rank;
	selectedFile = file;
	selectedPieceMoves = BOARD.generateLegalMoves(selectedRank, selectedFile);
}

void Game::checkValidSquare(const int rank, const int file) {
	// if no square has been selected yet
	const std::optional<Piece>& selectedPotentialPiece = BOARD.board[rank][file].piece;
	if (selectedRank == INVALID_COORD and selectedPotentialPiece.has_value()) {
		if (BOARD.currentTurn == selectedPotentialPiece->side)
			selectPiece(rank, file);
	}
	// if selectedRank and selectedFile already have a value
	else if (selectedRank != INVALID_COORD) {
		// if user clicks on the same square again
		if (selectedRank == rank and selectedFile == file) {
			resetSelectedSquare();
			return;	
		}

		// if the user clicks on another piece that is on the currentTurn's side
		if (selectedPotentialPiece.has_value()) {
			if (BOARD.currentTurn == selectedPotentialPiece->side) {
				resetSelectedSquare();
				selectPiece(rank, file);
				return;
			}
		}

		Move newMove{ selectedRank, selectedFile, rank, file };
		std::optional<Move> validMove = validateMove(newMove);
		if (validMove.has_value()) {
			Side movingSide = BOARD.currentTurn;

			updateMoveHistory(validMove.value(), movingSide);

			std::optional<Piece> capturedPiece = BOARD.makeMove(validMove.value(), lastMove);
			if (capturedPiece.has_value()) {
				addPieceToGraveyard(capturedPiece);
			}

			applyTimeIncrement(movingSide);
			
			checkWinner();
			makeNextMove();
		}

		resetSelectedSquare();
	}
}

void Game::updateMoveHistory(const Move& move, const Side movingSide) {
	std::string moveInSAN = BOARD.moveToSAN(move);
	if (movingSide == Side::White) {
		Turn newTurn;
		newTurn.moveNumber = static_cast<int>(BOARD.moveHistory.size()) + 1;
		newTurn.whiteMove = moveInSAN;
		BOARD.moveHistory.push_back(newTurn);
	}
	else {
		if (!BOARD.moveHistory.empty()) {
			BOARD.moveHistory.back().blackMove = moveInSAN;
		}
	}
}

void Game::makeNextMove() {
	if (opponentType == OpponentType::Human) {
		playerSide = BOARD.currentTurn;
	}
	else {
		makeAIMove();
	}
}

void Game::applyTimeIncrement(const Side sideThatMoved) {
	if (sideThatMoved == Side::White) {
		whiteTimeInSeconds += timeIncrement;
	}
	else {
		blackTimeInSeconds += timeIncrement;
	}
}

void Game::updateAI() {
	if (!isAIThinking)
		return;

	if (winner.has_value()) {
		isAIThinking = false;
		return;
	}
	
	if (aiFutureMove.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
		Side movingSide = BOARD.currentTurn;
		Move aiMove = aiFutureMove.get();

		updateMoveHistory(aiMove, movingSide);

		std::optional<Piece> capturedPiece = BOARD.makeMove(aiMove, lastMove);

		if (capturedPiece.has_value()) {
			addPieceToGraveyard(capturedPiece);
		}

		if (BOARD.isPromoPending()) {
			BOARD.handlePendingPromo(aiMove.promoteInto.value());
		}

		applyTimeIncrement(movingSide);

		checkWinner();
		isAIThinking = false;
	}
}

void Game::makeAIMove() {
	if (opponentType == OpponentType::Human) {
		return;
	}

	if (BOARD.currentTurn != playerSide and !BOARD.isPromoPending() and !winner.has_value() and !isAIThinking) {
		isAIThinking = true;
		Board::abortSearch = false;

		Board boardCopy = BOARD;
		int currentPieces = BOARD.getPieceCount();

		int aiTime = (playerSide == Side::White) ? blackTimeInSeconds * 1000 : whiteTimeInSeconds * 1000;
		int expectedMovesLeft = currentPieces + 5;

		int allocatedTimeMs = aiTime / expectedMovesLeft;

		if (allocatedTimeMs > aiTime - 500) {
			allocatedTimeMs = std::max(100, aiTime - 500);
		}

		aiFutureMove = std::async(std::launch::async, [boardCopy, allocatedTimeMs]() mutable {
			return boardCopy.getBestMoveIterative(allocatedTimeMs);
		});
	}
}

void Game::addPieceToGraveyard(const std::optional<Piece>& capturedPiece) {
	if (capturedPiece->side == Side::White) {
		whiteGraveyard.push_back(capturedPiece.value());
	}
	else {
		blackGraveyard.push_back(capturedPiece.value());
	}
}

std::optional<Move> Game::validateMove(Move attemptedMove) {
	for (const Move& move : selectedPieceMoves) {
		if (attemptedMove.toRank == move.toRank and attemptedMove.toFile == move.toFile) return move;
	}
	return std::nullopt;
}

void Game::resetSelectedSquare() {
	selectedRank = INVALID_COORD;
	selectedFile = INVALID_COORD;
	selectedPieceMoves.clear();
}

void Game::checkWinner() {
	const Side attackingPlayer = (BOARD.currentTurn == Side::White) ? Side::Black : Side::White;

	if (BOARD.halfmoveCounter >= 50) {
		winner = Side::None;
		gameOverType = GameOverType::FiftyMoveRule;
		return;
	}

	if (BOARD.isDrawByInsufficientMaterial()) {
		winner = Side::None;
		gameOverType = GameOverType::InsufficientMaterial;
		return;
	}

	if (BOARD.isThreeFoldRepetition()) {
		winner = Side::None;
		gameOverType = GameOverType::ThreefoldRepetition;
		return;
	}

	if (!BOARD.generateEveryLegalMove(BOARD.currentTurn).empty()) {
		return;
	}

	int kingRank = INVALID_COORD, kingFile = INVALID_COORD;
	if (BOARD.currentTurn == Side::White) {
		kingRank = BOARD.whiteKingRank;
		kingFile = BOARD.whiteKingFile;
	}
	else {
		kingRank = BOARD.blackKingRank;
		kingFile = BOARD.blackKingFile;
	}

	if (BOARD.isKingSafe(kingRank, kingFile, attackingPlayer)) {
		winner = Side::None;
		gameOverType = GameOverType::Stalemate;
	}
	else {
		winner = attackingPlayer;
		gameOverType = GameOverType::Checkmate;
	}
}

void Game::updateTimers(const float deltaTime) {
	if (winner.has_value()) {
		return;
	}

	auto handleTimer = [&](const Side& currentTurn, float& timer) {
		timer -= deltaTime;

		if (timer <= 0.f) {
			timer = 0.f;
			handleTimeout(currentTurn);
		}
	};

	if (BOARD.currentTurn == Side::White) {
		handleTimer(BOARD.currentTurn, whiteTimeInSeconds);
	}
	else {
		handleTimer(BOARD.currentTurn, blackTimeInSeconds);
	}
}

void Game::handleTimeout(const Side loser) {

	auto findWinner = [&](const Side sideWithTime) {
		if (BOARD.canSideCheckmate(sideWithTime)) {
			winner = sideWithTime;
			gameOverType = GameOverType::Timeout;
		}
		else {
			winner = Side::None;
			gameOverType = GameOverType::InsufficientTimeout;
		}
	};

	if (loser == Side::White) {
		findWinner(Side::Black);
	}
	else {
		findWinner(Side::White);
	}
}

const std::vector<Turn>& Game::getMoveHistory() const {
	return BOARD.moveHistory;
}

const bool Game::isPromoPending() const {
	return BOARD.isPromoPending();
}

const std::optional<Side> Game::getPromoMenuSide() const {
	return BOARD.promoMenuSide;
}

const std::array<std::array<Square, 8>, 8>& Game::getBoard() const {
	return BOARD.board;
}

const Side Game::getCurrentTurn() const {
	return BOARD.currentTurn;
}

const BoardMaterial& Game::getBoardMaterial() const {
	return BOARD.boardMaterial;
}

const std::vector<Piece>& Game::getWhiteGraveyard() const {
	return whiteGraveyard;
}

const std::vector<Piece>& Game::getBlackGraveyard() const {
	return blackGraveyard;
}

void Game::handlePendingPromo(const PieceType promoteTo) {
	BOARD.handlePendingPromo(promoteTo);
	if (!BOARD.moveHistory.empty()) {


		std::string& lastSAN = (BOARD.currentTurn == Side::Black) ?
			BOARD.moveHistory.back().whiteMove :
			BOARD.moveHistory.back().blackMove;

		char promoChar = 'Q';
		if (promoteTo == PieceType::Rook) promoChar = 'R';
		else if (promoteTo == PieceType::Bishop) promoChar = 'B';
		else if (promoteTo == PieceType::Knight) promoChar = 'N';

		lastSAN.back() = promoChar;
	}

	checkWinner();
	if (opponentType == OpponentType::Human) {
		playerSide = BOARD.currentTurn;
	}
	else {
		makeAIMove();
	}
}

const std::optional<Side>& Game::getWinner() const {
	return winner;
}

const int Game::getSelectedRank() const {
	return selectedRank;
}

const int Game::getSelectedFile() const {
	return selectedFile;
}

const std::optional<Move>& Game::getLastMove() const {
	return lastMove;
}

const std::vector<Move>& Game::getSelectedPieceMoves() const {
	return selectedPieceMoves;
}

const float Game::getWhiteTime() const {
	return whiteTimeInSeconds;
}

const float Game::getBlackTime() const {
	return blackTimeInSeconds;
}

const bool Game::getIsConfirmingResignation() const {
	return isConfirmingResignation;
}

const std::optional<GameOverType> Game::getGameOverType() const {
	return gameOverType;
}

void Game::setIsConfirmingResignation(const bool isConfirming) {
	isConfirmingResignation = isConfirming;
}

void Game::setWinner(const Side winner) {
	this->winner = winner;
	Board::abortSearch = true;
}

void Game::setGameOverType(const GameOverType type) {
	gameOverType = type;
}