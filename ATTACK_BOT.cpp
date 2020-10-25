//
// Created by Carter Sabadash on 2020-10-24
//
#include "ATTACK_BOT.hpp"

ATTACK_BOT::ATTACK_BOT(TSqueue<BasicCommand>* a_queue) 
    : TF_Agent(a_queue)
{
    defence = nullptr;
    scout = nullptr;
    resource = nullptr;
}

ATTACK_BOT::~ATTACK_BOT() {

}

void ATTACK_BOT::step(const sc2::GameInfo& gi) {

}

void ATTACK_BOT::addTask(Task t) {

}

void ATTACK_BOT::addUnit(TF_unit u) {

}

void ATTACK_BOT::buildingConstructionComplete(const sc2::Unit* u) {

}

void ATTACK_BOT::unitDestroyed(const sc2::Unit* u) {
    // get UnitTag, then compare with TF_unit.tag
    for (auto it = units.cbegin(); it != units.cend(); ++it)
    {
        if (it->tag == u->tag)
        {
            it = units.erase(it);
            return;
        }
    }
}

void ATTACK_BOT::unitCreated(const sc2::Unit* u) {

}

void ATTACK_BOT::unitEnterVision(const sc2::Unit* u) {

}

void ATTACK_BOT::unitIdle(const sc2::Unit* u) {

}

void ATTACK_BOT::upgradeCompleted(sc2::UpgradeID uid) {

}

void ATTACK_BOT::setAgents(const TF_Agent* defenceb, const TF_Agent* resourceb, const TF_Agent* scoutb) {
    this->defence = defenceb;
    this->resource = resourceb;
    this->scout = scoutb;
}