//
// Created by Carter Sabadash on 2020-10-24
//
#include "TF_Bot.hpp"

TF_Bot::TF_Bot() {
    a_queue = new TSqueue<BasicCommand>;
    attack = new ATTACK_BOT(a_queue);
    defence = new DEFENCE_BOT(a_queue);
    scout = new SCOUT_BOT(a_queue);
    resource = new RESOURCE_BOT(a_queue);
}

TF_Bot::~TF_Bot() {
    delete attack;
    delete defence;
    delete scout;
    delete resource;
}

void TF_Bot::OnGameStart() {
    attack->setAgents(defence, resource, scout);
    defence->setAgents(attack, resource, scout);
    resource->setAgents(defence, attack, scout);
    scout->setAgents(defence, attack, resource);

    resource->gameStart(Observation()->GetUnits());
}

void TF_Bot::OnGameEnd() {

}

void TF_Bot::OnStep() {
    // get game info
    const GameInfo& game_info = Observation()->GetGameInfo();
    defence->step(game_info);
    attack->step(game_info);
    resource->step(game_info);
    scout->step(game_info);


    while (!a_queue->empty()) {
        BasicCommand command = a_queue->dequeue();
        switch (command.t) {
        case CommandType::SELF: Actions()->UnitCommand(command.unit, command.aid);
            break;
        case CommandType::POINT: Actions()->UnitCommand(command.unit, command.aid, command.point);
            break;
        case CommandType::TARGET: Actions()->UnitCommand(command.unit, command.aid, command.target);
        }
    }
}

void TF_Bot::OnUnitDestroyed(const Unit* unit) {
    resource->unitDestroyed(unit);
}

void TF_Bot::OnUnitCreated(const Unit* unit) {
    resource->unitCreated(unit);
}

void TF_Bot::OnUnitIdle(const Unit* unit) {
    resource->unitIdle(unit);
}

void TF_Bot::OnUpgradeCompleted(UpgradeID uid) {

}

void TF_Bot::OnBuildingConstructionComplete(const Unit* unit) {

}

void TF_Bot::OnUnitEnterVision(const Unit* unit) {

}