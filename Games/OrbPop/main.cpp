#include"OrbPop.hpp"

int main()
{
    Game test("OrbPop");
    test.addSceneMaker<OrbPop>("OrbPop");
    Engine engine(test);
    engine.run();
    return 0;
}