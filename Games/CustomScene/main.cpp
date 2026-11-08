#include"CCustomScene.hpp"

int main()
{
    Game test("CustomScene");
    test.addSceneMaker<HomeScreen>("CustomScene");
    Engine engine(test);
    engine.run();
    return 0;
}