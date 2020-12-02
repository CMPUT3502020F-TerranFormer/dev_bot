//
// Created by Carter Sabadash on 2020-10-24
//
#include "ATTACK_BOT.hpp"
#include <iostream>

using namespace sc2;

ATTACK_BOT::ATTACK_BOT(TF_Bot *bot)
        : TF_Agent(bot) {
    defence = nullptr;
    scout = nullptr;
    resource = nullptr;
}

ATTACK_BOT::~ATTACK_BOT() {

}

void ATTACK_BOT::init() {
    buildingPlacementManager = new BuildingPlacementManager(observation, query);
    troopManager = new TroopManager(&task_queue, observation, scout);
}

void ATTACK_BOT::step() {
    // perform actions in the task queue
    while (!task_queue.empty()) {
        Task t = task_queue.pop();
        // push resource tasks from TroopManager into resources
        // and perform other tasks as necessary (sometimes re-using code from resources)
        switch (t.action) {
            case BUILD: {
                resource->addTask(t);
                break;
            }
            case TRAIN: {
                resource->addTask(t);
                break;
            }
            case ATTACK: {
                if (t.unit->unit_type == UNIT_TYPEID::TERRAN_SIEGETANKSIEGED) {
                    action->UnitCommand(t.unit, ABILITY_ID::MORPH_UNSIEGE);
                    action->UnitCommand(t.unit, ABILITY_ID::MOVE_MOVE, t.position, true);
                    action->UnitCommand(t.unit, ABILITY_ID::MORPH_SIEGEMODE, true);
                    action->UnitCommand(t.unit, ABILITY_ID::ATTACK_ATTACK, t.position, true);
                    action->SendActions();
                    return;
                }
                action->UnitCommand(t.unit, t.ability_id, t.position, true);
                attack_units.push_back(t.unit);
                if (attack_units.size() >= 20)
                {
                    action->SendActions();
                }
                break;
            }
            case REPAIR: {
                resource->addTask(t);
                break;
            }
            case UPGRADE: {
                resource->addTask(t);
                break;
            }
            case MOVE: {
                action->UnitCommand(observation->GetUnit(t.target), t.ability_id, t.position);
                break;
            }
            case TRANSFER: {
                TF_unit unit = TF_unit(observation->GetUnit(t.self)->unit_type.ToType(), t.self);

                // add to correct agent
                switch (t.source) {
                    case DEFENCE_AGENT:
                        defence->addUnit(unit);
                        break;
                    case RESOURCE_AGENT:
                        resource->addUnit(unit);
                        break;
                    case SCOUT_AGENT:
                        scout->addUnit(unit);
                        break;
                    default:
                        std::cerr << "TRANSFER to invalid agent requested!" << std::endl;
                        return;
                }

                // and remove from units
                for (auto it = units.cbegin(); it != units.cend(); ++it) {
                    if (*it == unit) {
                        units.erase(it);
                        break;
                    }
                }
                break;
            }
            default: {
                std::cerr << "RESOURCE Unrecognized Task: " << t.source << " " << t.action << std::endl;
            }
        }
    }
}

void ATTACK_BOT::addTask(Task t) {
    task_queue.push(t);
}

void ATTACK_BOT::addUnit(TF_unit u) {
    units.push_back(u);
}

void ATTACK_BOT::buildingConstructionComplete(const sc2::Unit *u) {
    switch (u->unit_type.ToType()) {
        case UNIT_TYPEID::TERRAN_BARRACKS:
        case UNIT_TYPEID::TERRAN_FACTORY:
        case UNIT_TYPEID::TERRAN_STARPORT:
            buildAddOn(u);
            break;

        default:
            break;
    }
}

void ATTACK_BOT::unitDestroyed(const sc2::Unit *u) {
    for (auto it = units.cbegin(); it != units.cend(); ++it) {
        if (it->tag == u->tag) {
            units.erase(it);
            return;
        }
    }
}

void ATTACK_BOT::unitCreated(const sc2::Unit *u) {
    // this is  where we want to check for building pre-requisites and try to build them.

    // for now, only allow this many barracks/factories/starports -> should have more complex conditions
    int command_count = observation->GetUnits(Unit::Alliance::Self, IsCommandCenter()).size();
    int barracks_count = observation->GetUnits(Unit::Alliance::Self, IsBarracks()).size();
    int factory_count = observation->GetUnits(Unit::Alliance::Self, IsFactory()).size();
    int starport_count = observation->GetUnits(Unit::Alliance::Self, IsStarport()).size();
    if (barracks_count < 1 + (2 * command_count)) {

        buildBarracks();
    }

    if (factory_count < 2 * command_count) {
        buildFactory();
    }

    if (starport_count < 1 * command_count) {
        buildStarport();
    }
}

void ATTACK_BOT::unitEnterVision(const sc2::Unit *u) {
}

void ATTACK_BOT::unitIdle(const sc2::Unit *u) {
    troopManager->unitIdle(u);
}

void ATTACK_BOT::upgradeCompleted(sc2::UpgradeID uid) {

}

void ATTACK_BOT::setAgents(TF_Agent *defenceb, TF_Agent *resourceb, TF_Agent *scoutb) {
    this->defence = defenceb;
    this->resource = resourceb;
    this->scout = scoutb;
}

void ATTACK_BOT::buildBarracks() {
    // Prereqs of building barrack: Supply Depot
    if (troopManager->CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
        return;
    }
    else {
        resource->addTask(
            Task(BUILD, 
                ATTACK_AGENT, 
                5, // we want to prioritize building units over buildings 
                UNIT_TYPEID::TERRAN_BARRACKS,
                ABILITY_ID::BUILD_BARRACKS, 
                buildingPlacementManager->getNextBarracksLocation()));
    }
}

void ATTACK_BOT::buildFactory() {
    //  Prereqs of building factory: Barracks
    if (troopManager->CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) < 1) {
        buildBarracks();
    }
    else {
        resource->addTask(
            Task(BUILD,
                ATTACK_AGENT,
                5,
                UNIT_TYPEID::TERRAN_FACTORY,
                ABILITY_ID::BUILD_FACTORY,
                buildingPlacementManager->getNextFactoryLocation())
        );
    }
}

void ATTACK_BOT::buildStarport() {
    // Prereqs of building starport: Factory
    if (troopManager->CountUnitType(UNIT_TYPEID::TERRAN_FACTORY) < 1) {
        buildFactory();
    }
    else {
        resource->addTask(
            Task(BUILD,
                ATTACK_AGENT,
                5,
                UNIT_TYPEID::TERRAN_STARPORT,
                ABILITY_ID::BUILD_STARPORT,
                buildingPlacementManager->getNextStarportLocation())
        );
    }
}

void ATTACK_BOT::buildAddOn(const Unit *u) {
    // proof of concept, build a reactor on each barracks (should not actually use priority 8)

    // TODO: For now all add-ons have been set to default, will need to implement a way to choose the add - on

    if (u->build_progress != 1) { return; }

    switch (u->unit_type.ToType()) {
        case UNIT_TYPEID::TERRAN_BARRACKS:
            resource->addTask(Task(TRAIN, ATTACK_AGENT, 6, UNIT_TYPEID::TERRAN_BARRACKS,
                                   ABILITY_ID::BUILD_TECHLAB_BARRACKS, u->tag));

        case UNIT_TYPEID::TERRAN_FACTORY:
            resource->addTask(
                    Task(TRAIN, ATTACK_AGENT, 7, UNIT_TYPEID::TERRAN_FACTORY, ABILITY_ID::BUILD_TECHLAB_FACTORY,
                         u->tag));

        case UNIT_TYPEID::TERRAN_STARPORT:
            resource->addTask(
                    Task(TRAIN, ATTACK_AGENT, 5, UNIT_TYPEID::TERRAN_STARPORT, ABILITY_ID::BUILD_TECHLAB_STARPORT,
                         u->tag));
    }
}

std::vector<Spotted_Enemy> ATTACK_BOT::last_seen_near(Point2D location, int radius, int since) {
    return scout->last_seen_near(location, radius, since);
}
