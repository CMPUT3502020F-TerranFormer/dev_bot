//
// Created by Carter Sabadash on 2020-10-24
//
#include "TF_Bot.hpp"

TF_Bot::TF_Bot() {
    TSqueue<BasicCommand>* a_queue = new TSqueue<BasicCommand>;
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

    resource->gameStart(Observation()->GetGameInfo());
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