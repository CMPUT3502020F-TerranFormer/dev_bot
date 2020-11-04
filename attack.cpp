#include "TF_Bot.hpp"
#include "attack.hpp"
#include "helper_functions.hpp"

void TF_Bot::attackIdle(const Unit *unit)
{
    // if unit is idle,
    switch (unit->unit_type.ToType())
    {

        // barracks train marine
        // TODO: possibly switch to Marauders if we already have a sufficient
        // amount of Marines
        //
        // to train marauders, check for the presence of a tech lab first

    case UNIT_TYPEID::TERRAN_BARRACKS:
    {
        const ObservationInterface *observation = Observation();
        if (CountUnitType(observation, UNIT_TYPEID::TERRAN_MARINE) < 25)
        {
            Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
        }

        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARAUDER);
        break;
    }

    case UNIT_TYPEID::TERRAN_STARPORT:
    {
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MEDIVAC);
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_BANSHEE);
    }

    case UNIT_TYPEID::TERRAN_MARINE:
    case UNIT_TYPEID::TERRAN_BANSHEE: //will have to be removed later
    {
        const ObservationInterface *observation = Observation();

        const GameInfo &game_info = Observation()->GetGameInfo();
        if (CountUnitType(observation, UNIT_TYPEID::TERRAN_MARINE) > 10)
        {
            Actions()->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations.front());
            Actions()->SendActions();
        }
    }
    case UNIT_TYPEID::TERRAN_MEDIVAC:
    {
        const ObservationInterface *observation = Observation();

        const GameInfo &game_info = Observation()->GetGameInfo();
        if (CountUnitType(observation, UNIT_TYPEID::TERRAN_MARINE) > 10)
        {
            Actions()->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations.front());
        }
    }
    }
}

// function to train units
// trainer_unit is the unit which trains the unit (e.g Barracks)
// unit_to_train is the unit that this building can train
//
// void TF_Bot::train_unit(const Unit *trainer_unit, ABILITY_ID unit_to_train)
// {
//     switch(trainer_unit->unit_type.ToType())
//     {
//         case UNIT_TYPEID::TERRAN_BARRACKS:
//         {
//             switch()
//         }
//     }
// }

void TF_Bot::attackStep()
{
    const ObservationInterface *observation = Observation();
    // Why am I building only 1 barracks, 1 starport, 1 factory
    // Popular opening in SC2
    if (CountUnitType(observation, UNIT_TYPEID::TERRAN_BARRACKS) < 2)
    {
        buildBarracks();
    }

    if (CountUnitType(observation, UNIT_TYPEID::TERRAN_FACTORY) < 1)
    {
        buildFactory();
    }

    if (CountUnitType(observation, UNIT_TYPEID::TERRAN_STARPORT) < 1)
    {
        buildStarport();
    }

    // uint32_t available_food = Observation()->GetFoodCap() - Observation()->GetFoodUsed();

    // // we don't want to remove tasks from queue if there are not enough resources to perform them
    // uint32_t available_minerals = Observation()->GetMinerals();
    // uint32_t available_vespene = Observation()->GetVespene();

    // bool task_success = true; // we also want to stop when we don't have resources to complete any other tasks
    // while (!attack_queue.empty() && task_success)
    // {
    //     Task t = attack_queue.top();
    //     switch (t.action)
    //     {
    //     case BUILD:
    //     {
    //         /* This will prevent multiple identical building from being produced at the same time
    //            unless specifically allowed. Identical units will be removed from the queue */
    //         // check that we have enough resources to do build unit
    //         UnitTypeData ut = Observation()->GetUnitTypeData()[(UnitTypeID)t.unit_typeid];
    //         if (ut.food_required > available_food && ut.mineral_cost > available_minerals && ut.vespene_cost > available_vespene)
    //         {
    //             task_success = false;
    //             break;
    //         }
    //         if (buildStructure(t.ability_id, t.position, t.target))
    //         { // if building succeeded
    //             // update available resources
    //             available_food -= ut.food_required;
    //             available_minerals -= ut.mineral_cost;
    //             available_vespene -= ut.vespene_cost;
    //         }
    //         resource_queue.pop();
    //         break;
    //     }
    //     case TRAIN:
    //     {
    //         /* This does not prevent multiple units from being produced at the same time */
    //         // get the producing unit
    //         if (t.target == -1)
    //         {
    //             resource_queue.pop();
    //             std::cout << "Invalid Task: No Source Unit Available: " << (UnitTypeID)t.source_unit << " Source : " << t.source << std::endl;
    //             break;
    //         }

    //         // check that we have enough resources to do ability
    //         UnitTypeData ut = Observation()->GetUnitTypeData()[(UnitTypeID)t.unit_typeid];
    //         if (ut.food_required > available_food && ut.mineral_cost > available_minerals && ut.vespene_cost > available_vespene)
    //         {
    //             task_success = false;
    //             break;
    //         }

    //         Actions()->UnitCommand(Observation()->GetUnit(t.target), t.ability_id, false);

    //         // update available resources
    //         available_food -= ut.food_required;
    //         available_minerals -= ut.mineral_cost;
    //         available_vespene -= ut.vespene_cost;
    //         resource_queue.pop();
    //         break;
    //     }
    //     case REPAIR:
    //     {
    //         // will repair unit anyway even if there is not enough resources
    //         // resource cost = %health lost * build cost
    //         const Unit *scv = Observation()->GetUnit(baseManager->getSCV().tag);
    //         const Unit *u = Observation()->GetUnit(t.target);
    //         Actions()->UnitCommand(scv, t.ability_id, u);

    //         // update resources
    //         UnitTypeData ut = Observation()->GetUnitTypeData()[u->unit_type];
    //         float lost_health = 1.0f - (u->health / u->health_max);
    //         available_minerals -= ut.mineral_cost * lost_health;
    //         available_vespene -= ut.vespene_cost * lost_health;
    //         break;
    //     }
    //     case UPGRADE:
    //     {
    //         // determine cost of upgrade, then implement similar to TRAIN
    //         break;
    //     }
    //     }
    // }
}

// helps count the number of units present in the current game state
int TF_Bot::CountUnitType(const ObservationInterface *observation, UnitTypeID unit_type)
{
    int count = 0;
    Units my_units = observation->GetUnits(Unit::Alliance::Self);
    for (const auto unit : my_units)
    {
        if (unit->unit_type == unit_type)
            ++count;
    }
    return count;
}

// function to build structure
bool TF_Bot::buildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type)
{
    const ObservationInterface *observation = Observation();

    // If a unit already is building a supply structure of this type, do nothing.
    // Also get an scv to build the structure.
    const Unit *unit_to_build = nullptr;
    Units units = observation->GetUnits(Unit::Alliance::Self);
    for (const auto &unit : units)
    {
        for (const auto &order : unit->orders)
        {
            if (order.ability_id == ability_type_for_structure)
            {
                return false;
            }
        }

        if (unit->unit_type == unit_type)
        {
            unit_to_build = unit;
        }
    }

    float rx = GetRandomScalar();
    float ry = GetRandomScalar();

    Actions()->UnitCommand(unit_to_build,
                           ability_type_for_structure,
                           Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));

    return true;
}

bool TF_Bot::buildAddOn(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type)
{
    // TODO: has to be improved for sure
    // build the add_on next to the building
    // I am assuming that the position means the center
    // Basic algorithm:
    

    const ObservationInterface *observation = Observation();

    // If a unit already is building a supply structure of this type, do nothing.
    const Unit *unit_to_build = nullptr;
    Units units = observation->GetUnits(Unit::Alliance::Self);

    for (const auto &unit : units)
    {
        for (const auto &order : unit->orders)
        {
            if (order.ability_id == ability_type_for_structure)
            {
                return false;
            }
        }

        if (unit->unit_type == unit_type)
        {
            unit_to_build = unit;
        }
    }

    QueryInterface *query = Query();

    float rx = GetRandomScalar();
    float ry = GetRandomScalar();

    float valid_x = unit_to_build->pos.x + rx * 12.0f;
    float valid_y = unit_to_build->pos.y + ry * 12.0f;

    std::cout << "Building px: " << unit_to_build->pos.x << std::endl;
    std::cout << "Building py: " << unit_to_build->pos.y << std::endl;

    Point2D add_on_pos(valid_x, valid_y);
    std::cout << "Add on px: " << valid_x << std::endl;
    std::cout << "Add on py: " << valid_y << std::endl;

    Actions()->UnitCommand(unit_to_build,
                           ability_type_for_structure,
                           add_on_pos);

    return true;
}

bool TF_Bot::buildBarracks()
{
    // Prereqs of building barrack: Supply Depot
    const ObservationInterface *observation = Observation();

    if (CountUnitType(observation, UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1)
    {
        return false;
    }

    return buildStructure(ABILITY_ID::BUILD_BARRACKS);
}

bool TF_Bot::buildFactory()
{
    //  Prereqs of building factory: Barracks
    const ObservationInterface *observation = Observation();

    if (CountUnitType(observation, UNIT_TYPEID::TERRAN_BARRACKS) < 1)
    {
        return false;
    }

    return buildStructure(ABILITY_ID::BUILD_FACTORY);
}

bool TF_Bot::buildStarport()
{
    // Prereqs of building starport: Factory
    const ObservationInterface *observation = Observation();

    if (CountUnitType(observation, UNIT_TYPEID::TERRAN_FACTORY) < 1)
    {
        return false;
    }

    return buildStructure(ABILITY_ID::BUILD_STARPORT);
}