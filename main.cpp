#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <regex>
#include <unordered_map>

using namespace std;

struct Move
{
    string whiteMove;
    string blackMove;

    Move(string _wMove, string _bMove) : whiteMove(_wMove), blackMove(_bMove) {}
};

enum MoveType
{
    PIECE_MOVE,
    CASTLING,
    RESULT
};

vector<Move> parsedMoves;

unordered_map<string, vector<pair<int, int>>> BlackPieceMap{
    {"Q", {{0, 3}}},
    {"K", {{0, 4}}},
};

unordered_map<string, vector<pair<int, int>>> WhitePieceMap{
    {"Q", {{7, 3}}},
    {"K", {{7, 4}}},
};

vector<vector<string>> chessBoard{{"bR", "bN", "bB", "bQ", "bK", "bB", "bN", "bR"},
                                  {"bP", "bP", "bP", "bP", "bP", "bP", "bP", "bP"},
                                  {"", "", "", "", "", "", "", ""},
                                  {"", "", "", "", "", "", "", ""},
                                  {"", "", "", "", "", "", "", ""},
                                  {"", "", "", "", "", "", "", ""},
                                  {"wP", "wP", "wP", "wP", "wP", "wP", "wP", "wP"},
                                  {"wR", "wN", "wB", "wQ", "wK", "wB", "wN", "wR"}};

// Function to determine the piece type from a PGN move string
char determinePieceFromMove(const std::string &pgnMove)
{
    if (pgnMove.size() >= 2)
    {
        char pieceAbbreviation = pgnMove[0]; // First character of the move string

        // Check if the first character represents a piece abbreviation
        if (pieceAbbreviation == 'K' || pieceAbbreviation == 'Q' || pieceAbbreviation == 'R' ||
            pieceAbbreviation == 'B' || pieceAbbreviation == 'N')
        {
            return pieceAbbreviation;
        }
    }
    // Default case: no piece abbreviation found (pawn move)
    return 'P';
}

void getDestinationFileAndRank(const string &pgnMove, int &destinationRank, int &destinationFile)
{
    destinationRank = 7 - (pgnMove.back() - '1');        // Row
    destinationFile = pgnMove[pgnMove.size() - 2] - 'a'; // Col
}

void processPawnMove(const string &pgnMove, bool isWhite)
{
    const int destRank = 7 - (pgnMove.back() - '1');        // Row
    const int destFile = pgnMove[pgnMove.size() - 2] - 'a'; // Col

    const string piece = isWhite ? "wP" : "bP";
    int direction = isWhite ? 1 : -1;

    int sourceRank = -1, sourceFile = -1;
    if (pgnMove.length() > 2 && pgnMove[1] == 'x')
    {
        sourceFile = pgnMove[0] - 'a';
        sourceRank = destRank + direction;
    }
    else
    {
        sourceFile = destFile;

        // Define the possible directions
        const std::vector<int> directions = {direction, 2 * direction};

        for (int dir : directions)
        {
            const int possibleSrcRank = destRank + dir;
            if (possibleSrcRank >= 0 &&
                possibleSrcRank < 8 &&
                possibleSrcRank != destRank &&
                possibleSrcRank != sourceRank &&
                chessBoard[possibleSrcRank][sourceFile] == piece)
            {
                sourceRank = possibleSrcRank;
                break;
            }
        }
    }

    if (sourceRank != -1)
    {
        chessBoard[sourceRank][sourceFile] = "";
        chessBoard[destRank][destFile] = piece;
    }
    else
    {
        cout << "Invalid pawn source for move= " << pgnMove << endl;
    }
}

void processRookMove(const std::string &pgnMove, bool isWhite)
{
    const int destRank = 7 - (pgnMove.back() - '1');        // Destination rank (row)
    const int destFile = pgnMove[pgnMove.size() - 2] - 'a'; // Destination file (column)

    const string piece = isWhite ? "wR" : "bR";

    int sourceRank = -1, sourceFile = -1;
    if (pgnMove.size() > 3 && pgnMove[1] != 'x')
    {
        if (isdigit(pgnMove[1]))
        {
            sourceRank = 7 - (pgnMove[1] - '1');
            sourceFile = destFile;
        }
        else if (isalpha(pgnMove[1]))
        {
            sourceFile = pgnMove[1] - 'a';
            sourceRank = destRank;
        }
    }
    else
    {
        for (int dir : {-1, 1})
        {
            int row = destRank;
            int col = destFile + dir;
            while (col >= 0 && col < 8)
            {
                if (chessBoard[row][col] == piece)
                {
                    sourceRank = row;
                    sourceFile = col;
                    break;
                }
                // Stop searching if a piece of any color is found
                if (chessBoard[row][col] != "")
                {
                    break;
                }
                col += dir;
            }

            row = destRank + dir;
            col = destFile;
            while (row >= 0 && row < 8)
            {
                if (chessBoard[row][col] == piece)
                {
                    sourceRank = row;
                    sourceFile = col;
                    break;
                }
                // Stop searching if a piece of any color is found
                if (chessBoard[row][col] != "")
                {
                    break;
                }
                row += dir;
            }
        }
    }

    if (sourceRank != -1)
    {
        chessBoard[sourceRank][sourceFile] = "";
        chessBoard[destRank][destFile] = piece;
    }
    else
    {
        cout << "Invalid rook source for move= " << pgnMove << endl;
    }
}

void processKingMove(const std::string &pgnMove, bool isWhite)
{
    const int destRank = 7 - (pgnMove[pgnMove.size() - 1] - '1'); // Destination rank (row)
    const int destFile = pgnMove[pgnMove.size() - 2] - 'a';       // Destination file (column)

    const string piece = isWhite ? "wK" : "bK";

    const auto sourcePosition = isWhite ? WhitePieceMap["K"][0] : BlackPieceMap["K"][0];
    chessBoard[sourcePosition.first][sourcePosition.second] = "";
    chessBoard[destRank][destFile] = piece;

    auto &sourcePositionRef = isWhite ? WhitePieceMap["K"][0] : BlackPieceMap["K"][0];
    sourcePositionRef.first = destRank;
    sourcePositionRef.second = destFile;
}

void processBishopMove(const std::string &pgnMove, bool isWhite)
{
    int destRank = 7 - (pgnMove.back() - '1');        // Destination rank (row)
    int destFile = pgnMove[pgnMove.size() - 2] - 'a'; // Destination file (column)

    // Set bishop piece code based on color
    std::string piece = isWhite ? "wB" : "bB";

    int sourceRank = -1, sourceFile = -1;
    for (int rowDir = -1; rowDir <= 1; rowDir += 2)
    {
        for (int colDir = -1; colDir <= 1; colDir += 2)
        {
            int row = destRank + rowDir;
            int col = destFile + colDir;
            while (row >= 0 && row < 8 && col >= 0 && col < 8)
            {
                if (chessBoard[row][col] == piece)
                {
                    sourceRank = row;
                    sourceFile = col;
                    break;
                }
                // Stop searching if a piece of any color is found
                if (chessBoard[row][col] != "")
                {
                    break;
                }
                row += rowDir;
                col += colDir;
            }
        }
    }

    if (sourceRank != -1)
    {
        chessBoard[sourceRank][sourceFile] = "";
        chessBoard[destRank][destFile] = piece;
    }
    else
    {
        cout << "Invalid bishop source for move= " << pgnMove << endl;
    }
}

void processKnightMove(const std::string &pgnMove, bool isWhite)
{
    // Extract destination file and rank
    const int destRank = 7 - (pgnMove[pgnMove.size() - 1] - '1'); // Destination rank (row)
    const int destFile = pgnMove[pgnMove.size() - 2] - 'a';       // Destination file (column)

    // Check all possible knight moves to find the source square
    std::vector<std::pair<int, int>> knightMoves = {
        {1, 2},
        {1, -2},
        {-1, 2},
        {-1, -2},
        {2, 1},
        {2, -1},
        {-2, 1},
        {-2, -1},
    };

    std::string piece = isWhite ? "wN" : "bN";

    int sourceRank = -1, sourceFile = -1;
    if (pgnMove.size() > 3 && pgnMove[1] != 'x')
    {
        if (isalpha(pgnMove[1]))
        {
            sourceFile = pgnMove[1] - 'a';
        }
    }

    for (const auto &move : knightMoves)
    {
        int row = destRank + move.first;
        int col = destFile + move.second;
        if (sourceFile != -1 && sourceFile != col)
        {
            continue;
        }
        if (row >= 0 && row < 8 && col >= 0 && col < 8 && chessBoard[row][col] == piece)
        {
            sourceRank = row;
            sourceFile = col;
            break;
        }
    }

    if (sourceRank != -1)
    {
        chessBoard[sourceRank][sourceFile] = "";
        chessBoard[destRank][destFile] = piece;
    }
    else
    {
        cout << "Invalid knight source for move= " << pgnMove << endl;
    }
}

void processQueenMove(const std::string &pgnMove, bool isWhite)
{
    const int destRank = 7 - (pgnMove[pgnMove.size() - 1] - '1'); // Destination rank (row)
    const int destFile = pgnMove[pgnMove.size() - 2] - 'a';       // Destination file (column)

    const string piece = isWhite ? "wQ" : "bQ";

    const auto sourcePosition = isWhite ? WhitePieceMap["Q"][0] : BlackPieceMap["Q"][0];
    chessBoard[sourcePosition.first][sourcePosition.second] = "";
    chessBoard[destRank][destFile] = piece;

    auto &sourcePositionRef = isWhite ? WhitePieceMap["Q"][0] : BlackPieceMap["Q"][0];
    sourcePositionRef.first = destRank;
    sourcePositionRef.second = destFile;
}

void processCastlingMove(const string &pgnMove, bool isWhite)
{
    // Determine the row index of the king and rook based on the side color
    const int kingRow = isWhite ? 7 : 0;
    const int rookRow = isWhite ? 7 : 0;

    // Determine the column index of the king and rook based on the castling direction
    const int kingDestCol = (pgnMove == "O-O") ? 6 : 2;
    const int rookDestCol = (pgnMove == "O-O") ? 5 : 3;

    // Move the king and rook to their destination squares and clear their original squares
    chessBoard[kingRow][kingDestCol] = isWhite ? "wK" : "bK";
    chessBoard[rookRow][rookDestCol] = isWhite ? "wR" : "bR";
    chessBoard[kingRow][4] = "";
    chessBoard[rookRow][(pgnMove == "O-O") ? 7 : 0] = "";

    auto &sourcePositionRef = isWhite ? WhitePieceMap["K"][0] : BlackPieceMap["K"][0];
    sourcePositionRef.first = kingRow;
    sourcePositionRef.second = kingDestCol;
}

// Function to determine the type of move from a PGN move string
MoveType determineMoveType(const std::string &pgnMove)
{
    // Check if the move is a piece move
    if (isalpha(pgnMove[0]))
    {
        // Check if it's a castling move
        if (pgnMove == "O-O" || pgnMove == "O-O-O")
        {
            return CASTLING;
        }
        else
        {
            return PIECE_MOVE;
        }
    }
    else
    {
        // Check if it's a game result
        if (pgnMove == "1-0" || pgnMove == "0-1" || pgnMove == "1/2-1/2")
        {
            return RESULT;
        }
    }

    return PIECE_MOVE; // or any other default value
}

void processMove(string &pgnMove, bool isWhite)
{
    const auto movetype = determineMoveType(pgnMove);
    if (movetype == MoveType::PIECE_MOVE)
    {
        char piece = determinePieceFromMove(pgnMove);
        if (pgnMove.back() == '+')
        {
            pgnMove.pop_back();
        }
        if (piece == 'P')
        {
            processPawnMove(pgnMove, isWhite);
        }
        else if (piece == 'R')
        {
            processRookMove(pgnMove, isWhite);
        }
        else if (piece == 'K')
        {
            processKingMove(pgnMove, isWhite);
        }
        else if (piece == 'B')
        {
            processBishopMove(pgnMove, isWhite);
        }
        else if (piece == 'N')
        {
            processKnightMove(pgnMove, isWhite);
        }
        else if (piece == 'Q')
        {
            processQueenMove(pgnMove, isWhite);
        }
    }
    else if (movetype == MoveType::CASTLING)
    {
        processCastlingMove(pgnMove, isWhite);
    }
}

vector<string> getMoves(const string &s, const regex &sep_regex)
{
    sregex_token_iterator iter(s.begin(), s.end(), sep_regex, -1);
    sregex_token_iterator end;
    return {iter, end};
}

// trim from start (in place)
void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
                                    { return !std::isspace(ch); }));
}

// trim from end (in place)
void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                         { return !std::isspace(ch); })
                .base(),
            s.end());
}

void trim(std::string &s)
{
    rtrim(s);
    ltrim(s);
}

string removeComments(const string &input)
{
    string op = input;
    size_t startComment = input.find('{');
    size_t endComment = input.find('}');

    if (startComment == string::npos && endComment == string::npos)
    {
        return op;
    }
    if (endComment != std::string::npos and startComment == string::npos)
    {
        op.erase(0, endComment + 1); // Remove the comment and space
    }
    else if (startComment != std::string::npos and endComment == string::npos)
    {
        op.erase(startComment); // Remove the comment
    }
    else
    {
        op.erase(startComment, endComment - startComment + 1);
    }
    return op;
}

void populateMoves(const string &s)
{
    string moveLine = removeComments(s);
    trim(moveLine);
    auto moves = getMoves(moveLine, regex{"\\d+\\.\\s"});

    for (int i = 0; i < moves.size(); ++i)
    {
        auto move = moves[i];
        if (!move.empty())
        {
            auto moveItems = getMoves(move, regex{"\\s+"});
            if (moveItems.size() >= 2)
            {
                parsedMoves.emplace_back(Move(moveItems[0], moveItems[1]));
            }
            if (moveItems.size() == 1)
            {
                auto &prevMove = parsedMoves.back();
                if (prevMove.whiteMove != "" and prevMove.blackMove != "")
                {
                    parsedMoves.emplace_back(Move(moveItems[0], ""));
                }
                else
                {
                    prevMove.blackMove = moveItems[0];
                }
            }
        }
    }
}

void loadGame(const string &pgnFileName)
{
    ifstream pgnFileStream(pgnFileName);

    // Check for failure
    if (pgnFileStream.fail())
    {
        throw 1;
    }

    string nextLine;
    while (getline(pgnFileStream, nextLine))
    {
        if (nextLine[0] == '[' or nextLine[0] == '%')
        {
            continue;
        }

        populateMoves(nextLine);
    }
}

void printBoard()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            cout << (chessBoard[i][j].empty() ? "  " : chessBoard[i][j]) << (j < 7 ? "|" : "\n");
        }
    }
}

int main()
{
    printBoard();
    const string filePath = "/Users/venk/Documents/GitHub/PgnChess/tests/pgn_files/test.pgn";
    loadGame(filePath);

    for (Move &move : parsedMoves)
    {
        processMove(move.whiteMove, true);
        processMove(move.blackMove, false);
    }

    cout << endl;
    printBoard();
    return 1;
}
