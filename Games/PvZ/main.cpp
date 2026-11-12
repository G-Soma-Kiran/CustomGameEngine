#include"PvZ.hpp"

int main()
{
    Game test("PvZ");
    test.addSceneMaker<PvZ>("PvZ");

    test.addLocalGameAsset(Asset("Background" , "Texture" , "Games/PvZ/Assets/Textures/finalMainMenuWithNoZombie.png") , {"PvZ"});
    test.addLocalGameAsset(Asset("SkyMask" , "Texture" , "Games/PvZ/Assets/Textures/SkyMask.png") , {"PvZ"});
    test.addLocalGameAsset(Asset("Zombie" , "Texture" , "Games/PvZ/Assets/Textures/zombieExtract.png") , {"PvZ"});
    test.addLocalGameAsset(Asset("Peashooter" , "Texture" , "Games/PvZ/Assets/Textures/peashooterExtract.png") , {"PvZ"});

    Engine engine(test);
    engine.run();
    return 0;
}