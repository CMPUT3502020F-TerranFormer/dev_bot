//
// Created by Carter Sabadash on 2020-10-24
//
#include "ATTACK_BOT.hpp"
#include <iostream>

using namespace sc2;

ATTACK_BOT::ATTACK_BOT(TF_Bot *bot)
    : TF_Agent(bot)
{
    defence = nullptr;
    scout = nullptr;
    resource = nullptr;
}

ATTACK_BOT::~ATTACK_BOT()
{
}

void ATTACK_BOT::init()
{
    troopManager = new TroopManager(&task_queue, observation, scout);
}

void ATTACK_BOT::step()
{
    // perform actions in the task queue
    while (!task_queue.empty())
    {
        Task t = task_queue.pop();
        // push resource tasks from TroopManager into resources
        // and perform other tasks as necessary (sometimes re-using code from resources)
        switch (t.action)
        {
        case BUILD:
        {
            resource->addTask(t);
            break;
        }
        case TRAIN:
        {
            resource->addTask(t);
            break;
        }
        case ATTACK:
        {
            // Classify the units in flying or ground
            // to facilitate movement
            // Because air units will travel faster than the ground units
            const Unit *air_unit = nullptr;
            const Unit *ground_unit = nullptr;
            if (IsAFlyingAttackUnit(t.unit_typeid))
            {
                air_unit = t.unit;
            }
            else
            {
                ground_unit = t.unit;
            }

            // if a tank is in siege mode, unsiege them
            if (t.unit->unit_type == UNIT_TYPEID::TERRAN_SIEGETANKSIEGED)
            {
                action->UnitCommand(t.unit, ABILITY_ID::MORPH_UNSIEGE);
            }

            if (t.unit->unit_type == UNIT_TYPEID::TERRAN_BANSHEE)
            {
                action->UnitCommand(t.unit, ABILITY_ID::BEHAVIOR_CLOAKON);
            }

            // Check that we are not duplicating units
            // My assumption is that since units attack only when the army count reaches squadron size
            // Units might be idle for multiple game cycles

            if (ground_unit != nullptr && std::find(ground_units.begin(), ground_units.end(), ground_unit) == ground_units.end())
            {
                // if the unit is not in the vector
                // add it
                ground_units.push_back(ground_unit);
                //action->SendChat("Adding ground units");
            }

            if (air_unit != nullptr && std::find(air_units.begin(), air_units.end(), t.unit) == air_units.end())
            {
                // if the unit is not in the vector
                // add it
                air_units.push_back(air_unit);
                //action->SendChat("Adding air units");
            }

            // if our current number of attacks units are enough to form a squadron
            // Command all units to attack
            if (ground_units.size() + air_units.size() >= troopManager->getSquadronSize())
            {
                // check that none of the units in the attack units are dead
                allAlive(ground_units);
                allAlive(air_units);
                action->UnitCommand(ground_units, t.ability_id, t.position);
                //action->SendChat("Moving ground units");

                auto size = ground_units.size();
                action->UnitCommand(air_units, t.ability_id, ground_units[size - 1]->pos);
                ground_units.clear();
                air_units.clear();
                troopManager->mark_location_visited();
            }

            // If there are a lot of units in our army, increase squadron size
            // if (observation->GetArmyCount() - troopManager->getSquadronSize() > 20)
            // {
            //     troopManager->incSquadronSize();
            // }
            break;
        }
        case REPAIR:
        {
            resource->addTask(t);
            break;
        }
        case UPGRADE:
        {
            resource->addTask(t);
            break;
        }
        case MOVE:
        {
            action->UnitCommand(observation->GetUnit(t.target), t.ability_id, t.position);
            break;
        }
        case TRANSFER:
        {
            TF_unit unit = TF_unit(observation->GetUnit(t.self)->unit_type.ToType(), t.self);

            // add to correct agent
            switch (t.source)
            {
            case DEFENCE_AGENT:
                defence->addUnit(unit);
                break;
            case RESOURCE_AGENT:
                resource->addUnit(unit);
                break;
            case SCOUT_AGENT:
                scout->addUnit(unit);
                break;
            default:
                std::cerr << "TRANSFER to invalid agent requested!" << std::endl;
                return;
            }

            // and remove from units
            for (auto it = units.cbegin(); it != units.cend(); ++it)
            {
                if (*it == unit)
                {
                    units.erase(it);
                    break;
                }
            }
            break;
        }
        default:
        {
            std::cerr << "RESOURCE Unrecognized Task: " << t.source << " " << t.action << std::endl;
        }
        }
    }
}

void ATTACK_BOT::addTask(Task t)
{
    task_queue.push(t);
}

void ATTACK_BOT::addUnit(TF_unit u)
{
    units.push_back(u);
}

void ATTACK_BOT::buildingConstructionComplete(const sc2::Unit *u)
{
    switch (u->unit_type.ToType())
    {
    case UNIT_TYPEID::TERRAN_BARRACKS:
    case UNIT_TYPEID::TERRAN_FACTORY:
    case UNIT_TYPEID::TERRAN_STARPORT:
        buildAddOn(u);
        break;

    default:
        break;
    }
}

void ATTACK_BOT::unitDestroyed(const sc2::Unit *u)
{
    for (auto it = units.cbegin(); it != units.cend(); ++it)
    {
        if (it->tag == u->tag)
        {
            units.erase(it);
            return;
        }
    }
}

void ATTACK_BOT::unitCreated(const sc2::Unit *u)
{
    // this is  where we want to check for building pre-requisites and try to build them.

    // for now, only allow this many barracks/factories/starports -> should have more complex conditions
    int command_count = observation->GetUnits(Unit::Alliance::Self, IsCommandCenter()).size();
    int barracks_count = observation->GetUnits(Unit::Alliance::Self, IsBarracks()).size();
    int factory_count = observation->GetUnits(Unit::Alliance::Self, IsFactory()).size();
    int starport_count = observation->GetUnits(Unit::Alliance::Self, IsStarport()).size();
    if (barracks_count < 1 + (2 * command_count))
    {

        buildBarracks();
    }

    if (factory_count < 2 * command_count)
    {
        buildFactory();
    }

    if (starport_count < 1 * command_count)
    {
        buildStarport();
    }
}

void ATTACK_BOT::unitEnterVision(const sc2::Unit *u)
{
}

void ATTACK_BOT::unitIdle(const sc2::Unit *u)
{
    troopManager->unitIdle(u);
}

void ATTACK_BOT::upgradeCompleted(sc2::UpgradeID uid)
{
}

void ATTACK_BOT::setAgents(TF_Agent *defenceb, TF_Agent *resourceb, TF_Agent *scoutb)
{
    this->defence = defenceb;
    this->resource = resourceb;
    this->scout = scoutb;
}

void ATTACK_BOT::buildBarracks()
{
    // Prereqs of building barrack: Supply Depot
    if (troopManager->CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1)
    {
        return;
    }
    else
    {
        resource->addTask(
            Task(BUILD,
                 ATTACK_AGENT,
                 5, // we want to prioritize building units over buildings
                 UNIT_TYPEID::TERRAN_BARRACKS,
                 ABILITY_ID::BUILD_BARRACKS));
    }
}

void ATTACK_BOT::buildFactory()
{
    //  Prereqs of building factory: Barracks
    if (troopManager->CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) < 1)
    {
        buildBarracks();
    }
    else
    {
        resource->addTask(
            Task(BUILD,
                 ATTACK_AGENT,
                 5,
                 UNIT_TYPEID::TERRAN_FACTORY,
                 ABILITY_ID::BUILD_FACTORY));
    }
}

void ATTACK_BOT::buildStarport()
{
    // Prereqs of building starport: Factory
    if (troopManager->CountUnitType(UNIT_TYPEID::TERRAN_FACTORY) < 1)
    {
        buildFactory();
    }
    else
    {
        resource->addTask(
            Task(BUILD,
                 ATTACK_AGENT,
                 5,
                 UNIT_TYPEID::TERRAN_STARPORT,
                 ABILITY_ID::BUILD_STARPORT));
    }
}

void ATTACK_BOT::buildAddOn(const Unit *u)
{
    // proof of concept, build a reactor on each barracks (should not actually use priority 8)

    // TODO: For now all add-ons have been set to default, will need to implement a way to choose the add - on

    if (u->build_progress != 1)
    {
        return;
    }

    switch (u->unit_type.ToType())
    {
    case UNIT_TYPEID::TERRAN_BARRACKS:
        if (observation->GetUnits(Unit::Alliance::Self, IsBarracks()).size() - 1 % 4 == 0) { // build a reactor first
            resource->addTask(Task(TRAIN, ATTACK_AGENT, 7, UNIT_TYPEID::TERRAN_BARRACKS,
                ABILITY_ID::BUILD_REACTOR_BARRACKS, u->tag));
        }
        else {
            resource->addTask(Task(TRAIN, ATTACK_AGENT, 7, UNIT_TYPEID::TERRAN_BARRACKS,
                ABILITY_ID::BUILD_TECHLAB_BARRACKS, u->tag));
        }

    case UNIT_TYPEID::TERRAN_FACTORY:
        resource->addTask(
            Task(TRAIN, ATTACK_AGENT, 7, UNIT_TYPEID::TERRAN_FACTORY, ABILITY_ID::BUILD_TECHLAB_FACTORY,
                 u->tag));

    case UNIT_TYPEID::TERRAN_STARPORT:
        resource->addTask(
            Task(TRAIN, ATTACK_AGENT, 7, UNIT_TYPEID::TERRAN_STARPORT, ABILITY_ID::BUILD_TECHLAB_STARPORT,
                 u->tag));
    }
}

void ATTACK_BOT::allAlive(std::vector<const Unit *> attack_units)
{
    for (int i = 0; i < attack_units.size(); ++i)
    {
        if (!attack_units[i]->is_alive)
        {
            auto position = attack_units.begin() + i;
            attack_units.erase(position);
        }
    }
}

// void ATTACK_BOT::slowestUnit(Units attack_units)
// {
//     UnitTypeData ut = observation->GetUnitTypeData()[(UnitTypeID)t.unit_typeid];
//     auto slowestUnit = attack_units.end();
//     for (auto unit : attack_units)
//     {
//         UnitTypeData()
//         if (unit->unit_type.)
//     }
// }

std::vector<Spotted_Enemy> ATTACK_BOT::last_seen_near(Point2D location, int radius, int since)
{
    return scout->last_seen_near(location, radius, since);
}
