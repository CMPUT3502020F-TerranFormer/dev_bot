#include "TF_Bot.hpp"

void TF_Bot::resourceGameStart(){
    for (auto& p : Observation()->GetUnits()) {
        baseManager->addUnit(p);
    }
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


	while (!resource_queue.empty()) {
		Task t = resource_queue.top();
        const Unit* u = Observation()->GetUnit(t.target->tag);
		switch (t.action) {
        case BUILD:
            //const sc2::Unit* u = baseManager->getFreeSCV();
            //action_queue->push(BasicCommand())
            break;
        case TRAIN:
            Actions()->UnitCommand(u, t.ability_id);
        case REPAIR:
            break;
        case UPGRADE:
            break;
        case MOVE:
            break;
        case TRANSFER:
            break;
        case ORBIT_SCOUT:
            //baseManager.
            break;
		}
        resource_queue.pop();
	}
}

void TF_Bot::resourceIdle(const Unit* u) {
    baseManager->idleUnit(u);
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