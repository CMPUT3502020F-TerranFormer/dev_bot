//
// Created by Carter Sabadash on 2020-10-24
//
#include "TF_Bot.hpp"

TF_Bot::TF_Bot() {
}

TF_Bot::~TF_Bot() {

}

void TF_Bot::OnGameStart() {
    // also need to get map name, enemy race -> create & store in TF_Bot variables
    baseManager = new BaseManager(&resource_queue, Observation());
    resourceGameStart();
}

void TF_Bot::OnGameEnd() {

}

void TF_Bot::OnStep() {
    resourceStep();

}

void TF_Bot::OnUnitDestroyed(const Unit* unit) {
    baseManager->deleteUnit(unit);
}

void TF_Bot::OnUnitCreated(const Unit* unit) {
    // template
    baseManager->addUnit(unit);
    resource_units.push_back(TF_unit(unit->unit_type, unit->tag));
}

void TF_Bot::OnUnitIdle(const Unit* unit) {
    resourceIdle(unit);
}

void TF_Bot::OnUpgradeCompleted(UpgradeID uid) {

}

void TF_Bot::OnBuildingConstructionComplete(const Unit* unit) {

}

void TF_Bot::OnUnitEnterVision(const Unit* unit) {

}