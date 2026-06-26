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
    Engine engine(test);
    engine.run();

    return 0;
}

