import chess
import chess.pgn
import glob

# Dictionary format: { "FEN_STRING": { "e2e4": 500, "d2d4": 450 } }
book = {}
MAX_PLIES = 20
MIN_VOTES = 10

sources = [
    {"folder": "Lichess Elite Database/*.pgn", "weight": 1, "min_elo": 2500},
    {"folder": "Grandmaster Games/*.pgn", "weight": 5, "min_elo": 0}
]

print("Hunting for PGN files...")

for source in sources:
    pgn_files = glob.glob(source["folder"])
    print(f"\nFound {len(pgn_files)} files in {source['folder']}!")

    for filename in pgn_files:
        print(f"Parsing {filename} with weight {source['weight']}...")
        games_kept = 0
        games_skipped = 0
        
        with open(filename, "r", encoding="utf-8", errors="ignore") as pgn_file:
            while True:
                anchor = pgn_file.tell()
                headers = chess.pgn.read_headers(pgn_file)
                
                if headers is None:
                    break
                
                if source["min_elo"] > 0:
                    try:
                        white_elo = int(headers.get("WhiteElo", 0))
                        black_elo = int(headers.get("BlackElo", 0))
                    except ValueError:
                        white_elo, black_elo = 0, 0
                        
                    if white_elo < source["min_elo"] or black_elo < source["min_elo"]:
                        chess.pgn.skip_game(pgn_file)
                        games_skipped += 1
                        continue
                    
                pgn_file.seek(anchor)
                
                try:
                    game = chess.pgn.read_game(pgn_file)
                    board = game.board()
                    games_kept += 1
                    
                    for i, move in enumerate(game.mainline_moves()):
                        if i >= MAX_PLIES:
                            break
                            
                        raw_fen = board.fen(en_passant='fen')
                        book_key = " ".join(raw_fen.split()[:4])
                        uci_move = move.uci()
                        
                        if book_key not in book:
                            book[book_key] = {}
                            
                        if uci_move not in book[book_key]:
                            book[book_key][uci_move] = 0
                            
                        book[book_key][uci_move] += source["weight"] 
                        
                        board.push(move)
                except Exception:
                    continue

        print(f"Finished {filename}! Kept {games_kept} games (Skipped {games_skipped}).")

print(f"\nTotal unique board positions found: {len(book)}")
print("Writing to file...")

with open("opening-book-file.txt", "w", encoding="utf-8") as out:
    for fen, moves in book.items():
        valid_moves = []
        for move, votes in moves.items():
            if votes >= MIN_VOTES: 
                valid_moves.append(f"{move}:{votes}") 
                
        if valid_moves:
            out.write(f"{fen} {' '.join(valid_moves)}\n")

print("Saved to opening-book-file.txt!")