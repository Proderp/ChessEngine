#include "game-elements.hpp"

Piece::Piece() : side(Side::White), type(PieceType::Pawn), hasMoved(false) {}

Piece::Piece(Side side, PieceType type, bool hasMoved) : side(side), type(type), hasMoved(hasMoved) {}

Square::Square() : colour(Colour::Light) {}

Square::Square(Colour colour) : colour(colour) {} 