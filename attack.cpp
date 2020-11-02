#include "TF_Bot.hpp"
#include "attack.hpp"

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
        Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
        break;
    }
    }
}

void TF_Bot::attackStep()
{
    
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

        if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) < 1)
        {
            return false;
        }

        return buildStructure(ABILITY_ID::BUILD_FACTORY);
    }

    bool TF_Bot::buildStarport()
    {
        // Prereqs of building starport: Factory
        const ObservationInterface *observation = Observation();

        if (CountUnitType(UNIT_TYPEID::TERRAN_FACTORY) < 1)
        {
            return false;
        }

        return buildStructure(ABILITY_ID::BUILD_STARPORT);
    }

    bool TF_Bot::buildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV)
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