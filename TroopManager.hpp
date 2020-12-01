#ifndef TROOPMANAGER_HPP
#define TROOPMANAGER_HPP

//
// Created by Gary Ng on 1st Nov
//

#include "Task.hpp"
#include "threadsafe_priority_queue.h"
#include <vector>
#include "TF_unit.hpp"
#include <iostream>

/**
 * The purpose of this class is to train troops and allow for upgrades at training buildings.
 */

using namespace sc2;

class TroopManager
{
public:
    TroopManager(threadsafe_priority_queue<Task> *t_queue, const ObservationInterface *obs, TF_Agent *scout)
        : task_queue(t_queue), observation(obs), scout(scout)
    {
        enemy_locations = observation->GetGameInfo().enemy_start_locations; // positions of possible enemy starting locations
    }

    void unitIdle(const Unit *unit)
    {
        // if unit is idle,
        switch (unit->unit_type.ToType())
        {
            // barracks train marine
            // TODO: possibly switch to Marauders if we already have a sufficient amount of Marines
            //
            // to train marauders, check for the presence of a tech lab first
        case UNIT_TYPEID::TERRAN_BARRACKSREACTOR:
        case UNIT_TYPEID::TERRAN_BARRACKSTECHLAB:
        case UNIT_TYPEID::TERRAN_BARRACKS:
        {
            if (CountUnitType(UNIT_TYPEID::TERRAN_MARINE) < 30)
            {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 6, ABILITY_ID::TRAIN_MARINE, UNIT_TYPEID::TERRAN_MARINE,
                                      UNIT_TYPEID::TERRAN_BARRACKS, unit->tag));
            }

            if (CountUnitType(UNIT_TYPEID::TERRAN_MARAUDER) < 10)
            {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 6, ABILITY_ID::TRAIN_MARAUDER, UNIT_TYPEID::TERRAN_MARAUDER,
                                      UNIT_TYPEID::TERRAN_BARRACKS, unit->tag));
            }
            break;
        }
        case UNIT_TYPEID::TERRAN_STARPORTREACTOR:
        case UNIT_TYPEID::TERRAN_STARPORTTECHLAB:
        case UNIT_TYPEID::TERRAN_STARPORT:
        {
            // Anti Marines and Tanks
            if (CountUnitType(UNIT_TYPEID::TERRAN_BANSHEE) < 4)
            {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 6, ABILITY_ID::TRAIN_BANSHEE, UNIT_TYPEID::TERRAN_BANSHEE,
                                      UNIT_TYPEID::TERRAN_STARPORT, unit->tag));
            }

            // Anti air units
            if (CountUnitType(UNIT_TYPEID::TERRAN_VIKINGFIGHTER) < 4)
            {
               task_queue->push(Task(TRAIN, ATTACK_AGENT, 6, ABILITY_ID::TRAIN_VIKINGFIGHTER, UNIT_TYPEID::TERRAN_VIKINGFIGHTER,
                                      UNIT_TYPEID::TERRAN_STARPORT, unit->tag));
            }
            
            // Detector troops
            if (CountUnitType(UNIT_TYPEID::TERRAN_RAVEN) < 2)
            {
                 task_queue->push(Task(TRAIN, ATTACK_AGENT, 7, ABILITY_ID::TRAIN_RAVEN, UNIT_TYPEID::TERRAN_RAVEN,
                                      UNIT_TYPEID::TERRAN_STARPORT, unit->tag));
            }

            // Healer
            if (CountUnitType(UNIT_TYPEID::TERRAN_MEDIVAC) < 2)
            {
                 task_queue->push(Task(TRAIN, ATTACK_AGENT, 7, ABILITY_ID::TRAIN_MEDIVAC, UNIT_TYPEID::TERRAN_MEDIVAC,
                                      UNIT_TYPEID::TERRAN_STARPORT, unit->tag));
            }

            break;
        }
        case UNIT_TYPEID::TERRAN_FACTORY:
        case UNIT_TYPEID::TERRAN_FACTORYTECHLAB:
        case UNIT_TYPEID::TERRAN_FACTORYREACTOR:
        {
            if (CountUnitType(UNIT_TYPEID::TERRAN_SIEGETANK) < 5)
            {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 6, ABILITY_ID::TRAIN_SIEGETANK, UNIT_TYPEID::TERRAN_SIEGETANK, UNIT_TYPEID::TERRAN_FACTORY, unit->tag));
            }
            break;
        }

        case UNIT_TYPEID::TERRAN_MARINE:
        case UNIT_TYPEID::TERRAN_SIEGETANK:
        case UNIT_TYPEID::TERRAN_BANSHEE:
        case UNIT_TYPEID::TERRAN_RAVEN:
        case UNIT_TYPEID::TERRAN_MARAUDER:
        case UNIT_TYPEID::TERRAN_MEDIVAC:
        case UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
        {
            if (CountUnitType(UNIT_TYPEID::TERRAN_MARINE) > 20 && CountUnitType(UNIT_TYPEID::TERRAN_MARAUDER) > 5)
            {
                if (enemy_locations.size() == 0)
                {
                    // Locations of enemies spotted by the scouting agent anywhere on the map within the last 10 seconds
                    for (auto &record : scout->last_seen_near(sc2::Point2D(0, 0), std::numeric_limits<int>::max(), 10))
                    {
                        enemy_locations.push_back(record.location);
                    }
                }
                Point2D enemy_loc = enemy_locations.back();
                task_queue->push(
                    Task(ATTACK, ATTACK_AGENT, 6, unit, ABILITY_ID::ATTACK_ATTACK, enemy_locations.back()));

                // Tried to limit choke points by designing an area as accepted rather than a point
                if (abs(unit->pos.x - enemy_loc.x) < 5 && abs(unit->pos.y - enemy_loc.y) < 5)
                {
                    enemy_locations.pop_back();
                }
            }
            break;
        }

            // Support Units don't attack, so gotta give them a differnt ABILITY_ID
            // case UNIT_TYPEID::TERRAN_MEDIVAC:
            // {
            //     if (CountUnitType(UNIT_TYPEID::TERRAN_MARINE) > 20)
            //     {
            //         Point2D enemy_loc = enemy_locations.back();
            //         task_queue->push(
            //             Task(ATTACK, ATTACK_AGENT, 5, unit, ABILITY_ID::EFFECT_HEAL, enemy_locations.back()));
            //         if (abs(unit->pos.x - enemy_loc.x) < 5 && abs(unit->pos.y - enemy_loc.y) < 5)
            //         {
            //             enemy_locations.pop_back();
            //         }
            //     }
            //     break;
            // }
        }
    }

    // helps count the number of units present in the current game state
    // note that it does not account for variations caused by add-ons
    int CountUnitType(UnitTypeID unit_type)
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

private:
    threadsafe_priority_queue<Task> *task_queue;
    const ObservationInterface *observation;
    TF_Agent *scout;
    std::vector<Point2D> enemy_locations;
};

#endif