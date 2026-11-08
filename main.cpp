#include"Games/CustomScene/CCustomScene.hpp"
#include"Games/InputTest/InputTest.hpp"
#include"Games/Chess/Chess.hpp"
#include"Games/OrbPop/OrbPop.hpp"
int main()
{
    Game test("Default");
    test.addSceneMaker<HomeScreen>("Default");
    test.addSceneMaker<InputTestScene>("InputTest");
    test.addSceneMaker<Chess>("Chess");
    test.addSceneMaker<OrbPop>("OrbPop");

    test.addGameAsset(Asset("BlackPawn" , "Texture" , "Games/Chess/Assets/Textures/BlackPawn.png") , {"Chess"});
    test.addGameAsset(Asset("BlackKnight" , "Texture" , "Games/Chess/Assets/Textures/BlackKnight.png") , {"Chess"});
    test.addGameAsset(Asset("BlackBishop" , "Texture" , "Games/Chess/Assets/Textures/BlackBishop.png") , {"Chess"});
    test.addGameAsset(Asset("BlackRook" , "Texture" , "Games/Chess/Assets/Textures/BlackRook.png") , {"Chess"});
    test.addGameAsset(Asset("BlackQueen" , "Texture" , "Games/Chess/Assets/Textures/BlackQueen.png") , {"Chess"});
    test.addGameAsset(Asset("BlackKing" , "Texture" , "Games/Chess/Assets/Textures/BlackKing.png") , {"Chess"});
    
    test.addGameAsset(Asset("WhitePawn" , "Texture" , "Games/Chess/Assets/Textures/WhitePawn.png") , {"Chess"});
    test.addGameAsset(Asset("WhiteKnight" , "Texture" , "Games/Chess/Assets/Textures/WhiteKnight.png") , {"Chess"});
    test.addGameAsset(Asset("WhiteBishop" , "Texture" , "Games/Chess/Assets/Textures/WhiteBishop.png") , {"Chess"});
    test.addGameAsset(Asset("WhiteRook" , "Texture" , "Games/Chess/Assets/Textures/WhiteRook.png") , {"Chess"});
    test.addGameAsset(Asset("WhiteQueen" , "Texture" , "Games/Chess/Assets/Textures/WhiteQueen.png") , {"Chess"});
    test.addGameAsset(Asset("WhiteKing" , "Texture" , "Games/Chess/Assets/Textures/WhiteKing.png") , {"Chess"});
    Engine engine(test);
    engine.run();
    return 0;
}