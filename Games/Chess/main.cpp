#include"Chess.hpp"

int main()
{
    Game test("Chess");
    test.addSceneMaker<Chess>("Chess");
    test.addLocalGameAsset(Asset("BlackPawn" , "Texture" , "Games/Chess/Assets/Textures/BlackPawn.png") , {"Chess"});
    test.addLocalGameAsset(Asset("BlackKnight" , "Texture" , "Games/Chess/Assets/Textures/BlackKnight.png") , {"Chess"});
    test.addLocalGameAsset(Asset("BlackBishop" , "Texture" , "Games/Chess/Assets/Textures/BlackBishop.png") , {"Chess"});
    test.addLocalGameAsset(Asset("BlackRook" , "Texture" , "Games/Chess/Assets/Textures/BlackRook.png") , {"Chess"});
    test.addLocalGameAsset(Asset("BlackQueen" , "Texture" , "Games/Chess/Assets/Textures/BlackQueen.png") , {"Chess"});
    test.addLocalGameAsset(Asset("BlackKing" , "Texture" , "Games/Chess/Assets/Textures/BlackKing.png") , {"Chess"});
    
    test.addLocalGameAsset(Asset("WhitePawn" , "Texture" , "Games/Chess/Assets/Textures/WhitePawn.png") , {"Chess"});
    test.addLocalGameAsset(Asset("WhiteKnight" , "Texture" , "Games/Chess/Assets/Textures/WhiteKnight.png") , {"Chess"});
    test.addLocalGameAsset(Asset("WhiteBishop" , "Texture" , "Games/Chess/Assets/Textures/WhiteBishop.png") , {"Chess"});
    test.addLocalGameAsset(Asset("WhiteRook" , "Texture" , "Games/Chess/Assets/Textures/WhiteRook.png") , {"Chess"});
    test.addLocalGameAsset(Asset("WhiteQueen" , "Texture" , "Games/Chess/Assets/Textures/WhiteQueen.png") , {"Chess"});
    test.addLocalGameAsset(Asset("WhiteKing" , "Texture" , "Games/Chess/Assets/Textures/WhiteKing.png") , {"Chess"});
    Engine engine(test);
    engine.run();
    return 0;
}