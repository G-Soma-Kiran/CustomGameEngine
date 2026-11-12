#include"Chess.hpp"
#include<iostream>
#include<fstream>
#include<algorithm>
#include<cmath>
#include<stdexcept>


bool Chess::renderBelow()
{
    return false;
}

bool Chess::updateBelow()
{
    return false;
}

void Chess::onDelete()
{
    
}

void Chess::onLoad( )
{

    std::ifstream file("Games/Chess/Assets/Configs/ChessConfig.txt");
    if(!file.is_open())
    {
        throw std::runtime_error("Chess: could not open ChessConfig.txt (check the working directory)");
    }
    std::string temp;
    while(file >> temp)
    {
        m_fillConfig(temp , file);
    }

    m_defaultCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);
    sf::Image image;
    image.loadFromFile("Games/Chess/Assets/Textures/openhand48.png");
    m_openHandCursor = sf::Cursor(image.getPixelsPtr(), image.getSize(), {24, 24});
    image.loadFromFile("Games/Chess/Assets/Textures/closedhand48.png");
    m_closedHandCursor = sf::Cursor(image.getPixelsPtr(), image.getSize(), {24, 24});

    {
        ActionBinding& binding = this->registerActionBinding("LeftClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Left);
    }
    {
        ActionBinding& binding = this->registerActionBinding("RightClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Right);
    }
    {
        ActionBinding& binding = this->registerActionBinding("LeftDrag");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Drag , sf::Mouse::Button::Left);
    }
    {
        ActionBinding& binding = this->registerActionBinding("Close");
        binding.addCondition<KeyCondition>(sf::Keyboard::Key::Escape);
        binding.addCondition<ModifierCondition>(false , false , false);
    }
    {
        ActionBinding& binding = this->registerActionBinding("Undo");
        binding.addCondition<KeyCondition>(sf::Keyboard::Key::Z);
        binding.addCondition<ModifierCondition>(true , false , false);
    }
    {
        ActionBinding& binding = this->registerActionBinding("Redo");
        binding.addCondition<KeyCondition>(sf::Keyboard::Key::Y);
        binding.addCondition<ModifierCondition>(true , false , false);
    }
    {
        ActionBinding& binding = this->registerActionBinding("NextScene");
        binding.addCondition<KeyCondition>(sf::Keyboard::Key::N);
        binding.addCondition<ModifierCondition>(false , false , false);
    }
    {
        ActionBinding& binding = this->registerActionBinding("PreviousScene");
        binding.addCondition<KeyCondition>(sf::Keyboard::Key::P);
        binding.addCondition<ModifierCondition>(false , false , false);
    }

    setUpTiles();
    setUpPieces();

    syncView();
}

void Chess::sDoAction(RefinedAction& input)
{
    

    if(input.getName() == "Close" && input.getType() == ActionType::Started)
    {
        m_requests.push_back(Request(RequestCommand::Pop , "InputTest"));
    }

    if(input.getName() == "LeftClick")
    {

        const size_t hit = input.getTriggerEntityId();

        if(tracker.isTracking())
        {

            if(hit != NoEntity && hit != tracker.trackingEntityId && isPlayersPiece(hit))
            {
                tracker.resetTracking();
                syncView();
                selectPiece(hit);
                return;
            }

            const Cell to = m_getBoardPositionOfWorldPos(sf::Vector2f(input.getTriggerMousePos()));
            validateMove(tracker.trackingCell , to);
            tracker.resetTracking();
            syncView();
        }
        else if(hit != NoEntity)
        {
            selectPiece(hit);
        }
    }
    else if(input.getName() == "LeftDrag")
    {
        if(input.getType() == ActionType::Started)
        {
            tracker.resetTracking();
            syncView();
            
            if(input.getInitiateEntityId() != NoEntity)
            {
                tracker.dragging=selectPiece(input.getInitiateEntityId());
                if(tracker.dragging) cancelSlide(tracker.trackingEntityId);
            }
        }
        else if(input.getType() == ActionType::OnGoing)
        {
            if(tracker.isTracking() && tracker.hasCell())
            {
                auto& visualComp = m_entities.storage<CVisual>().getComponent(tracker.trackingEntityId);
                const sf::Vector2f mouse(m_uisystem.getMousePos());
                
                visualComp.sprite.setPosition(mouse);

                const Cell hover = m_getBoardPositionOfWorldPos(sf::Vector2f(m_uisystem.getMousePos()));
                m_showHover = hover.isValid();
                if(m_showHover)
                {
                    m_hoverRect.setPosition(m_getWorldPosOfBoardPosition(hover.row , hover.col));
                }
            }
        }
        else if(input.getType() == ActionType::Ended)
        {
            if(tracker.isTracking() && tracker.hasCell())
            {
                
                const Cell to = m_getBoardPositionOfWorldPos(sf::Vector2f(m_uisystem.getMousePos()));
                validateMove(tracker.trackingCell , to);
            }
            tracker.resetTracking();
            syncView();
        }
        else if(input.getType() == ActionType::Cancelled)
        {
            tracker.resetTracking();
            syncView();
        }
    }
    else if(input.getName() == "RightClick")
    {
        tracker.resetTracking();
        syncView();
    }
    else if(input.getName() == "Undo")
    {
        if(input.getType() == ActionType::Started && m_historyPointer > 0)
        {
            --m_historyPointer;
            tracker.resetTracking();
            syncView();
        }
    }
    else if(input.getName() == "Redo")
    {
        if(input.getType() == ActionType::Started && m_historyPointer + 1 < m_history.size())
        {
            ++m_historyPointer;
            tracker.resetTracking();
            syncView();
        }
    }
    else if(input.getName() == "NextScene" && input.getType() == ActionType::Started)
    {
        m_appliedCursor.reset();
        m_requests.push_back(Request(RequestCommand::Push , "OrbPop"));
    }
    else if(input.getName() == "PreviousScene" && input.getType() == ActionType::Started)
    {
        m_requests.push_back(Request(RequestCommand::Pop , "Chess"));
    }
}

void Chess::sUpdate(sf::Time dt)
{
    const float d = std::min(dt.asSeconds() , 0.1f);     

    for(Slide& s : m_slides)
    {
        s.elapsed += d;
        const float t = std::min(s.elapsed / s.duration , 1.f);
        const float e = t < 0.5f ? 4.f * t * t * t : 1.f - std::pow(-2.f * t + 2.f , 3.f) / 2.f;
        auto& sprite = m_entities.storage<CVisual>().getComponent(s.id).sprite;

        if(t >= 1.f)
        {
            sprite.setPosition(s.to);                       
            sprite.setRotation(sf::degrees(0.f));
        }
        else
        {
            sprite.setPosition(s.from + (s.to - s.from) * e);
            sprite.setRotation(sf::degrees(s.tilt * std::sin(t * std::numbers::pi_v<float>)));
        }
    }
    std::erase_if(m_slides , [](const Slide& s){ return s.elapsed >= s.duration; });
}

void Chess::sRender(sf::RenderWindow& gameWindow)
{
    updateCursor(gameWindow);
    for(int i=0;i<8;i++)
    {
        for(int j=0;j<8;j++)
        {
            gameWindow.draw(m_tiles[i][j].tile);
            if(m_tiles[i][j].showAttackCircle)
            {
                gameWindow.draw(m_tiles[i][j].attackCircle);
            }
            if(m_tiles[i][j].showMoveCircle)
            {
                gameWindow.draw(m_tiles[i][j].moveCircle);
            }
        }
    }
    if(m_showHover) gameWindow.draw(m_hoverRect);

    for(auto& entity : m_entities.getEntities())
    {
        if(entity->getId() == tracker.trackingEntityId) continue;   
        if(!m_entities.storage<CRender>().getComponent(entity->getId()).render) continue;
        gameWindow.draw(m_entities.storage<CVisual>().getComponent(entity->getId()).sprite);
    }

    if(tracker.isTracking())
    {
        gameWindow.draw(m_entities.storage<CVisual>().getComponent(tracker.trackingEntityId).sprite);
    }

    

}

sf::Vector2f Chess::m_getWorldPosOfBoardPosition(int row, int col)
{
    return m_config.BoardPosition +
        sf::Vector2f({
            col * m_config.cellSize,
            row * m_config.cellSize
        });
}

void Chess::m_fillConfig(std::string name , std::ifstream& file)
{
    if(name == "BoardPosition")
    {
        float x , y;
        file >> x >> y;
        m_config.BoardPosition = sf::Vector2f(x ,y);
    }
    else if(name == "CellSize")
    {
        file >> m_config.cellSize;
    }
    else if(name == "evenTileColor")
    {
        int r , g , b , a;
        file >> r >> g >> b >> a;
        m_config.evenTileColor = sf::Color(r,g,b,a);
    }
    else if(name == "oddTileColor")
    {
        int r , g , b , a;
        file >> r >> g >> b >> a;
        m_config.oddTileColor = sf::Color(r,g,b,a);
    }
    else if(name == "selectedEvenTileColor")
    {
        int r , g , b , a;
        file >> r >> g >> b >> a;
        m_config.selectedEvenTileColor = sf::Color(r,g,b,a);
    }
    else if(name == "selectedOddTileColor")
    {
        int r , g , b , a;
        file >> r >> g >> b >> a;
        m_config.selectedOddTileColor = sf::Color(r,g,b,a);
    }
    else if(name == "attackingCircleColor")
    {
        int r , g , b , a;
        file >> r >> g >> b >> a;
        m_config.attackingCircleColor = sf::Color(r,g,b,a);
    }
}

Cell Chess::m_getBoardPositionOfWorldPos(sf::Vector2f pos)
{
    
    Cell returnVal;
    returnVal.row = static_cast<int>(std::floor((pos.y - m_config.BoardPosition.y) / m_config.cellSize));
    returnVal.col = static_cast<int>(std::floor((pos.x - m_config.BoardPosition.x) / m_config.cellSize));
    return returnVal;
}

sf::Color Chess::m_getFillColor(int row , int col , bool selected)
{
    if(selected)
    {
        if((row+col)%2 == 0) return m_config.selectedEvenTileColor;
        return m_config.selectedOddTileColor;
    }
    else
    {
        if((row+col)%2 == 0) return m_config.evenTileColor;
        return m_config.oddTileColor;
    }
}

void Chess::setUpTiles()
{
    const sf::Vector2f half(m_config.cellSize / 2.f , m_config.cellSize / 2.f);

    for(int i=0;i<8;i++)
    {
        for(int j=0;j<8;j++)
        {
            auto& tile = m_tiles[i][j];
            auto currPos = m_getWorldPosOfBoardPosition(i , j);

            tile.tile.setSize({m_config.cellSize , m_config.cellSize});
            tile.tile.setPosition(currPos);
            tile.tile.setFillColor(m_getFillColor(i , j));

            tile.moveCircle.setRadius(0.17f * m_config.cellSize);
            tile.moveCircle.setOrigin(tile.moveCircle.getGeometricCenter());
            tile.moveCircle.setPosition(currPos + half);
            tile.moveCircle.setFillColor(m_config.attackingCircleColor);

            
            tile.attackCircle.setRadius(0.5f * m_config.cellSize);
            tile.attackCircle.setOrigin(tile.attackCircle.getGeometricCenter());
            tile.attackCircle.setPosition(currPos + half);
            tile.attackCircle.setFillColor(sf::Color::Transparent);
            tile.attackCircle.setOutlineThickness(-10.f);
            tile.attackCircle.setOutlineColor(m_config.attackingCircleColor);
        }
    }
    m_hoverRect.setSize({m_config.cellSize , m_config.cellSize});
    m_hoverRect.setFillColor(sf::Color::Transparent);
    m_hoverRect.setOutlineThickness(-6.f);                 
    m_hoverRect.setOutlineColor(sf::Color(255 , 255 , 255 , 200));
}

std::string pieceNameOf(PieceType type)
{
    switch(type)
    {
        case PieceType::Pawn:   return "Pawn";
        case PieceType::Knight: return "Knight";
        case PieceType::Bishop: return "Bishop";
        case PieceType::Rook:   return "Rook";
        case PieceType::Queen:  return "Queen";
        case PieceType::King:   return "King";
    }
    return "";
}
std::string textureNameOf(PieceType type , PieceColor color)
{
    return std::string(color == PieceColor::White ? "White" : "Black") + pieceNameOf(type);
}

void Chess::spawnPiece(Snapshot& snapshot , PieceType type , PieceColor color , int row , int col)
{
    auto entity = m_entities.addEntity("ChessPiece");
    const size_t id = entity->getId();

    m_entities.storage<CTransform>().addComponent(id , m_getWorldPosOfBoardPosition(row , col) );
    auto& visualComp =  m_entities.storage<CVisual>().addComponent(id , m_assetManager.getTexture(textureNameOf(type , color)));
    visualComp.sprite.setScale({m_config.cellSize / visualComp.sprite.getLocalBounds().size.x ,m_config.cellSize / visualComp.sprite.getLocalBounds().size.y });
    visualComp.sprite.setOrigin(visualComp.sprite.getLocalBounds().size / 2.f);
    m_entities.storage<CMouseInteraction>().addComponent(id , sf::FloatRect({0.f , 0.f} ,{m_config.cellSize , m_config.cellSize}) );
    m_entities.storage<CRender>().addComponent(id).zIndex = 1;

    
    
    std::vector<std::pair<int , int>> directions;
    int maxMoves = 0;

    if(type == PieceType::Bishop)
    {
        directions = {{-1 , 1} , {1 , -1} , {1 , 1} , {-1 , -1}};
        maxMoves = 7;
    }
    else if(type == PieceType::King)
    {
        directions = {{-1 , 1} , {1 , -1} , {1 , 1} , {-1 , -1} , {0 , 1} , {0 , -1} , {1,0} , {-1 ,0}};
        maxMoves = 1;
    }
    else if(type == PieceType::Knight)
    {
        directions = { {-2 , -1} , {-2 , 1} , {-1 , -2} , {-1 , 2} , {1 , -2} , {1 , 2} , {2 , -1} , {2 , 1} };
        maxMoves = 1;
    }
    else if(type == PieceType::Queen)
    {
        directions = {{-1 , 1} , {1 , -1} , {1 , 1} , {-1 , -1} , {0 , 1} , {0 , -1} , {1,0} , {-1 ,0}};
        maxMoves = 7;
    }
    else if(type == PieceType::Rook)
    {
        directions = {{1 , 0} ,{-1 , 0} , {0 , 1} , {0 , -1}};
        maxMoves = 7;
    }

    m_entities.storage<CChessPiece>().addComponent(id , type , color , directions , maxMoves);

    snapshot.squares[row][col] = id;
}

void Chess::setUpPieces()
{
    const PieceType backRank[8] = {
        PieceType::Rook , PieceType::Knight , PieceType::Bishop , PieceType::Queen ,
        PieceType::King , PieceType::Bishop , PieceType::Knight , PieceType::Rook
    };

    Snapshot start;
    start.gameState = GameState::Playing;
    start.Turn = PieceColor::White;
    for(int col = 0; col < 8; ++col)
    {
        spawnPiece(start , backRank[col]   , PieceColor::Black , 0 , col);
        spawnPiece(start , PieceType::Pawn , PieceColor::Black , 1 , col);
        spawnPiece(start , PieceType::Pawn , PieceColor::White , 6 , col);
        spawnPiece(start , backRank[col]   , PieceColor::White , 7 , col);
    }

    m_history.clear();
    m_history.push_back(start);
    m_historyPointer = 0;
}

void Chess::startSlide(size_t id , sf::Vector2f from , sf::Vector2f to)
{
    const float dx = to.x - from.x;
    const float dy = to.y - from.y;
    const bool forward = (dx != 0.f) ? (dx > 0.f) : (dy > 0.f);
    const float tilt = forward ? 7.f : -7.f;


    for(Slide& s : m_slides)
    {
        if(s.id != id) continue;
        if(s.to == to) return;
        s.from = from;
        s.to = to;
        s.elapsed = 0.f;
        s.tilt = tilt;
        s.duration = std::min(0.45f , 0.14f + 0.035f * std::hypot(to.x - from.x , to.y - from.y) / static_cast<float>(m_config.cellSize));
        return;
    }

    Slide s;
    s.id = id;
    s.from = from;
    s.to = to;
    s.tilt = tilt;
    s.duration = std::min(0.45f , 0.14f + 0.035f * std::hypot(to.x - from.x , to.y - from.y) / static_cast<float>(m_config.cellSize));
    m_slides.push_back(s);
}

void Chess::cancelSlide(size_t id)
{
    auto& sprite = m_entities.storage<CVisual>().getComponent(id).sprite;
    sprite.setRotation(sf::degrees(0.f));
}

void Chess::syncView()
{
    m_showHover = false;
    auto& curr = m_history[m_historyPointer];

    for(auto& entity : m_entities.getEntities())
    {
        m_entities.storage<CRender>().getComponent(entity->getId()).render = false;
    }

    for(int i=0;i<8;i++)
    {
        for(int j=0;j<8;j++)
        {
            const size_t id = curr.squares[i][j];
            if(id != NoEntity)
            {
                const sf::Vector2f pos = m_getWorldPosOfBoardPosition(i , j);
                
                m_entities.storage<CTransform>().getComponent(id).position = pos;
                auto& sprite = m_entities.storage<CVisual>().getComponent(id).sprite;
                const sf::Vector2f center = getCellCenter(i , j);
                if(sprite.getPosition() != center) startSlide(id , sprite.getPosition() , center);
                m_entities.storage<CRender>().getComponent(id).render = true;
            }

            const bool highlighted = (curr.From == Cell{i , j}) || (curr.To == Cell{i , j});
            m_tiles[i][j].tile.setFillColor(m_getFillColor(i , j , highlighted));

            m_tiles[i][j].showAttackCircle = false;
            m_tiles[i][j].showMoveCircle = false;
        }
    }
}

std::vector<Cell> Chess::getPseudoLegalMoves(Cell from)
{
    std::vector<Cell> moves;
    if(!from.isValid()) return moves;

    const auto& squares = m_history[m_historyPointer].squares;
    const size_t id = squares[from.row][from.col];
    if(id == NoEntity) return moves;

    const auto& piece = m_entities.storage<CChessPiece>().getComponent(id);

    auto colorAt = [&](Cell c)
    {
        return m_entities.storage<CChessPiece>().getComponent(squares[c.row][c.col]).color;
    };

    if(piece.type == PieceType::Pawn)
    {
        const int dir      = (piece.color == PieceColor::White) ? -1 : 1;
        const int startRow = (piece.color == PieceColor::White) ?  6 : 1;

        const Cell one{from.row + dir , from.col};
        if(one.isValid() && squares[one.row][one.col] == NoEntity)
        {
            moves.push_back(one);

            const Cell two{from.row + 2 * dir , from.col};
            if(from.row == startRow && squares[two.row][two.col] == NoEntity)
            {
                moves.push_back(two);
            }
        }

        for(int dc : {-1 , 1})
        {
            const Cell diag{from.row + dir , from.col + dc};
            if(diag.isValid() && squares[diag.row][diag.col] != NoEntity && colorAt(diag) != piece.color)
            {
                moves.push_back(diag);
            }
        }
        return moves;
    }

    for(const auto& [dr , dc] : piece.directions)
    {
        for(int i = 1; i <= piece.maxMoves; i++)
        {
            const Cell c{from.row + i * dr , from.col + i * dc};
            if(!c.isValid()) break;

            if(squares[c.row][c.col] == NoEntity)
            {
                moves.push_back(c);
                continue;
            }

            if(colorAt(c) != piece.color)
            {
                moves.push_back(c);
            }
            break;                                              
        }
    }
    return moves;
}

bool Chess::isPlayersPiece(size_t entityId)
{
    if(!m_entities.storage<CChessPiece>().hasComponent(entityId)) return false;
    return m_entities.storage<CChessPiece>().getComponent(entityId).color == m_history[m_historyPointer].Turn;
}

bool Chess::selectPiece(size_t entityId)
{
    if(!isPlayersPiece(entityId)) return false;

    
    const auto& squares = m_history[m_historyPointer].squares;
    Cell cell = {-1 , -1};
    for(int i=0;i<8;i++)
        for(int j=0;j<8;j++)
            if(squares[i][j] == entityId) cell = {i , j};

    if(!cell.isValid()) return false;   

    tracker.trackingEntityId = entityId;
    tracker.trackingCell = cell;
    m_tiles[cell.row][cell.col].tile.setFillColor(m_getFillColor(cell.row , cell.col , true));
    if(m_history[m_historyPointer].gameState == GameState::Checkmate || m_history[m_historyPointer].gameState == GameState::Stalemate) return true;
    showLegalMovesOfTrackingPiece();
    return true;
}

const CChessPiece* Chess::pieceAt(const Squares& sq , Cell c)
{
    const size_t id = sq[c.row][c.col];
    if(id == NoEntity) return nullptr;
    return &m_entities.storage<CChessPiece>().getComponent(id);
}

Cell Chess::findKing(const Squares& sq , PieceColor color)
{
    for(int r = 0; r < 8; ++r)
        for(int c = 0; c < 8; ++c)
        {
            const CChessPiece* p = pieceAt(sq , {r , c});
            if(p && p->type == PieceType::King && p->color == color) { return Cell{r , c} ;}
        }
    
    return Cell{-1 , -1};
}

bool Chess::isCellUnderAttack(const Squares& sq , Cell target , PieceColor by)
{
    
    const int pawnRow = (by == PieceColor::White) ? 1 : -1;
    for(int dc : {-1 , 1})
    {
        const Cell c{target.row + pawnRow , target.col + dc};
        if(!c.isValid()) continue;
        const CChessPiece* p = pieceAt(sq , c);
        if(p && p->color == by && p->type == PieceType::Pawn) return true;
    }

    
    static const int knight[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for(const auto& d : knight)
    {
        const Cell c{target.row + d[0] , target.col + d[1]};
        if(!c.isValid()) continue;
        const CChessPiece* p = pieceAt(sq , c);
        if(p && p->color == by && p->type == PieceType::Knight) return true;
    }

    
    static const int dirs[8][2] = {{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1}};
    for(int k = 0; k < 8; ++k)
    {
        const bool diagonal = (k >= 4);
        for(int i = 1; i < 8; ++i)
        {
            const Cell c{target.row + i * dirs[k][0] , target.col + i * dirs[k][1]};
            if(!c.isValid()) break;

            const CChessPiece* p = pieceAt(sq , c);
            if(!p) continue;                       

            if(p->color == by)
            {
                if(p->type == PieceType::Queen) return true;
                if(p->type == (diagonal ? PieceType::Bishop : PieceType::Rook)) return true;
                if(i == 1 && p->type == PieceType::King) return true;
            }
            break;                                 
        }
    }
    return false;
}

bool Chess::isKingUnderAttack(const Squares& sq , PieceColor color)
{
    Cell king = findKing(sq , color);
    if(king == Cell{-1 , -1}) return false;
    return isCellUnderAttack(sq , king , color == PieceColor::White ? PieceColor::Black : PieceColor::White);
}

void Chess::checkGameEnd()
{
    Snapshot& s = m_history[m_historyPointer];
    if(hasAnyLegalMove())
        s.gameState = GameState::Playing;
    else
        s.gameState = isKingUnderAttack(s.squares , s.Turn) ? GameState::Checkmate: GameState::Stalemate;
}

bool Chess::validateMove(Cell from , Cell to)
{
    if(m_history[m_historyPointer].gameState == GameState::Checkmate || m_history[m_historyPointer].gameState == GameState::Stalemate) return false;
    if(!from.isValid() || !to.isValid() || from == to) return false;

    const Snapshot& current = m_history[m_historyPointer];

    const size_t movingId = current.squares[from.row][from.col];
    if(movingId == NoEntity) return false;
    if(!isPlayersPiece(movingId)) return false;

    const auto pseudoLegal = getPseudoLegalMoves(from);
    if(std::find(pseudoLegal.begin() , pseudoLegal.end() , to) == pseudoLegal.end()) return false;

    Snapshot next = current;
    next.squares[to.row][to.col] = movingId;      
    next.squares[from.row][from.col] = NoEntity;
    next.From = from;
    next.To = to;
    next.Turn = (current.Turn == PieceColor::White) ? PieceColor::Black : PieceColor::White;

    const bool kingIsUnderAttack = isKingUnderAttack(next.squares , current.Turn ); 
    if(kingIsUnderAttack) 
    {

        return false;
    }
    else
    {
        
    }
    
    m_history.erase(m_history.begin() + m_historyPointer + 1 , m_history.end());
    m_history.push_back(next);
    m_historyPointer = m_history.size() - 1;

    
    checkGameEnd();
    return true;
}

//Expects valid from , to i.e from should contain a piece and to should be a valid cell.
bool Chess::leavesKingSafe(Cell from , Cell to)
{
    const Squares& squares = m_history[m_historyPointer].squares;
    const CChessPiece* mover = pieceAt(squares , from);

    Squares after = squares;
    after[to.row][to.col] = after[from.row][from.col];
    after[from.row][from.col] = NoEntity;
    return !isKingUnderAttack(after , mover->color);
}

std::vector<Cell> Chess::getLegalMoves(Cell from)
{
    std::vector<Cell> legal;
    for(const Cell& to : getPseudoLegalMoves(from)) // getPseudoLegalMoves() returns empty vector if cell is not valid hence loop never runs hence no invalid cell is passed to leavesKingSafe.
    {
        if(leavesKingSafe(from , to)) legal.push_back(to);
    }
    return legal;
}

bool Chess::hasAnyLegalMove()
{
    const Snapshot& s = m_history[m_historyPointer];
    for(int r = 0; r < 8; ++r)
        for(int c = 0; c < 8; ++c)
        {
            const CChessPiece* p = pieceAt(s.squares , {r , c});
            if(!p || p->color != s.Turn) continue;

            for(const Cell& to : getPseudoLegalMoves({r , c}))
            {
                if(leavesKingSafe({r , c} , to)) return true;   // early exit i.e found at least one valud move.
            }
        }
    return false;
}

void Chess::showLegalMovesOfTrackingPiece()
{
    const auto& squares = m_history[m_historyPointer].squares;

    for(const Cell& c : getLegalMoves(tracker.trackingCell))
    {
        if(squares[c.row][c.col] != NoEntity)
            m_tiles[c.row][c.col].showAttackCircle = true;
        else
            m_tiles[c.row][c.col].showMoveCircle = true;
    }
}

void Chess::updateCursor(sf::RenderWindow& window)
{
    CursorType wanted = CursorType::Default;

    if(tracker.dragging)
    {
        wanted = CursorType::Closed;
    }
    else
    {
        const size_t hovered = m_entities.findTopMostEntityIdUnderMouse(sf::Mouse::getPosition(window));
        if(hovered != NoEntity && isPlayersPiece(hovered)) wanted = CursorType::Open;
    }

    if(m_appliedCursor == wanted) return;     
    m_appliedCursor = wanted;

    switch(wanted)
    {
        case CursorType::Default: window.setMouseCursor(*m_defaultCursor);    break;
        case CursorType::Open:    window.setMouseCursor(*m_openHandCursor);   break;
        case CursorType::Closed:  window.setMouseCursor(*m_closedHandCursor); break;
    }
}

sf::Vector2f Chess::getCellCenter(int row , int col)
{
    return m_getWorldPosOfBoardPosition(row , col) + sf::Vector2f(m_config.cellSize / 2.f , m_config.cellSize / 2.f);
}

// #include"Chess.hpp"
// #include<algorithm>
// #include<array>
// #include<cmath>
// #include<cstdint>
// #include<iostream>
// #include<numbers>

// // =====================================================================
// //  Small helpers (no scene state)
// // =====================================================================
// namespace
// {
//     struct MoveRule
//     {
//         std::vector<std::pair<int , int>> directions;   // (row step, col step)
//         int maxMoves;                                    // how many steps along one direction
//     };

//     // Data-driven movement for everything except pawns (pawns are special-cased).
//     const MoveRule& moveRuleOf(PieceType type)
//     {
//         static const MoveRule straight { { {1 , 0} , {-1 , 0} , {0 , 1} , {0 , -1} } , 7 };
//         static const MoveRule diagonal { { {1 , 1} , {1 , -1} , {-1 , 1} , {-1 , -1} } , 7 };
//         static const MoveRule allEight { { {1 , 0} , {-1 , 0} , {0 , 1} , {0 , -1} , {1 , 1} , {1 , -1} , {-1 , 1} , {-1 , -1} } , 7 };
//         static const MoveRule king     { { {1 , 0} , {-1 , 0} , {0 , 1} , {0 , -1} , {1 , 1} , {1 , -1} , {-1 , 1} , {-1 , -1} } , 1 };
//         static const MoveRule knight   { { {-2 , -1} , {-2 , 1} , {-1 , -2} , {-1 , 2} , {1 , -2} , {1 , 2} , {2 , -1} , {2 , 1} } , 1 };
//         static const MoveRule none     { {} , 0 };

//         switch(type)
//         {
//             case PieceType::Rook:   return straight;
//             case PieceType::Bishop: return diagonal;
//             case PieceType::Queen:  return allEight;
//             case PieceType::King:   return king;
//             case PieceType::Knight: return knight;
//             case PieceType::Pawn:   break;
//         }
//         return none;
//     }

//     bool inBounds(int row , int col) { return row >= 0 && row <= 7 && col >= 0 && col <= 7; }

//     PieceColor opposite(PieceColor color)
//     {
//         return color == PieceColor::White ? PieceColor::Black : PieceColor::White;
//     }

//     // White always starts at the bottom (rows 6,7) and moves up the screen (-1).
//     int pawnDirection(PieceColor color)    { return color == PieceColor::White ? -1 : 1; }
//     int pawnStartRow(PieceColor color)     { return color == PieceColor::White ?  6 : 1; }
//     int promotionRow(PieceColor color)     { return color == PieceColor::White ?  0 : 7; }
//     int homeRow(PieceColor color)          { return color == PieceColor::White ?  7 : 0; }

//     const char* pieceNameOf(PieceType type)
//     {
//         switch(type)
//         {
//             case PieceType::Pawn:   return "Pawn";
//             case PieceType::Knight: return "Knight";
//             case PieceType::Bishop: return "Bishop";
//             case PieceType::Rook:   return "Rook";
//             case PieceType::Queen:  return "Queen";
//             case PieceType::King:   return "King";
//         }
//         return "";
//     }

//     // matches the asset names in main.cpp: "WhitePawn", "BlackKnight", ...
//     std::string textureNameOf(PieceType type , PieceColor color)
//     {
//         return std::string(color == PieceColor::White ? "White" : "Black") + pieceNameOf(type);
//     }

//     // order of the pieces offered in the promotion HUD (same as chess.com)
//     constexpr PieceType kPromotionChoices[4] = { PieceType::Queen , PieceType::Knight , PieceType::Rook , PieceType::Bishop };

//     constexpr float kOverlayFillSeconds = 0.22f;      // time for a highlight circle to fill a square

//     float easeOutCubic(float t) { return 1.f - std::pow(1.f - t, 3.f); }

//     // ---- mouse cursors ----
//     // SFML has a system "hand" cursor but no "closed fist", so the fist is drawn into a
//     // 32x32 pixel buffer from a few circles + a rounded rectangle (white fill, black outline).
//     std::array<std::uint8_t , 32 * 32 * 4> makeFistPixels()
//     {
//         constexpr int size = 32;
//         std::array<std::uint8_t , size * size * 4> pixels{};         // all zero = transparent

//         struct Circle { float x; float y; float r; };
//         const Circle circles[] = {
//             {10.5f , 12.5f , 3.8f} , {14.8f , 11.0f , 3.8f} , {19.2f , 11.0f , 3.8f} , {23.5f , 12.5f , 3.8f} , // knuckles
//             { 8.5f , 19.5f , 4.4f }                                                                             // thumb
//         };

//         // signed distance to the fist silhouette (<= 0 means inside)
//         auto distance = [&](float px , float py)
//         {
//             float d = 1e9f;
//             for(const Circle& c : circles)
//                 d = std::min(d , std::hypot(px - c.x , py - c.y) - c.r);

//             const float qx = std::max(std::abs(px - 16.f)  - (9.5f - 4.f) , 0.f);   // palm: rounded rectangle
//             const float qy = std::max(std::abs(py - 19.5f) - (6.5f - 4.f) , 0.f);
//             return std::min(d , std::hypot(qx , qy) - 4.f);
//         };
//         const float creaseX[] = {12.7f , 17.0f , 21.4f};                              // lines between the fingers

//         for(int y = 0; y < size; ++y)
//             for(int x = 0; x < size; ++x)
//             {
//                 const float px = x + 0.5f;
//                 const float py = y + 0.5f;
//                 const float d = distance(px , py);

//                 std::uint8_t value = 0;
//                 std::uint8_t alpha = 0;
//                 if(d <= 0.f)       { value = 255; alpha = 255; }       // white fill
//                 else if(d <= 1.4f) { value = 0;   alpha = 255; }       // black outline

//                 if(d <= 0.f && py > 12.5f && py < 18.f)
//                     for(float cx : creaseX)
//                         if(std::abs(px - cx) < 0.6f) { value = 0; alpha = 255; }

//                 const size_t i = static_cast<size_t>(y * size + x) * 4;
//                 pixels[i] = pixels[i + 1] = pixels[i + 2] = value;
//                 pixels[i + 3] = alpha;
//             }
//         return pixels;
//     }

//     // Created once and kept alive until the program ends: the window keeps using the cursor
//     // it was given, so these must outlive any scene.
//     struct CursorSet
//     {
//         std::optional<sf::Cursor> arrow;
//         std::optional<sf::Cursor> hand;
//         std::optional<sf::Cursor> fist;
//     };

//     CursorSet& cursors()
//     {
//         static CursorSet set = []
//         {
//             CursorSet s;
//             s.arrow = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);
//             s.hand  = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand);
//             const auto pixels = makeFistPixels();
//             s.fist  = sf::Cursor::createFromPixels(pixels.data() , {32u , 32u} , {16u , 16u});
//             return s;
//         }();
//         return set;
//     }
// }

// // =====================================================================
// //  Scene plumbing
// // =====================================================================
// bool Chess::renderBelow()
// {
//     return false;
// }

// bool Chess::updateBelow()
// {
//     return false;
// }

// void Chess::onDelete()
// {
//     //Have to implement
// }

// Chess::~Chess()
// {
//     // give the window its normal cursor back when the scene goes away
//     if(m_window && cursors().arrow)
//         m_window->setMouseCursor(*cursors().arrow);
// }

// void Chess::onLoad()
// {

//     std::ifstream file("ChessConfig.txt");
//     if(!file.is_open())
//         std::cerr << "ChessConfig.txt not found, using default config\n";
//     std::string temp;
//     while(file >> temp)
//     {
//         m_fillConfig(temp , file);
//     }
//     loadFont();

//     {
//         ActionBinding& binding = this->registerActionBinding("LeftClick");
//         binding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Left);
//     }
//     {
//         ActionBinding& binding = this->registerActionBinding("RightClick");
//         binding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Right);
//     }
//     {
//         ActionBinding& binding = this->registerActionBinding("LeftDrag");
//         binding.setGesture<MouseGestureCondition>(mouseGestureType::Drag , sf::Mouse::Button::Left);
//     }
//     {
//         ActionBinding& binding = this->registerActionBinding("Close");
//         binding.addCondition<KeyCondition>(sf::Keyboard::Key::Escape);
//         binding.addCondition<ModifierCondition>(false , false , false);
//     }
//     {   // Ctrl+Z  (modifier order: ctrl, shift, alt)
//         ActionBinding& binding = this->registerActionBinding("Undo");
//         binding.addCondition<KeyCondition>(sf::Keyboard::Key::Z);
//         binding.addCondition<ModifierCondition>(true , false , false);
//     }
//     {   // Ctrl+Y
//         ActionBinding& binding = this->registerActionBinding("Redo");
//         binding.addCondition<KeyCondition>(sf::Keyboard::Key::Y);
//         binding.addCondition<ModifierCondition>(true , false , false);
//     }
//     {   // R
//         ActionBinding& binding = this->registerActionBinding("Restart");
//         binding.addCondition<KeyCondition>(sf::Keyboard::Key::R);
//         binding.addCondition<ModifierCondition>(false , false , false);
//     }

//     setUpTiles();
//     setUpPieces();
//     syncView(false);                 // first draw: pieces appear in place, no sliding
//     m_entities.update();
// }

// void Chess::sUpdate(sf::Time dt)
// {
//     m_entities.update();
//     updateAnimations(dt.asSeconds());
// }

// void Chess::sRender(sf::RenderWindow& gameWindow)
// {
//     // The Engine already clears and displays the window around sceneRender.
//     m_window = &gameWindow;
//     if(m_titleDirty)
//     {
//         gameWindow.setTitle(statusText());
//         m_titleDirty = false;
//     }

//     // raw window pixels, the same space the input system reports its positions in
//     const sf::Vector2f mouse(sf::Mouse::getPosition(gameWindow));
//     updateCursor(gameWindow , mouse);

//     drawTiles(gameWindow);
//     drawPieces(gameWindow);

//     if(m_pendingPromotion)
//         drawPromotionHud(gameWindow , mouse);
//     else if(m_showGameOverPanel)
//         drawGameOverHud(gameWindow , mouse);
// }

// // =====================================================================
// //  Input
// // =====================================================================
// bool Chess::canUseBoard() const
// {
//     return m_status == GameStatus::Playing && !m_pendingPromotion;
// }

// void Chess::sDoAction(RefinedAction& input)
// {
//     const std::string& name = input.getName();
//     const ActionType type = input.getType();

//     if(type == ActionType::Started)
//     {
//         if(name == "Close")
//         {
//             if(m_pendingPromotion) cancelPromotion();      // Escape closes the promotion HUD first
//             else m_requests.push_back(Request(RequestCommand::Pop , ""));
//             return;
//         }
//         if(name == "Undo")    { undo();    return; }
//         if(name == "Redo")    { redo();    return; }
//         if(name == "Restart") { restart(); return; }
//     }

//     if(name == "LeftClick" && type == ActionType::Pulsed)
//     {
//         onLeftClick(sf::Vector2f(input.getTriggerMousePos()));
//     }
//     else if(name == "RightClick" && type == ActionType::Pulsed)
//     {
//         if(m_pendingPromotion) cancelPromotion();
//         else clearSelection();
//     }
//     else if(name == "LeftDrag" && canUseBoard())
//     {
//         onLeftDrag(input);
//     }
// }

// // The grid is the hit-test: the click position is converted to a cell, so no entity
// // picking (CMouseInteraction / z-index) is needed, and empty squares are clickable too.
// void Chess::onLeftClick(sf::Vector2f mouse)
// {
//     // 1. promotion HUD open: pick a piece, or click anywhere else to cancel
//     if(m_pendingPromotion)
//     {
//         for(int i = 0; i < 4; ++i)
//         {
//             if(promotionOptionRect(i).contains(mouse))
//             {
//                 choosePromotion(kPromotionChoices[i]);
//                 return;
//             }
//         }
//         cancelPromotion();
//         return;
//     }

//     // 2. game over: only the panel buttons react
//     if(m_status != GameStatus::Playing)
//     {
//         if(m_showGameOverPanel)
//         {
//             if(gameOverButtonRect(0).contains(mouse))      restart();
//             else if(gameOverButtonRect(1).contains(mouse)) m_showGameOverPanel = false;
//         }
//         return;
//     }

//     // 3. the board
//     const Cell cell = m_getBoardPositionOfWorldPos(mouse);
//     if(!cell.isValid())
//     {
//         clearSelection();
//         return;
//     }

//     // clicked a highlighted square of the selected piece -> try to move there
//     if(selectedPiece != NoEntity && isMoveTarget(cell))
//     {
//         validateMove(selectedCell , cell , false);
//         return;
//     }

//     // only the side to move can be selected: the opponent's squares are never shown
//     const Square& square = current().squares[cell.row][cell.col];
//     if(square.id != NoEntity && square.color == current().turn && !(cell == selectedCell))
//     {
//         selectedCell = cell;
//         selectedPiece = square.id;
//         setUpToShowMovesOfSelectedPiece();
//     }
//     else
//     {
//         clearSelection();                    // empty square, enemy piece, or the selected piece again
//     }
// }

// void Chess::onLeftDrag(const RefinedAction& input)
// {
//     // the piece is found from the position where the press happened, not from an entity id
//     const sf::Vector2f mouse(m_uisystem.getMousePos());

//     switch(input.getType())
//     {
//         case ActionType::Started:
//         {
//             const Cell cell = m_getBoardPositionOfWorldPos(sf::Vector2f(input.getInitiateMousePos()));
//             if(!cell.isValid()) break;
//             const Square& square = current().squares[cell.row][cell.col];
//             if(square.id == NoEntity || square.color != current().turn) break;     // only your own pieces

//             selectedCell = cell;
//             selectedPiece = square.id;
//             m_draggedPiece = square.id;
//             m_entities.storage<CPieceAnim>().getComponent(square.id).active = false;   // stop a slide in progress
//             setUpToShowMovesOfSelectedPiece();
//             break;
//         }
//         case ActionType::OnGoing:
//         {
//             if(m_draggedPiece != NoEntity)
//                 m_entities.storage<CTransform>().getComponent(m_draggedPiece).position = mouse;    // sprite origin is its centre
//             break;
//         }
//         case ActionType::Ended:
//         {
//             if(m_draggedPiece == NoEntity) break;
//             m_draggedPiece = NoEntity;

//             const Cell cell = m_getBoardPositionOfWorldPos(mouse);
//             if(cell.isValid() && isMoveTarget(cell))
//                 validateMove(selectedCell , cell , true);
//             else
//                 syncView();                            // illegal drop: the piece slides back
//             break;
//         }
//         default:
//             break;
//     }
// }

// // =====================================================================
// //  Selection / moving
// // =====================================================================
// void Chess::resetInteraction()
// {
//     selectedPiece = NoEntity;
//     selectedCell = Cell{};
//     m_legalMoves.clear();
//     m_draggedPiece = NoEntity;
//     m_pendingPromotion.reset();
// }

// void Chess::clearSelection()
// {
//     resetInteraction();
//     syncView();
// }

// void Chess::setUpToShowMovesOfSelectedPiece()
// {
//     m_legalMoves = generateLegalMoves(current() , selectedCell);
//     syncView();                  // syncView paints the dots / rings from m_legalMoves
// }

// bool Chess::isMoveTarget(Cell cell) const
// {
//     for(const Move& move : m_legalMoves)
//         if(move.to == cell) return true;
//     return false;
// }

// // Final gate before a move enters the history. Legality (king safety, castling rules...)
// // was already enforced when m_legalMoves was generated.
// void Chess::validateMove(Cell From , Cell To , bool fromDrag)
// {
//     const Square& mover = current().squares[From.row][From.col];
//     if(mover.id == NoEntity || mover.color != current().turn)
//     {
//         syncView();
//         return;
//     }

//     for(const Move& move : m_legalMoves)
//     {
//         if(move.from != From || move.to != To) continue;

//         if(move.isPromotion)
//         {
//             beginPromotion(move , fromDrag);
//             return;
//         }

//         if(fromDrag)
//         {
//             // the piece is already under the cursor: put it exactly on the square so it doesn't slide
//             m_entities.storage<CTransform>().getComponent(mover.id).position = m_getPieceWorldPos(To.row , To.col);
//             m_entities.storage<CPieceAnim>().getComponent(mover.id).active = false;
//         }
//         commit(move);              // a click-move keeps its old position, so syncView() slides it
//         return;
//     }
//     syncView();
// }

// // ---- promotion HUD flow ----
// void Chess::beginPromotion(Move move , bool fromDrag)
// {
//     m_pendingPromotion = move;
//     m_legalMoves.clear();
//     m_draggedPiece = NoEntity;

//     const sf::Vector2f target = m_getPieceWorldPos(move.to.row , move.to.col);
//     CTransform& transform = m_entities.storage<CTransform>().getComponent(selectedPiece);
//     CPieceAnim& anim = m_entities.storage<CPieceAnim>().getComponent(selectedPiece);

//     if(fromDrag)
//     {
//         transform.position = target;
//         anim.active = false;
//         anim.currentTilt = 0.f;
//     }
//     else
//     {
//         startAnimation(selectedPiece , transform.position , target);
//     }
//     syncView();                  // hides the dots; the pawn stays on the target square (see syncView)
// }

// void Chess::choosePromotion(PieceType type)
// {
//     Move move = *m_pendingPromotion;
//     move.promotion = type;
//     m_pendingPromotion.reset();
//     commit(move);
// }

// void Chess::cancelPromotion()
// {
//     resetInteraction();          // also clears m_pendingPromotion
//     syncView();                  // the pawn slides back to where it came from
// }

// // =====================================================================
// //  History
// // =====================================================================
// void Chess::commit(Move move)
// {
//     Snapshot next = current();           // copy of the position on screen
//     applyMove(next , move);              // pure logic on the copy

//     m_history.resize(m_cursor + 1);      // a new move cuts off the redo branch
//     m_history.push_back(next);
//     ++m_cursor;

//     resetInteraction();
//     syncView();
// }

// void Chess::undo()
// {
//     if(m_cursor == 0) return;
//     --m_cursor;
//     resetInteraction();
//     syncView();
// }

// void Chess::redo()
// {
//     if(m_cursor + 1 >= m_history.size()) return;
//     ++m_cursor;
//     resetInteraction();
//     syncView();
// }

// void Chess::restart()
// {
//     m_history.resize(1);                 // index 0 is always the starting position
//     m_cursor = 0;
//     resetInteraction();
//     syncView(false);                     // pieces jump back, no sliding
// }

// // =====================================================================
// //  Rules (pure: only look at / change the Snapshot they are given)
// // =====================================================================
// Cell Chess::findKing(const Snapshot& s , PieceColor color) const
// {
//     for(int row = 0; row < 8; ++row)
//         for(int col = 0; col < 8; ++col)
//         {
//             const Square& sq = s.squares[row][col];
//             if(sq.id != NoEntity && sq.type == PieceType::King && sq.color == color)
//                 return Cell{row , col};
//         }
//     return Cell{};
// }

// // Is `cell` attacked by any piece of colour `by`?  Looks outward from the square,
// // so it never has to generate the enemy's moves.
// bool Chess::isSquareAttacked(const Snapshot& s , Cell cell , PieceColor by) const
// {
//     auto isPiece = [&](int row , int col , PieceType type) -> bool
//     {
//         if(!inBounds(row , col)) return false;
//         const Square& sq = s.squares[row][col];
//         return sq.id != NoEntity && sq.color == by && sq.type == type;
//     };

//     // pawns: a pawn of colour `by` attacks diagonally one step in ITS forward direction
//     const int pawnDir = pawnDirection(by);
//     if(isPiece(cell.row - pawnDir , cell.col - 1 , PieceType::Pawn)) return true;
//     if(isPiece(cell.row - pawnDir , cell.col + 1 , PieceType::Pawn)) return true;

//     for(const auto& [dr , dc] : moveRuleOf(PieceType::Knight).directions)
//         if(isPiece(cell.row + dr , cell.col + dc , PieceType::Knight)) return true;

//     for(const auto& [dr , dc] : moveRuleOf(PieceType::King).directions)
//         if(isPiece(cell.row + dr , cell.col + dc , PieceType::King)) return true;

//     // sliding pieces: walk each ray until the first piece
//     for(const auto& [dr , dc] : moveRuleOf(PieceType::Queen).directions)
//     {
//         const bool isDiagonal = (dr != 0 && dc != 0);
//         int row = cell.row + dr;
//         int col = cell.col + dc;
//         while(inBounds(row , col))
//         {
//             const Square& sq = s.squares[row][col];
//             if(sq.id != NoEntity)
//             {
//                 if(sq.color == by &&
//                    (sq.type == PieceType::Queen || sq.type == (isDiagonal ? PieceType::Bishop : PieceType::Rook)))
//                     return true;
//                 break;
//             }
//             row += dr;
//             col += dc;
//         }
//     }
//     return false;
// }

// bool Chess::isInCheck(const Snapshot& s , PieceColor color) const
// {
//     const Cell king = findKing(s , color);
//     if(!king.isValid()) return false;
//     return isSquareAttacked(s , king , opposite(color));
// }

// // Moves a piece could make if its own king didn't matter.
// std::vector<Move> Chess::generatePseudoMoves(const Snapshot& s , Cell from) const
// {
//     std::vector<Move> moves;
//     const Square& piece = s.squares[from.row][from.col];
//     if(piece.id == NoEntity) return moves;

//     auto add = [&](int row , int col , bool isCapture , MoveKind kind = MoveKind::Normal)
//     {
//         Move move{from , Cell{row , col} , kind , isCapture};
//         move.isPromotion = (piece.type == PieceType::Pawn && row == promotionRow(piece.color));
//         moves.push_back(move);
//     };

//     // ---- pawns ----
//     if(piece.type == PieceType::Pawn)
//     {
//         const int dir = pawnDirection(piece.color);
//         const int oneStep = from.row + dir;

//         if(inBounds(oneStep , from.col) && s.squares[oneStep][from.col].id == NoEntity)
//         {
//             add(oneStep , from.col , false);
//             const int twoSteps = from.row + 2 * dir;
//             if(from.row == pawnStartRow(piece.color) && s.squares[twoSteps][from.col].id == NoEntity)
//                 add(twoSteps , from.col , false , MoveKind::DoublePush);
//         }

//         for(int dc : {-1 , 1})
//         {
//             const int col = from.col + dc;
//             if(!inBounds(oneStep , col)) continue;
//             const Square& target = s.squares[oneStep][col];

//             if(target.id != NoEntity && target.color != piece.color)
//                 add(oneStep , col , true);
//             else if(target.id == NoEntity && piece.color == s.turn &&
//                     s.enPassant.row == oneStep && s.enPassant.col == col)
//                 add(oneStep , col , true , MoveKind::EnPassant);
//         }
//         return moves;
//     }

//     // ---- everything else: walk each direction up to maxMoves ----
//     const MoveRule& rule = moveRuleOf(piece.type);
//     for(const auto& [dr , dc] : rule.directions)
//     {
//         int row = from.row;
//         int col = from.col;
//         for(int step = 0; step < rule.maxMoves; ++step)
//         {
//             row += dr;
//             col += dc;
//             if(!inBounds(row , col)) break;

//             const Square& target = s.squares[row][col];
//             if(target.id == NoEntity)
//             {
//                 add(row , col , false);
//                 continue;
//             }
//             if(target.color != piece.color)
//                 add(row , col , true);
//             break;                                   // blocked by any piece
//         }
//     }

//     // ---- castling ----
//     if(piece.type == PieceType::King && !piece.moved && from.col == 4 && from.row == homeRow(piece.color))
//     {
//         const PieceColor enemy = opposite(piece.color);
//         if(!isSquareAttacked(s , from , enemy))
//         {
//             auto rookReady = [&](int col)
//             {
//                 const Square& rook = s.squares[from.row][col];
//                 return rook.id != NoEntity && rook.type == PieceType::Rook && rook.color == piece.color && !rook.moved;
//             };
//             auto isEmpty = [&](int col) { return s.squares[from.row][col].id == NoEntity; };
//             auto isSafe  = [&](int col) { return !isSquareAttacked(s , Cell{from.row , col} , enemy); };

//             if(rookReady(7) && isEmpty(5) && isEmpty(6) && isSafe(5) && isSafe(6))
//                 add(from.row , 6 , false , MoveKind::CastleShort);
//             if(rookReady(0) && isEmpty(1) && isEmpty(2) && isEmpty(3) && isSafe(2) && isSafe(3))
//                 add(from.row , 2 , false , MoveKind::CastleLong);
//         }
//     }
//     return moves;
// }

// // Pseudo moves minus the ones that leave your own king attacked.
// // Each candidate is tried on a COPY of the position, so nothing has to be undone.
// std::vector<Move> Chess::generateLegalMoves(const Snapshot& s , Cell from) const
// {
//     std::vector<Move> legal;
//     const Square& piece = s.squares[from.row][from.col];
//     if(piece.id == NoEntity) return legal;

//     for(const Move& move : generatePseudoMoves(s , from))
//     {
//         Snapshot next = s;
//         applyMove(next , move);
//         if(!isInCheck(next , piece.color))
//             legal.push_back(move);
//     }
//     return legal;
// }

// bool Chess::hasAnyLegalMove(const Snapshot& s , PieceColor color) const
// {
//     for(int row = 0; row < 8; ++row)
//         for(int col = 0; col < 8; ++col)
//         {
//             const Square& sq = s.squares[row][col];
//             if(sq.id != NoEntity && sq.color == color)
//                 if(!generateLegalMoves(s , Cell{row , col}).empty())
//                     return true;
//         }
//     return false;
// }

// // Plays a move on a snapshot. Captured pieces just disappear from the snapshot
// // (their entity stays alive, so undo can bring them back).
// void Chess::applyMove(Snapshot& s , const Move& move) const
// {
//     Square moving = s.squares[move.from.row][move.from.col];

//     if(move.kind == MoveKind::EnPassant)
//         s.squares[move.from.row][move.to.col] = Square{};      // the captured pawn sits beside the mover

//     s.squares[move.from.row][move.from.col] = Square{};
//     moving.moved = true;

//     if(moving.type == PieceType::Pawn && move.to.row == promotionRow(moving.color))
//         moving.type = move.promotion;                           // chosen in the promotion HUD (queen by default)

//     s.squares[move.to.row][move.to.col] = moving;               // overwrites a captured piece, if any

//     if(move.kind == MoveKind::CastleShort || move.kind == MoveKind::CastleLong)
//     {
//         const int rookFrom = (move.kind == MoveKind::CastleShort) ? 7 : 0;
//         const int rookTo   = (move.kind == MoveKind::CastleShort) ? 5 : 3;
//         Square rook = s.squares[move.to.row][rookFrom];
//         s.squares[move.to.row][rookFrom] = Square{};
//         rook.moved = true;
//         s.squares[move.to.row][rookTo] = rook;
//     }

//     s.enPassant = (move.kind == MoveKind::DoublePush)
//                     ? Cell{(move.from.row + move.to.row) / 2 , move.from.col}
//                     : Cell{};

//     s.turn = opposite(s.turn);
//     s.From = move.from;
//     s.To = move.to;
// }

// // =====================================================================
// //  Status
// // =====================================================================
// void Chess::updateStatus()
// {
//     const GameStatus previous = m_status;
//     const Snapshot& snap = current();
//     m_inCheck = isInCheck(snap , snap.turn);

//     if(hasAnyLegalMove(snap , snap.turn))
//         m_status = GameStatus::Playing;
//     else
//         m_status = m_inCheck ? GameStatus::Checkmate : GameStatus::Stalemate;

//     // the panel opens when the game ends and goes away when it is playing again (undo / restart)
//     if(m_status == GameStatus::Playing)       m_showGameOverPanel = false;
//     else if(previous == GameStatus::Playing)  m_showGameOverPanel = true;

//     m_titleDirty = true;
// }

// std::string Chess::statusText() const
// {
//     const bool whiteToMove = (current().turn == PieceColor::White);

//     switch(m_status)
//     {
//         case GameStatus::Playing:
//             return std::string(whiteToMove ? "White" : "Black") + " to move" + (m_inCheck ? " - CHECK" : "")
//                    + "   [Ctrl+Z undo, Ctrl+Y redo, R restart]";
//         case GameStatus::Checkmate:
//             return std::string("Checkmate - ") + (whiteToMove ? "Black" : "White") + " wins   [Ctrl+Z undo, R restart]";
//         case GameStatus::Stalemate:
//             return "Stalemate - draw   [Ctrl+Z undo, R restart]";
//     }
//     return "Chess";
// }

// // =====================================================================
// //  Animation
// // =====================================================================
// void Chess::startAnimation(size_t id , sf::Vector2f from , sf::Vector2f to)
// {
//     CPieceAnim& anim = m_entities.storage<CPieceAnim>().getComponent(id);
//     const float cell = static_cast<float>(m_config.cellSize);
//     const float distanceInCells = std::hypot(to.x - from.x , to.y - from.y) / cell;

//     anim.from = from;
//     anim.to = to;
//     anim.elapsed = 0.f;
//     anim.progress = 0.f;
//     anim.duration = std::min(0.45f , 0.14f + 0.035f * std::hypot(to.x - from.x , to.y - from.y) / cell);     // longer trips take a bit longer
//     anim.currentTilt = 0.f;

//     // lean in the direction of travel
//     const float dx = to.x - from.x;
//     const float dy = to.y - from.y;
//     const bool forward = (dx != 0.f) ? (dx > 0.f) : (dy > 0.f);
//     anim.tiltDegrees = forward ? 7.f : -7.f;
//     anim.active = true;
// }

// void Chess::updateAnimations(float dt)
// {
//     // highlight circles growing to fill their square
//     for(auto& rowOfTiles : m_tiles)
//         for(Tile& tile : rowOfTiles)
//             if(tile.hasOverlay && tile.overlayProgress < 1.f)
//                 tile.overlayProgress = std::min(1.f , tile.overlayProgress + dt / kOverlayFillSeconds);

//     // sliding pieces
//     auto& anims = m_entities.storage<CPieceAnim>();
//     auto& transforms = m_entities.storage<CTransform>();
//     for(size_t id : m_pieceIds)
//     {
//         CPieceAnim& anim = anims.getComponent(id);
//         if(!anim.active || id == m_draggedPiece) continue;

//         anim.elapsed += dt;
//         anim.progress = std::min(1.f , anim.elapsed / anim.duration);

//         CTransform& transform = transforms.getComponent(id);
//         transform.position = anim.from + (anim.to - anim.from) * easeOutCubic(anim.progress);
//         anim.currentTilt = anim.tiltDegrees * std::sin(std::numbers::pi_v<float> * anim.progress);

//         if(anim.progress >= 1.f)
//         {
//             anim.active = false;
//             anim.currentTilt = 0.f;
//             transform.position = anim.to;
//         }
//     }
// }

// // =====================================================================
// //  View
// // =====================================================================
// sf::Vector2f Chess::m_getWorldPosOfBoardPosition(int row , int col) const
// {
//     return m_config.BoardPosition +
//         sf::Vector2f(static_cast<float>(col * m_config.cellSize) ,
//                      static_cast<float>(row * m_config.cellSize));
// }

// // pieces fill their whole square and are drawn around their centre (so they can tilt)
// sf::Vector2f Chess::m_getPieceWorldPos(int row , int col) const
// {
//     const float half = 0.5f * m_config.cellSize;
//     return m_getWorldPosOfBoardPosition(row , col) + sf::Vector2f(half , half);
// }

// // floor (not int truncation), so positions left of / above the board don't land in cell 0
// Cell Chess::m_getBoardPositionOfWorldPos(sf::Vector2f pos) const
// {
//     return Cell{
//         static_cast<int>(std::floor((pos.y - m_config.BoardPosition.y) / m_config.cellSize)) ,
//         static_cast<int>(std::floor((pos.x - m_config.BoardPosition.x) / m_config.cellSize))
//     };
// }

// sf::Color Chess::m_getFillColor(int row , int col , bool selected) const
// {
//     const bool even = ((row + col) % 2 == 0);
//     if(selected)
//         return even ? m_config.selectedEvenTileColor : m_config.selectedOddTileColor;
//     return even ? m_config.evenTileColor : m_config.oddTileColor;
// }

// void Chess::m_fillConfig(std::string name , std::ifstream& file)
// {
//     auto readColor = [&file]()
//     {
//         int r , g , b , a;
//         file >> r >> g >> b >> a;
//         return sf::Color(static_cast<std::uint8_t>(r) , static_cast<std::uint8_t>(g) ,
//                          static_cast<std::uint8_t>(b) , static_cast<std::uint8_t>(a));
//     };

//     if(name == "BoardPosition")
//     {
//         float x , y;
//         file >> x >> y;
//         m_config.BoardPosition = sf::Vector2f(x , y);
//     }
//     else if(name == "CellSize")              { file >> m_config.cellSize; }
//     else if(name == "FontPath")              { file >> m_config.fontPath; }
//     else if(name == "evenTileColor")         { m_config.evenTileColor = readColor(); }
//     else if(name == "oddTileColor")          { m_config.oddTileColor = readColor(); }
//     else if(name == "selectedEvenTileColor") { m_config.selectedEvenTileColor = readColor(); }
//     else if(name == "selectedOddTileColor")  { m_config.selectedOddTileColor = readColor(); }
//     else if(name == "attackingCircleColor")  { m_config.attackingCircleColor = readColor(); }
//     else if(name == "checkTileColor")        { m_config.checkTileColor = readColor(); }
//     else std::cerr << "ChessConfig.txt: unknown key '" << name << "'\n";
// }

// void Chess::setUpTiles()
// {
//     const float cell = static_cast<float>(m_config.cellSize);
//     const float ringThickness = 0.08f * cell;
//     const float ringRadius = 0.47f * cell - ringThickness;    // outline grows outwards, so the ring stays inside the square
//     const float dotRadius = 0.17f * cell;

//     for(int row = 0; row < 8; ++row)
//     {
//         for(int col = 0; col < 8; ++col)
//         {
//             Tile& tile = m_tiles[row][col];
//             const sf::Vector2f origin = m_getWorldPosOfBoardPosition(row , col);
//             const sf::Vector2f center = origin + sf::Vector2f(cell / 2.f , cell / 2.f);

//             tile.tile.setSize({cell , cell});
//             tile.tile.setPosition(origin);
//             tile.tile.setFillColor(m_getFillColor(row , col));

//             tile.moveCircle.setRadius(dotRadius);
//             tile.moveCircle.setOrigin({dotRadius , dotRadius});
//             tile.moveCircle.setPosition(center);
//             tile.moveCircle.setFillColor(m_config.attackingCircleColor);

//             tile.attackCircle.setRadius(ringRadius);
//             tile.attackCircle.setOrigin({ringRadius , ringRadius});
//             tile.attackCircle.setPosition(center);
//             tile.attackCircle.setFillColor(sf::Color::Transparent);     // ring only, no filled disc
//             tile.attackCircle.setOutlineThickness(ringThickness);
//             tile.attackCircle.setOutlineColor(m_config.attackingCircleColor);
//         }
//     }
// }

// // pieces are drawn at exactly one cell, rotating around their centre
// void Chess::fitSpriteToCell(sf::Sprite& sprite) const
// {
//     const sf::Vector2u size = sprite.getTexture().getSize();
//     const float target = static_cast<float>(m_config.cellSize);
//     sprite.setScale({target / static_cast<float>(size.x) , target / static_cast<float>(size.y)});
//     sprite.setOrigin({size.x / 2.f , size.y / 2.f});
// }

// // Entities are created ONCE (32 of them) and live for the whole scene.
// // Captured pieces just aren't in the current snapshot, so undo can bring them back.
// void Chess::spawnPiece(Snapshot& snapshot , PieceType type , PieceColor color , int row , int col)
// {
//     auto entity = m_entities.addEntity("ChessPiece");
//     const size_t id = entity->getId();

//     m_entities.storage<CTransform>().addComponent(id , m_getPieceWorldPos(row , col));
//     CVisual& visual = m_entities.storage<CVisual>().addComponent(id , m_assetManager.getTexture(textureNameOf(type , color)));
//     fitSpriteToCell(visual.sprite);
//     m_entities.storage<CPieceAnim>().addComponent(id);
//     m_pieceIds.push_back(id);

//     Square& square = snapshot.squares[row][col];
//     square.id = id;
//     square.type = type;
//     square.color = color;
//     square.moved = false;
// }

// void Chess::setUpPieces()
// {
//     const PieceType backRank[8] = {
//         PieceType::Rook , PieceType::Knight , PieceType::Bishop , PieceType::Queen ,
//         PieceType::King , PieceType::Bishop , PieceType::Knight , PieceType::Rook
//     };

//     m_pieceIds.clear();
//     Snapshot start;
//     start.turn = PieceColor::White;
//     for(int col = 0; col < 8; ++col)
//     {
//         spawnPiece(start , backRank[col]   , PieceColor::Black , 0 , col);
//         spawnPiece(start , PieceType::Pawn , PieceColor::Black , 1 , col);
//         spawnPiece(start , PieceType::Pawn , PieceColor::White , 6 , col);
//         spawnPiece(start , backRank[col]   , PieceColor::White , 7 , col);
//     }

//     m_history.clear();
//     m_history.push_back(start);
//     m_cursor = 0;
// }

// // Makes everything on screen match m_history[m_cursor] plus the UI state (selection).
// // `animate` = slide pieces to their new squares (false for restart / first draw).
// void Chess::syncView(bool animate)
// {
//     const Snapshot& snap = current();
//     updateStatus();

//     for(auto& rowOfTiles : m_tiles)
//         for(Tile& tile : rowOfTiles)
//         {
//             tile.showMoveCircle = false;
//             tile.showAttackCircle = false;
//         }

//     // ---- square highlights: last move, selection, king in check (check wins) ----
//     std::array<std::array<std::optional<sf::Color>, 8>, 8> wanted{};
//     if(snap.From.isValid())
//     {
//         wanted[snap.From.row][snap.From.col] = m_getFillColor(snap.From.row , snap.From.col , true);
//         wanted[snap.To.row][snap.To.col]     = m_getFillColor(snap.To.row , snap.To.col , true);
//     }
//     if(selectedPiece != NoEntity && selectedCell.isValid())
//         wanted[selectedCell.row][selectedCell.col] = m_getFillColor(selectedCell.row , selectedCell.col , true);
//     if(m_inCheck)
//     {
//         const Cell king = findKing(snap , snap.turn);
//         if(king.isValid()) wanted[king.row][king.col] = m_config.checkTileColor;
//     }

//     // a highlight only (re)starts its fill animation when its colour actually changes
//     for(int row = 0; row < 8; ++row)
//         for(int col = 0; col < 8; ++col)
//         {
//             Tile& tile = m_tiles[row][col];
//             const std::optional<sf::Color>& color = wanted[row][col];
//             if(!color)
//             {
//                 tile.hasOverlay = false;
//             }
//             else if(!tile.hasOverlay || tile.overlayColor != *color)
//             {
//                 tile.hasOverlay = true;
//                 tile.overlayColor = *color;
//                 tile.overlayProgress = 0.f;
//             }
//         }

//     // ---- legal move dots / capture rings ----
//     if(selectedPiece != NoEntity)
//     {
//         for(const Move& move : m_legalMoves)
//         {
//             Tile& tile = m_tiles[move.to.row][move.to.col];
//             if(move.isCapture) tile.showAttackCircle = true;
//             else               tile.showMoveCircle = true;
//         }
//     }

//     // ---- pieces: texture + where they should be ----
//     auto& transforms = m_entities.storage<CTransform>();
//     auto& visuals = m_entities.storage<CVisual>();
//     auto& anims = m_entities.storage<CPieceAnim>();

//     for(int row = 0; row < 8; ++row)
//         for(int col = 0; col < 8; ++col)
//         {
//             const Square& square = snap.squares[row][col];
//             if(square.id == NoEntity) continue;

//             // the texture changes when a pawn is promoted or a promotion is undone
//             CVisual& visual = visuals.getComponent(square.id);
//             visual.sprite.setTexture(m_assetManager.getTexture(textureNameOf(square.type , square.color)) , true);
//             fitSpriteToCell(visual.sprite);

//             if(square.id == m_draggedPiece) continue;                                    // follows the cursor
//             if(m_pendingPromotion && square.id == selectedPiece) continue;               // waits on the target square for the choice

//             const sf::Vector2f target = m_getPieceWorldPos(row , col);
//             CTransform& transform = transforms.getComponent(square.id);
//             CPieceAnim& anim = anims.getComponent(square.id);

//             if(anim.active && anim.to == target) continue;                               // already on its way
//             if(transform.position == target)
//             {
//                 anim.active = false;
//                 anim.currentTilt = 0.f;
//                 continue;
//             }

//             if(animate)
//             {
//                 startAnimation(square.id , transform.position , target);
//             }
//             else
//             {
//                 transform.position = target;
//                 anim.active = false;
//                 anim.currentTilt = 0.f;
//             }
//         }
// }

// // A circle growing from the middle of the square, clipped to the square.
// // Clamping the circle's outline points into the square gives exactly "circle ∩ square",
// // drawn as a triangle fan from the centre.
// void Chess::drawSquareFill(sf::RenderWindow& gameWindow , sf::Vector2f topLeft , const sf::Color& color , float progress)
// {
//     const float cell = static_cast<float>(m_config.cellSize);

//     if(progress >= 1.f)
//     {
//         sf::RectangleShape full({cell , cell});
//         full.setPosition(topLeft);
//         full.setFillColor(color);
//         gameWindow.draw(full);
//         return;
//     }

//     constexpr int segments = 40;
//     const float radius = easeOutCubic(progress) * 0.7072f * cell;          // 0.7071 * cell reaches the corners
//     const sf::Vector2f center = topLeft + sf::Vector2f(cell / 2.f , cell / 2.f);

//     auto makeVertex = [&color](sf::Vector2f position)
//     {
//         sf::Vertex vertex;
//         vertex.position = position;
//         vertex.color = color;
//         return vertex;
//     };

//     std::array<sf::Vertex , segments + 2> fan;
//     fan[0] = makeVertex(center);
//     for(int i = 0; i <= segments; ++i)
//     {
//         const float angle = 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / segments;
//         sf::Vector2f point = center + sf::Vector2f(std::cos(angle) , std::sin(angle)) * radius;
//         point.x = std::clamp(point.x , topLeft.x , topLeft.x + cell);
//         point.y = std::clamp(point.y , topLeft.y , topLeft.y + cell);
//         fan[i + 1] = makeVertex(point);
//     }
//     gameWindow.draw(fan.data() , fan.size() , sf::PrimitiveType::TriangleFan);
// }

// void Chess::drawTiles(sf::RenderWindow& gameWindow)
// {
//     for(int row = 0; row < 8; ++row)
//         for(int col = 0; col < 8; ++col)
//         {
//             Tile& tile = m_tiles[row][col];
//             gameWindow.draw(tile.tile);
//             if(tile.hasOverlay)
//                 drawSquareFill(gameWindow , tile.tile.getPosition() , tile.overlayColor , tile.overlayProgress);
//             if(tile.showAttackCircle) gameWindow.draw(tile.attackCircle);
//             if(tile.showMoveCircle)   gameWindow.draw(tile.moveCircle);
//         }
// }

// void Chess::drawPieces(sf::RenderWindow& gameWindow)
// {
//     const Snapshot& snap = current();
//     auto& transforms = m_entities.storage<CTransform>();
//     auto& visuals = m_entities.storage<CVisual>();
//     auto& anims = m_entities.storage<CPieceAnim>();

//     std::vector<size_t> resting;
//     std::vector<size_t> moving;
//     for(int row = 0; row < 8; ++row)
//         for(int col = 0; col < 8; ++col)
//         {
//             const size_t id = snap.squares[row][col].id;
//             if(id == NoEntity || id == m_draggedPiece) continue;
//             (anims.getComponent(id).active ? moving : resting).push_back(id);
//         }

//     auto drawPiece = [&](size_t id)
//     {
//         CVisual& visual = visuals.getComponent(id);
//         visual.sprite.setColor(sf::Color::White);
//         visual.sprite.setRotation(sf::degrees(anims.getComponent(id).currentTilt));
//         visual.sprite.setPosition(transforms.getComponent(id).position);
//         gameWindow.draw(visual.sprite);
//     };

//     for(size_t id : resting) drawPiece(id);

//     // sliding pieces are drawn above the others, with a faint trail behind them (dim motion blur)
//     for(size_t id : moving)
//     {
//         const CPieceAnim& anim = anims.getComponent(id);
//         CVisual& visual = visuals.getComponent(id);
//         visual.sprite.setRotation(sf::degrees(anim.currentTilt));

//         for(int k = 4; k >= 1; --k)
//         {
//             const float ghostProgress = anim.progress - 0.07f * static_cast<float>(k);
//             if(ghostProgress <= 0.f) continue;
//             visual.sprite.setPosition(anim.from + (anim.to - anim.from) * easeOutCubic(ghostProgress));
//             visual.sprite.setColor(sf::Color(255 , 255 , 255 , static_cast<std::uint8_t>(64 / k)));
//             gameWindow.draw(visual.sprite);
//         }
//         drawPiece(id);
//     }

//     if(m_draggedPiece != NoEntity)
//         drawPiece(m_draggedPiece);                 // the dragged piece floats above everything
// }

// // =====================================================================
// //  HUD
// // =====================================================================
// void Chess::loadFont()
// {
//     std::vector<std::string> candidates;
//     if(!m_config.fontPath.empty()) candidates.push_back(m_config.fontPath);
//     for(const char* path : {"Assets/Fonts/Font.ttf" , "Assets/Fonts/font.ttf" ,
//                             "C:/Windows/Fonts/segoeui.ttf" , "C:/Windows/Fonts/arial.ttf" ,
//                             "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf" ,
//                             "/System/Library/Fonts/Helvetica.ttc" , "/Library/Fonts/Arial.ttf"})
//         candidates.push_back(path);

//     for(const std::string& path : candidates)
//     {
//         if(m_font.openFromFile(path))
//         {
//             m_hasFont = true;
//             return;
//         }
//     }
//     std::cerr << "No font found (set FontPath in ChessConfig.txt): the game over panel will use icons instead of text\n";
// }

// void Chess::drawCenteredText(sf::RenderWindow& gameWindow , const std::string& text , unsigned size , sf::Vector2f center , const sf::Color& color)
// {
//     sf::Text label(m_font , text , size);
//     label.setFillColor(color);
//     const sf::FloatRect bounds = label.getLocalBounds();
//     label.setOrigin(bounds.position + bounds.size / 2.f);
//     label.setPosition(center);
//     gameWindow.draw(label);
// }

// sf::Sprite Chess::makePieceSprite(PieceType type , PieceColor color , sf::Vector2f center , float scale)
// {
//     sf::Sprite sprite(m_assetManager.getTexture(textureNameOf(type , color)));
//     const sf::Vector2u size = sprite.getTexture().getSize();
//     const float target = scale * static_cast<float>(m_config.cellSize);
//     sprite.setScale({target / static_cast<float>(size.x) , target / static_cast<float>(size.y)});
//     sprite.setOrigin({size.x / 2.f , size.y / 2.f});
//     sprite.setPosition(center);
//     return sprite;
// }

// // Column of 4 squares starting on the promotion square and running back into the board.
// sf::FloatRect Chess::promotionOptionRect(int index) const
// {
//     const Move& move = *m_pendingPromotion;
//     const int step = (current().turn == PieceColor::White) ? 1 : -1;
//     const float cell = static_cast<float>(m_config.cellSize);
//     return sf::FloatRect(m_getWorldPosOfBoardPosition(move.to.row + index * step , move.to.col) , {cell , cell});
// }

// sf::FloatRect Chess::gameOverPanelRect() const
// {
//     const float cell = static_cast<float>(m_config.cellSize);
//     const sf::Vector2f boardCenter = m_config.BoardPosition + sf::Vector2f(4.f * cell , 4.f * cell);
//     return sf::FloatRect(boardCenter - sf::Vector2f(2.4f * cell , 1.5f * cell) , {4.8f * cell , 3.0f * cell});
// }

// sf::FloatRect Chess::gameOverButtonRect(int index) const
// {
//     const float cell = static_cast<float>(m_config.cellSize);
//     const sf::FloatRect panel = gameOverPanelRect();
//     const float width = 2.0f * cell;
//     const float height = 0.75f * cell;
//     const float gap = 0.2f * cell;
//     const float left = panel.position.x + (panel.size.x - (2.f * width + gap)) / 2.f;
//     const float top = panel.position.y + panel.size.y - 0.35f * cell - height;
//     return sf::FloatRect({left + static_cast<float>(index) * (width + gap) , top} , {width , height});
// }

// void Chess::drawPromotionHud(sf::RenderWindow& gameWindow , sf::Vector2f mouse)
// {
//     const float cell = static_cast<float>(m_config.cellSize);
//     const PieceColor color = current().turn;

//     // dim the board
//     sf::RectangleShape dim({8.f * cell , 8.f * cell});
//     dim.setPosition(m_config.BoardPosition);
//     dim.setFillColor(sf::Color(0 , 0 , 0 , 130));
//     gameWindow.draw(dim);

//     // soft shadow behind the column
//     const sf::FloatRect first = promotionOptionRect(0);
//     const sf::FloatRect last = promotionOptionRect(3);
//     sf::RectangleShape shadow({cell , 4.f * cell});
//     shadow.setPosition({first.position.x + 4.f , std::min(first.position.y , last.position.y) + 5.f});
//     shadow.setFillColor(sf::Color(0 , 0 , 0 , 90));
//     gameWindow.draw(shadow);

//     for(int i = 0; i < 4; ++i)
//     {
//         const sf::FloatRect rect = promotionOptionRect(i);
//         sf::RectangleShape button(rect.size);
//         button.setPosition(rect.position);
//         button.setFillColor(rect.contains(mouse) ? sf::Color(214 , 232 , 184) : sf::Color(247 , 247 , 247));
//         gameWindow.draw(button);
//         gameWindow.draw(makePieceSprite(kPromotionChoices[i] , color , rect.position + rect.size / 2.f , 1.f));
//     }
// }

// void Chess::drawGameOverHud(sf::RenderWindow& gameWindow , sf::Vector2f mouse)
// {
//     const float cell = static_cast<float>(m_config.cellSize);
//     const bool checkmate = (m_status == GameStatus::Checkmate);
//     const PieceColor winner = (current().turn == PieceColor::White) ? PieceColor::Black : PieceColor::White;

//     sf::RectangleShape dim(sf::Vector2f(gameWindow.getSize()));
//     dim.setFillColor(sf::Color(0 , 0 , 0 , 120));
//     gameWindow.draw(dim);

//     const sf::FloatRect panel = gameOverPanelRect();
//     sf::RectangleShape panelShape(panel.size);
//     panelShape.setPosition(panel.position);
//     panelShape.setFillColor(sf::Color(38 , 36 , 33 , 245));
//     panelShape.setOutlineThickness(2.f);
//     panelShape.setOutlineColor(sf::Color(90 , 88 , 84));
//     gameWindow.draw(panelShape);

//     const float centerX = panel.position.x + panel.size.x / 2.f;
//     if(m_hasFont)
//     {
//         drawCenteredText(gameWindow , checkmate ? "Checkmate" : "Stalemate" ,
//                          static_cast<unsigned>(0.42f * cell) , {centerX , panel.position.y + 0.65f * cell} , sf::Color::White);
//         drawCenteredText(gameWindow , checkmate ? (winner == PieceColor::White ? "White wins" : "Black wins") : "Draw" ,
//                          static_cast<unsigned>(0.26f * cell) , {centerX , panel.position.y + 1.2f * cell} , sf::Color(200 , 200 , 200));
//     }
//     else
//     {
//         // no font: show the winning king (or both kings for a draw)
//         const float y = panel.position.y + 0.95f * cell;
//         if(checkmate)
//         {
//             gameWindow.draw(makePieceSprite(PieceType::King , winner , {centerX , y} , 1.2f));
//         }
//         else
//         {
//             gameWindow.draw(makePieceSprite(PieceType::King , PieceColor::White , {centerX - 0.7f * cell , y} , 1.1f));
//             gameWindow.draw(makePieceSprite(PieceType::King , PieceColor::Black , {centerX + 0.7f * cell , y} , 1.1f));
//         }
//     }

//     for(int i = 0; i < 2; ++i)
//     {
//         const sf::FloatRect rect = gameOverButtonRect(i);
//         const bool hovered = rect.contains(mouse);
//         const sf::Vector2f center = rect.position + rect.size / 2.f;

//         sf::RectangleShape button(rect.size);
//         button.setPosition(rect.position);
//         if(i == 0) button.setFillColor(hovered ? sf::Color(140 , 175 , 100) : sf::Color(118 , 149 , 86));
//         else       button.setFillColor(hovered ? sf::Color(112 , 110 , 106) : sf::Color(86 , 84 , 80));
//         gameWindow.draw(button);

//         if(m_hasFont)
//         {
//             drawCenteredText(gameWindow , i == 0 ? "Play Again" : "Close" , static_cast<unsigned>(0.3f * cell) , center , sf::Color::White);
//         }
//         else if(i == 0)
//         {
//             sf::ConvexShape triangle(3);                       // "play" icon
//             triangle.setPoint(0 , {-0.14f * cell , -0.2f * cell});
//             triangle.setPoint(1 , {-0.14f * cell ,  0.2f * cell});
//             triangle.setPoint(2 , { 0.22f * cell ,  0.f});
//             triangle.setPosition(center);
//             triangle.setFillColor(sf::Color::White);
//             gameWindow.draw(triangle);
//         }
//         else
//         {
//             for(float angle : {45.f , -45.f})                  // "x" icon
//             {
//                 sf::RectangleShape bar({0.45f * cell , 0.08f * cell});
//                 bar.setOrigin(bar.getSize() / 2.f);
//                 bar.setPosition(center);
//                 bar.setRotation(sf::degrees(angle));
//                 bar.setFillColor(sf::Color::White);
//                 gameWindow.draw(bar);
//             }
//         }
//     }
// }

// // =====================================================================
// //  Mouse cursor: hand over your own pieces / buttons, fist while dragging
// // =====================================================================
// void Chess::updateCursor(sf::RenderWindow& gameWindow , sf::Vector2f mouse)
// {
//     CursorKind wanted = CursorKind::Arrow;

//     if(m_draggedPiece != NoEntity)
//     {
//         wanted = CursorKind::Fist;
//     }
//     else if(m_pendingPromotion)
//     {
//         for(int i = 0; i < 4; ++i)
//             if(promotionOptionRect(i).contains(mouse)) wanted = CursorKind::Hand;
//     }
//     else if(m_showGameOverPanel)
//     {
//         if(gameOverButtonRect(0).contains(mouse) || gameOverButtonRect(1).contains(mouse))
//             wanted = CursorKind::Hand;
//     }
//     else if(m_status == GameStatus::Playing)
//     {
//         const Cell cell = m_getBoardPositionOfWorldPos(mouse);
//         if(cell.isValid())
//         {
//             const Square& square = current().squares[cell.row][cell.col];
//             if(square.id != NoEntity && square.color == current().turn)
//                 wanted = CursorKind::Hand;
//         }
//     }

//     if(wanted == m_cursorKind) return;
//     m_cursorKind = wanted;

//     CursorSet& set = cursors();
//     switch(wanted)
//     {
//         case CursorKind::Arrow: if(set.arrow) gameWindow.setMouseCursor(*set.arrow); break;
//         case CursorKind::Hand:  if(set.hand)  gameWindow.setMouseCursor(*set.hand);  break;
//         case CursorKind::Fist:  if(set.fist)  gameWindow.setMouseCursor(*set.fist);  break;
//     }
// }