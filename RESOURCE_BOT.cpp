//
// Created by Carter Sabadash on 2020-10-24
//
#include "RESOURCE_BOT.hpp"

RESOURCE_BOT::RESOURCE_BOT(TF_Bot* bot)
    : TF_Agent(bot)
{
    // baseManager must be initialized when the game starts
    defence = nullptr;
    attack = nullptr;
    scout = nullptr;
}

RESOURCE_BOT::~RESOURCE_BOT() {

}


void RESOURCE_BOT::gameStart() {
    // units are added via UnitCreated at GameStart()
    std::cout << observation->GetStartLocation().x << "|" << observation->GetStartLocation().y << std::endl;
    std::cout << "Map: " << observation->GetGameInfo().map_name << std::endl;

    baseManager = new BaseManager(&task_queue, bot->Observation(), units);
}

void RESOURCE_BOT::step() {
    /*MODIFY THIS AND HELPER METHODS TO CHECK IF THERE ARE ENOUGH RESOURCES TO BUILD FIRST
    AND WHEN BUILDING BUILDINGS; ONLY BUILD ONE OF EACH TYPE AT A TIME (REMOVE DUPLICATES FROM THE QUEUE)
    MAKE SURE THAT WE DON'T INTERRUPT ACTIONS AND MAKE SURE THIS WORKS WITH BARRACKS & OTHER UNIT TYPES
    **THIS WILL PROBABLY BE CHANGED WHEN THE TASK_QUEUE IS CHANGED AND MOVED INTO ONUNITIDLE()
    */

    // check if we need to build a supply depot
    uint32_t available_food = bot->Observation()->GetFoodCap() - bot->Observation()->GetFoodUsed();

    if (available_food <= baseManager->getSupplyFloat()) {
        buildSupplyDepot();
    }

    // we don't want to remove tasks from queue if there are not enough resources to perform them
    uint32_t available_minerals = bot->Observation()->GetMinerals();
    uint32_t available_vespene = bot->Observation()->GetVespene();

    bool task_success = true; // we also want to stop when we don't have resources to complete any other tasks
    while (!task_queue.empty() && task_success) {
        Task t = task_queue.top();
        switch (t.action) {
        case HARVEST: {
            bot->Actions()->UnitCommand(bot->Observation()->GetUnit(t.self), t.ability_id, bot->Observation()->GetUnit(t.target));
            task_queue.pop();
            break;
        }
        case BUILD: {
            // This will prevent multiple identical building from being produced at the same time
            //   unless specifically allowed. Identical units will be removed from the queue
            // check that we have enough resources to do build unit
            UnitTypeData ut = bot->Observation()->GetUnitTypeData()[(UnitTypeID)t.unit_typeid];
            if (ut.food_required > available_food
                || ut.mineral_cost > available_minerals
                || ut.vespene_cost > available_vespene)
            {
                task_success = false;
                break;
            }
            if (buildStructure(t.ability_id, t.position, t.target)) { // if building succeeded
                // update available resources
                available_food -= ut.food_required;
                available_minerals -= ut.mineral_cost;
                available_vespene -= ut.vespene_cost;
            }
            task_queue.pop();
            break;
        }
        case TRAIN: {
            // This does not prevent multiple units from being produced at the same time
            // get the producing unit
            if (t.target == -1) {
                // try to get a target unit of the required type, we'll just take the first
                Units workers = bot->Observation()->GetUnits(Unit::Alliance::Self, IsUnit(t.source_unit));
                if (workers.size() == 0) {
                    task_queue.pop();
                    std::cout << "Invalid Task: No Source Unit Available: " << (UnitTypeID)t.source_unit << " Source : " << t.source << std::endl;
                    break;
                }
                t.target = workers.front()->tag;
            }

            // check that we have enough resources to do ability
            UnitTypeData ut = bot->Observation()->GetUnitTypeData()[(UnitTypeID)t.unit_typeid];
            if (ut.food_required > available_food
                || ut.mineral_cost > available_minerals
                || ut.vespene_cost > available_vespene)
            {
                task_success = false;
                break;
            }

            bot->Actions()->UnitCommand(bot->Observation()->GetUnit(t.target), t.ability_id, true);

            // update available resources
            available_food -= ut.food_required;
            available_minerals -= ut.mineral_cost;
            available_vespene -= ut.vespene_cost;
            task_queue.pop();
            break;
        }
        case REPAIR: {
            // will repair unit anyway even if there is not enough resources
            // resource cost = %health lost * build cost
            const Unit* scv = bot->Observation()->GetUnit(baseManager->getSCV().tag);
            const Unit* u = bot->Observation()->GetUnit(t.target);
            bot->Actions()->UnitCommand(scv, t.ability_id, u, true);

            // update resources
            UnitTypeData ut = bot->Observation()->GetUnitTypeData()[u->unit_type];
            float lost_health = 1.0f - (u->health / u->health_max);
            available_minerals -= ut.mineral_cost * lost_health;
            available_vespene -= ut.vespene_cost * lost_health;
            break;
        }
        case UPGRADE: {
            // determine cost of upgrade, then implement similar to TRAIN
            std::cout << "Not Implemented: RESOURCE -> UPGRADE" << std::endl;
            break;
        }
        case MOVE: {
            bot->Actions()->UnitCommand(bot->Observation()->GetUnit(t.target), t.ability_id, t.position);
            task_queue.pop();
            break;
        }
        case TRANSFER: {
            // Transfer a unit to another agent, and remove from resource unit list
            // find in our unit list
            std::cout << "Not Implemented: RESOURCE -> TRANSFER" << std::endl;
            break;
        }
        default: {
            std::cerr << "RESOURCE Unrecognized Task: " << t.source << std::endl;
            task_queue.pop();
        }
        }
    }
}

void RESOURCE_BOT::addTask(Task t) {
    // add a task to the task queue
    task_queue.push(t);
}

void RESOURCE_BOT::addUnit(TF_unit u) {
    // add a unit to units
    units.push_back(u.tag);
}

void RESOURCE_BOT::buildingConstructionComplete(const sc2::Unit* u) {
    // we are just paying attention to command centers... to build the vespene
    // if we don't actually see it (which may be the case when starting to build one
    // then we don't have the tag to it
    baseManager->buildRefineries(u);
}

void RESOURCE_BOT::unitDestroyed(const sc2::Unit* u) {
    baseManager->deleteUnit(u);
    for (auto it = units.cbegin(); it != units.cend(); ++it) {
        if (*it == u->tag) {
            units.erase(it);
            return;
        }
    }
}

void RESOURCE_BOT::unitCreated(const sc2::Unit* u) {
    baseManager->addUnit(u);
    addUnit(TF_unit(u->unit_type, u->tag));
}

void RESOURCE_BOT::unitEnterVision(const sc2::Unit* u) {


}

void RESOURCE_BOT::unitIdle(const sc2::Unit* u) {
    baseManager->unitIdle(u);
}

void RESOURCE_BOT::upgradeCompleted(sc2::UpgradeID uid) {
    // as resources, we are specifically paying attention to ___ so ___

}

void RESOURCE_BOT::setAgents(TF_Agent* defenceb, TF_Agent* attackb, TF_Agent* scoutb) {
    this->defence = defenceb;
    this->attack = attackb;
    this->scout = scoutb;
}

void RESOURCE_BOT::buildSupplyDepot() {
    // checks that a supply depot is not already in progress then
    // find a space where a supply depot can be build, then buildStructure
    // for now, choose a (semi)-random point, in the future, have a policy to choose the point

    Point2D point(0, 0);
    while (!query->Placement(ABILITY_ID::BUILD_SUPPLYDEPOT, point)) {
        const Unit* scv = observation->GetUnit(baseManager->getSCV(point).tag);
        point = scv->pos;
        point = Point2D(point.x + GetRandomScalar() * 15.0f, point.y + GetRandomScalar() * 15.0f);
    }
    buildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT, point);
}

bool RESOURCE_BOT::buildStructure(ABILITY_ID ability_to_build_structure, Point2D point, Tag target) {
    // checks that a unit of the same type is not being built (can change this later)
    for (const auto& unit : observation->GetUnits(Unit::Alliance::Self)) {
        for (const auto& order : unit->orders) {
            if (order.ability_id == ability_to_build_structure
                && ability_to_build_structure != ABILITY_ID::BUILD_REFINERY) { // exclude Vespene refinery
                return false;
            }
        }
    }

    const Unit* scv = observation->GetUnit(baseManager->getSCV().tag);
    // commands are queued just in case the same scv is returned several times
    if (target != -1) { // build on a target unit
        const Unit* building = observation->GetUnit(target);
        Units units = observation->GetUnits(IsUnit(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER));
        if (query->Placement(ability_to_build_structure, building->pos, building)) {
            action->UnitCommand(scv, ability_to_build_structure, building, true);
            return true;
        }
    }
    else { // build at a location
        if (query->Placement(ability_to_build_structure, point)) {
            action->UnitCommand(scv, ability_to_build_structure, point, true);
            return true;
        }
    }
    return false;
}