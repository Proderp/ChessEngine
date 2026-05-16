#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <array>
#include <iostream>
#include <optional>
#include <map>
#include <string>
#include <utility>
#include <cmath>
#include <sstream>
#include <unordered_map>
#include <future>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <random>
extern "C" {
	#include "tbprobe.h"
	#include "tbcore.h"
}

static constexpr int FILES = 8, RANKS = 8;
static constexpr int INVALID_COORD = -1;

static constexpr unsigned int DEFAULT_WINDOW_WIDTH = 960;
static constexpr unsigned int DEFAULT_WINDOW_HEIGHT = 630;

static constexpr int pawnValue = 100;
static constexpr int knightValue = 300;
static constexpr int bishopValue = 300;
static constexpr int rookValue = 500;
static constexpr int queenValue = 900;

static constexpr int WHITE_LOSS = -9999999;
static constexpr int BLACK_LOSS = 9999999;

static constexpr int MAX_SEARCH_DEPTH = 100;

static constexpr float animationDuration = 0.275f;
static constexpr float animationLockdownDuration = 0.2f;

enum class Colour {Light, Dark};
enum class Side { White, Black, Random, None  };
enum class PieceType { Pawn, Knight, Bishop, Rook, Queen, King };

enum class GameState {
	MainMenu,
	Settings,
	PrePlay,
	Playing,
	GameOver
};

enum class GameOverType {
	Checkmate,
	Resignation,
	Stalemate,
	InsufficientMaterial,
	ThreefoldRepetition,
	Timeout,
	InsufficientTimeout,
	FiftyMoveRule
};

enum class OpponentType { Engine, Human };
enum class TimeControl { Bullet, Blitz, Rapid };
static constexpr std::array<int, 5> timeIncrementChoices = { 0, 1, 2, 5, 10 };

static constexpr std::array<std::pair<int, int>, 8> knightDirections = { {
	{-2, -1}, {-2, 1}, {-1, 2}, {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}
} };
 
static constexpr std::array<std::pair<int, int>, 4> bishopDirections = { {
	{-1, 1}, {1, 1}, {1, -1}, {-1, -1}
} };

static constexpr std::array<std::pair<int, int>, 4> rookDirections = { {
	{-1, 0}, {0, 1}, {1, 0}, {0, -1}
} };

static constexpr std::array<std::pair<int, int>, 8> kingDirections = { {
	{-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}
} };

static constexpr std::array<PieceType, 4> promotionPieceTypes = {
	PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight
};

// piece square tables

static constexpr std::array<int, 64> pawnPST = {
	0,  0,  0,  0,  0,  0,  0,  0,
	50, 50, 50, 50, 50, 50, 50, 50,
	10, 10, 20, 30, 30, 20, 10, 10,
	 5,  5, 10, 25, 25, 10,  5,  5,
	 0,  0,  0, 20, 20,  0,  0,  0,
	 5, -5,-10,  0,  0,-10, -5,  5,
	 5, 10, 10,-20,-20, 10, 10,  5,
	 0,  0,  0,  0,  0,  0,  0,  0
};

static constexpr std::array<int, 64> knightPST = {
	-50,-40,-30,-30,-30,-30,-40,-50,
	-40,-20,  0,  0,  0,  0,-20,-40,
	-30,  0, 10, 15, 15, 10,  0,-30,
	-30,  5, 15, 20, 20, 15,  5,-30,
	-30,  0, 15, 20, 20, 15,  0,-30,
	-30,  5, 10, 15, 15, 10,  5,-30,
	-40,-20,  0,  5,  5,  0,-20,-40,
	-50,-40,-30,-30,-30,-30,-40,-50
};

static constexpr std::array<int, 64> bishopPST = {
	-20,-10,-10,-10,-10,-10,-10,-20,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-10,  0,  5, 10, 10,  5,  0,-10,
	-10,  5,  5, 10, 10,  5,  5,-10,
	-10,  0, 10, 10, 10, 10,  0,-10,
	-10, 10, 10, 10, 10, 10, 10,-10,
	-10,  5,  0,  0,  0,  0,  5,-10,
	-20,-10,-10,-10,-10,-10,-10,-20
};

static constexpr std::array<int, 64> rookPST = {
	0,  0,  0,  0,  0,  0,  0,  0,
	  5, 10, 10, 10, 10, 10, 10,  5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	  0,  0,  0,  5,  5,  0,  0,  0
};

static constexpr std::array<int, 64> queenPST = {
	-20,-10,-10, -5, -5,-10,-10,-20,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-10,  0,  5,  5,  5,  5,  0,-10,
	 -5,  0,  5,  5,  5,  5,  0, -5,
	  0,  0,  5,  5,  5,  5,  0, -5,
	-10,  5,  5,  5,  5,  5,  0,-10,
	-10,  0,  5,  0,  0,  0,  0,-10,
	-20,-10,-10, -5, -5,-10,-10,-20
};

static constexpr std::array<int, 64> kingPSTMiddleGame = {
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-20,-30,-30,-40,-40,-30,-30,-20,
	-10,-20,-20,-20,-20,-20,-20,-10,
	 20, 20,  0,  0,  0,  0, 20, 20,
	 20, 30, 10,  0,  0, 10, 30, 20
};

static constexpr std::array<int, 64> kingPSTEndGame = {
	-50,-40,-30,-20,-20,-30,-40,-50,
	-30,-20,-10,  0,  0,-10,-20,-30,
	-30,-10, 20, 30, 30, 20,-10,-30,
	-30,-10, 30, 40, 40, 30,-10,-30,
	-30,-10, 30, 40, 40, 30,-10,-30,
	-30,-10, 20, 30, 30, 20,-10,-30,
	-30,-30,  0,  0,  0,  0,-30,-30,
	-50,-30,-30,-30,-30,-30,-30,-50
};