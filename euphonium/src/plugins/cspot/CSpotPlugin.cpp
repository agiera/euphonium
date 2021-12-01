#include "CSpotPlugin.h"

#include <thread>

std::shared_ptr<ConfigJSON> configMan;

CSpotPlugin::CSpotPlugin() : bell::Task("cspot", 4 * 1024, 1)
{
    auto file = std::make_shared<CliFile>();
    configMan = std::make_shared<ConfigJSON>("test.json", file);
    name = "cspot";
}

void CSpotPlugin::loadScript(std::shared_ptr<ScriptLoader> scriptLoader)
{
    scriptLoader->loadScript("cspot_plugin", berry);
}

void CSpotPlugin::setupBindings()
{
    berry->export_this("cspotConfigUpdated", this, &CSpotPlugin::configurationUpdated);
}

void CSpotPlugin::configurationUpdated()
{
    std::cout << "CSpotPlugin::configurationUpdated()" << std::endl;
    shutdown();
    //startAudioThread();
}

void CSpotPlugin::shutdown() {
    this->isRunning = false;
    std::scoped_lock(this->runningMutex);
    spircController->stopPlayer();

    mercuryManager->stop();
    BELL_SLEEP_MS(50);
    spircController.reset();
    mercuryManager.reset();
    BELL_SLEEP_MS(50);
    status = ModuleStatus::SHUTDOWN;
}

void CSpotPlugin::runTask()
{
    status = ModuleStatus::RUNNING;
    std::scoped_lock lock(runningMutex);
    this->isRunning = true;

    EUPH_LOG(info, "cspot", "Starting CSpot");
    this->audioBuffer->shutdownExcept(name);

    auto session = std::make_unique<Session>();
    session->connectWithRandomAp();

    EUPH_LOG(info, "cspot", "Got session");
    auto token = session->authenticate(authBlob);

    EUPH_LOG(info, "cspot", "Auth");
    if (token.size() > 0)
    {
        // @TODO Actually store this token somewhere
        mercuryManager = std::make_shared<MercuryManager>(std::move(session));
        mercuryManager->startTask();
        auto audioSink = std::make_shared<FakeAudioSink>(this->audioBuffer, this->luaEventBus);
        spircController = std::make_shared<SpircController>(mercuryManager, authBlob->username, audioSink);
        spircController->setTrackChangedCallback([this](TrackInfo &track)
                                                 {
                                                     auto sourceName = std::string("cspot");
                                                     auto event = std::make_unique<SongChangedEvent>(track.name, track.album, track.artist, sourceName, track.imageUrl);
                                                     EUPH_LOG(info, "cspot", "Song name changed");
                                                     this->luaEventBus->postEvent(std::move(event));
                                                 });

        mercuryManager->reconnectedCallback = [this]()
        {
            return this->spircController->subscribe();
        };

        while (this->isRunning)
        {
            mercuryManager->updateQueue();
        }
    }
}

void CSpotPlugin::mapConfig()
{
    configMan->volume = 255;
    configMan->deviceName = std::any_cast<std::string>(config["receiverName"]);
    std::string bitrateString = std::any_cast<std::string>(config["audioBitrate"]);
    switch (std::stoi(bitrateString))
    {
    case 160:
        configMan->format = AudioFormat::OGG_VORBIS_160;
        break;
    case 96:
        configMan->format = AudioFormat::OGG_VORBIS_96;
        break;
    default:
        configMan->format = AudioFormat::OGG_VORBIS_320;
        break;
    }
}

void CSpotPlugin::startAudioThread()
{
    mapConfig();
    if (this->authenticator == nullptr)
    {
        createPlayerCallback = [this](std::shared_ptr<LoginBlob> blob)
        {
            if (this->isRunning)
            {
                configurationUpdated();
            }

            this->authBlob = blob;
            EUPH_LOG(info, "cspot", "Authenticated");
            startTask();
            EUPH_LOG(info, "cspot", "Detached");
        };

        authenticator = std::make_shared<ZeroconfAuthenticator>(createPlayerCallback, mainServer);
        authenticator->registerHandlers();
    }
}