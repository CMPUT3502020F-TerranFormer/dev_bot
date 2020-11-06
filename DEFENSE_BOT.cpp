//
// Created by Carter Sabadash on 2020-10-24
//
#include "DEFENCE_BOT.hpp"

DEFENCE_BOT::DEFENCE_BOT(TF_Bot* bot)
    : TF_Agent(bot)
{
    scout = nullptr;
    attack = nullptr;
    resource = nullptr;
}

DEFENCE_BOT::~DEFENCE_BOT() {

}

void DEFENCE_BOT::step() {

}

void DEFENCE_BOT::addTask(Task t) {

}

void DEFENCE_BOT::addUnit(TF_unit u) {

}

void DEFENCE_BOT::buildingConstructionComplete(const sc2::Unit* u) {

}

void DEFENCE_BOT::unitDestroyed(const sc2::Unit* u) {

}

void DEFENCE_BOT::unitCreated(const sc2::Unit* u) {

}

void DEFENCE_BOT::unitEnterVision(const sc2::Unit* u) {

}

void DEFENCE_BOT::unitIdle(const sc2::Unit* u) {

}

void DEFENCE_BOT::upgradeCompleted(sc2::UpgradeID uid) {

}

void DEFENCE_BOT::setAgents(TF_Agent* attackb, TF_Agent* resourceb, TF_Agent* scoutb) {
    this->attack = attackb;
    this->resource = resourceb;
    this->scout = scoutb;
}