//
// Created by Carter Sabadash on 2020-10-24
//
#include "ATTACK_BOT.hpp"

ATTACK_BOT::ATTACK_BOT(TF_Bot* bot)
    : TF_Agent(bot)
{
    defence = nullptr;
    scout = nullptr;
    resource = nullptr;
}

ATTACK_BOT::~ATTACK_BOT() {

}

void ATTACK_BOT::step() {

}

void ATTACK_BOT::addTask(Task t) {

}

void ATTACK_BOT::addUnit(TF_unit u) {

}

void ATTACK_BOT::buildingConstructionComplete(const sc2::Unit* u) {

}

void ATTACK_BOT::unitDestroyed(const sc2::Unit* u) {

}

void ATTACK_BOT::unitCreated(const sc2::Unit* u) {

}

void ATTACK_BOT::unitEnterVision(const sc2::Unit* u) {

}

void ATTACK_BOT::unitIdle(const sc2::Unit* u) {

}

void ATTACK_BOT::upgradeCompleted(sc2::UpgradeID uid) {

}

void ATTACK_BOT::setAgents(TF_Agent* defenceb, TF_Agent* resourceb, TF_Agent* scoutb) {
    this->defence = defenceb;
    this->resource = resourceb;
    this->scout = scoutb;
}