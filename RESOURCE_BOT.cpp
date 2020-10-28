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


void RESOURCE_BOT::gameStart(const sc2::Units alliedUnits) {
    // get the initial scv's, command center -> add to units

    // ideally we want 66 scv's, 22 for each active command center
    // 3 for each vespene, 3 for each mineral field
    // maybe a few extra for building, etc -> 70?

    // 3 bases -> 3 bases maximally mining resources at any given time
    // the other will just sit?
    // so perhaps keep track of 3 "Bases" which track the command center,
    // scv's, mineral, vespene, and when resources run out (or almost) lift off or
    // build a new command center (if converting to fortress) until there are no spaces 
    // for bases left on map

    for (auto& p : alliedUnits) {
        units.push_back(TF_unit(p->unit_type.ToType(), p->tag));
    }
}

void RESOURCE_BOT::step(const sc2::GameInfo& gi) {
    // actions to perform during a game step
    // build supply depots, after the first 4 supply cost unit, try to keep 6 supply ahead
    // remember that command centers also produce supple (?), so if building one, we can wait a bit
    // complete as many tasks as possible with the given resources

    // check status of "Bases"


}

void RESOURCE_BOT::addTask(Task t) {
    // add a task to the task queue
    task_queue.push(t);
}

void RESOURCE_BOT::addUnit(TF_unit u) {
    // add a unit to units

}

void RESOURCE_BOT::buildingConstructionComplete(const sc2::Unit* u) {
    // notify agents when construction is complete (upon command center, possibly move scv's to it)

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

    // if unit was an scv make a new one
    // if it was a command center (with available resources) make a new one (this shouldn't happen)
    // this may not actually happen because at this point defense will likely be ordering units, and 
    // by this point should have moved the command center
}

void RESOURCE_BOT::unitCreated(const sc2::Unit* u) {
    // this is called when a unit is created --> add to appropriate agent
    // (if different agents can ask for the same unit -> keep track)
    // otherwise go by unit id

    // if scv, add to "Base"
    // if command center, add to "Base" which will choose what to do
    for (auto it = training.cbegin(); it != training.cend(); ++it) {
        if (it->second == u->unit_type) {
            TF_unit unit(u->unit_type, u->tag);
            switch (it->first) {
            case SCOUT_AGENT: scout->addUnit(unit);
                break;
            case DEFENCE_AGENT: defence->addUnit(unit);
                break;
            case ATTACK_AGENT: attack->addUnit(unit);
                break;
            case RESOURCE_AGENT: addUnit(unit);
                break;
            }
            return;
        }
    }

}

void RESOURCE_BOT::unitEnterVision(const sc2::Unit* u) {
    // we only use this to protect scv's from getting killed
    // perhaps have defense send a notification??

}

void RESOURCE_BOT::unitIdle(const sc2::Unit* u) {
    // determine the type of unit idling (only pay attention to resource units)
    // scv's should never idle
    // command centers should first build scv's
    // then we need to determine a policy (Mules, supply drops)
        // only scan on request

    if (u->unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER) {
        action_queue->push(BasicCommand(SELF, u, sc2::ABILITY_ID::TRAIN_SCV));
    }
}

void RESOURCE_BOT::upgradeCompleted(sc2::UpgradeID uid) {
    // as resources, we are specifically paying attention to ___ so ___

}

void RESOURCE_BOT::setAgents(const TF_Agent* defenceb, const TF_Agent* attackb, const TF_Agent* scoutb) {
    this->defence = defenceb;
    this->attack = attackb;
    this->scout = scoutb;
}