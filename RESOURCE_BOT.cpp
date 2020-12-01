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

    buildingPlacementManager = new BuildingPlacementManager(observation, query);
    baseManager = new BaseManager(&task_queue, observation, &units, buildingPlacementManager);
}

/**
 * This handles all actions taken by the bases
 * And it also does resource management for all the agents
 */
void RESOURCE_BOT::step() {
    // actions for bases
    Units scvs = observation->GetUnits(Unit::Alliance::Self, IsSCV()); // so we only need to do this once/step

    baseManager->step(scvs);

    // Resource Management
    // check if we need to build a supply depot
    uint32_t available_food = observation->GetFoodCap() - observation->GetFoodUsed();

    if (available_food <= baseManager->getSupplyFloat()) {
        buildSupplyDepot(scvs);
    }

    // we don't want to remove tasks from queue if there are not enough resources to perform them
    uint32_t available_minerals = observation->GetMinerals();
    uint32_t available_vespene = observation->GetVespene();

    bool task_success = true; // we also want to stop when we don't have resources to complete any other tasks
    while (!task_queue.empty() && task_success) {
        Task t = task_queue.pop();
        switch (t.action) {
        case HARVEST: {
            action->UnitCommand(observation->GetUnit(t.self), t.ability_id, observation->GetUnit(t.target));
            break;
        }
        case BUILD: {
            /* This will prevent multiple identical building from being produced at the same time
             * unless specifically allowed. Identical units will be removed from the queue
             * check that we have enough resources to do build unit
             */
            UnitTypeData ut = observation->GetUnitTypeData()[(UnitTypeID)t.unit_typeid];
            if (ut.food_required > available_food
                || ut.mineral_cost > available_minerals
                || ut.vespene_cost > available_vespene)
            {
                task_queue.push(t);
                task_success = false;
                break;
            }
            if (buildStructure(scvs, t.ability_id, t.position, t.target)) { // if building succeeded
                // update available resources
                available_food -= ut.food_required;
                available_minerals -= ut.mineral_cost;
                available_vespene -= ut.vespene_cost;
                task_success = false; // only take one build task / step -> ensure that unnecessary duplicates aren't created
            }
            break;
        }
        case TRAIN: {
            /* This does not prevent multiple units from being produced at the same time */

            // get the producing unit if not specified
            const Unit* worker = nullptr;
            if (t.target == -1) {
                // try to get a target unit of the required type, we'll just take the first
                Units workers = observation->GetUnits(Unit::Alliance::Self, IsUnit(t.source_unit));
                if (workers.size() == 0) {
                    std::cerr << "Invalid Task: No Source Unit Available: " << (UnitTypeID)t.source_unit << " Source : " << t.source << std::endl;
                    break;
                }
                worker = workers.front();
            }
            else { worker = observation->GetUnit(t.target); }
            if (worker == nullptr) { break; }

            // check that we have enough resources to do ability
            UnitTypeData ut = observation->GetUnitTypeData()[(UnitTypeID)t.unit_typeid];
            if (ut.food_required > available_food
                || ut.mineral_cost > available_minerals
                || ut.vespene_cost > available_vespene)
            {
                task_queue.push(t);
                task_success = false;
                break;
            }

            // when the unit is created we want to add it to the correct agent
            ordered_units.push_back(order_unit(t.source, t.unit_typeid));

            action->UnitCommand(worker, t.ability_id, true);

            // update available resources -- don't worry about supply for queued units
            // units can be queued without it, they just won't build
            // units that overfill the queue will be discarded
            if (worker->orders.size() == 0) { available_food -= ut.food_required; }
            available_minerals -= ut.mineral_cost;
            available_vespene -= ut.vespene_cost;
            break;
        }
        case REPAIR: {
            // will repair unit anyway even if there is not enough resources
            // resource cost = %health lost * build cost
            // we must also limit the number of scvs that can repair a unit at once
            // -> t.count
            int current_scvs = 0;
            for (auto& s : scvs) {
                for (auto& order : s->orders) {
                    if (order.ability_id == ABILITY_ID::EFFECT_REPAIR
                        && order.target_unit_tag == t.target) {
                        ++current_scvs;
                        break;
                    }
                }
            }
            if (current_scvs < t.count) {
                Tag scv_tag = baseManager->getSCV(scvs).tag;
                if (scv_tag == -1) { continue; } // no scv exists
                const Unit* scv = observation->GetUnit(scv_tag);
                const Unit* u = observation->GetUnit(t.target);
                if (u == nullptr) { // an error occurred somewhere
                    task_queue.pop();
                    return;
                }
                if (u != nullptr) {
                    action->UnitCommand(scv, t.ability_id, u, true);

                    // don't update resources as they have not been used yet
                    // and we want to flush out all extra repair tasks
                }
            }
            break;
        }
        case UPGRADE: {
            // determine cost of upgrade, then implement similar to TRAIN
            UpgradeData ud = observation->GetUpgradeData()[(UpgradeID) t.upgrade_id];
            if (ud.mineral_cost > available_minerals
                || ud.vespene_cost > available_vespene)
            {
                task_queue.push(t);
                task_success = false;
                break;
            }
            action->UnitCommand(observation->GetUnit(t.self), t.ability_id);
            break;
        }
        case MOVE: {
            action->UnitCommand(observation->GetUnit(t.target), t.ability_id, t.position);
            break;
        }
        case TRANSFER: {
            /* Transfer a unit to another agent, and remove from resource unit list
            * Either the exact unit be specified, or it is an scv 
            * The unit must exist (unless it is an scv) for this to behave properly
            */
            TF_unit unit;
            if (t.self == -1) {
                TF_unit scv = baseManager->getSCV(scvs);
                if (scv.tag == -1) {
                    break;
                }
                unit = scv;
            }
            else { unit = TF_unit(observation->GetUnit(t.self)->unit_type, t.self); }

            // add to correct agent
            switch (t.source) {
            case DEFENCE_AGENT: defence->addUnit(unit);
                break;
            case ATTACK_AGENT: attack->addUnit(unit);
                break;
            case SCOUT_AGENT: scout->addUnit(unit);
                break;
            default:
                std::cerr << "TRANSFER to invalid agent requested!" << std::endl;
                return;
            }

            // and remove from resource_units
            for (auto it = units.cbegin(); it != units.cend(); ++it) {
                if (*it == t.self) { 
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

void RESOURCE_BOT::addTask(Task t) {
    // add a task to the task queue
    task_queue.push(t);
}

void RESOURCE_BOT::addUnit(TF_unit u) {
    // add a unit to units, and baseManager
    units.push_back(u.tag);
    baseManager->addUnit(observation->GetUnit(u.tag));
}

void RESOURCE_BOT::buildingConstructionComplete(const sc2::Unit* u) {
    // deal with morphs here; not necessary yet -> when we will use orbital command
    // and planetary fortresses

}

void RESOURCE_BOT::unitDestroyed(const sc2::Unit* u) {
    if (u->alliance != Unit::Alliance::Self) { return; }
    baseManager->deleteUnit(u);
    for (auto it = units.cbegin(); it != units.cend(); ++it) {
        if (*it == u->tag) {
            units.erase(it);
            return;
        }
    }
}

void RESOURCE_BOT::unitCreated(const sc2::Unit* u) {
    // add it to the correct agent
    auto tfu = TF_unit(u->unit_type, u->tag);
    for (auto it = ordered_units.cbegin(); it != ordered_units.cend(); ++it) {
        if (it->second == u->unit_type) {
            switch (it->first) {
            case SCOUT_AGENT: 
                scout->addUnit(tfu);
                break;
            case DEFENCE_AGENT: 
                defence->addUnit(tfu);
                break;
            case ATTACK_AGENT: 
                attack->addUnit(tfu);
                break;
            default: // RESOURCE_AGENT
                addUnit(tfu);
                break;
            }
            // and remove the unit from ordered_units
            ordered_units.erase(it);
            return;
        }
    }
    // not in ordered_units -> resource agent
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

void RESOURCE_BOT::buildSupplyDepot(Units scvs) {
    // gets a location to build the supply depot then buildStructure
    // which will prevent 2 from being built at the same time
    Point2D point = buildingPlacementManager->getNextSupplyDepotLocation();
    buildStructure(scvs, ABILITY_ID::BUILD_SUPPLYDEPOT, point);
}

/* We pass in scvs because this may be called multiple times/step so it's more efficient */
bool RESOURCE_BOT::buildStructure(Units scvs, ABILITY_ID ability_to_build_structure, Point2D point, Tag target) {
    Tag scv_tag = baseManager->getSCV(scvs).tag;
    if (scv_tag == -1) { return false; } // there are no scvs
    const Unit* scv = observation->GetUnit(scv_tag);
    if (scv == nullptr) { return false; }

    // commands are queued just in case the same scv is returned several times
    if (target != -1) { // build on a target unit
        const Unit* building = observation->GetUnit(target);
        if (building == nullptr) { return false; }
        if (query->Placement(ability_to_build_structure, building->pos, building)
            && !buildCheckDuplicate(scvs, ability_to_build_structure)) {
            action->UnitCommand(scv, ability_to_build_structure, building, true);
            return true;
        }
    }
    else { // build at a location
        if (query->Placement(ability_to_build_structure, point)
            && !buildCheckDuplicate(scvs, ability_to_build_structure)) {
            action->UnitCommand(scv, ability_to_build_structure, point, true);
            return true;
        }
    }
    return false;
}

bool RESOURCE_BOT::buildCheckDuplicate(Units scvs, ABILITY_ID ability_to_build_structure) {
    // checks that a unit of the same type is not being built (can change this later)
    for (const auto& unit : scvs) {
        for (const auto& order : unit->orders) {
            if (order.ability_id == ability_to_build_structure
                && ability_to_build_structure != ABILITY_ID::BUILD_REFINERY) { // exclude Vespene refinery
                return true;
            }
        }
    }
    return false;
}

std::vector<Spotted_Enemy> RESOURCE_BOT::last_seen_near(Point2D location, int radius, int since) {
    // do nothing
}
