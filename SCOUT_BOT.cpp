//
// Created by Carter Sabadash on 2020-10-24
//
#include "SCOUT_BOT.hpp"

SCOUT_BOT::SCOUT_BOT(TF_Bot* bot)
    : TF_Agent(bot)
{
    defence = nullptr;
    attack = nullptr;
    resource = nullptr;
}

SCOUT_BOT::~SCOUT_BOT() {

}

void SCOUT_BOT::step() {

}

void SCOUT_BOT::addTask(Task t) {

}

void SCOUT_BOT::addUnit(TF_unit u) {

}

void SCOUT_BOT::buildingConstructionComplete(const sc2::Unit* u) {

}

void SCOUT_BOT::unitDestroyed(const sc2::Unit* u) {

}

void SCOUT_BOT::unitCreated(const sc2::Unit* u) {

}

void SCOUT_BOT::unitEnterVision(const sc2::Unit* u) {

}

void SCOUT_BOT::unitIdle(const sc2::Unit* u) {

}

void SCOUT_BOT::upgradeCompleted(sc2::UpgradeID uid) {

}

void SCOUT_BOT::setAgents(TF_Agent* defenceb, TF_Agent* attackb, TF_Agent* resourceb) {
    this->defence = defenceb;
    this->attack = attackb;
    this->resource = resourceb;
}