//
// Created by Carter Sabadash on 2020-10-24
//
#include "RESOURCE_BOT.hpp"

RESOURCE_BOT::RESOURCE_BOT(TSqueue<BasicCommand>* a_queue)
    : TF_Agent(a_queue)
{
    defence = nullptr;
    attack = nullptr;
    scout = nullptr;
}

RESOURCE_BOT::~RESOURCE_BOT() {

}

void RESOURCE_BOT::gameStart(const sc2::GameInfo &gi) {

}

void RESOURCE_BOT::step(const sc2::GameInfo& gi) {

}

void RESOURCE_BOT::addTask(Task t) {

}

void RESOURCE_BOT::addUnit(TF_unit u) {

}

void RESOURCE_BOT::buildingConstructionComplete(const sc2::Unit* u) {

}

void RESOURCE_BOT::unitDestroyed(const sc2::Unit* u) {
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

void RESOURCE_BOT::unitCreated(const sc2::Unit* u) {

}

void RESOURCE_BOT::unitEnterVision(const sc2::Unit* u) {

}

void RESOURCE_BOT::unitIdle(const sc2::Unit* u) {

}

void RESOURCE_BOT::upgradeCompleted(sc2::UpgradeID uid) {

}

void RESOURCE_BOT::setAgents(const TF_Agent* defenceb, const TF_Agent* attackb, const TF_Agent* scoutb) {
    this->defence = defenceb;
    this->attack = attackb;
    this->scout = scoutb;
}