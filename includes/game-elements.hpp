#pragma once
#include "includes.hpp"

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

struct Turn {
	int moveNumber{};
	std::string whiteMove;
	std::string blackMove;
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