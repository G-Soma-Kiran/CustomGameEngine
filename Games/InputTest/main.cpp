#include"InputTest.hpp"

int main()
{
    Game test("InputTest");
    test.addSceneMaker<InputTestScene>("InputTest");
    Engine engine(test);
    engine.run();
    return 0;
}