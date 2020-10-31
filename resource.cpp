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
        case BUILD:
            //const sc2::Unit* u = baseManager->getFreeSCV();
            //action_queue->push(BasicCommand())
            break;
        case TRAIN: {
            /* This does not prevent multiple units from being produced at the same time */
            // get the producing unit
            sc2::Tag unit = getUnit(t);
            if (unit == -1) { 
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

            Actions()->UnitCommand(Observation()->GetUnit(unit), t.ability_id, false);
            
            // update available resources
            available_food -= ut.food_required;
            available_minerals -= ut.mineral_cost;
            available_vespene -= ut.vespene_cost;
            resource_queue.pop();
            break;
        }
        case REPAIR:
            break;
        case UPGRADE:
            break;
        case MOVE:
            break;
        case TRANSFER:
            break;
        case ORBIT_SCOUT:
            //baseManager // orbital scan
            break;
        default:
            std::cerr << "RESOURCE Unrecognized Task: " << t.source << std::endl;
            resource_queue.pop();
		}
	}
}

void TF_Bot::resourceIdle(const Unit* u) {
    baseManager->idleUnit(u);
}

sc2::Tag TF_Bot::getUnit(Task& t) {
    if (t.target != nullptr) { return t.target->tag;  }
    // otherwise use t.source_unit to get a unit of the same type, return nullptr if one does not exist

    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    for (const auto& unit : units) {
        if (unit->unit_type == t.source_unit) {
            return unit->tag;
        }
    }
    return -1; // overflow, max val; this will never occur naturally
}

void TF_Bot::buildSupplyDepot() {
    // checks that a supply depot is not already in progress then
    // find a space where a supply depot can be build, then buildStructure
    // for now, choose a (semi)-random point, in the future, have a policy to choose the point

    for (const auto& unit : Observation()->GetUnits(Unit::Alliance::Self)) {
        for (const auto& order : unit->orders) {
            if (order.ability_id == ABILITY_ID::BUILD_SUPPLYDEPOT) {
                return;
            }
        }
    }
    Point2D point;
    while (!Query()->Placement(ABILITY_ID::BUILD_SUPPLYDEPOT, point)) {
        point = Observation()->GetUnit(baseManager->getSCV().tag)->pos;
        point = Point2D(point.x + GetRandomScalar() * 15.0f, point.y + GetRandomScalar() * 15.0f);
    }
    buildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT, point);
}

bool TF_Bot::buildStructure(ABILITY_ID ability_to_build_structure, Point2D point) {
    const Unit* scv = Observation()->GetUnit(baseManager->getSCV().tag);
    if (Query()->Placement(ability_to_build_structure, point)) {
        Actions()->UnitCommand(scv, ability_to_build_structure, point);
        return true;
    }
    return false;
}