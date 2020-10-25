//
// Created by Carter Sabadash on 2020-10-24
//
#include "DEFENCE_BOT.hpp"

DEFENCE_BOT::DEFENCE_BOT(TSqueue<BasicCommand>* a_queue)
    : TF_Agent(a_queue)
{
    scout = nullptr;
    attack = nullptr;
    resource = nullptr;
}

DEFENCE_BOT::~DEFENCE_BOT() {

}

void DEFENCE_BOT::step(const sc2::GameInfo& gi) {

}

void DEFENCE_BOT::addTask(Task t) {

}

void DEFENCE_BOT::addUnit(TF_unit u) {

}

void DEFENCE_BOT::buildingConstructionComplete(const sc2::Unit* u) {

}

void DEFENCE_BOT::unitDestroyed(const sc2::Unit* u) {
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

void DEFENCE_BOT::unitCreated(const sc2::Unit* u) {

}

void DEFENCE_BOT::unitEnterVision(const sc2::Unit* u) {

}

void DEFENCE_BOT::unitIdle(const sc2::Unit* u) {

}

void DEFENCE_BOT::upgradeCompleted(sc2::UpgradeID uid) {

}

void DEFENCE_BOT::setAgents(const TF_Agent* attackb, const TF_Agent* resourceb, const TF_Agent* scoutb) {
    this->attack = attackb;
    this->resource = resourceb;
    this->scout = scoutb;
}