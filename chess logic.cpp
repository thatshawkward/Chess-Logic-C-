#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cctype>
enum class PieceType {
    EMPTY, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};
enum class Color {
    NONE, WHITE, BLACK
};
struct Piece {
    PieceType type;
    Color color;
    
    Piece() : type(PieceType::EMPTY), color(Color::NONE) {}
    Piece(PieceType t, Color c) : type(t), color(c) {}
    
    char getSymbol() const {
        char symbol = ' ';
        switch (type) {
            case PieceType::PAWN: symbol = 'P'; break;
            case PieceType::KNIGHT: symbol = 'N'; break;
            case PieceType::BISHOP: symbol = 'B'; break;
            case PieceType::ROOK: symbol = 'R'; break;
            case PieceType::QUEEN: symbol = 'Q'; break;
            case PieceType::KING: symbol = 'K'; break;
            default: return ' ';
        }
        return color == Color::WHITE ? symbol : tolower(symbol);
    }
};
struct Position {
    int row;
    int col;
    
    Position() : row(0), col(0) {}
    Position(int r, int c) : row(r), col(c) {}
    
    bool isValid() const {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }
    
    std::string toAlgebraic() const {
        if (!isValid()) return "invalid";
        return std::string(1, 'a' + col) + std::string(1, '8' - row);
    }
    
    static Position fromAlgebraic(const std::string& algebraic) {
        if (algebraic.length() != 2) return Position(-1, -1);
        int col = algebraic[0] - 'a';
        int row = '8' - algebraic[1];
        return Position(row, col);
    }
    
    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
    
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};
struct Move {
    Position from;
    Position to;
    PieceType promotion;
    
    Move() : from(), to(), promotion(PieceType::EMPTY) {}
    Move(Position f, Position t) : from(f), to(t), promotion(PieceType::EMPTY) {}
    Move(Position f, Position t, PieceType p) : from(f), to(t), promotion(p) {}
    
    std::string toString() const {
        std::string result = from.toAlgebraic() + to.toAlgebraic();
        if (promotion != PieceType::EMPTY) {
            switch (promotion) {
                case PieceType::QUEEN: result += 'q'; break;
                case PieceType::ROOK: result += 'r'; break;
                case PieceType::BISHOP: result += 'b'; break;
                case PieceType::KNIGHT: result += 'n'; break;
                default: break;
            }
        }
        return result;
    }
};
class ChessBoard {
private:
    std::vector<std::vector<Piece>> board;
    Color currentPlayer;
    bool whiteKingMoved;
    bool blackKingMoved;
    bool whiteQueenRookMoved;
    bool whiteKingRookMoved;
    bool blackQueenRookMoved;
    bool blackKingRookMoved;
    Position enPassantTarget;
    
public:
    ChessBoard() : currentPlayer(Color::WHITE), 
                  whiteKingMoved(false), blackKingMoved(false),
                  whiteQueenRookMoved(false), whiteKingRookMoved(false),
                  blackQueenRookMoved(false), blackKingRookMoved(false),
                  enPassantTarget(Position(-1, -1)) {
        //initialize the board
        board.resize(8, std::vector<Piece>(8, Piece()));
        resetBoard();
    }
    
    void resetBoard() {
        //pawns
        for (int col = 0; col < 8; col++) {
            board[1][col] = Piece(PieceType::PAWN, Color::BLACK);
            board[6][col] = Piece(PieceType::PAWN, Color::WHITE);
        }
        
        //rooks
        board[0][0] = Piece(PieceType::ROOK, Color::BLACK);
        board[0][7] = Piece(PieceType::ROOK, Color::BLACK);
        board[7][0] = Piece(PieceType::ROOK, Color::WHITE);
        board[7][7] = Piece(PieceType::ROOK, Color::WHITE);
        
        //knights
        board[0][1] = Piece(PieceType::KNIGHT, Color::BLACK);
        board[0][6] = Piece(PieceType::KNIGHT, Color::BLACK);
        board[7][1] = Piece(PieceType::KNIGHT, Color::WHITE);
        board[7][6] = Piece(PieceType::KNIGHT, Color::WHITE);
        
        //bishops
        board[0][2] = Piece(PieceType::BISHOP, Color::BLACK);
        board[0][5] = Piece(PieceType::BISHOP, Color::BLACK);
        board[7][2] = Piece(PieceType::BISHOP, Color::WHITE);
        board[7][5] = Piece(PieceType::BISHOP, Color::WHITE);
        
        //queens
        board[0][3] = Piece(PieceType::QUEEN, Color::BLACK);
        board[7][3] = Piece(PieceType::QUEEN, Color::WHITE);
        
        //kings
        board[0][4] = Piece(PieceType::KING, Color::BLACK);
        board[7][4] = Piece(PieceType::KING, Color::WHITE);
        
        //clear middle rows
        for (int row = 2; row < 6; row++) {
            for (int col = 0; col < 8; col++) {
                board[row][col] = Piece();
            }
        }
        
        //reset state variables
        currentPlayer = Color::WHITE;
        whiteKingMoved = false;
        blackKingMoved = false;
        whiteQueenRookMoved = false;
        whiteKingRookMoved = false;
        blackQueenRookMoved = false;
        blackKingRookMoved = false;
        enPassantTarget = Position(-1, -1);
    }
    
    void displayBoard() const {
        std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
        for (int row = 0; row < 8; row++) {
            std::cout << (8 - row) << " |";
            for (int col = 0; col < 8; col++) {
                std::cout << " " << board[row][col].getSymbol() << " |";
            }
            std::cout << std::endl << "  +---+---+---+---+---+---+---+---+" << std::endl;
        }
        std::cout << "    a   b   c   d   e   f   g   h  " << std::endl;
    }
    
    Piece getPiece(const Position& pos) const {
        if (!pos.isValid()) return Piece();
        return board[pos.row][pos.col];
    }
    
    void setPiece(const Position& pos, const Piece& piece) {
        if (pos.isValid()) {
            board[pos.row][pos.col] = piece;
        }
    }
    
    Color getCurrentPlayer() const {
        return currentPlayer;
    }
    
    void switchPlayer() {
        currentPlayer = (currentPlayer == Color::WHITE) ? Color::BLACK : Color::WHITE;
    }
    
    Position findKing(Color color) const {
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                Piece piece = board[row][col];
                if (piece.type == PieceType::KING && piece.color == color) {
                    return Position(row, col);
                }
            }
        }
        return Position(-1, -1); // King not foundz
    }
    
    bool isCheck(Color color) const {
        Position kingPos = findKing(color);
        return isPositionUnderAttack(kingPos, color);
    }
    
    bool isPositionUnderAttack(const Position& pos, Color defendingColor) const {
        Color attackingColor = (defendingColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
        
        //check for pawn attacks
        int pawnRow = (defendingColor == Color::WHITE) ? -1 : 1;
        if ((pos.row + pawnRow >= 0 && pos.row + pawnRow < 8) && 
            (pos.col - 1 >= 0 && board[pos.row + pawnRow][pos.col - 1].type == PieceType::PAWN && 
             board[pos.row + pawnRow][pos.col - 1].color == attackingColor)) {
            return true;
        }
        if ((pos.row + pawnRow >= 0 && pos.row + pawnRow < 8) && 
            (pos.col + 1 < 8 && board[pos.row + pawnRow][pos.col + 1].type == PieceType::PAWN && 
             board[pos.row + pawnRow][pos.col + 1].color == attackingColor)) {
            return true;
        }
        
        //knight moves
        std::vector<std::pair<int, int>> knightMoves = {
            {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}
        };
        for (const auto& move : knightMoves) {
            int newRow = pos.row + move.first;
            int newCol = pos.col + move.second;
            if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
                Piece piece = board[newRow][newCol];
                if (piece.type == PieceType::KNIGHT && piece.color == attackingColor) {
                    return true;
                }
            }
        }
        
        //directions for sliding pieces
        std::vector<std::pair<int, int>> directions = {
            {-1, 0}, {1, 0}, {0, -1}, {0, 1}, // Rook/Queen
            {-1, -1}, {-1, 1}, {1, -1}, {1, 1} // Bishop/Queen
        };
        
        for (const auto& dir : directions) {
            int dr = dir.first;
            int dc = dir.second;
            int newRow = pos.row + dr;
            int newCol = pos.col + dc;
            
            while (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
                Piece piece = board[newRow][newCol];
                if (piece.type != PieceType::EMPTY) {
                    if (piece.color == attackingColor) {
                        bool isRookMove = (dr == 0 || dc == 0);
                        bool isBishopMove = (abs(dr) == abs(dc));
                        
                        if ((isRookMove && (piece.type == PieceType::ROOK || piece.type == PieceType::QUEEN)) ||
                            (isBishopMove && (piece.type == PieceType::BISHOP || piece.type == PieceType::QUEEN))) {
                            return true;
                        }
                    }
                    break; // Stop in this direction after finding any piece
                }
                newRow += dr;
                newCol += dc;
            }
        }
        
        //king attacks
        std::vector<std::pair<int, int>> kingMoves = {
            {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}
        };
        for (const auto& move : kingMoves) {
            int newRow = pos.row + move.first;
            int newCol = pos.col + move.second;
            if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
                Piece piece = board[newRow][newCol];
                if (piece.type == PieceType::KING && piece.color == attackingColor) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
    bool makeMove(const Move& move) {
        if (!move.from.isValid() || !move.to.isValid()) {
            return false;
        }
        Piece piece = getPiece(move.from);
        
        //check if the piece belongs to the current player
        if (piece.color != currentPlayer) {
            return false;
        }
        //check if the move is valid for this piece
        if (!isValidMove(move)) {
            return false;
        }
        
        //save the state before the move for check validation
        Piece capturedPiece = getPiece(move.to);
        bool wasKingMoved = (currentPlayer == Color::WHITE) ? whiteKingMoved : blackKingMoved;
        bool wasQueenRookMoved = (currentPlayer == Color::WHITE) ? whiteQueenRookMoved : blackQueenRookMoved;
        bool wasKingRookMoved = (currentPlayer == Color::WHITE) ? whiteKingRookMoved : blackKingRookMoved;
        
        //execute move
        Position oldEnPassantTarget = enPassantTarget;
        executeMove(move);
        
        //check if move puts or leaves the player's king in check
        if (isCheck(currentPlayer)) {
            // Undo the move
            undoMove(move, capturedPiece, wasKingMoved, wasQueenRookMoved, wasKingRookMoved, oldEnPassantTarget);
            return false;
        }
        switchPlayer();
        return true;
    }
    void executeMove(const Move& move) {
        Piece piece = getPiece(move.from);
        Piece capturedPiece = getPiece(move.to);
        
        //update castling flags
        if (piece.type == PieceType::KING) {
            if (piece.color == Color::WHITE) {
                whiteKingMoved = true;
            } else {
                blackKingMoved = true;
            }
            
            //handle castling move
            if (abs(move.to.col - move.from.col) == 2) {
                // King-side castling
                if (move.to.col == 6) {
                    // Move the rook
                    Piece rook = getPiece(Position(move.from.row, 7));
                    setPiece(Position(move.from.row, 5), rook);
                    setPiece(Position(move.from.row, 7), Piece());
                }
                //queen-side castling
                else if (move.to.col == 2) {
                    // Move the rook
                    Piece rook = getPiece(Position(move.from.row, 0));
                    setPiece(Position(move.from.row, 3), rook);
                    setPiece(Position(move.from.row, 0), Piece());
                }
            }
        }
        
        //update rook moved flags
        if (piece.type == PieceType::ROOK) {
            if (piece.color == Color::WHITE) {
                if (move.from == Position(7, 0)) {
                    whiteQueenRookMoved = true;
                } else if (move.from == Position(7, 7)) {
                    whiteKingRookMoved = true;
                }
            } else {
                if (move.from == Position(0, 0)) {
                    blackQueenRookMoved = true;
                } else if (move.from == Position(0, 7)) {
                    blackKingRookMoved = true;
                }
            }
        }
        
        // handle en passant capture
        if (piece.type == PieceType::PAWN && move.to == enPassantTarget) {
            int captureRow = (piece.color == Color::WHITE) ? move.to.row + 1 : move.to.row - 1;
            setPiece(Position(captureRow, move.to.col), Piece());
        }
        
        //new en passant target if this is a double pawn move
        enPassantTarget = Position(-1, -1); // Reset en passant target
        if (piece.type == PieceType::PAWN && abs(move.to.row - move.from.row) == 2) {
            int targetRow = (move.from.row + move.to.row) / 2;
            enPassantTarget = Position(targetRow, move.from.col);
        }
        if (piece.type == PieceType::PAWN && (move.to.row == 0 || move.to.row == 7)) {
            if (move.promotion != PieceType::EMPTY) {
                piece.type = move.promotion;
            } else {
                piece.type = PieceType::QUEEN; // Default promotion to queen
            }
        }
        setPiece(move.to, piece);
        setPiece(move.from, Piece());
    }
    
    void undoMove(const Move& move, const Piece& capturedPiece, bool wasKingMoved, 
                 bool wasQueenRookMoved, bool wasKingRookMoved, const Position& oldEnPassantTarget) {
        Piece piece = getPiece(move.to);
        
        //restore the moved piece to its original position
        setPiece(move.from, piece);
        setPiece(move.to, capturedPiece);
        
        //restore castling flags
        if (currentPlayer == Color::WHITE) {
            whiteKingMoved = wasKingMoved;
            whiteQueenRookMoved = wasQueenRookMoved;
            whiteKingRookMoved = wasKingRookMoved;
        } else {
            blackKingMoved = wasKingMoved;
            blackQueenRookMoved = wasQueenRookMoved;
            blackKingRookMoved = wasKingRookMoved;
        }
        
        //undo castling move if needed
        if (piece.type == PieceType::KING && abs(move.to.col - move.from.col) == 2) {
            // King-side castling
            if (move.to.col == 6) {
                Piece rook = getPiece(Position(move.from.row, 5));
                setPiece(Position(move.from.row, 7), rook);
                setPiece(Position(move.from.row, 5), Piece());
            }
            // Queen-side castling
            else if (move.to.col == 2) {
                Piece rook = getPiece(Position(move.from.row, 3));
                setPiece(Position(move.from.row, 0), rook);
                setPiece(Position(move.from.row, 3), Piece());
            }
        }
        
        //rstore en passant target
        enPassantTarget = oldEnPassantTarget;
        
        //if this was an en passant capture, restore the captured pawn
        if (piece.type == PieceType::PAWN && move.to == oldEnPassantTarget) {
            int captureRow = (piece.color == Color::WHITE) ? move.to.row + 1 : move.to.row - 1;
            setPiece(Position(captureRow, move.to.col), 
                    Piece(PieceType::PAWN, (piece.color == Color::WHITE) ? Color::BLACK : Color::WHITE));
        }
    }
    
    bool isValidMove(const Move& move) const {
        Piece piece = getPiece(move.from);
        Piece targetPiece = getPiece(move.to);
        
        //can't capture own piece
        if (targetPiece.type != PieceType::EMPTY && targetPiece.color == piece.color) {
            return false;
        }
        
        switch (piece.type) {
            case PieceType::PAWN:
                return isValidPawnMove(move);
            case PieceType::KNIGHT:
                return isValidKnightMove(move);
            case PieceType::BISHOP:
                return isValidBishopMove(move);
            case PieceType::ROOK:
                return isValidRookMove(move);
            case PieceType::QUEEN:
                return isValidQueenMove(move);
            case PieceType::KING:
                return isValidKingMove(move);
            default:
                return false;
        }
    }
    
    bool isValidPawnMove(const Move& move) const {
        Piece pawn = getPiece(move.from);
        Piece targetPiece = getPiece(move.to);
        
        int direction = (pawn.color == Color::WHITE) ? -1 : 1;
        int startRow = (pawn.color == Color::WHITE) ? 6 : 1;
        
        //regular move forward
        if (move.from.col == move.to.col) {
            // Single step forward
            if (move.to.row == move.from.row + direction) {
                return targetPiece.type == PieceType::EMPTY;
            }
            //double step from starting position
            if (move.from.row == startRow && move.to.row == move.from.row + 2 * direction) {
                Position intermediate(move.from.row + direction, move.from.col);
                return targetPiece.type == PieceType::EMPTY && 
                       getPiece(intermediate).type == PieceType::EMPTY;
            }
        }
        else if (abs(move.to.col - move.from.col) == 1 && move.to.row == move.from.row + direction) {
            //regular capture
            if (targetPiece.type != PieceType::EMPTY && targetPiece.color != pawn.color) {
                return true;
            }
            //en passant capture
            if (targetPiece.type == PieceType::EMPTY && move.to == enPassantTarget) {
                return true;
            }
        }
        
        return false;
    }
    
    bool isValidKnightMove(const Move& move) const {
        int rowDiff = abs(move.to.row - move.from.row);
        int colDiff = abs(move.to.col - move.from.col);
        
        return (rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2);
    }
    
    bool isValidBishopMove(const Move& move) const {
        int rowDiff = abs(move.to.row - move.from.row);
        int colDiff = abs(move.to.col - move.from.col);
        
        if (rowDiff != colDiff) {
            return false;
        }
        
        //check if the path is clear
        int rowStep = (move.to.row > move.from.row) ? 1 : -1;
        int colStep = (move.to.col > move.from.col) ? 1 : -1;
        
        for (int i = 1; i < rowDiff; i++) {
            Position pos(move.from.row + i * rowStep, move.from.col + i * colStep);
            if (getPiece(pos).type != PieceType::EMPTY) {
                return false;
            }
        }
        
        return true;
    }
    
    bool isValidRookMove(const Move& move) const {
        int rowDiff = abs(move.to.row - move.from.row);
        int colDiff = abs(move.to.col - move.from.col);
        
        if (rowDiff != 0 && colDiff != 0) {
            return false;
        }
        
        //Check if the path is clear
        if (rowDiff == 0) {
            int step = (move.to.col > move.from.col) ? 1 : -1;
            for (int col = move.from.col + step; col != move.to.col; col += step) {
                Position pos(move.from.row, col);
                if (getPiece(pos).type != PieceType::EMPTY) {
                    return false;
                }
            }
        } else {
            int step = (move.to.row > move.from.row) ? 1 : -1;
            for (int row = move.from.row + step; row != move.to.row; row += step) {
                Position pos(row, move.from.col);
                if (getPiece(pos).type != PieceType::EMPTY) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    bool isValidQueenMove(const Move& move) const {
        return isValidBishopMove(move) || isValidRookMove(move);
    }
    
    bool isValidKingMove(const Move& move) const {
        Piece king = getPiece(move.from);
        int rowDiff = abs(move.to.row - move.from.row);
        int colDiff = abs(move.to.col - move.from.col);
        
        //Normal king move
        if (rowDiff <= 1 && colDiff <= 1) {
            return true;
        }
        
        //castling
        if (rowDiff == 0 && colDiff == 2) {
            // Check if the king has already moved
            if ((king.color == Color::WHITE && whiteKingMoved) || 
                (king.color == Color::BLACK && blackKingMoved)) {
                return false;
            }
            if (isCheck(king.color)) {
                return false;
            }
          
            int row = move.from.row;
            
            //king-side castling
            if (move.to.col == 6) {
                //check if the rook has moved
                if ((king.color == Color::WHITE && whiteKingRookMoved) ||
                    (king.color == Color::BLACK && blackKingRookMoved)) {
                    return false;
                }
                
                // Check if the path is clear
                if (getPiece(Position(row, 5)).type != PieceType::EMPTY || 
                    getPiece(Position(row, 6)).type != PieceType::EMPTY) {
                    return false;
                }
                
                //check if the squares the king passes through are under attack
                if (isPositionUnderAttack(Position(row, 5), king.color)) {
                    return false;
                }
                
                //check ifrook is in place
                Piece rook = getPiece(Position(row, 7));
                return rook.type == PieceType::ROOK && rook.color == king.color;
            }
            //queen-side castling
            else if (move.to.col == 2) {
                // Check if the rook has moved
                if ((king.color == Color::WHITE && whiteQueenRookMoved) ||
                    (king.color == Color::BLACK && blackQueenRookMoved)) {
                    return false;
                }
                if (getPiece(Position(row, 1)).type != PieceType::EMPTY || 
                    getPiece(Position(row, 2)).type != PieceType::EMPTY ||
                    getPiece(Position(row, 3)).type != PieceType::EMPTY) {
                    return false;
                }
                if (isPositionUnderAttack(Position(row, 3), king.color)) {
                    return false;
                }
                Piece rook = getPiece(Position(row, 0));
                return rook.type == PieceType::ROOK && rook.color == king.color;
            }
        }
        
        return false;
    }
    
    std::vector<Move> getAllLegalMoves() const {
        std::vector<Move> legalMoves;
        
        for (int fromRow = 0; fromRow < 8; fromRow++) {
            for (int fromCol = 0; fromCol < 8; fromCol++) {
                Position from(fromRow, fromCol);
                Piece piece = getPiece(from);
                
                if (piece.type != PieceType::EMPTY && piece.color == currentPlayer) {
                    for (int toRow = 0; toRow < 8; toRow++) {
                        for (int toCol = 0; toCol < 8; toCol++) {
                            Position to(toRow, toCol);
                            Move move(from, to);
                            
                            //check if the move is valid without making it
                            if (isValidMove(move)) {
                                //save the state before the move
                                Piece capturedPiece = getPiece(to);
                                bool wasKingMoved = (currentPlayer == Color::WHITE) ? whiteKingMoved : blackKingMoved;
                                bool wasQueenRookMoved = (currentPlayer == Color::WHITE) ? whiteQueenRookMoved : blackQueenRookMoved;
                                bool wasKingRookMoved = (currentPlayer == Color::WHITE) ? whiteKingRookMoved : blackKingRookMoved;
                                Position oldEnPassantTarget = enPassantTarget;
                                
                                const_cast<ChessBoard*>(this)->executeMove(move);
                                
                                //check if the move leaves the king in check
                                bool isLegal = !const_cast<ChessBoard*>(this)->isCheck(currentPlayer);
                                
                                const_cast<ChessBoard*>(this)->undoMove(move, capturedPiece, wasKingMoved, 
                                                                      wasQueenRookMoved, wasKingRookMoved, 
                                                                      oldEnPassantTarget);
                                
                                if (isLegal) {
                                    //add promotions for pawns reaching the last rank
                                    if (piece.type == PieceType::PAWN && (to.row == 0 || to.row == 7)) {
                                        legalMoves.push_back(Move(from, to, PieceType::QUEEN));
                                        legalMoves.push_back(Move(from, to, PieceType::ROOK));
                                        legalMoves.push_back(Move(from, to, PieceType::BISHOP));
                                        legalMoves.push_back(Move(from, to, PieceType::KNIGHT));
                                    } else {
                                        legalMoves.push_back(move);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return legalMoves;
    }
    
    bool isCheckmate() const {
        return isCheck(currentPlayer) && getAllLegalMoves().empty();
    }
    
    bool isStalemate() const {
        return !isCheck(currentPlayer) && getAllLegalMoves().empty();
    }
    bool isDraw() const {
        // Stalemate
        if (isStalemate()) {
            return true;
        }
        bool insufficientMaterial = true;
        int bishopsWhite = 0;
        int bishopsBlack = 0;
        int knightsWhite = 0;
        int knightsBlack = 0;
        int otherPieces = 0;
        
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                Piece piece = board[row][col];
                if (piece.type == PieceType::EMPTY) continue;
                
                if (piece.type == PieceType::PAWN || piece.type == PieceType::ROOK || piece.type == PieceType::QUEEN) {
                    insufficientMaterial = false;
                    break;
                } else if (piece.type == PieceType::BISHOP) {
                    if (piece.color == Color::WHITE) {
                        bishopsWhite++;
                    } else {
                        bishopsBlack++;
                    }
                } else if (piece.type == PieceType::KNIGHT) {
                    if (piece.color == Color::WHITE) {
                        knightsWhite++;
                    } else {
                        knightsBlack++;
                    }
                }
            }
            
            if (!insufficientMaterial) break;
        }
        
        otherPieces = bishopsWhite + bishopsBlack + knightsWhite + knightsBlack;
        
        // King vs. King
        if (otherPieces == 0) {
            return true;
        }
        
        // King and Bishop vs. King
        if ((bishopsWhite == 1 && bishopsBlack == 0 && knightsWhite == 0 && knightsBlack == 0) ||
            (bishopsWhite == 0 && bishopsBlack == 1 && knightsWhite == 0 && knightsBlack == 0)) {
            return true;
        }
        
        // King and Knight vs. King
        if ((knightsWhite == 1 && knightsBlack == 0 && bishopsWhite == 0 && bishopsBlack == 0) ||
            (knightsWhite == 0 && knightsBlack == 1 && bishopsWhite == 0 && bishopsBlack == 0)) {
            return true;
        }
        
        return false;
    }
    
    std::string getGameState() const {
        if (isCheckmate()) {
            return (currentPlayer == Color::WHITE) ? "Black wins by checkmate" : "White wins by checkmate";
        } else if (isStalemate()) {
            return "Draw by stalemate";
        } else if (isDraw()) {
            return "Draw by insufficient material";
        } else if (isCheck(currentPlayer)) {
            return (currentPlayer == Color::WHITE) ? "White is in check" : "Black is in check";
        } else {
            return (currentPlayer == Color::WHITE) ? "White to move" : "Black to move";
        }
    }
};

//game controller class
class ChessGame {
private:
    ChessBoard board;
    std::vector<Move> moveHistory;
    
public:
    ChessGame() : board() {}
    
    void start() {
        board.resetBoard();
        moveHistory.clear();
    }
    
    void printBoard() const {
        board.displayBoard();
        std::cout << board.getGameState() << std::endl;
    }
    
    bool makeMove(const std::string& moveStr) {
        if (moveStr.length() < 4) {
            std::cout << "Invalid move format. Please use format like 'e2e4' or 'e7e8q' for promotion." << std::endl;
            return false;
        }
        
        Position from = Position::fromAlgebraic(moveStr.substr(0, 2));
        Position to = Position::fromAlgebraic(moveStr.substr(2, 2));
        
        PieceType promotion = PieceType::EMPTY;
        if (moveStr.length() >= 5) {
            char promotionChar = std::tolower(moveStr[4]);
            switch (promotionChar) {
                case 'q': promotion = PieceType::QUEEN; break;
                case 'r': promotion = PieceType::ROOK; break;
                case 'b': promotion = PieceType::BISHOP; break;
                case 'n': promotion = PieceType::KNIGHT; break;
                default: 
                    std::cout << "Invalid promotion piece. Use q, r, b, or n." << std::endl;
                    return false;
            }
        }
        
        Move move(from, to, promotion);
        
        if (board.makeMove(move)) {
            moveHistory.push_back(move);
            return true;
        } else {
            std::cout << "Invalid move." << std::endl;
            return false;
        }
    }
    
    bool isGameOver() const {
        return board.isCheckmate() || board.isStalemate() || board.isDraw();
    }
    
    std::string getResult() const {
        return board.getGameState();
    }
    
    Color getCurrentPlayer() const {
        return board.getCurrentPlayer();
    }
    
    std::vector<Move> getLegalMoves() const {
        return board.getAllLegalMoves();
    }
    
    void printLegalMoves() const {
        std::vector<Move> moves = getLegalMoves();
        std::cout << "Legal moves: ";
        for (const auto& move : moves) {
            std::cout << move.toString() << " ";
        }
        std::cout << std::endl;
    }
    
    std::vector<Move> getMoveHistory() const {
        return moveHistory;
    }
};

//main function
int main() {
    ChessGame game;
    game.start();
    
    std::string input;
    while (!game.isGameOver()) {
        game.printBoard();
        
        std::cout << "Enter move (e.g., 'e2e4') or 'q' to quit, 'l' for legal moves: ";
        std::cin >> input;
        
        if (input == "q") {
            break;
        } else if (input == "l") {
            game.printLegalMoves();
        } else {
            game.makeMove(input);
        }
    }
    
    if (game.isGameOver()) {
        game.printBoard();
        std::cout << "Game over: " << game.getResult() << std::endl;
    }
    
    return 0;
}