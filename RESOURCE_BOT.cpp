//
// Created by Carter Sabadash on 2020-10-24
//
#include "RESOURCE_BOT.hpp"

RESOURCE_BOT::RESOURCE_BOT(const ObservationInterface* obs, const ActionInterface* act, const QueryInterface* query)
    : TF_Agent(obs, act, query)
{
    baseManager = new BaseManager(&task_queue, obs);
    defence = nullptr;
    attack = nullptr;
    scout = nullptr;
}

RESOURCE_BOT::~RESOURCE_BOT() {

}


#include <iostream>
void RESOURCE_BOT::gameStart() {
    std::cout << observation->GetGameInfo().map_name << std::endl;
}

void RESOURCE_BOT::step() {

}

void RESOURCE_BOT::addTask(Task t) {
    // add a task to the task queue
    task_queue.push(t);
}

void RESOURCE_BOT::addUnit(TF_unit u) {
    // add a unit to units
    // need to figure out how to organize up to 3 mining bases and their scv's
    units.push_back(u.tag);
}

void RESOURCE_BOT::buildingConstructionComplete(const sc2::Unit* u) {
    // notify agents when construction is complete (upon command center, possibly move scv's to it)
    // ???

}

void RESOURCE_BOT::unitDestroyed(const sc2::Unit* u) {

}

void RESOURCE_BOT::unitCreated(const sc2::Unit* u) {

}

void RESOURCE_BOT::unitEnterVision(const sc2::Unit* u) {


}

void RESOURCE_BOT::unitIdle(const sc2::Unit* u) {

}

void RESOURCE_BOT::upgradeCompleted(sc2::UpgradeID uid) {
    // as resources, we are specifically paying attention to ___ so ___

}

void RESOURCE_BOT::setAgents(TF_Agent* defenceb, TF_Agent* attackb, TF_Agent* scoutb) {
    this->defence = defenceb;
    this->attack = attackb;
    this->scout = scoutb;
}