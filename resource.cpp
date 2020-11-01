#include "TF_Bot.hpp"

void TF_Bot::resourceGameStart(){
    // units are added via UnitCreated at GameStart()
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
        case BUILD: {
            /* This will prevent multiple identical building from being produced at the same time
               Identical units will be removed from the queue */
            // check that we have enough resources to do build unit
            UnitTypeData ut = Observation()->GetUnitTypeData()[(UnitTypeID) t.unit_typeid];
            if (ut.food_required > available_food
                && ut.mineral_cost > available_minerals
                && ut.vespene_cost > available_vespene)
            {
                task_success = false;
                break;
            }
            if (buildStructure(t.ability_id, t.position)) { // if building succeeded
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
                && ut.mineral_cost > available_minerals
                && ut.vespene_cost > available_vespene)
            {
                task_success = false;
                break;
            }

            Actions()->UnitCommand(Observation()->GetUnit(t.target), t.ability_id, false);
            
            // update available resources
            available_food -= ut.food_required;
            available_minerals -= ut.mineral_cost;
            available_vespene -= ut.vespene_cost;
            resource_queue.pop();
            break;
        }
        case REPAIR: {
            // determine cost of repair then implement similar to TRAIN
            break;
        }
        case UPGRADE: {
            // determine cost of upgrade, then implement similar to TRAIN
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
            break;
        }
        case ORBIT_SCOUT: {
            //baseManager // orbital scan
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
    baseManager->idleUnit(u);
}

void TF_Bot::buildSupplyDepot() {
    // checks that a supply depot is not already in progress then
    // find a space where a supply depot can be build, then buildStructure
    // for now, choose a (semi)-random point, in the future, have a policy to choose the point

    Point2D point;
    while (!Query()->Placement(ABILITY_ID::BUILD_SUPPLYDEPOT, point)) {
        point = Observation()->GetUnit(baseManager->getSCV().tag)->pos;
        point = Point2D(point.x + GetRandomScalar() * 15.0f, point.y + GetRandomScalar() * 15.0f);
    }
    buildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT, point);
}

bool TF_Bot::buildStructure(ABILITY_ID ability_to_build_structure, Point2D point) {
    // checks that a unit of the same type is not being built
    for (const auto& unit : Observation()->GetUnits(Unit::Alliance::Self)) {
        for (const auto& order : unit->orders) {
            if (order.ability_id == ABILITY_ID::BUILD_SUPPLYDEPOT) {
                return false;
            }
        }
    }

    const Unit* scv = Observation()->GetUnit(baseManager->getSCV().tag);
    if (Query()->Placement(ability_to_build_structure, point)) {
        Actions()->UnitCommand(scv, ability_to_build_structure, point);
        return true;
    }
    return false;
}