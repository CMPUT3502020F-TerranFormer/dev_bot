//
// Created by Carter Sabadash on 2020-10-24
//
#include "TF_Bot.hpp"
#include "ATTACK_BOT.hpp"
#include "DEFENCE_BOT.hpp"
#include "RESOURCE_BOT.hpp"
#include "SCOUT_BOT.hpp"

TF_Bot::TF_Bot() {
    attack = new ATTACK_BOT(this);
    defence = new DEFENCE_BOT(this);
    scout = new SCOUT_BOT(this);
    resource = new RESOURCE_BOT(this);
}

TF_Bot::~TF_Bot() {
    // free mem
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

    resource->gameStart();
    attack->init();
    scout->init();
    defence->init();
}

void TF_Bot::OnGameEnd() {

}

void TF_Bot::OnStep() {
    resource->step();
    attack->step();
    defence->step();
    scout->step();
}

void TF_Bot::OnUnitDestroyed(const Unit* unit) {
    resource->unitDestroyed(unit);
    defence->unitDestroyed(unit);
    attack->unitDestroyed(unit);
    scout->unitDestroyed(unit);
}

void TF_Bot::OnUnitCreated(const Unit* unit) {
    resource->unitCreated(unit); // resource will call addUnit() to the appropriate agent
    attack->unitCreated(unit); // this is where most of the logic for when to build what is
    defence->unitCreated(unit);
    scout->unitCreated(unit);
}

void TF_Bot::OnUnitIdle(const Unit* unit) {
    resource->unitIdle(unit);
    attack->unitIdle(unit);
    defence->unitIdle(unit);
    scout->unitIdle(unit);
}

void TF_Bot::OnUpgradeCompleted(UpgradeID uid) {
    resource->upgradeCompleted(uid);
    attack->upgradeCompleted(uid);
    defence->upgradeCompleted(uid);
    scout->upgradeCompleted(uid);
}

void TF_Bot::OnBuildingConstructionComplete(const Unit* unit) {
    resource->buildingConstructionComplete(unit);
    attack->buildingConstructionComplete(unit);
    defence->buildingConstructionComplete(unit);
    scout->buildingConstructionComplete(unit);
}

void TF_Bot::OnUnitEnterVision(const Unit* unit) {
    resource->unitEnterVision(unit);
    defence->unitEnterVision(unit);
    attack->unitEnterVision(unit);
    scout->unitEnterVision(unit);
}
