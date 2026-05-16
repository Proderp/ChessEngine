#include "board.hpp"

std::atomic<bool> Board::abortSearch{ false };

Colour Board::determineSquareColour(const int rank, const int file) {
	if (rank % 2 == 0) {
		return (file % 2 == 0) ? Colour::Light : Colour::Dark;
	}
	else {
		return (file % 2 == 0) ? Colour::Dark : Colour::Light;
	}
}

std::optional<Piece> Board::handleEnPassant(Piece& selectedPiece, const Move& move) {
	std::optional<Piece> capturedPiece = std::nullopt;
	if (selectedPiece.type == PieceType::Pawn and move.toRank == enPassantRank and move.toFile == enPassantFile) {
		capturedPiece = board[move.fromRank][move.toFile].piece.value();
		board[move.fromRank][move.toFile].piece.reset();
	}

	enPassantRank = INVALID_COORD;
	enPassantFile = INVALID_COORD;

	if (selectedPiece.type == PieceType::Pawn and (std::abs(move.toRank - move.fromRank) == 2)) {
		enPassantRank = move.fromRank + ((selectedPiece.side == Side::White) ? -1 : 1);
		enPassantFile = move.fromFile;
	}

	return capturedPiece;
}
void Board::handleCastling(Piece& selectedPiece, const Move& move) {

	auto moveRook = [&](std::optional<Piece>& rook, const int newFile) {
		rook->hasMoved = true;
		board[move.fromRank][newFile].piece = rook.value();
		rook.reset();
	};

	if (move.toFile > move.fromFile) {
		std::optional<Piece>& kingSideRook = board[move.fromRank][7].piece;
		moveRook(kingSideRook, move.toFile - 1);
	}
	else {
		std::optional<Piece>& queenSideRook = board[move.fromRank][0].piece;
		moveRook(queenSideRook, move.toFile + 1);
	}
}

void Board::pawnPromotion(Piece& pawn, const PieceType promoteTo) {
	pawn.type = promoteTo;
	countMaterial();
}
bool Board::isPromoPending() const {
	return pendingPromo.has_value() and promoMenuSide.has_value();
}
void Board::handlePendingPromo(const PieceType promoteTo) {
	if (!isPromoPending()) 
		return;

	const Move move = pendingPromo.value();
	Square& toSquare = board[move.toRank][move.toFile];

	pawnPromotion(toSquare.piece.value(), promoteTo);

	pendingPromo.reset();
	promoMenuSide.reset();
	
	currentTurn = (currentTurn == Side::White) ? Side::Black : Side::White;
	if (currentTurn == Side::White) {
		moveCounter++;
	}

	positionHistory.clear();
	halfmoveCounter = 0;

	evaluate();
	updatePositionHistory();
}

std::string Board::moveToSAN(const Move& move) {
	Square& fromSquare = board[move.fromRank][move.fromFile];
	Square& toSquare = board[move.toRank][move.toFile];
	Piece& movingPiece = fromSquare.piece.value();

	if (movingPiece.type == PieceType::King and std::abs(move.toFile - move.fromFile) == 2) {
		if (move.toFile > move.fromFile) 
			return "O-O";
		else 
			return "O-O-O";
	}

	std::string san = "";

	if (movingPiece.type == PieceType::Knight) 
		san += "N";
	else if (movingPiece.type == PieceType::Bishop) 
		san += "B";
	else if (movingPiece.type == PieceType::Rook) 
		san += "R";
	else if (movingPiece.type == PieceType::Queen) 
		san += "Q";
	else if (movingPiece.type == PieceType::King) 
		san += "K";

	if (movingPiece.type != PieceType::King and movingPiece.type != PieceType::Pawn) {
		bool isAmbiguous = false;
		bool sameFile = false;
		bool sameRank = false;

		std::vector<Move> legalMoves = generateEveryLegalMove(currentTurn);

		for (const Move& altMove : legalMoves) {
			if (altMove.toRank == move.toRank and altMove.toFile == move.toFile) {
				if (altMove.fromRank != move.fromRank or altMove.fromFile != move.fromFile) {
					const std::optional<Piece>& altPiece = board[altMove.fromRank][altMove.fromFile].piece;

					if (altPiece.has_value() and altPiece->type == movingPiece.type) {
						isAmbiguous = true;
						if (altMove.fromFile == move.fromFile) sameFile = true;
						if (altMove.fromRank == move.fromRank) sameRank = true;
					}
				}
			}
		}

		if (isAmbiguous) {
			if (sameFile and sameRank) {
				san += static_cast<char>('a' + move.fromFile);
				san += std::to_string(8 - move.fromRank);
			}
			else if (sameFile) {
				san += std::to_string(8 - move.fromRank);
			}
			else {
				san += static_cast<char>('a' + move.fromFile);
			}
		}
	}

	bool isCapture = toSquare.piece.has_value();

	if (movingPiece.type == PieceType::Pawn and move.fromFile != move.toFile and !toSquare.piece.has_value()) {
		isCapture = true;
	}

	if (isCapture) {
		if (movingPiece.type == PieceType::Pawn) {
			san += static_cast<char>('a' + move.fromFile);
		}
		san += "x";
	}

	san += static_cast<char>('a' + move.toFile);
	san += std::to_string(8 - move.toRank);

	if (move.promoteInto.has_value()) {
		san += "=";
		if (move.promoteInto.value() == PieceType::Queen) 
			san += "Q";
		else if (move.promoteInto.value() == PieceType::Rook) 
			san += "R";
		else if (move.promoteInto.value() == PieceType::Bishop) 
			san += "B";
		else if (move.promoteInto.value() == PieceType::Knight) 
			san += "N";
	}

	std::optional<Piece> capturedPieceTemp = std::nullopt;
	makeTempMove(move, capturedPieceTemp);

	Side enemySide = (currentTurn == Side::White) ? Side::Black : Side::White;
	int enemyKingRank = (enemySide == Side::White) ? whiteKingRank : blackKingRank;
	int enemyKingFile = (enemySide == Side::White) ? whiteKingFile : blackKingFile;

	bool isCheck = !isKingSafe(enemyKingRank, enemyKingFile, currentTurn);
	bool isMate = false;

	if (isCheck) {
		currentTurn = enemySide;
		std::vector<Move> enemyMoves = generateEveryLegalMove(enemySide);
		currentTurn = (enemySide == Side::White) ? Side::Black : Side::White;

		if (enemyMoves.empty()) {
			isMate = true;
		}
	}

	unmakeMove(move, capturedPieceTemp);

	if (isMate) 
		san += "#";
	else if (isCheck) 
		san += "+";

	return san;
}

std::optional<Piece> Board::makeMove(const Move& move, std::optional<Move>& lastMove) {
	Square& fromSquare = board[move.fromRank][move.fromFile];
	Square& toSquare = board[move.toRank][move.toFile];
	Piece& selectedPiece = fromSquare.piece.value();

	std::optional<Piece> capturedPiece = std::nullopt;
	capturedPiece = handleEnPassant(selectedPiece, move);

	if (selectedPiece.type == PieceType::King) {
		handleKingMove(selectedPiece, move);
	}

	selectedPiece.hasMoved = true;

	bool isCapture = capturedPiece.has_value();

	if (toSquare.piece.has_value()) {
		capturedPiece = toSquare.piece.value();
		isCapture = true;
	}
	
	toSquare.piece = selectedPiece;
	fromSquare.piece.reset();
	lastMove = move;
	
	if (isCapture) {
		countMaterial();
	}

	if (move.promoteInto.has_value()) {
		pendingPromo = move;
		promoMenuSide = selectedPiece.side;
		return capturedPiece;
	}

	pendingPromo.reset();
	promoMenuSide.reset();

	currentTurn = (currentTurn == Side::White) ? Side::Black : Side::White;
	if (currentTurn == Side::White) {
		moveCounter++;
	}
	
	if (isCapture or selectedPiece.type == PieceType::Pawn) {
		positionHistory.clear();
		halfmoveCounter = 0;
	}
	
	halfmoveCounter++;
	evaluate();
	updatePositionHistory();
	return capturedPiece;
}

void Board::handleKingMove(Piece & selectedPiece, const Move & move) {
	if (selectedPiece.side == Side::White) {
		whiteKingRank = move.toRank;
		whiteKingFile = move.toFile;
	}
	else {
		blackKingRank = move.toRank;
		blackKingFile = move.toFile;
	}

	if (std::abs(move.toFile - move.fromFile) == 2) {
		handleCastling(selectedPiece, move);
	}
}


std::vector<Move> Board::generateLegalMoves(const int rank, const int file) {
	std::vector<Move> pseudoLegalMoves = generatePseudoLegalMoves(rank, file);
	std::vector<Move> legalMoves;

	const Side& pieceSide = board[rank][file].piece->side;
	const Side& enemySide = (pieceSide == Side::White) ? Side::Black : Side::White;

	for (Move& move : pseudoLegalMoves) {
		std::optional<Piece> capturedPiece = board[move.toRank][move.toFile].piece;

		makeTempMove(move, capturedPiece);

		int kingRank = INVALID_COORD, kingFile = INVALID_COORD;
		if (pieceSide == Side::White) {
			kingRank = whiteKingRank;
			kingFile = whiteKingFile;
		}
		else {
			kingRank = blackKingRank;
			kingFile = blackKingFile;
		}

		if (isKingSafe(kingRank, kingFile, enemySide)) {
			move.score = scoreMove(move, capturedPiece);
			legalMoves.push_back(move);
		}

		if (capturedPiece.has_value() or move.promoteInto.has_value()) {
			move.isLoudMove = true;
		}

		unmakeMove(move, capturedPiece);
	}

	return legalMoves;
}
std::vector<Move> Board::generateEveryLegalMove(const Side side) {
	std::vector<Move> singlePieceMoves;
	std::vector<Move> moves;

	for (int rank = 0; rank < RANKS; rank++) {
		for (int file = 0; file < FILES; file++) {
			const std::optional<Piece> piece = board[rank][file].piece;

			if (piece.has_value() and piece->side == side) {
				singlePieceMoves = generateLegalMoves(rank, file);
				moves.insert(std::end(moves), std::begin(singlePieceMoves), std::end(singlePieceMoves));
			}
		}
	}


	return moves;
}


int Board::scoreMove(const Move& move, const std::optional<Piece>& capturedPiece) {
	int score{};

	if (capturedPiece.has_value()) {
		const std::optional<Piece>& attackingPiece = board[move.fromRank][move.fromFile].piece;

		int capturedPieceValue = 10 * getPieceSquareValue(capturedPiece.value(), move.toRank, move.toFile);
		int attackingPieceValue{};
		if (attackingPiece.has_value()) {
			attackingPieceValue = getPieceSquareValue(attackingPiece.value(), move.fromRank, move.fromFile);
		}
		score = capturedPieceValue - attackingPieceValue;
	}

	if (move.promoteInto.has_value()) {
		if (move.promoteInto.value() == PieceType::Queen) {
			score += 90000;
		}
	}

	return score;
}

void Board::makeTempMove(const Move& move, std::optional<Piece>& capturedPiece) {
	Square& fromSquare = board[move.fromRank][move.fromFile];
	Square& toSquare = board[move.toRank][move.toFile];
	Piece& selectedPiece = fromSquare.piece.value();

	MoveSnapshot newSnapshot = { enPassantRank, enPassantFile, selectedPiece.hasMoved, halfmoveCounter, currentEvaluation };
	moveSnapshots.push_back(newSnapshot);

	enPassantRank = INVALID_COORD;
	enPassantFile = INVALID_COORD;
	if (selectedPiece.type == PieceType::Pawn and (std::abs(move.toRank - move.fromRank) == 2)) {
		enPassantRank = move.fromRank + ((selectedPiece.side == Side::White) ? -1 : 1);
		enPassantFile = move.fromFile;
	}

	const bool moveIsEnPassant = selectedPiece.type == PieceType::Pawn and move.fromFile != move.toFile and !toSquare.piece.has_value();
	if (moveIsEnPassant) {
		capturedPiece = board[move.fromRank][move.toFile].piece;
		board[move.fromRank][move.toFile].piece.reset();
		updateMaterialCount(capturedPiece.value(), move.fromRank, move.toFile, -1);
	}
	else {
		capturedPiece = toSquare.piece;
		if (capturedPiece.has_value())
			updateMaterialCount(capturedPiece.value(), move.toRank, move.toFile, -1);
	}

	if (capturedPiece.has_value() or selectedPiece.type == PieceType::Pawn) {
		halfmoveCounter = 0;
	}
	
	halfmoveCounter++;
	
	if (selectedPiece.type == PieceType::King and std::abs(move.toFile - move.fromFile) == 2) {
		if (move.toFile > move.fromFile) {
			board[move.toRank][move.toFile - 1].piece = board[move.fromRank][7].piece;
			board[move.fromRank][7].piece.reset();
		}
		else {
			board[move.toRank][move.toFile + 1].piece = board[move.fromRank][0].piece;
			board[move.fromRank][0].piece.reset();
		}
	}

	if (move.promoteInto.has_value()) {
		updateMaterialCount(selectedPiece, move.fromRank, move.fromFile, -1);
		selectedPiece.type = move.promoteInto.value();

		Piece promotedPiece(selectedPiece.side, move.promoteInto.value(), true);
		updateMaterialCount(promotedPiece, move.toRank, move.toFile, 1);
	}

	if (selectedPiece.type == PieceType::King) {
		if (selectedPiece.side == Side::White) {
			whiteKingRank = move.toRank;
			whiteKingFile = move.toFile;
		}
		else {
			blackKingRank = move.toRank;
			blackKingFile = move.toFile;
		}
	}

	int oldSquareValue = getPieceSquareValue(selectedPiece, move.fromRank, move.fromFile);
	int newSquareValue = getPieceSquareValue(selectedPiece, move.toRank, move.toFile);

	int sign = (selectedPiece.side == Side::White) ? 1 : -1;

	currentEvaluation -= (oldSquareValue * sign);
	currentEvaluation += (newSquareValue * sign);

	if (capturedPiece.has_value()) {
		int capturedValue = getPieceSquareValue(capturedPiece.value(), move.toRank, move.toFile);
		int capturedSign = (capturedPiece.value().side == Side::White) ? 1 : -1;

		currentEvaluation -= (capturedValue * capturedSign);
	}

	selectedPiece.hasMoved = true;
	toSquare.piece = selectedPiece;
	fromSquare.piece.reset();
}

void Board::updateMaterialCount(const Piece& piece, const int rank, const int file, const int amount) {
	Material& materialSide = (piece.side == Side::White) ? boardMaterial.white : boardMaterial.black;

	switch (piece.type) {
	case PieceType::Pawn:
		materialSide.pawns += amount;
		break;
	case PieceType::Knight:
		materialSide.knights += amount;
		break;
	case PieceType::Bishop:
		if (determineSquareColour(rank, file) == Colour::Light) {
			materialSide.lightSquaredBishops += amount;
		}
		else {
			materialSide.darkSquaredBishops += amount;
		}
		break;
	case PieceType::Rook:
		materialSide.rooks += amount;
		break;
	case PieceType::Queen:
		materialSide.queens += amount;
		break;
	case PieceType::King:
		break;
	}
}

int Board::getPieceSquareValue(const Piece& piece, const int rank, const int file) {
	int pstIndex{};
	if (piece.side == Side::White) {
		pstIndex = rank * 8 + file;
	}
	else {
		pstIndex = (7 - rank) * 8 + file;
	}

	int pstValue{};
	switch (piece.type) {
	case PieceType::Pawn:
		pstValue = (pawnPST[pstIndex] + pawnValue);
		break;
	case PieceType::Knight:
		pstValue = (knightPST[pstIndex] + knightValue);
		break;
	case PieceType::Bishop:
		pstValue = (bishopPST[pstIndex] + bishopValue);
		break;
	case PieceType::Rook:
		pstValue = (rookPST[pstIndex] + rookValue);
		break;
	case PieceType::Queen:
		pstValue = (queenPST[pstIndex] + queenValue);
		break;
	case PieceType::King:
		pstValue = kingPSTMiddleGame[pstIndex];
		break;
	}

	return pstValue;;
}

bool Board::isKingSafe(const int& kingRank, const int& kingFile, const Side& enemySide) {
	const Side friendlySide = (enemySide == Side::White) ? Side::Black : Side::White;
	const int friendlyPawnDirection = (friendlySide == Side::White) ? -1 : 1;

	if (pawnCheck(friendlySide, enemySide, kingRank, kingFile, friendlyPawnDirection) == false) 
		return false;

	if (slidingPieceCheck(PieceType::Bishop, friendlySide, enemySide, kingRank, kingFile) == false) 
		return false;

	if (slidingPieceCheck(PieceType::Rook, friendlySide, enemySide, kingRank, kingFile) == false) 
		return false;

	if (knightCheck(friendlySide, enemySide, kingRank, kingFile) == false) 
		return false;

	if (kingCheck(friendlySide, enemySide, kingRank, kingFile) == false)
		return false;

	return true;
}

bool Board::pawnCheck(const Side friendlySide, const Side enemySide, const int kingRank, const int kingFile, const int friendlyPawnDirection) {
	const std::array<std::pair<int, int>, 2> pawnDirections = { {
		{friendlyPawnDirection, -1}, {friendlyPawnDirection, 1}
	} };

	for (const std::pair<int, int>& direction : pawnDirections) {
		const int newRank = kingRank + direction.first;
		const int newFile = kingFile + direction.second;

		if (newRank > 7 or newRank < 0 or newFile > 7 or newFile < 0) 
			continue;

		const std::optional<Piece>& potentialPiece = board[newRank][newFile].piece;
		if (potentialPiece.has_value()) {
			if (potentialPiece->side == friendlySide)
				continue;
			else if (potentialPiece->side == enemySide and potentialPiece->type == PieceType::Pawn)
				return false;
		}
	}

	return true;
}
bool Board::knightCheck(const Side friendlySide, const Side enemySide, const int kingRank, const int kingFile) {
	for (const std::pair<int, int>& direction : knightDirections) {
		const int newRank = kingRank + direction.first;
		const int newFile = kingFile + direction.second;

		if (newRank > 7 or newRank < 0 or newFile > 7 or newFile < 0) 
			continue;

		const std::optional<Piece>& potentialPiece = board[newRank][newFile].piece;

		if (potentialPiece.has_value()) {
			if (potentialPiece->side == enemySide and potentialPiece->type == PieceType::Knight)
				return false;
		}
	}
	return true;
}
bool Board::slidingPieceCheck(const PieceType slidingPiece, const Side friendlySide, const Side enemySide, const int kingRank, const int kingFile) {
	for (const std::pair<int, int>& direction : (slidingPiece == PieceType::Bishop) ? bishopDirections : rookDirections) {
		int countToEdge = 1;

		int newRank = kingRank + direction.first;
		int newFile = kingFile + direction.second;

		bool withinBoard = !(newRank > 7 or newRank < 0 or newFile > 7 or newFile < 0);

		while (withinBoard) {
			const std::optional<Piece>& potentialPiece = board[newRank][newFile].piece;

			if (potentialPiece.has_value()) {
				if (potentialPiece->side == friendlySide)
					break;
				else if (potentialPiece->side == enemySide and
					    (potentialPiece->type == slidingPiece or
						 potentialPiece->type == PieceType::Queen))
				{
					return false;
				}
				else
					break;
			}

			countToEdge++;
			newRank = kingRank + direction.first * countToEdge;
			newFile = kingFile + direction.second * countToEdge;
			withinBoard = !(newRank > 7 or newRank < 0 or newFile > 7 or newFile < 0);
		}
	}

	return true;
}
bool Board::kingCheck(const Side friendlySide, const Side enemySide, const int kingRank, const int kingFile) {
	for (const std::pair<int, int>& direction : kingDirections) {
		const int newRank = kingRank + direction.first;
		const int newFile = kingFile + direction.second;

		if (newRank > 7 or newRank < 0 or newFile > 7 or newFile < 0) 
			continue;

		const std::optional<Piece>& potentialPiece = board[newRank][newFile].piece;
		if (potentialPiece.has_value()) {
			if (potentialPiece->side == friendlySide)
				continue;
			else if (potentialPiece->side == enemySide and potentialPiece->type == PieceType::King)
				return false;
		}
	}

	return true;
}

void Board::unmakeMove(const Move& move, const std::optional<Piece>& capturedPiece) {
	Square& fromSquare = board[move.fromRank][move.fromFile];
	Square& toSquare = board[move.toRank][move.toFile];
	Piece selectedPiece = toSquare.piece.value();

	MoveSnapshot lastSnapshot = moveSnapshots.back();
	moveSnapshots.pop_back();

	enPassantRank = lastSnapshot.previousEnPassantRank;
	enPassantFile = lastSnapshot.previousEnPassantFile;
	selectedPiece.hasMoved = lastSnapshot.pieceHasAlreadyMoved;
	halfmoveCounter = lastSnapshot.previousHalfmoveCounter;
	currentEvaluation = lastSnapshot.previousEvaluation;

	if (move.promoteInto.has_value()) {
		selectedPiece.type = PieceType::Pawn;
		Piece promotedPiece(selectedPiece.side, move.promoteInto.value(), true);
		updateMaterialCount(promotedPiece, move.toRank, move.toFile, -1);

		Piece originalPawn(selectedPiece.side, PieceType::Pawn, true);
		updateMaterialCount(originalPawn, move.fromRank, move.fromFile, 1);
	}

	if (selectedPiece.type == PieceType::King and std::abs(move.toFile - move.fromFile) == 2) {
		if (move.toFile > move.fromFile) {
			board[move.fromRank][7].piece = board[move.toRank][move.toFile - 1].piece;
			board[move.toRank][move.toFile - 1].piece.reset();
		}
		else {
			board[move.fromRank][0].piece = board[move.toRank][move.toFile + 1].piece;
			board[move.toRank][move.toFile + 1].piece.reset();
		}
	}

	const bool moveWasEnPassant = selectedPiece.type == PieceType::Pawn and move.fromFile != move.toFile and move.toRank == enPassantRank and move.toFile == enPassantFile;
	if (moveWasEnPassant) {
		board[move.fromRank][move.toFile].piece = capturedPiece;
		updateMaterialCount(capturedPiece.value(), move.fromRank, move.toFile, 1);
		toSquare.piece.reset();
	}
	else {
		toSquare.piece = capturedPiece;
		if (toSquare.piece.has_value())
			updateMaterialCount(toSquare.piece.value(), move.toRank, move.toFile, 1);
	}

	if (selectedPiece.type == PieceType::King) {
		if (selectedPiece.side == Side::White) {
			whiteKingRank = move.fromRank;
			whiteKingFile = move.fromFile;
		}
		else {
			blackKingRank = move.fromRank;
			blackKingFile = move.fromFile;
		}
	}

	fromSquare.piece = selectedPiece;
}

std::vector<Move> Board::generatePseudoLegalMoves(const int rank, const int file) {
	std::vector<Move> pseudoLegalMoves;
	Piece& movingPiece = board[rank][file].piece.value();

	switch (movingPiece.type) {
	case PieceType::Pawn:
		generatePawnMoves(movingPiece, rank, file, pseudoLegalMoves);
		break;
	case PieceType::Knight:
		generateKnightMoves(movingPiece, rank, file, pseudoLegalMoves);
		break;
	case PieceType::Bishop:
		generateSlidingPieceMoves(PieceType::Bishop, rank, file, pseudoLegalMoves);
		break;
	case PieceType::Rook:
		generateSlidingPieceMoves(PieceType::Rook, rank, file, pseudoLegalMoves);
		break;
	case PieceType::Queen:
		generateQueenMoves(PieceType::Queen, rank, file, pseudoLegalMoves);
		break;
	case PieceType::King:
		generateKingMoves(movingPiece, rank, file, pseudoLegalMoves);
		break;
	}

	return pseudoLegalMoves;
}

void Board::generatePawnMoves(const Piece& pawn, const int rank, const int file, std::vector<Move>& generatedMoves) {
	int dir = (pawn.side == Side::White) ? -1 : 1;
	int oneRank = rank + dir;
	int twoRanks = rank + dir * 2;

	std::array<std::pair<int, int>, 2> captureSquares = { {
		{dir, -1}, {dir, 1}
	} };

	if (oneRank >= 0 and oneRank < RANKS) {
		if (!board[oneRank][file].piece.has_value()) {
			if (oneRank == 0 or oneRank == 7) {
				for (const PieceType& pieceType : promotionPieceTypes) {
					generatedMoves.push_back(Move{ rank, file, oneRank, file, pieceType });
				}
			}
			else
				generatedMoves.push_back(Move{ rank, file, oneRank, file });

			if (!pawn.hasMoved and twoRanks >= 0 and twoRanks < RANKS) {
				if (!board[twoRanks][file].piece.has_value()) {
					generatedMoves.push_back(Move{ rank, file, twoRanks, file });
				}
			}
		}
	}

	for (const std::pair<int, int>& captureSquare : captureSquares) {
		int newRank = rank + captureSquare.first;
		int newFile = file + captureSquare.second;

		if (newRank > 7 or newRank < 0 or newFile > 7 or newFile < 0) 
			continue;

		std::optional<Piece>& potentialPiece = board[newRank][newFile].piece;

		if (potentialPiece.has_value()) {
			if ((newRank == 0 or newRank == 7) and potentialPiece->side != pawn.side) {
				for (const PieceType& pieceType : promotionPieceTypes) {
					generatedMoves.push_back(Move{ rank, file, newRank, newFile, pieceType });
				}
			}
			else if (potentialPiece->side != pawn.side) {
				generatedMoves.push_back(Move{ rank, file, newRank, newFile });
			}
		}
		else if (newRank == enPassantRank and newFile == enPassantFile) {
			generatedMoves.push_back(Move{ rank, file, newRank, newFile });
		}
	}
}
void Board::generateKnightMoves(const Piece& knight, const int rank, const int file, std::vector<Move>& generatedMoves) {
	for (const std::pair<int, int>& direction : knightDirections) {
		const int newRank = rank + direction.first;
		const int newFile = file + direction.second;

		if (newRank > 7 or newRank < 0 or newFile > 7 or newFile < 0) continue;

		const std::optional<Piece>& potentialPiece = board[newRank][newFile].piece;

		if (potentialPiece.has_value()) {
			if (potentialPiece->side == knight.side) 
				continue;
		}

		if (!potentialPiece.has_value() or
			potentialPiece->side != knight.side) {
			generatedMoves.push_back(Move{ rank, file, newRank, newFile });
		}
	}
}
void Board::generateSlidingPieceMoves(const PieceType& slidingPieceType, const int rank, const int file, std::vector<Move>& generatedMoves) {
	for (const std::pair<int, int>& direction : (slidingPieceType == PieceType::Bishop) ? bishopDirections : rookDirections) {
		int countToEdge = 1;
		const Piece& slidingPiece = board[rank][file].piece.value();

		int newRank = rank + direction.first;
		int newFile = file + direction.second;

		bool withinBoard = !(newRank > 7 or newRank < 0 or newFile > 7 or newFile < 0);

		while (withinBoard) {
			const std::optional<Piece>& potentialPiece = board[newRank][newFile].piece;

			if (potentialPiece.has_value()) {
				if (potentialPiece->side == slidingPiece.side) 
					break;

				generatedMoves.push_back(Move{ rank, file, newRank, newFile });
				break;
			}

			generatedMoves.push_back(Move{ rank, file, newRank, newFile });
			countToEdge++;
			newRank = rank + direction.first * countToEdge;
			newFile = file + direction.second * countToEdge;
			withinBoard = !(newRank > 7 or newRank < 0 or newFile > 7 or newFile < 0);
		}
	}
}
void Board::generateQueenMoves(const PieceType& slidingPieceType, const int rank, const int file, std::vector<Move>& generatedMoves) {
	generateSlidingPieceMoves(PieceType::Bishop, rank, file, generatedMoves);
	generateSlidingPieceMoves(PieceType::Rook, rank, file, generatedMoves);
}
void Board::generateKingMoves(const Piece& king, const int rank, const int file, std::vector<Move>& generatedMoves) {
	for (const std::pair<int, int>& direction : kingDirections) {
		int newRank = rank + direction.first;
		int newFile = file + direction.second;

		if (newRank > 7 or newRank < 0 or newFile > 7 or newFile < 0) 
			continue;

		const std::optional<Piece>& potentialPiece = board[newRank][newFile].piece;

		if (potentialPiece.has_value()) {
			if (potentialPiece->side == king.side) 
				continue;
		}

		if (!potentialPiece.has_value() or
			potentialPiece->side != king.side) {
			generatedMoves.push_back(Move{ rank, file, newRank, newFile });
		}
	} 

	if (king.hasMoved)
		return;

	const Side& enemySide = (currentTurn == Side::White) ? Side::Black : Side::White;

	if (!isKingSafe(rank, file, enemySide))
		return;

	generateCastlingMoves(enemySide, rank, file, generatedMoves);
}

void Board::generateCastlingMoves(const Side enemySide, const int rank, const int file, std::vector<Move>& generatedMoves) {
	generateKingsideCastle(enemySide, rank, file, generatedMoves);
	generateQueensideCastle(enemySide, rank, file, generatedMoves);
}
void Board::generateKingsideCastle(const Side enemySide, const int rank, const int file, std::vector<Move>& generatedMoves) {
	if (!(file <= 5)) {
		return;
	}

	const std::optional<Piece>& kingRook = board[rank][7].piece;

	if (!kingRook.has_value()) {
		return;
	}

	if (kingRook->type != PieceType::Rook or kingRook->hasMoved) {
		return;
	}

	if (board[rank][file + 1].piece.has_value() or
		board[rank][file + 2].piece.has_value())
	{
		return;
	}

	if (isKingSafe(rank, file + 1, enemySide)) {
		generatedMoves.push_back(Move{ rank, file, rank, file + 2 });
	}
}
void Board::generateQueensideCastle(const Side enemySide, const int rank, const int file, std::vector<Move>& generatedMoves) {
	if (!(file >= 3)) {
		return;
	}

	const std::optional<Piece>& queenRook = board[rank][0].piece;

	if (!queenRook.has_value()) {
		return;
	}

	if (queenRook->type != PieceType::Rook or queenRook->hasMoved) {
		return;
	}

	if (board[rank][file - 1].piece.has_value() or
		board[rank][file - 2].piece.has_value() or
		board[rank][file - 3].piece.has_value())
	{
		return;
	}

	if (isKingSafe(rank, file - 1, enemySide)) {
		generatedMoves.push_back(Move{ rank, file, rank, file - 2 });
	}
}

void Board::countMaterial() {
	boardMaterial = BoardMaterial();

	auto incrementBishopCount = [](Material& materialSide, const Colour bishopColour) {
		if (bishopColour == Colour::Light)
			materialSide.lightSquaredBishops++;
		else
			materialSide.darkSquaredBishops++;
	};
	
	for (int rank = 0; rank < RANKS; rank++) {
		for (int file = 0; file < FILES; file++) {

			const std::optional<Piece>& potentialPiece = board[rank][file].piece;
			if (!potentialPiece.has_value())
				continue;

			switch (potentialPiece->type) {
			case PieceType::Pawn:
				if (potentialPiece->side == Side::White)
					boardMaterial.white.pawns++;
				else 
					boardMaterial.black.pawns++;
				break;
			case PieceType::Knight:
				if (potentialPiece->side == Side::White)
					boardMaterial.white.knights++;
				else
					boardMaterial.black.knights++;
				break;
			case PieceType::Bishop:
				incrementBishopCount((potentialPiece->side == Side::White) ? boardMaterial.white : boardMaterial.black, board[rank][file].colour);
				break;
			case PieceType::Rook:
				if (potentialPiece->side == Side::White)
					boardMaterial.white.rooks++;
				else
					boardMaterial.black.rooks++;
				break;
			case PieceType::Queen:
				if (potentialPiece->side == Side::White)
					boardMaterial.white.queens++;
				else
					boardMaterial.black.queens++;
				break;
			}
		}
	}
}

int Board::getPieceCount() const {
	int count = 2;
	count += boardMaterial.white.pawns + boardMaterial.black.pawns;
	count += boardMaterial.white.knights + boardMaterial.black.knights;
	count += boardMaterial.white.lightSquaredBishops + boardMaterial.white.darkSquaredBishops;
	count += boardMaterial.black.lightSquaredBishops + boardMaterial.black.darkSquaredBishops;
	count += boardMaterial.white.rooks + boardMaterial.black.rooks;
	count += boardMaterial.white.queens + boardMaterial.black.queens;

	return count;
}

unsigned Board::probeTablebaseWDL() const {
	SyzygyBoard bitboard = getSyzygyBoards();

	unsigned epSquare = 0;
	if (enPassantRank != INVALID_COORD and enPassantFile != INVALID_COORD) {
		epSquare = (7 - enPassantRank) * 8 + enPassantFile;
	}

	return tb_probe_wdl(
		bitboard.white, bitboard.black, 
		bitboard.kings, bitboard.queens, bitboard.rooks, bitboard.bishops, bitboard.knights, bitboard.pawns, 
		0, 0, epSquare, (currentTurn == Side::White)
	);
}

SyzygyBoard Board::getSyzygyBoards() const {
	SyzygyBoard bits = { 0 };
	for (int rank = 0; rank < RANKS; rank++) {
		for (int file = 0; file < FILES; file++) {
			const std::optional<Piece>& potentialPiece = board[rank][file].piece;
			if (!potentialPiece.has_value()) continue;

			int bitPos = (7 - rank) * 8 + file;
			uint64_t bit = 1ULL << bitPos;

			if (potentialPiece->side == Side::White) bits.white |= bit;
			else bits.black |= bit;

			switch (potentialPiece->type) {
			case PieceType::King:
				bits.kings |= bit;
				break;
			case PieceType::Queen:
				bits.queens |= bit;
				break;
			case PieceType::Rook:
				bits.rooks |= bit;
				break;
			case PieceType::Bishop:
				bits.bishops |= bit;
				break;
			case PieceType::Knight:
				bits.knights |= bit;
				break;
			case PieceType::Pawn:
				bits.pawns |= bit;
				break;
			}
		}
	}
	return bits;
}

std::string Board::generateFenString() {
	std::stringstream stringStream;

	generateBoardPart(stringStream);

	stringStream << ((currentTurn == Side::White) ? " w" : " b");

	generateCastlingPart(stringStream);
	generateEnPassantPart(stringStream);

	return stringStream.str();
}
void Board::generateBoardPart(std::stringstream& stringStream) const {
	for (int rank = 0; rank < RANKS; rank++) {
		int spaces = 0;

		for (int file = 0; file < FILES; file++) {
			const std::optional<Piece>& potentialPiece = board[rank][file].piece;

			if (!potentialPiece.has_value()) {
				spaces++;
				continue;
			}

			if (spaces != 0) {
				stringStream << spaces;
				spaces = 0;
			}

			const bool isWhitePiece = potentialPiece->side == Side::White;

			switch (potentialPiece->type) {
			case PieceType::Pawn:
				stringStream << ((isWhitePiece) ? 'P' : 'p');
				break;
			case PieceType::Knight:
				stringStream << ((isWhitePiece) ? 'N' : 'n');
				break;
			case PieceType::Bishop:
				stringStream << ((isWhitePiece) ? 'B' : 'b');
				break;
			case PieceType::Rook:
				stringStream << ((isWhitePiece) ? 'R' : 'r');
				break;
			case PieceType::Queen:
				stringStream << ((isWhitePiece) ? 'Q' : 'q');
				break;
			case PieceType::King:
				stringStream << ((isWhitePiece) ? 'K' : 'k');
				break;
			}
		}

		if (spaces != 0) {
			stringStream << spaces;
		}

		if (rank != RANKS - 1) {
			stringStream << '/';
		}
	}
}

void Board::generateCastlingPart(std::stringstream& stringStream) {
	const Piece& whiteKing = board[whiteKingRank][whiteKingFile].piece.value();
	const Piece& blackKing = board[blackKingRank][blackKingFile].piece.value();

	if (whiteKing.hasMoved and blackKing.hasMoved) {
		stringStream << " -";
	}
	else {
		checkCastlingRights(stringStream, whiteKing, blackKing);
	}
}
void Board::checkCastlingRights(std::stringstream& stringStream, const Piece& whiteKing, const Piece& blackKing) {
	stringStream << ' ';
	bool rightsAdded = false;

	const std::optional<Piece>& whiteKingsideSpot = board[whiteKingRank][7].piece;
	const std::optional<Piece>& whiteQueensideSpot = board[whiteKingRank][0].piece;

	const std::optional<Piece>& blackKingsideSpot = board[blackKingRank][7].piece;
	const std::optional<Piece>& blackQueensideSpot = board[blackKingRank][0].piece;

	auto checkIfRookCanCastle = [&](const char letter, const std::optional<Piece>& rook) -> void {
		if (rook.has_value()) {
			if (rook->type == PieceType::Rook and !rook->hasMoved) {
				stringStream << letter;
				rightsAdded = true;
			}
		}
	};

	if (!whiteKing.hasMoved) {
		checkIfRookCanCastle('K', whiteKingsideSpot);
		checkIfRookCanCastle('Q', whiteQueensideSpot);
	}

	if (!blackKing.hasMoved) {
		checkIfRookCanCastle('k', blackKingsideSpot);
		checkIfRookCanCastle('q', blackQueensideSpot);
	}

	if (!rightsAdded) {
		stringStream << '-';
	}
}
void Board::generateEnPassantPart(std::stringstream& stringStream) const {
	if (enPassantRank == INVALID_COORD) {
		stringStream << " -";
	}
	else {
		const int rank = 8 - enPassantRank;
		const char file = static_cast<char>('a' + enPassantFile);

		stringStream << ' ' << file << rank;
	}
}

void Board::updatePositionHistory() {
	const std::string currentFen = generateFenString();
	positionHistory[currentFen]++;
}

bool Board::isDrawByInsufficientMaterial() const {
	if (boardMaterial.white.pawns != 0 or boardMaterial.black.pawns != 0 or
		boardMaterial.white.rooks != 0 or boardMaterial.black.rooks != 0 or
		boardMaterial.white.queens != 0 or boardMaterial.black.queens != 0) {
		return false;
	}

	const int whiteMinorPieceCount = boardMaterial.white.knights + boardMaterial.white.lightSquaredBishops + boardMaterial.white.darkSquaredBishops;
	const int blackMinorPieceCount = boardMaterial.black.knights + boardMaterial.black.lightSquaredBishops + boardMaterial.black.darkSquaredBishops;

	if ((whiteMinorPieceCount <= 1 and blackMinorPieceCount == 0) or (whiteMinorPieceCount == 0 and blackMinorPieceCount <= 1)) {
		return true;
	}
	else if (whiteMinorPieceCount == 1 and blackMinorPieceCount == 1) {
		if ((boardMaterial.white.lightSquaredBishops == 1 and boardMaterial.black.lightSquaredBishops == 1) or
			(boardMaterial.white.darkSquaredBishops == 1 and boardMaterial.black.darkSquaredBishops == 1)) {
			return true;
		}
	}
	
	if ((boardMaterial.white.knights == 2 and whiteMinorPieceCount == 2 and blackMinorPieceCount == 0) or 
		(boardMaterial.black.knights == 2 and blackMinorPieceCount == 2 and whiteMinorPieceCount == 0)) {
		return true;
	}

	return false;
}
bool Board::isThreeFoldRepetition() const {
	for (const auto& position : positionHistory) {
		if (position.second >= 3) {
			return true;
		}
	}
	return false;
}
bool Board::canSideCheckmate(const Side& side) const {

	auto hasSufficientMaterial = [](const Material& materialSide) -> bool {
		if (materialSide.pawns != 0 or
			materialSide.rooks != 0 or
			materialSide.queens != 0) {
			return true;
		}

		const int minorPieceCount = materialSide.knights + materialSide.lightSquaredBishops + materialSide.darkSquaredBishops;

		if (minorPieceCount >= 2) {
			if (materialSide.knights == 2 and minorPieceCount == 2) {
				return false;
			}
			return true;
		}

		return false;
	};

	if (side == Side::White) {
		return hasSufficientMaterial(boardMaterial.white);
	}
	else {
		return hasSufficientMaterial(boardMaterial.black);
	}

	return false;
}

void Board::evaluate() {
	int whiteMaterial = 0;
	int blackMaterial = 0;
	int totalNonPawnMaterial = 0;

	for (int rank = 0; rank < RANKS; rank++) {
		for (int file = 0; file < FILES; file++) {
			const std::optional<Piece>& potentialPiece = board[rank][file].piece;
			if (!potentialPiece.has_value()) continue;

			int value = 0;
			bool isPawn = false;

			switch (potentialPiece->type) {
				case PieceType::Pawn:   
					value = pawnValue; 
					isPawn = true; 
					break;
				case PieceType::Knight: 
					value = knightValue; 
					break;
				case PieceType::Bishop: 
					value = bishopValue; 
					break;
				case PieceType::Rook:   
					value = rookValue; 
					break;
				case PieceType::Queen:  
					value = queenValue; 
					break;
				case PieceType::King:   
					value = 0; 
					break;
			}

			if (potentialPiece->side == Side::White) {
				whiteMaterial += value;
			}
			else {
				blackMaterial += value;
			}

			if (!isPawn and potentialPiece->type != PieceType::King) {
				totalNonPawnMaterial += value;
			}
		}
	}

	endgameFactor = 1.0f - (static_cast<float>(totalNonPawnMaterial) / 6200.0f);
	if (endgameFactor < 0.0f) endgameFactor = 0.0f;
	if (endgameFactor > 1.0f) endgameFactor = 1.0f;

	int pstIndex{};
	int pstValue{};
	currentEvaluation = 0;

	for (int rank = 0; rank < RANKS; rank++) {
		for (int file = 0; file < FILES; file++) {
			const std::optional<Piece>& potentialPiece = board[rank][file].piece;

			if (!potentialPiece.has_value()) {
				continue;
			}

			const bool isWhitePiece = (potentialPiece->side == Side::White) ? true : false;
			if (isWhitePiece) {
				pstIndex = rank * 8 + file;
			}
			else {
				pstIndex = (7 - rank) * 8 + file;
			}

			const int sign = (isWhitePiece) ? 1 : -1;

			switch (potentialPiece->type) {
			case PieceType::Pawn:
				currentEvaluation += sign * (pawnPST[pstIndex] + pawnValue);
				break;
			case PieceType::Knight:
				currentEvaluation += sign * (knightPST[pstIndex] + knightValue);
				break;
			case PieceType::Bishop:
				currentEvaluation += sign * (bishopPST[pstIndex] + bishopValue);
				break;
			case PieceType::Rook:
				currentEvaluation += sign * (rookPST[pstIndex] + rookValue);
				break;
			case PieceType::Queen:
				currentEvaluation += sign * (queenPST[pstIndex] + queenValue);
				break;
			case PieceType::King:
				float blendedKingPST = (kingPSTMiddleGame[pstIndex] * (1.0f - endgameFactor)) + 
									   (kingPSTEndGame[pstIndex] * endgameFactor);
				currentEvaluation += sign * static_cast<int>(blendedKingPST);
				break;
			}
		}
	}

	int materialAdvantage = whiteMaterial - blackMaterial;

	if (materialAdvantage > 300) {
		currentEvaluation += static_cast<int>(getMopUpScore(Side::White) * endgameFactor);
	}
	else if (materialAdvantage < -300) {
		currentEvaluation -= static_cast<int>(getMopUpScore(Side::Black) * endgameFactor);
	}

	int sign = (currentTurn == Side::White) ? 1 : -1;
	currentEvaluation -= static_cast<int>(halfmoveCounter * 5) * sign;
}

void Board::loadFen(const std::string& fen) {
	std::stringstream stringStream(fen);
	std::string boardPart, turnPart, castlingPart, enPassantPart;

	stringStream >> boardPart >> turnPart >> castlingPart >> enPassantPart >> halfmoveCounter >> moveCounter;

	currentTurn = (turnPart == "w") ? Side::White : Side::Black;

	bool whiteKingSide = false, whiteQueenSide = false;
	bool blackKingSide = false, blackQueenSide = false;

	for (const char c : castlingPart) {
		switch (c) {
		case '-':
			break;
		case 'K':
			whiteKingSide = true;
			break;
		case 'Q':
			whiteQueenSide = true;
			break;
		case 'k':
			blackKingSide = true;
			break;
		case 'q':
			blackQueenSide = true;
			break;
		}
	}

	loadEnPassantPart(enPassantPart);

	loadBoardPart(boardPart, whiteKingSide, whiteQueenSide, blackKingSide, blackQueenSide);

	evaluate();
}

void Board::loadEnPassantPart(const std::string& enPassantPart) {
	for (const char c : enPassantPart) {
		if (c == '-') return;

		if (std::isdigit(static_cast<unsigned char>(c))) {
			enPassantRank = 8 - (c - '0');
		}
		else {
			enPassantFile = c - 'a';
		}
	}
}

void Board::createPiece(const int rank, const int file, const Side side, const PieceType type, const bool hasMoved) {
	Piece newPiece(side, type, hasMoved);
	board[rank][file].piece = newPiece;
}

void Board::loadBoardPart(const std::string& boardPart, bool whiteKingSide, bool whiteQueenSide, bool blackKingSide, bool blackQueenSide) {

	int rank = 0;
	int file = 0;

	for (const char c : boardPart) {
		if (c == '/') {
			rank++;
			file = 0;
			continue;
		}

		if (std::isdigit(static_cast<unsigned char>(c))) {
			file += c - '0';
			continue;
		}

		const Side side = std::isupper(static_cast<unsigned char>(c)) ? Side::White : Side::Black;
		bool hasMoved = false;

		PieceType type;
		switch (static_cast<char>(std::tolower(static_cast<unsigned char>(c)))) {
		case 'p':
			type = PieceType::Pawn;
			if (rank != 1 and side == Side::Black) hasMoved = true;
			else if (rank != 6 and side == Side::White) hasMoved = true;
			break;
		case 'n':
			type = PieceType::Knight;
			break;
		case 'b':
			type = PieceType::Bishop;
			break;
		case 'r':
			type = PieceType::Rook;
			hasMoved = true;

			if (side == Side::White) {
				if ((rank == 7 and file == 7) and whiteKingSide)
					hasMoved = false;
				else if ((rank == 7 and file == 0) and whiteQueenSide)
					hasMoved = false;
			}
			else {
				if ((rank == 0 and file == 7) and blackKingSide)
					hasMoved = false;
				else if ((rank == 0 and file == 0) and blackQueenSide)
					hasMoved = false;
			}
			break;
		case 'q':
			type = PieceType::Queen;
			break;
		case 'k':
			type = PieceType::King;
			hasMoved = true;

			if (side == Side::White) {
				if (whiteKingSide or whiteQueenSide)
					hasMoved = false;
				whiteKingRank = rank;
				whiteKingFile = file;
			}
			else {
				if (blackKingSide or blackQueenSide)
					hasMoved = false;
				blackKingRank = rank;
				blackKingFile = file;
			}
			break;
		default:
			continue;
		}

		if (rank < RANKS && file < FILES) {
			createPiece(rank, file, side, type, hasMoved);
		}
		file++;
	}
}

void Board::loadOpeningBook() {
	std::ifstream openingBookFile("data/opening-book-file.txt");
	std::string line;

	while (std::getline(openingBookFile, line)) {
		std::istringstream stringStream(line);
		std::vector<std::string> lineParts;
		std::string part;

		while (stringStream >> part) {
			lineParts.push_back(part);
		}

		if (lineParts.size() < 5)
			continue;

		std::string fullFen = lineParts.at(0) + " " + lineParts.at(1) + " " + lineParts.at(2) + " " + lineParts.at(3);

		std::string move;
		std::vector<std::string> grandmasterMoves;

		for (std::size_t i = 4; i < lineParts.size(); ++i) {
			std::size_t colonPos = lineParts[i].find(':');

			if (colonPos != std::string::npos) {
				OpeningBookMove bookMove;
				bookMove.uciMove = lineParts[i].substr(0, colonPos);
				bookMove.weight = std::stoi(lineParts[i].substr(colonPos + 1));

				openingBook[fullFen].push_back(bookMove);
			}
		}
	}

	openingBookFile.close();
}

int Board::getMopUpScore(Side winningSide) const {
	int mopUpScore = 0;

	int friendlyRank = (winningSide == Side::White) ? whiteKingRank : blackKingRank;
	int friendlyFile = (winningSide == Side::White) ? whiteKingFile : blackKingFile;
	int enemyRank = (winningSide == Side::White) ? blackKingRank : whiteKingRank;
	int enemyFile = (winningSide == Side::White) ? blackKingFile : whiteKingFile;

	int rankDistToCenter = std::max(3 - enemyRank, enemyRank - 4);
	int fileDistToCenter = std::max(3 - enemyFile, enemyFile - 4);
	mopUpScore += (rankDistToCenter + fileDistToCenter) * 10;

	int kingsRankDist = std::abs(friendlyRank - enemyRank);
	int kingsFileDist = std::abs(friendlyFile - enemyFile);
	mopUpScore += (14 - (kingsRankDist + kingsFileDist)) * 4;

	return mopUpScore;
}

int Board::getCurrentEvaluation() const {
	return currentEvaluation;
}

Move Board::getBestMoveIterative(int allocatedTimeMs) {
	auto startTime = std::chrono::high_resolution_clock::now();

	std::vector<Move> everyPossibleMove = generateEveryLegalMove(currentTurn);
	if (everyPossibleMove.empty()) return Move{};

	if (getPieceCount() <= 5) {
		SyzygyBoard syzygyBoard = getSyzygyBoards();
		unsigned epSquare = (enPassantRank != INVALID_COORD) ? ((7 - enPassantRank) * 8 + enPassantFile) : 0;

		unsigned tbResult = tb_probe_root(syzygyBoard.white, syzygyBoard.black, syzygyBoard.kings, syzygyBoard.queens, syzygyBoard.rooks, syzygyBoard.bishops, syzygyBoard.knights, syzygyBoard.pawns, static_cast<unsigned int>(halfmoveCounter), 0, epSquare, (currentTurn == Side::White), nullptr);

		if (tbResult != TB_RESULT_FAILED and tbResult != TB_RESULT_CHECKMATE and tbResult != TB_RESULT_STALEMATE) {
			int fromSquare = TB_GET_FROM(tbResult);
			int toSquare = TB_GET_TO(tbResult);
			int fathomPromo = TB_GET_PROMOTES(tbResult);

			int fromRank = 7 - (fromSquare / 8);
			int fromFile = fromSquare % 8;
			int toRank = 7 - (toSquare / 8);
			int toFile = toSquare % 8;

			for (const Move& move : everyPossibleMove) {
				if (move.fromRank == fromRank and move.fromFile == fromFile and
					move.toRank == toRank and move.toFile == toFile) {

					if (fathomPromo != TB_PROMOTES_NONE) {
						PieceType promoType = PieceType::Queen;
						if (fathomPromo == TB_PROMOTES_ROOK) 
							promoType = PieceType::Rook;
						else if (fathomPromo == TB_PROMOTES_BISHOP) 
							promoType = PieceType::Bishop;
						else if (fathomPromo == TB_PROMOTES_KNIGHT) 
							promoType = PieceType::Knight;

						if (move.promoteInto == promoType) {
							return move;
						}
					}
					else {
						return move;
					}
				}
			}
		}
	}

	std::sort(everyPossibleMove.begin(), everyPossibleMove.end(), [](const Move& moveOne, const Move& moveTwo) {
		return moveOne.score > moveTwo.score;
	});

	Move bestMove = everyPossibleMove[0];

	const std::string currentFen = generateFenString();

	if (openingBook.contains(currentFen)) {
		bestMove = getMoveInOpeningBook(everyPossibleMove, currentFen);
		return bestMove;
	}

	for (int currentDepth = 1; currentDepth <= MAX_SEARCH_DEPTH; currentDepth++) {
		Move bestMoveThisDepth = everyPossibleMove[0];

		int bestScore = (currentTurn == Side::White) ? WHITE_LOSS : BLACK_LOSS;
		int whiteGuaranteed = WHITE_LOSS;
		int blackGuaranteed = BLACK_LOSS;

		for (const Move& move : everyPossibleMove) {

			auto currentTime = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();

			if (abortSearch or (currentDepth > 1 and elapsed >= allocatedTimeMs)) {
				return bestMove;
			}

			std::optional<Piece> capturedPiece = std::nullopt;
			makeTempMove(move, capturedPiece);
			currentTurn = (currentTurn == Side::White) ? Side::Black : Side::White;

			const std::string currentFen = generateFenString();
			positionHistory[currentFen]++;

			int score = Minimax(currentDepth - 1, whiteGuaranteed, blackGuaranteed, currentTurn);

			positionHistory[currentFen]--;
			if (positionHistory[currentFen] == 0) {
				positionHistory.erase(currentFen);
			}

			currentTurn = (currentTurn == Side::White) ? Side::Black : Side::White;
			unmakeMove(move, capturedPiece);

			if (currentTurn == Side::White) {
				if (score > bestScore) {
					bestScore = score;
					bestMoveThisDepth = move;
				}
				whiteGuaranteed = std::max(whiteGuaranteed, bestScore);
			}
			else {
				if (score < bestScore) {
					bestScore = score;
					bestMoveThisDepth = move;
				}
				blackGuaranteed = std::min(blackGuaranteed, bestScore);
			}
		}

		bestMove = bestMoveThisDepth;
	}

	return bestMove;
}

Move Board::getMoveInOpeningBook(const std::vector<Move>& everyPossibleMove, const std::string currentFen) {
	const std::vector<OpeningBookMove>& moveChoices = openingBook[currentFen];

	std::vector<int> weights;
	for (const OpeningBookMove& moveChoice : moveChoices) {
		weights.push_back(moveChoice.weight);
	}
	
	int chosenUCIIndex = getUCIIndexFromDistribution(weights);
	std::string chosenUCIMove = moveChoices.at(chosenUCIIndex).uciMove;

	return parseUCI(everyPossibleMove, chosenUCIMove);
}

int Board::getUCIIndexFromDistribution(const std::vector<int>& weights) {
	static std::random_device randomDevice;
	static std::mt19937 engine(randomDevice());
	std::discrete_distribution<int> range(weights.begin(), weights.end());

	return range(engine);
}

Move Board::parseUCI(const std::vector<Move>& everyPossibleMove, const std::string uciMove) {
	Move parsedMove;

	int fromFile = uciMove[0] - 'a';
	int fromRank = 7 - (uciMove[1] - '1');
	int toFile = uciMove[2] - 'a';
	int toRank = 7 - (uciMove[3] - '1');

	std::optional<PieceType> promoType = std::nullopt;
	if (uciMove.length() == 5) {
		switch (uciMove[4]) {
		case 'n':
			promoType = PieceType::Knight;
			break;
		case 'b':
			promoType = PieceType::Bishop;
			break;
		case 'r':
			promoType = PieceType::Rook;
			break;
		case 'q':
			promoType = PieceType::Queen;
			break;
		default:
			break;
		}
	}

	for (const Move& move : everyPossibleMove) {
		if (move.fromRank == fromRank and move.fromFile == fromFile and
			move.toRank == toRank and move.toFile == toFile) {

			if (promoType.has_value()) {
				if (move.promoteInto == promoType) {
					parsedMove = move;
				}
			}
			else {
				parsedMove = move;
			}
		}
	}

	return parsedMove;
}

int Board::Minimax(std::size_t depth, int whiteGuaranteed, int blackGuaranteed, Side maximizingPlayer) {
	if (isDrawByInsufficientMaterial() or halfmoveCounter >= 100 or isThreeFoldRepetition()) {
		return 0;
	}

	if (abortSearch) {
		return 0;
	}

	if (getPieceCount() <= 5) {
		unsigned wdl = probeTablebaseWDL();

		if (wdl != TB_RESULT_FAILED) {

			if (wdl == TB_WIN or wdl == TB_CURSED_WIN) {
				return (currentTurn == Side::White) ? (BLACK_LOSS + static_cast<int>(depth)) : (WHITE_LOSS - static_cast<int>(depth));
			}
			else if (wdl == TB_LOSS or wdl == TB_BLESSED_LOSS) {
				return (currentTurn == Side::White) ? (WHITE_LOSS - static_cast<int>(depth)) : (BLACK_LOSS + static_cast<int>(depth));
			}
			else {
				return 0;
			}
		}
	}

	if (depth == 0) {
		return quiescenceSearch(whiteGuaranteed, blackGuaranteed, currentTurn);
	}

	std::vector<Move> everyPossibleMove = generateEveryLegalMove(currentTurn);
	std::sort(everyPossibleMove.begin(), everyPossibleMove.end(), [](const Move& moveOne, const Move& moveTwo) {
		return moveOne.score > moveTwo.score;
	});

	if (everyPossibleMove.empty()) {
		
		int kingRank = INVALID_COORD, kingFile = INVALID_COORD;
		Side enemySide;
		
		if (currentTurn == Side::White) {
			kingRank = whiteKingRank;
			kingFile = whiteKingFile;
			enemySide = Side::Black;
		}
		else {
			kingRank = blackKingRank;
			kingFile = blackKingFile;
			enemySide = Side::White;
		}
		
		if (!isKingSafe(kingRank, kingFile, enemySide)) {
			return (maximizingPlayer == Side::White) ? (WHITE_LOSS - static_cast<int>(depth)) : (BLACK_LOSS + static_cast<int>(depth));
		}
		else {
			return 0;
		}
	}

	if (maximizingPlayer == Side::White) {
		int score = WHITE_LOSS;

		for (const Move& move : everyPossibleMove) {
			std::optional<Piece> capturedPiece = std::nullopt;
			makeTempMove(move, capturedPiece);

			currentTurn = Side::Black;
			const std::string currentFen = generateFenString();
			positionHistory[currentFen]++;

			int evaluation = Minimax(depth - 1, whiteGuaranteed, blackGuaranteed, currentTurn);

			positionHistory[currentFen]--;
			if (positionHistory[currentFen] == 0) {
				positionHistory.erase(currentFen);
			}

			score = std::max(score, evaluation);

			currentTurn = Side::White;
			unmakeMove(move, capturedPiece);
			
			whiteGuaranteed = std::max(whiteGuaranteed, score);
			if (whiteGuaranteed >= blackGuaranteed) {
				break;
			}
		}

		return score;
	}
	else {
		int score = BLACK_LOSS;

		for (const Move& move : everyPossibleMove) {
			std::optional<Piece> capturedPiece = std::nullopt;
			makeTempMove(move, capturedPiece);

			currentTurn = Side::White;
			const std::string currentFen = generateFenString();
			positionHistory[currentFen]++;

			int evaluation = Minimax(depth - 1, whiteGuaranteed, blackGuaranteed, currentTurn);

			positionHistory[currentFen]--;
			if (positionHistory[currentFen] == 0) {
				positionHistory.erase(currentFen);
			}

			score = std::min(score, evaluation);

			currentTurn = Side::Black;
			unmakeMove(move, capturedPiece);
			
			blackGuaranteed = std::min(blackGuaranteed, score);
			if (whiteGuaranteed >= blackGuaranteed) {
				break;
			}
		}

		return score;
	}

	return INVALID_COORD;
}

int Board::quiescenceSearch(int whiteGuaranteed, int blackGuaranteed, Side maximizingPlayer, int qsDepth) {
	int kingRank = (currentTurn == Side::White) ? whiteKingRank : blackKingRank;
	int kingFile = (currentTurn == Side::White) ? whiteKingFile : blackKingFile;
	Side enemySide = (currentTurn == Side::White) ? Side::Black : Side::White;

	bool inCheck = !isKingSafe(kingRank, kingFile, enemySide);

	int standPat = (maximizingPlayer == Side::White) ? WHITE_LOSS : BLACK_LOSS;

	if (!inCheck) {
		evaluate();
		standPat = getCurrentEvaluation();

		if (maximizingPlayer == Side::White) {
			if (standPat >= blackGuaranteed) return blackGuaranteed;
			if (standPat > whiteGuaranteed) whiteGuaranteed = standPat;
		}
		else {
			if (standPat <= whiteGuaranteed) return whiteGuaranteed;
			if (standPat < blackGuaranteed) blackGuaranteed = standPat;
		}
	}

	if (qsDepth > 4) return standPat;

	std::vector<Move> movesToSearch = inCheck ? generateEveryLegalMove(currentTurn) : generateCapturesOnly(currentTurn);

	std::sort(movesToSearch.begin(), movesToSearch.end(), [](const Move& moveOne, const Move& moveTwo) {
		return moveOne.score > moveTwo.score;
	});

	if (inCheck and movesToSearch.empty()) {
		return (maximizingPlayer == Side::White) ? WHITE_LOSS : BLACK_LOSS;
	}

	if (maximizingPlayer == Side::White) {
		int score = standPat;
		for (const Move& move : movesToSearch) {
			std::optional<Piece> capturedPiece = std::nullopt;
			makeTempMove(move, capturedPiece);

			currentTurn = Side::Black;
			int evaluation = quiescenceSearch(whiteGuaranteed, blackGuaranteed, currentTurn, qsDepth + 1);

			currentTurn = Side::White;
			unmakeMove(move, capturedPiece);

			score = std::max(score, evaluation);
			whiteGuaranteed = std::max(whiteGuaranteed, score);
			if (whiteGuaranteed >= blackGuaranteed) {
				break;
			}
		}
		return score;
	}
	else {
		int score = standPat;
		for (const Move& move : movesToSearch) {
			std::optional<Piece> capturedPiece = std::nullopt;
			makeTempMove(move, capturedPiece);

			currentTurn = Side::White;
			int evaluation = quiescenceSearch(whiteGuaranteed, blackGuaranteed, currentTurn, qsDepth + 1);

			currentTurn = Side::Black;
			unmakeMove(move, capturedPiece);

			score = std::min(score, evaluation);
			blackGuaranteed = std::min(blackGuaranteed, score);
			if (whiteGuaranteed >= blackGuaranteed) {
				break;
			}
		}
		return score;
	}
}

std::vector<Move> Board::generateCapturesOnly(const Side side) {
	std::vector<Move> legalMoves = generateEveryLegalMove(side);
	std::vector<Move> capturesOnly;

	for (const Move& move : legalMoves) {
		if (move.isLoudMove) {
			capturesOnly.push_back(move);
		}
	}

	return capturesOnly;
}

float Board::getEndgameFactor() const {
	return endgameFactor;
}