//
// Created by Carter Sabadash on 2020-10-24
//
#include "SCOUT_BOT.hpp"

SCOUT_BOT::SCOUT_BOT(TSqueue<BasicCommand>* a_queue)
    : TF_Agent(a_queue)
{
    defence = nullptr;
    attack = nullptr;
    resource = nullptr;
}

SCOUT_BOT::~SCOUT_BOT() {

}

void SCOUT_BOT::step(const sc2::GameInfo& gi) {

}

void SCOUT_BOT::addTask(Task t) {

}

void SCOUT_BOT::addUnit(TF_unit u) {

}

void SCOUT_BOT::buildingConstructionComplete(const sc2::Unit* u) {

}

void SCOUT_BOT::unitDestroyed(const sc2::Unit* u) {
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

void SCOUT_BOT::unitCreated(const sc2::Unit* u) {

}

void SCOUT_BOT::unitEnterVision(const sc2::Unit* u) {

}

void SCOUT_BOT::unitIdle(const sc2::Unit* u) {

}

void SCOUT_BOT::upgradeCompleted(sc2::UpgradeID uid) {

}

void SCOUT_BOT::setAgents(const TF_Agent* defenceb, const TF_Agent* attackb, const TF_Agent* resourceb) {
    this->defence = defenceb;
    this->attack = attackb;
    this->resource = resourceb;
}