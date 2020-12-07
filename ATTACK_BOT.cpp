//
// Created by Carter Sabadash on 2020-10-24
//
#include "ATTACK_BOT.hpp"
#include <iostream>
#include <string>

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
            // if a tank is in siege mode, unsiege them
            if (t.unit->unit_type == UNIT_TYPEID::TERRAN_SIEGETANKSIEGED)
            {
                action->UnitCommand(t.unit, ABILITY_ID::MORPH_UNSIEGE);
            }

            if (t.unit->unit_type == UNIT_TYPEID::TERRAN_BANSHEE)
            {
                action->UnitCommand(t.unit, ABILITY_ID::BEHAVIOR_CLOAKON);
            }

            // Classify the units in flying or ground
            // to facilitate movement
            // Because air units will travel faster than the ground units
            const Unit *air_unit = nullptr;
            const Unit *ground_unit = nullptr;
            const Unit *support_unit = nullptr;

            if (t.unit->is_flying)
            {
                if (IsAFlyingAttackUnit(t.unit->unit_type))
                {
                    air_unit = t.unit;
                    if (air_unit != nullptr && IsNotInVector(air_units, air_unit))
                    {
                        // if the unit is not in the vector
                        // add it
                        air_units.push_back(air_unit);
                    }
                }
                else if (IsASupportUnit(t.unit->unit_type))
                {
                    support_unit = t.unit;
                    if (support_unit != nullptr && IsNotInVector(support_units, support_unit))
                    {
                        // if the unit is not in the vector
                        // add it
                        support_units.push_back(support_unit);
                    }
                }
            }
            else
            {
                ground_unit = t.unit;
                if (ground_unit != nullptr && IsNotInVector(ground_units, ground_unit))
                {
                    // if the unit is not in the vector
                    // add it
                    ground_units.push_back(ground_unit);
                }
            }
            // if our current number of attacks units are enough to form a squadron
            // Command all units to attack
            if (ground_units.size() + air_units.size() >= troopManager->getSquadronSize())
            {
                if (!IsPositionValid(t.position))
                {
                    return;
                }

                // check that none of the units in the attack units are dead
                allAlive(ground_units);
                action->UnitCommand(ground_units, t.ability_id, t.position);

                allAlive(air_units);
                allAlive(support_units);
                slowest_unit = slowestUnit(ground_units);
                if (slowest_unit == nullptr)
                {
                    return;
                }
                // Point2D su_pos(slowest_unit->pos.x, slowest_unit->pos.y); // position of the slowest unit
                action->UnitCommand(air_units, t.ability_id, t.position);
                action->UnitCommand(support_units, ABILITY_ID::MOVE_MOVE, t.position);
                // air_units.clear();
                // support_units.clear();
                // std::cout << "Target coordinates are " + std::to_string(t.position.x) + " " + std::to_string(t.position.y) << std::endl;

                ground_units.clear();
            }

            // If there are a lot of units in our army, increase squadron size
            // if ((observation->GetArmyCount() - troopManager->getSquadronSize()) >= 15)
            // {
            //     troopManager->incSquadronSize();
            //     action->SendChat("Army count: " + std::to_string(observation->GetArmyCount()));
            //     action->SendChat("Squadron size incremented");
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

void ATTACK_BOT::allAlive(std::vector<const Unit *> attack_units)
{
    if (!attack_units.empty())
    {
        for (int i = 0; i < attack_units.size(); ++i)
        {
            if (!attack_units[i]->is_alive || attack_units[i] == nullptr)
            {
                auto position = attack_units.begin() + i;
                attack_units.erase(position);
            }
        }
    }
}

bool ATTACK_BOT::IsAFlyingAttackUnit(UNIT_TYPEID unit_typeid)
{
    switch (unit_typeid)
    {
    case UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
    case UNIT_TYPEID::VIKING:
    case UNIT_TYPEID::TERRAN_LIBERATOR:
    case UNIT_TYPEID::TERRAN_BANSHEE:
    case UNIT_TYPEID::TERRAN_BATTLECRUISER:
        return true;
    }
    return false;
}

bool ATTACK_BOT::IsASupportUnit(UNIT_TYPEID unit_typeid)
{
    switch (unit_typeid)
    {
    case UNIT_TYPEID::TERRAN_RAVEN:
    case UNIT_TYPEID::TERRAN_MEDIVAC:
        return true;
    }
    return false;
}

const Unit *ATTACK_BOT::slowestUnit(Units attack_units)
{
    const Unit *slowestUnit = nullptr;
    if (!attack_units.empty())
    {
        UnitTypeData ut = observation->GetUnitTypeData()[(UnitTypeID)attack_units[0]->unit_type];
        slowestUnit = attack_units[0];

        float min_speed = ut.movement_speed;
        float current_speed = ut.movement_speed;
        int unit_idx = 0;

        for (int i = 0; i < attack_units.size(); ++i)
        {
            ut = observation->GetUnitTypeData()[(UnitTypeID)attack_units[i]->unit_type];
            current_speed = ut.movement_speed;
            if (current_speed < min_speed && attack_units[i]->unit_type != UNIT_TYPEID::TERRAN_SIEGETANKSIEGED)
            {
                min_speed = current_speed;
                slowestUnit = attack_units[i];
            }
        }
    }

    return slowestUnit;
}

bool ATTACK_BOT::IsNotInVector(Units vector, const Unit *u)
{
    return std::find(vector.begin(), vector.end(), u) == vector.end();
}

bool ATTACK_BOT::IsPositionValid(Point2D target_pt)
{
    int map_height = observation->GetGameInfo().height;
    int map_width = observation->GetGameInfo().width;
    
    if (target_pt.x <= 0  && target_pt.x >= map_width)
    {
        return false;
    }

    if (target_pt.y <= 0 && target_pt.y >= map_width)
    {
        return false;
    }
    return true;
}

std::vector<Spotted_Enemy> ATTACK_BOT::last_seen_near(Point2D location, int radius, int since)
{
    return scout->last_seen_near(location, radius, since);
}
