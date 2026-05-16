#pragma once
#include "includes.hpp"
#include "game-elements.hpp"

class Board {
private:
	void makeTempMove(const Move& move, std::optional<Piece>& capturedPiece);
	void updateMaterialCount(const Piece& piece, const int rank, const int file, const int amount);
	int getPieceSquareValue(const Piece& piece, const int rank, const int file);

	bool pawnCheck(const Side friendlySide, const Side enemySide, const int kingRank, const int kingFile, const int friendlyPawnDirection);
	bool knightCheck(const Side friendlySide, const Side enemySide, const int kingRank, const int kingFile);
	bool slidingPieceCheck(const PieceType slidingPiece, const Side friendlySide, const Side enemySide, const int kingRank, const int kingFile);
	bool kingCheck(const Side friendlySide, const Side enemySide, const int kingRank, const int kingFile);

	void unmakeMove(const Move& move, const std::optional<Piece>& capturedPiece);

	std::optional<Piece> handleEnPassant(Piece& selectedPiece, const Move& move); 
	void handleCastling(Piece& selectedPiece, const Move& move);
	void pawnPromotion(Piece& pawn, const PieceType promoteTo);

	void generatePawnMoves(const Piece& pawn, const int rank, const int file, std::vector<Move>& generatedMoves);
	void generateKnightMoves(const Piece& knight, const int rank, const int file, std::vector<Move>& generatedMoves);
	void generateSlidingPieceMoves(const PieceType& slidingPieceType, const int rank, const int file, std::vector<Move>& generatedMoves);
	void generateQueenMoves(const PieceType& slidingPieceType, const int rank, const int file, std::vector<Move>& generatedMoves);

	void generateKingMoves(const Piece& king, const int rank, const int file, std::vector<Move>& generatedMoves);
	void generateCastlingMoves(const Side enemySide, const int rank, const int file, std::vector<Move>& generatedMoves);
	void generateKingsideCastle(const Side enemySide, const int rank, const int file, std::vector<Move>& generatedMoves);
	void generateQueensideCastle(const Side enemySide, const int rank, const int file, std::vector<Move>& generatedMoves);

	void generateBoardPart(std::stringstream& stringStream) const;
	void generateCastlingPart(std::stringstream& stringStream);
	void checkCastlingRights(std::stringstream& stringStream, const Piece& whiteKing, const Piece& blackKing);
	void generateEnPassantPart(std::stringstream& stringStream) const;

	void updatePositionHistory();
	void handleKingMove(Piece& selectedPiece, const Move& move);

	int Minimax(std::size_t depth, int whiteGuaranteed, int blackGuaranteed, Side maximizingPlayer);
	int quiescenceSearch(int whiteGuaranteed, int blackGuaranteed, Side maximizingPlayer, int qsDepth = 0);
	std::vector<Move> generateCapturesOnly(const Side side);

	std::unordered_map<std::string, int> positionHistory;

	std::vector<MoveSnapshot> moveSnapshots;

	int currentEvaluation{};
	float endgameFactor;

	std::unordered_map<std::string, std::vector<OpeningBookMove>> openingBook = {};

public:
	Colour determineSquareColour(const int rank, const int file);

	std::optional<Piece> makeMove(const Move& move, std::optional<Move>& lastMove);
	Move getBestMoveIterative(int allocatedTimeMs);
	Move getMoveInOpeningBook(const std::vector<Move>& everyPossibleMove, const std::string currentFen);
	int getUCIIndexFromDistribution(const std::vector<int>& weights);
	Move parseUCI(const std::vector<Move>& everyPossibleMove, const std::string uciMove);
	SyzygyBoard getSyzygyBoards() const;
	
	std::vector<Move> generatePseudoLegalMoves(const int rank, const int file);
	std::vector<Move> generateLegalMoves(const int rank, const int file);
	std::vector<Move> generateEveryLegalMove(const Side side);
	int scoreMove(const Move& move, const std::optional<Piece>& capturedPiece);

	bool isKingSafe(const int& kingRank, const int& kingFile, const Side& enemySide);

	std::string moveToSAN(const Move& move);

	int enPassantRank = INVALID_COORD, enPassantFile = INVALID_COORD;

	int whiteKingRank = INVALID_COORD, whiteKingFile = INVALID_COORD;
	int blackKingRank = INVALID_COORD, blackKingFile = INVALID_COORD;

	Side currentTurn;

	BoardMaterial boardMaterial;

	std::size_t halfmoveCounter = 0;
	std::size_t moveCounter = 1;

	std::vector<Turn> moveHistory;

	std::array<std::array<Square, RANKS>, FILES> board;

	std::optional<Side> promoMenuSide;
	std::optional<Move> pendingPromo;

	static std::atomic<bool> abortSearch;
	
	bool isPromoPending() const;
	void handlePendingPromo(const PieceType promoteTo);

	void countMaterial();
	int getPieceCount() const;
	unsigned probeTablebaseWDL() const;

	bool isDrawByInsufficientMaterial() const;
	bool isThreeFoldRepetition() const;
	bool canSideCheckmate(const Side& side) const;

	std::string generateFenString();

	void evaluate();
	int getMopUpScore(Side winningSide) const;
	float getEndgameFactor() const;

	void loadFen(const std::string& fen);
	void createPiece(const int rank, const int file, const Side side, const PieceType type, const bool hasMoved);
	void loadEnPassantPart(const std::string& enPassantPart);
	void loadBoardPart(const std::string& boardPart, bool whiteKingSide, bool whiteQueenSide, bool blackKingSide, bool blackQueenSide);
	void loadOpeningBook();

	int getCurrentEvaluation() const;
};

