#pragma once
#include"CGameEngine.hpp"

enum class PieceColor
{
    Black,
    White
};

enum class PieceType
{
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

struct Cell
{
    int row;
    int col;
    bool isValid() const { return row >= 0 && row <= 7 && col >= 0 && col <= 7; }
    bool operator==(const Cell& other) const { return row == other.row && col == other.col; }
};
enum class GameState { Playing , Checkmate , Stalemate };

struct Snapshot
{
    GameState gameState;
    std::array<std::array<size_t, 8>, 8> squares{};   
    PieceColor Turn = PieceColor::White;               
    Cell From = {-1,-1};
    Cell To = {-1,-1};
};

using Squares = std::array<std::array<size_t, 8>, 8> ;

class CChessPiece
{
    public:
        PieceType type;
        PieceColor color;
        std::vector<std::pair<int , int>> directions;
        int maxMoves;
        CChessPiece(PieceType type , PieceColor color ,std::vector<std::pair<int , int>> directions , int maxMoves ) : type(type) , color(color) , directions(directions) , maxMoves(maxMoves){}
};

class Tile
{
    public:
        sf::RectangleShape tile;
        sf::CircleShape moveCircle;
        sf::CircleShape attackCircle;
        bool showMoveCircle = false;
        bool showAttackCircle = false;
};

struct Config
{
    sf::Vector2f BoardPosition = {0.f , 0.f};
    float cellSize = 100.f;
    sf::Color evenTileColor;
    sf::Color oddTileColor;
    sf::Color selectedEvenTileColor;
    sf::Color selectedOddTileColor;
    sf::Color attackingCircleColor;
};

class PieceVisualTracker
{
    public:
        size_t trackingEntityId = NoEntity;
        Cell trackingCell={-1,-1};
        bool dragging = false;

        bool isTracking() const { return trackingEntityId != NoEntity;}
        bool hasCell() const { return trackingCell.isValid();}
        void resetTracking()
        {
            trackingEntityId = NoEntity;
            trackingCell = {-1,-1};
            dragging = false;
        }
};

enum class CursorType
{
    Open,
    Closed,
    Default
};

class Chess : public Scene
{
    private:
        Config m_config;
        std::vector<Snapshot> m_history;
        size_t m_historyPointer = 0;
        std::optional<sf::Cursor> m_openHandCursor;
        std::optional<sf::Cursor> m_closedHandCursor;
        std::optional<sf::Cursor> m_defaultCursor;
        std::optional<CursorType> m_appliedCursor; 
        void updateCursor(sf::RenderWindow& window);

        std::array<std::array<Tile, 8>, 8> m_tiles{};
        void setUpTiles();
        void spawnPiece(Snapshot& snapshot , PieceType type , PieceColor color , int row , int col);
        void setUpPieces();

        PieceVisualTracker tracker;
        sf::RectangleShape m_hoverRect;
        bool m_showHover = false;
        std::vector<Cell> getPseudoLegalMoves(Cell from);
        std::vector<Cell> getLegalMoves(Cell from);
        bool isPlayersPiece(size_t entityId);          
        bool selectPiece(size_t entityId);             
        void showLegalMovesOfTrackingPiece();
        bool leavesKingSafe(Cell from , Cell to);
        bool hasAnyLegalMove();
        void checkGameEnd();
        
        const CChessPiece* pieceAt(const Squares& sq , Cell c);
        Cell findKing(const Squares& sq , PieceColor color);
        bool isCellUnderAttack(const Squares& sq , Cell target , PieceColor by);
        bool isKingUnderAttack(const Squares& sq , PieceColor color);
        bool validateMove(Cell from , Cell to);        
        void syncView();

        sf::Vector2f m_getWorldPosOfBoardPosition(int row , int col);
        Cell m_getBoardPositionOfWorldPos(sf::Vector2f pos);
        void m_fillConfig(std::string name , std::ifstream& file);
        sf::Color m_getFillColor(int row , int col , bool selected = false);

    public:
        void sDoAction(RefinedAction& input) override ;
        void sUpdate(sf::Time dt) override ;
        void sRender(sf::RenderWindow& gameWindow)  override;
        void onLoad(const std::optional<std::vector<Asset>>& assets) override ;
        void onDelete()  override;
        bool renderBelow() override ;
        bool updateBelow() override ;
};






// #pragma once
// #include"CGameEngine.hpp"
// #include"CConstantsAndDefines.hpp"
// #include<array>
// #include<fstream>
// #include<optional>
// #include<string>
// #include<vector>

// enum class PieceColor { Black, White };
// enum class PieceType  { Pawn, Knight, Bishop, Rook, Queen, King };
// enum class MoveKind   { Normal, DoublePush, EnPassant, CastleShort, CastleLong };
// enum class GameStatus { Playing, Checkmate, Stalemate };

// struct Cell
// {
//     int row = -1;
//     int col = -1;
//     bool isValid() const { return row >= 0 && row <= 7 && col >= 0 && col <= 7; }
//     bool operator==(const Cell&) const = default;
// };

// // Everything the rules need to know about one square.
// // The entity id only says WHICH sprite to show; type/color/moved are the real game state.
// struct Square
// {
//     size_t id = NoEntity;
//     PieceType type = PieceType::Pawn;
//     PieceColor color = PieceColor::White;
//     bool moved = false;               // needed for castling, restored by undo
// };

// // One position in the game. A plain value: copying it is a snapshot.
// struct Snapshot
// {
//     std::array<std::array<Square, 8>, 8> squares{};
//     PieceColor turn = PieceColor::White;     // side to move
//     Cell enPassant;                          // square a pawn just skipped over, invalid if none
//     Cell From;                               // last move (for highlighting), invalid at the start
//     Cell To;
// };

// struct Move
// {
//     Cell from;
//     Cell to;
//     MoveKind kind = MoveKind::Normal;
//     bool isCapture = false;
//     bool isPromotion = false;                // a pawn reaching the last row
//     PieceType promotion = PieceType::Queen;  // what it becomes (chosen in the promotion HUD)
// };

// // Slide animation of one piece (component on the piece entity)
// struct CPieceAnim
// {
//     sf::Vector2f from;
//     sf::Vector2f to;
//     float elapsed = 0.f;
//     float duration = 0.f;
//     float progress = 1.f;        // 0..1, linear
//     float tiltDegrees = 0.f;     // maximum lean for this move (signed)
//     float currentTilt = 0.f;
//     bool active = false;
// };

// class Tile
// {
//     public:
//         sf::RectangleShape tile;         // base colour, never changes
//         sf::CircleShape moveCircle;      // dot on empty target squares
//         sf::CircleShape attackCircle;    // ring on capture squares
//         bool showMoveCircle = false;
//         bool showAttackCircle = false;

//         // highlight that "fills up" the square like a growing circle
//         bool hasOverlay = false;
//         sf::Color overlayColor;
//         float overlayProgress = 1.f;
// };

// struct Config
// {
//     sf::Vector2f BoardPosition = {500.f , 200.f};
//     int cellSize = 100;
//     sf::Color evenTileColor          = sf::Color(234 , 237 , 209 , 255);
//     sf::Color oddTileColor           = sf::Color(118 , 149 , 86  , 255);
//     sf::Color selectedEvenTileColor  = sf::Color(245 , 246 , 130 , 255);
//     sf::Color selectedOddTileColor   = sf::Color(185 , 202 , 67  , 255);
//     sf::Color attackingCircleColor   = sf::Color(0   , 0   , 0   , 36);
//     sf::Color checkTileColor         = sf::Color(224 , 84  , 72  , 255);
//     std::string fontPath;            // optional: FontPath <path> in ChessConfig.txt
// };

// class Chess : public Scene
// {
//     private:
//         Config m_config;

//         // ---- history (undo / redo) : m_history[m_cursor] is the position on screen ----
//         std::vector<Snapshot> m_history;
//         size_t m_cursor = 0;
//         const Snapshot& current() const { return m_history[m_cursor]; }
//         void commit(Move move);
//         void undo();
//         void redo();
//         void restart();

//         // ---- UI state (never stored in history) ----
//         size_t selectedPiece = NoEntity;
//         Cell selectedCell;
//         std::vector<Move> m_legalMoves;              // legal moves of the selected piece
//         size_t m_draggedPiece = NoEntity;
//         std::optional<Move> m_pendingPromotion;      // promotion move waiting for the player's choice
//         bool m_showGameOverPanel = false;
//         bool canUseBoard() const;
//         void resetInteraction();
//         void clearSelection();
//         void setUpToShowMovesOfSelectedPiece();
//         void validateMove(Cell From , Cell To , bool fromDrag = false);
//         void beginPromotion(Move move , bool fromDrag);
//         void choosePromotion(PieceType type);
//         void cancelPromotion();
//         bool isMoveTarget(Cell cell) const;
//         void onLeftClick(sf::Vector2f mouse);
//         void onLeftDrag(const RefinedAction& input);

//         // ---- rules: pure logic on a Snapshot, no entities involved ----
//         std::vector<Move> generatePseudoMoves(const Snapshot& s , Cell from) const;
//         std::vector<Move> generateLegalMoves(const Snapshot& s , Cell from) const;
//         void applyMove(Snapshot& s , const Move& move) const;
//         bool isSquareAttacked(const Snapshot& s , Cell cell , PieceColor by) const;
//         bool isInCheck(const Snapshot& s , PieceColor color) const;
//         Cell findKing(const Snapshot& s , PieceColor color) const;
//         bool hasAnyLegalMove(const Snapshot& s , PieceColor color) const;

//         // ---- status ----
//         GameStatus m_status = GameStatus::Playing;
//         bool m_inCheck = false;
//         bool m_titleDirty = true;
//         void updateStatus();
//         std::string statusText() const;

//         // ---- animation ----
//         std::vector<size_t> m_pieceIds;              // all 32 piece entities
//         void startAnimation(size_t id , sf::Vector2f from , sf::Vector2f to);
//         void updateAnimations(float dt);

//         // ---- view: everything on screen is derived from m_history[m_cursor] + UI state ----
//         std::array<std::array<Tile, 8>, 8> m_tiles{};
//         void setUpTiles();
//         void setUpPieces();
//         void spawnPiece(Snapshot& snapshot , PieceType type , PieceColor color , int row , int col);
//         void fitSpriteToCell(sf::Sprite& sprite) const;
//         void syncView(bool animate = true);
//         void drawTiles(sf::RenderWindow& gameWindow);
//         void drawSquareFill(sf::RenderWindow& gameWindow , sf::Vector2f topLeft , const sf::Color& color , float progress);
//         void drawPieces(sf::RenderWindow& gameWindow);

//         // ---- HUD ----
//         sf::Font m_font;
//         bool m_hasFont = false;
//         void loadFont();
//         void drawCenteredText(sf::RenderWindow& gameWindow , const std::string& text , unsigned size , sf::Vector2f center , const sf::Color& color);
//         sf::Sprite makePieceSprite(PieceType type , PieceColor color , sf::Vector2f center , float scale);
//         sf::FloatRect promotionOptionRect(int index) const;
//         sf::FloatRect gameOverPanelRect() const;
//         sf::FloatRect gameOverButtonRect(int index) const;      // 0 = play again, 1 = close
//         void drawPromotionHud(sf::RenderWindow& gameWindow , sf::Vector2f mouse);
//         void drawGameOverHud(sf::RenderWindow& gameWindow , sf::Vector2f mouse);

//         // ---- mouse cursor ----
//         enum class CursorKind { Arrow , Hand , Fist };
//         CursorKind m_cursorKind = CursorKind::Arrow;
//         sf::RenderWindow* m_window = nullptr;
//         void updateCursor(sf::RenderWindow& gameWindow , sf::Vector2f mouse);

//         // ---- helpers ----
//         sf::Vector2f m_getWorldPosOfBoardPosition(int row , int col) const;
//         sf::Vector2f m_getPieceWorldPos(int row , int col) const;       // centre of the square
//         Cell m_getBoardPositionOfWorldPos(sf::Vector2f pos) const;
//         void m_fillConfig(std::string name , std::ifstream& file);
//         sf::Color m_getFillColor(int row , int col , bool selected = false) const;

//     public:
//         ~Chess() override;
//         void sDoAction(RefinedAction& input) override ;
//         void sUpdate(sf::Time dt) override ;
//         void sRender(sf::RenderWindow& gameWindow) override ;
//         void onLoad(const std::optional<std::vector<Asset>>& assets) override ;
//         void onDelete() override ;
//         bool renderBelow() override ;
//         bool updateBelow() override ;
// };