#pragma once
#include "includes.h"
#include "game-elements.h"
#include "board.h"

class Game {
private:
	void createBoard();

	void selectPiece(const int rank, const int file);
	 
	void checkValidSquare(const int rank, const int file);
	std::optional<Move> validateMove(Move attemptedMove);
	void makeNextMove();

	void addPieceToGraveyard(const std::optional<Piece>& capturedPiece);
	void applyTimeIncrement(const Side sideThatMoved);

	void checkWinner();

	void checkPerftTests(const int depth);

	void updateMoveHistory(const Move& move, const Side movingSide);

	Board BOARD;

	std::vector<Piece> whiteGraveyard;
	std::vector<Piece> blackGraveyard;
	
	std::optional<Side> winner;
	std::optional<GameOverType> gameOverType;

	int selectedRank = INVALID_COORD, selectedFile = INVALID_COORD;
	std::optional<Move> lastMove;

	std::vector<Move> selectedPieceMoves;

	OpponentType opponentType = OpponentType::Engine;

	float whiteTimeInSeconds = 0.f;
	float blackTimeInSeconds = 0.f;
	float timeIncrement = 0.f;

	bool isConfirmingResignation = false;

	std::future<Move> aiFutureMove;
	bool isAIThinking = false;
public:
	Side playerSide = Side::None;

	Game();

	Game(const std::string& fen, const PlayerChoices& playerChoices);
	void setUpGame(const PlayerChoices& playerChoices);

	void processBoardClick(const int rank, const int file);
	
	void resetSelectedSquare();

	void updateTimers(const float deltaTime);
	void updateAI();
	void makeAIMove();

	void handleTimeout(const Side loser);

	const std::vector<Turn>& getMoveHistory() const;
	const bool isPromoPending() const;
	const std::optional<Side> getPromoMenuSide() const;
	const std::array<std::array<Square, 8>, 8>& getBoard() const;
	const Side getCurrentTurn() const;
	const BoardMaterial& getBoardMaterial() const;
	const std::vector<Piece>& getWhiteGraveyard() const;
	const std::vector<Piece>& getBlackGraveyard() const;
	const std::optional<Side>& getWinner() const;
	const int getSelectedRank() const;
	const int getSelectedFile() const;
	const std::optional<Move>& getLastMove() const;
	const std::vector<Move>& getSelectedPieceMoves() const;
	const float getWhiteTime() const;
	const float getBlackTime() const;
	const bool getIsConfirmingResignation() const;
	const std::optional<GameOverType> getGameOverType() const;

	void setIsConfirmingResignation(const bool isConfirming);
	void setWinner(const Side winner);
	void setGameOverType(const GameOverType type);

	void handlePendingPromo(const PieceType promoteTo);
};