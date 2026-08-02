#include"CActionManager.hpp"
#include"CAssetManager.hpp"
#include"CComponents.hpp"
#include"CEntityManager.hpp"
#include"CGameEngine.hpp"
#include"CScene.hpp"
#include"CVector.hpp"
#include"CCustomScene.hpp"

int main()
{
    Game test;
    test.addSceneMaker<HomeScreen>("Default");
    // test.addGameAsset(Asset("Button" , "Texture" , "Assets/Textures/ButtonTest(20by12).png") , {"Default"});
    // test.addGameAsset(Asset("Background" , "Texture" , "Assets/Textures/PvZ(MainMenu)ReCanvas.png") , {"Default"});
    // test.addGameAsset(Asset("Pea" , "Texture" , "Assets/Textures/peashooter2.png") , {"Default"});
    Engine engine(test);
    engine.run();
    return 0;
}