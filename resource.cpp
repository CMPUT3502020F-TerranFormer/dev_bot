#include "TF_Bot.hpp"

void TF_Bot::resourceGameStart(){
    for (auto& p : Observation()->GetUnits()) {
        baseManager->addUnit(p);
    }
}

void TF_Bot::resourceStep() {
	
    // check if we need to build a supply depot
    uint32_t available_food = Observation()->GetFoodCap() - Observation()->GetFoodUsed();

    if (available_food <= baseManager->getSupplyFloat()) {
        buildSupplyDepot();
    }


	while (resource_queue.empty()) {
		Task t = resource_queue.top();
		switch (t.action) {
        case BUILD:
            //const sc2::Unit* u = baseManager->getFreeSCV();
            //action_queue->push(BasicCommand())
            break;
        case TRAIN:
            break;
        case REPAIR:
            break;
        case UPGRADE:
            break;
        case MOVE:
            break;
        case TRANSFER:
            break;
		}
	}
}

void TF_Bot::resourceIdle(const Unit* u) {
    baseManager->idleUnit(u);
}

void TF_Bot::buildSupplyDepot() {
    // find a space where a supply depot can be build, then buildStructure
    // for now, choose a (semi)-random point, in the future, have a policy to choose the point
    Point2D point;
    while (!Query()->Placement(ABILITY_ID::BUILD_SUPPLYDEPOT, point)) {
        point = baseManager->getSCV()->pos;
        point = Point2D(point.x + GetRandomScalar() * 15.0f, point.y + GetRandomScalar() * 15.0f);
    }
    buildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT, point);
}

bool TF_Bot::buildStructure(ABILITY_ID ability_to_build_structure, Point2D point) {
    const Unit* scv = baseManager->getSCV(point);
    if (Query()->Placement(ability_to_build_structure, point)) {
        Actions()->UnitCommand(scv, ability_to_build_structure, point);
        return true;
    }
    return false;
}