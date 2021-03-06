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
    baseManager = new BaseManager(&task_queue, observation, &units);
}

/**
 * This handles all actions taken by the bases
 * And it also does resource management for all the agents
 */
void RESOURCE_BOT::step() {
    // actions for bases

    Units scvs = observation->GetUnits(Unit::Alliance::Self, IsSCV()); // so we only need to do this once/step
    baseManager->step(scvs);

    if ((observation->GetGameLoop() + 1) % 4 != 0) { return; } // offset from BaseManager timing

    // Resource Management
    // check if we need to build a supply depot
    uint32_t available_food = observation->GetFoodCap() - observation->GetFoodUsed();

    if (available_food <= baseManager->getSupplyFloat()
        && observation->GetFoodCap() < 200) {
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
            const Unit* unit = observation->GetUnit(t.self);
            if (unit == nullptr) { continue; }
            if (IsCarryingMinerals(*unit) || IsCarryingVespene(*unit)) {
                action->UnitCommand(unit, ABILITY_ID::HARVEST_RETURN, true);
            }
            else {
                action->UnitCommand(unit, t.ability_id, observation->GetUnit(t.target), true);
            }
            action->SendActions();
            break;
        }
        case BUILD: {
            /* This will prevent multiple identical building from being produced at the same time
             * unless specifically allowed. Identical units will be removed from the queue
             * and check that we have enough resources to build unit
             */
            UnitTypeData ut = observation->GetUnitTypeData()[(UnitTypeID)t.unit_typeid];
            if (ut.food_required > available_food
                || ut.mineral_cost > available_minerals
                || ut.vespene_cost > available_vespene)
            {
                if (observation->GetFoodCap() < 200
                    && (ut.mineral_cost > available_minerals || ut.vespene_cost > available_vespene))
                { // don't keep tasks when we reach max supply, and have the resources
                    task_queue.push(t);
                    task_success = false;
                    break;
                }
            }
            if (buildStructure(scvs, t.unit_typeid, t.ability_id, t.position, t.target)) { // if building succeeded
                action->SendActions();
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
                std::sort(workers.begin(), workers.end(), [](const Unit* a, const Unit* b) { return a->orders.size() < b->orders.size(); });
                worker = workers.front(); // the one with the fewest things in its queue
            }
            else { worker = observation->GetUnit(t.target); }

            UnitTypeData ut = observation->GetUnitTypeData()[(UnitTypeID) t.unit_typeid];
            if (worker == nullptr) { 
                std::cerr << "Invalid Worker, Tag: " << t.target << " Source Agent: " << t.source << " Tried to Train: " << ut.name << std::endl;
                break; 
            }

            // check that we have enough resources to do ability
            if (ut.food_required > available_food
                || ut.mineral_cost > available_minerals
                || ut.vespene_cost > available_vespene)
            {
                if (observation->GetFoodCap() < 200
                    && (ut.mineral_cost > available_minerals || ut.vespene_cost > available_vespene))
                { // don't keep tasks when we reach max supply, and have the resources
                    task_queue.push(t);
                    task_success = false;
                }
                break;
            }

            // when the unit is created we want to add it to the correct agent
            ordered_units.push_back(order_unit(t.source, t.unit_typeid));

            action->UnitCommand(worker, t.ability_id, true);
            action->SendActions();

            // update available resources 
            available_food -= ut.food_required;
            available_minerals -= ut.mineral_cost;
            available_vespene -= ut.vespene_cost;
            break;
        }
        case REPAIR: {
            // will repair unit anyway even if there is not enough resources
            // resource cost = %health lost * build cost
            // we must also limit the number of scvs that can repair a unit at -> t.count
            int current_scvs = 0;
            for (auto& s : scvs) {
                for (auto& order : s->orders) {
                    if (order.target_unit_tag == t.target) {
                        ++current_scvs;
                        break;
                    }
                }
            }
            if (current_scvs < t.count) {
                const Unit* u = observation->GetUnit(t.target);
                if (u != nullptr) {
                    Tag scv_tag = baseManager->getSCV(scvs, u->pos).tag;
                    if (scv_tag == -1) { continue; } // no scv exists
                    const Unit* scv = observation->GetUnit(scv_tag);
                    action->UnitCommand(scv, ABILITY_ID::HARVEST_RETURN, true);
                    action->UnitCommand(scv, t.ability_id, u, true);
                    action->SendActions();
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
            auto worker = observation->GetUnit(t.self);
            if (worker == nullptr) { break; }
            action->UnitCommand(worker, t.ability_id, true);
            action->SendActions();
            break;
        }
        case MOVE: {
            auto unit = observation->GetUnit(t.target);
            if (unit == nullptr) { break; }
            action->UnitCommand(unit, t.ability_id, t.position, true);
            action->SendActions();
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
                    std::cerr << "TRANSFER from " << t.source << " No scv available" << std::endl;
                    break;
                }
                unit = scv;
                action->UnitCommand(observation->GetUnit(unit.tag), ABILITY_ID::HARVEST_RETURN, true);
            }
            else { 
                auto u = observation->GetUnit(t.self);
                if (u == nullptr) { break; }
                unit = TF_unit(u->unit_type, t.self); 
            }

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
    for (auto it = units.begin(); it != units.end(); ++it) {
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
    // push task at max build priority (resource tasks are preferred at same priority)
    // this way it will check for minerals and wait for them before doing anything else
    // don't make this the first thing we build/train (start on an scv first)
    if (observation->GetGameLoop() / 16 < 15) { return; }
    task_queue.push(Task(BUILD, RESOURCE_AGENT, 10, UNIT_TYPEID::TERRAN_SUPPLYDEPOT, ABILITY_ID::BUILD_SUPPLYDEPOT));
}

/* We pass in scvs because this may be called multiple times/step so it's more efficient */
bool RESOURCE_BOT::buildStructure(Units scvs, UNIT_TYPEID unit_type, 
    ABILITY_ID ability_to_build_structure, Point2D point, Tag target) {

    if (target != -1) { // build on a target unit
        const Unit* building = observation->GetUnit(target);
        if (building == nullptr) { 
            std::cerr << "Building Error: Invalid Target Unit" << std::endl;
            return false; 
        }

        // get close scv
        Tag scv_tag = baseManager->getSCV(scvs, building->pos).tag;
        if (scv_tag == -1) {
            std::cerr << "Error building: There are no scvs" << std::endl;
            return false;
        } // there are no scvs
        const Unit* scv = observation->GetUnit(scv_tag);
        if (scv == nullptr) {
            std::cerr << "Error building: invalid scv" << std::endl;
            return false;
        }

        if (query->Placement(ability_to_build_structure, building->pos, building)
            && !buildCheckDuplicate(scvs, ability_to_build_structure)) {
            if (IsCarryingMinerals(*scv) || IsCarryingVespene(*scv)) {
                action->UnitCommand(scv, ABILITY_ID::HARVEST_RETURN, true);
            }
            action->UnitCommand(scv, ability_to_build_structure, building, true);
            return true;
        }
    }
    else { // build at a location
        if (!buildCheckDuplicate(scvs, ability_to_build_structure)) {
            Point2D new_point = buildingPlacementManager->getNextLocation(unit_type, point);
            if (new_point == Point2D(0, 0)) { return false; }

            // get close scv
            Tag scv_tag = baseManager->getSCV(scvs, new_point).tag;
            if (scv_tag == -1) {
                std::cerr << "Error building: There are no scvs" << std::endl;
                return false;
            } // there are no scvs
            const Unit* scv = observation->GetUnit(scv_tag);
            if (scv == nullptr) {
                std::cerr << "Error building: invalid scv" << std::endl;
                return false;
            }
            if (IsCarryingMinerals(*scv) || IsCarryingVespene(*scv)) {
                action->UnitCommand(scv, ABILITY_ID::HARVEST_RETURN, true);
            }
            action->UnitCommand(scv, ability_to_build_structure, new_point, true);
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
    return scout->last_seen_near(location, radius, since);
}
