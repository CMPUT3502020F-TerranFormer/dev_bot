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
    std::thread resource_thread([this]() { resource->step(); });
    std::thread attack_thread([this]() { attack->step(); });
    std::thread defence_thread([this]() { defence->step(); });
    std::thread scout_thread([this]() { scout->step(); });

    resource_thread.join();
    defence_thread.join();
    attack_thread.join();
    scout_thread.join();


    // resource->step();
    // attack->step();
    // defence->step();
    // scout->step();
}

void TF_Bot::OnUnitDestroyed(const Unit* unit) {
    std::thread resource_thread([this, unit]() { resource->unitDestroyed(unit); });
    std::thread attack_thread([this, unit]() { attack->unitDestroyed(unit); });
    std::thread defence_thread([this, unit]() { defence->unitDestroyed(unit); });
    std::thread scout_thread([this, unit]() { scout->unitDestroyed(unit); });

    resource_thread.join();
    defence_thread.join();
    attack_thread.join();
    scout_thread.join();

    // resource->unitDestroyed(unit);
    // defence->unitDestroyed(unit);
    // attack->unitDestroyed(unit);
    // scout->unitDestroyed(unit);
}

void TF_Bot::OnUnitCreated(const Unit* unit) {
    // resource will call addUnit() to the appropriate agent 
    std::thread resource_thread([this, unit]() { resource->unitCreated(unit); });
    // this is where most of the logic for when to build what is
    std::thread attack_thread([this, unit]() { attack->unitCreated(unit); });
    std::thread defence_thread([this, unit]() { defence->unitCreated(unit); });
    std::thread scout_thread([this, unit]() { scout->unitCreated(unit); });

    resource_thread.join();
    defence_thread.join();
    attack_thread.join();
    scout_thread.join();

    // resource->unitCreated(unit); // resource will call addUnit() to the appropriate agent
    // attack->unitCreated(unit); // this is where most of the logic for when to build what is
    // defence->unitCreated(unit);
    // scout->unitCreated(unit);
}

void TF_Bot::OnUnitIdle(const Unit* unit) {
    std::thread resource_thread([this, unit]() { resource->unitIdle(unit); });
    std::thread attack_thread([this, unit]() { attack->unitIdle(unit); });
    std::thread defence_thread([this, unit]() { defence->unitIdle(unit); });
    std::thread scout_thread([this, unit]() { scout->unitIdle(unit); });

    resource_thread.join();
    defence_thread.join();
    attack_thread.join();
    scout_thread.join();

    // resource->unitIdle(unit);
    // attack->unitIdle(unit);
    // defence->unitIdle(unit);
    // scout->unitIdle(unit);
}

void TF_Bot::OnUpgradeCompleted(UpgradeID uid) {
    std::thread resource_thread([this, uid]() { resource->upgradeCompleted(uid); });
    std::thread attack_thread([this, uid]() { attack->upgradeCompleted(uid); });
    std::thread defence_thread([this, uid]() { defence->upgradeCompleted(uid); });
    std::thread scout_thread([this, uid]() { scout->upgradeCompleted(uid); });

    resource_thread.join();
    defence_thread.join();
    attack_thread.join();
    scout_thread.join();

    // resource->upgradeCompleted(uid);
    // attack->upgradeCompleted(uid);
    // defence->upgradeCompleted(uid);
    // scout->upgradeCompleted(uid);
}

void TF_Bot::OnBuildingConstructionComplete(const Unit* unit) {
    std::thread resource_thread([this, unit]() { resource->buildingConstructionComplete(unit); });
    std::thread attack_thread([this, unit]() { attack->buildingConstructionComplete(unit); });
    std::thread defence_thread([this, unit]() { defence->buildingConstructionComplete(unit); });
    std::thread scout_thread([this, unit]() { scout->buildingConstructionComplete(unit); });

    resource_thread.join();
    defence_thread.join();
    attack_thread.join();
    scout_thread.join();

    // resource->buildingConstructionComplete(unit);
    // attack->buildingConstructionComplete(unit);
    // defence->buildingConstructionComplete(unit);
    // scout->buildingConstructionComplete(unit);
}

void TF_Bot::OnUnitEnterVision(const Unit* unit) {
    std::thread resource_thread([this, unit]() { resource->unitEnterVision(unit); });
    std::thread attack_thread([this, unit]() { attack->unitEnterVision(unit); });
    std::thread defence_thread([this, unit]() { defence->unitEnterVision(unit); });
    std::thread scout_thread([this, unit]() { scout->unitEnterVision(unit); });

    resource_thread.join();
    defence_thread.join();
    attack_thread.join();
    scout_thread.join();

    // resource->unitEnterVision(unit);
    // defence->unitEnterVision(unit);
    // attack->unitEnterVision(unit);
    // scout->unitEnterVision(unit);
}
