#include <sc2api/sc2_api.h>
#include <iostream>

#include "TF_Bot/ATTACK_BOT.hpp"
#include "TF_Bot/DEFENCE_BOT.hpp"
#include "TF_Bot/RESOURCE_BOT.hpp"
#include "TF_Bot/SCOUT_BOT.hpp"
#include "TF_Bot/Task.hpp"

using namespace sc2;

class TF_Bot : public Agent 
{
public:
    virtual void OnGameStart() final {
        attack.setAgents(&defence, &resource, &scout);
        defence.setAgents(&attack, &resource, &scout);
        resource.setAgents(&defence, &attack, &scout);
        scout.setAgents(&defence, &attack, &resource);

        resource.gameStart(Observation()->GetGameInfo);
    }

    virtual void OnGameEnd() final {

    }

    virtual void OnStep() final {
        // get game info
        const GameInfo &game_info = Observation()->GetGameInfo();
        defence.step(game_info);
        attack.step(game_info);
        resource.step(game_info);
        scout.step(game_info);
    }

    virtual void OnUnitIdle(const Unit *unit) final {
        // agent.unitIdle(unit);
    }

    virtual void OnUpgradeCompleted(UpgradeID uid) final {
        // agent.upgradeCompleted(uid);
    }

    virtual void OnBuildingConstructionComplete(const Unit* unit) final {
        // agent.buildingConstructionComplete(unit);
    }

private:
    ATTACK_BOT attack;
    DEFENCE_BOT defence;
    RESOURCE_BOT resource;
    SCOUT_BOT scout;
};

int main(int argc, char *argv[]) {
    Coordinator coordinator;
    coordinator.LoadSettings(argc, argv);

    TF_Bot bot;
    coordinator.SetParticipants({
                CreateParticipant(Race::Terran, &bot),
                CreateComputer(Race::Zerg)
            });

    coordinator.LaunchStarcraft();
    coordinator.StartGame(sc2::kMapBelShirVestigeLE);

    while (coordinator.Update()) {
    }
    return 0;
}
