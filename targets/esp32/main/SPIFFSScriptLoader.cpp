#include <SPIFFSScriptLoader.h>

SPIFFSScriptLoader::SPIFFSScriptLoader()
{
}

void SPIFFSScriptLoader::loadScript(std::string scriptName, std::shared_ptr<Berry> berry)
{
    auto scriptContent = loadFile("/spiffs/" + scriptName + ".be");
    berry->execute_string(scriptContent);
}

std::string SPIFFSScriptLoader::loadFile(std::string fileName)
{
    std::ifstream indexFile(fileName);
    std::string indexContent((std::istreambuf_iterator<char>(indexFile)),
                             std::istreambuf_iterator<char>());

    return indexContent;
}
