#include "TF_Bot.hpp"

void TF_Bot::resourceGameStart(){
    // units are added via UnitCreated at GameStart()
    std::cout << Observation()->GetStartLocation().x << "|" << Observation()->GetStartLocation().y << std::endl;
    std::cout << "Map: " << Observation()->GetGameInfo().map_name << std::endl;
}

void TF_Bot::resourceStep() {
    /*MODIFY THIS AND HELPER METHODS TO CHECK IF THERE ARE ENOUGH RESOURCES TO BUILD FIRST
    AND WHEN BUILDING BUILDINGS; ONLY BUILD ONE OF EACH TYPE AT A TIME (REMOVE DUPLICATES FROM THE QUEUE)
    MAKE SURE THAT WE DON'T INTERRUPT ACTIONS AND MAKE SURE THIS WORKS WITH BARRACKS & OTHER UNIT TYPES
    **THIS WILL PROBABLY BE CHANGED WHEN THE TASK_QUEUE IS CHANGED AND MOVED INTO ONUNITIDLE()
    */
	
    // check if we need to build a supply depot
    uint32_t available_food = Observation()->GetFoodCap() - Observation()->GetFoodUsed();

    if (available_food <= baseManager->getSupplyFloat()) {
        buildSupplyDepot();
    }

    // we don't want to remove tasks from queue if there are not enough resources to perform them
    uint32_t available_minerals = Observation()->GetMinerals();
    uint32_t available_vespene = Observation()->GetVespene();

    bool task_success = true; // we also want to stop when we don't have resources to complete any other tasks
	while (!resource_queue.empty() && task_success) {
		Task t = resource_queue.top();
		switch (t.action) {
        case HARVEST: {
            Actions()->UnitCommand(Observation()->GetUnit(t.self), t.ability_id, Observation()->GetUnit(t.target));
            resource_queue.pop();
            break;
        }
        case BUILD: {
            /* This will prevent multiple identical building from being produced at the same time
               unless specifically allowed. Identical units will be removed from the queue */
            // check that we have enough resources to do build unit
            UnitTypeData ut = Observation()->GetUnitTypeData()[(UnitTypeID) t.unit_typeid];
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
            resource_queue.pop();
            break;
        }
        case TRAIN: {
            /* This does not prevent multiple units from being produced at the same time */
            // get the producing unit
            if (t.target == -1) { 
                resource_queue.pop();
                std::cout << "Invalid Task: No Source Unit Available: " << (UnitTypeID) t.source_unit << " Source : " << t.source << std::endl;
                break; 
            }
            
            // check that we have enough resources to do ability
            UnitTypeData ut = Observation()->GetUnitTypeData()[(UnitTypeID) t.unit_typeid];
            if (ut.food_required > available_food
                || ut.mineral_cost > available_minerals
                || ut.vespene_cost > available_vespene)
            {
                task_success = false;
                break;
            }

            Actions()->UnitCommand(Observation()->GetUnit(t.target), t.ability_id, true);
            
            // update available resources
            available_food -= ut.food_required;
            available_minerals -= ut.mineral_cost;
            available_vespene -= ut.vespene_cost;
            resource_queue.pop();
            break;
        }
        case REPAIR: {
            // will repair unit anyway even if there is not enough resources
            // resource cost = %health lost * build cost
            const Unit* scv = Observation()->GetUnit(baseManager->getSCV().tag);
            const Unit* u = Observation()->GetUnit(t.target);
            Actions()->UnitCommand(scv, t.ability_id, u, true);

            // update resources
            UnitTypeData ut = Observation()->GetUnitTypeData()[u->unit_type];
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
            Actions()->UnitCommand(Observation()->GetUnit(t.target), t.ability_id, t.position);
            resource_queue.pop();
            break;
        }
        case TRANSFER: {
            // Transfer a unit to another agent, and remove from resource unit list
            // find in our unit list
            std::cout << "Not Implemented: RESOURCE -> TRANSFER" << std::endl;
            break;
        }
        case ORBIT_SCOUT: {
            //baseManager // orbital scan
            std::cout << "Not Implemented: RESOURCE -> ORBIT_SCOUT" << std::endl;
            break;
        }
        default: {
            std::cerr << "RESOURCE Unrecognized Task: " << t.source << std::endl;
            resource_queue.pop();
        }
		}
	}
}

void TF_Bot::resourceIdle(const Unit* u) {
    baseManager->unitIdle(u);
}

void TF_Bot::resourceBuildingComplete(const Unit* u) {
    // we are just paying attention to command centers... to build the vespene
    // if we don't actually see it (which may be the case when starting to build one
    // then we don't have the tag to it
    baseManager->buildRefineries(u);
}

void TF_Bot::buildSupplyDepot() {
    // checks that a supply depot is not already in progress then
    // find a space where a supply depot can be build, then buildStructure
    // for now, choose a (semi)-random point, in the future, have a policy to choose the point

    Point2D point(0, 0);
    while (!Query()->Placement(ABILITY_ID::BUILD_SUPPLYDEPOT, point)) {
        const Unit* scv = Observation()->GetUnit(baseManager->getSCV(point).tag);
        point = scv->pos;
        point = Point2D(point.x + GetRandomScalar() * 15.0f, point.y + GetRandomScalar() * 15.0f);
    }
    buildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT, point);
}

bool TF_Bot::buildStructure(ABILITY_ID ability_to_build_structure, Point2D point, Tag target) {
    // checks that a unit of the same type is not being built (can change this later)
    for (const auto& unit : Observation()->GetUnits(Unit::Alliance::Self)) {
        for (const auto& order : unit->orders) {
            if (order.ability_id == ability_to_build_structure
                && ability_to_build_structure != ABILITY_ID::BUILD_REFINERY) { // exclude Vespene refinery
                return false;
            }
        }
    }

    const Unit* scv = Observation()->GetUnit(baseManager->getSCV().tag);
    // commands are queued just in case the same scv is returned several times
    if (target != -1) { // build on a target unit
        const Unit* building = Observation()->GetUnit(target);
        Units units = Observation()->GetUnits(IsUnit(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER));
        if (Query()->Placement(ability_to_build_structure, building->pos, building)) {
            Actions()->UnitCommand(scv, ability_to_build_structure, building, true);
            return true;
        }
    } else { // build at a location
        if (Query()->Placement(ability_to_build_structure, point)) {
            Actions()->UnitCommand(scv, ability_to_build_structure, point, true);
            return true;
        }
    }
    return false;
}