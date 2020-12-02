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
        possible_enemy_locations = observation->GetGameInfo().enemy_start_locations; // positions of possible enemy starting locations
    }

    void unitIdle(const Unit *unit)
    {
        // if unit is idle,
        switch (unit->unit_type.ToType())
        {
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
        case UNIT_TYPEID::TERRAN_MARAUDER:
        case UNIT_TYPEID::TERRAN_THOR:
        case UNIT_TYPEID::TERRAN_BATTLECRUISER:
        case UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
        case UNIT_TYPEID::TERRAN_SIEGETANKSIEGED:
        {
            if (squadron_units.size() > squadron_size)
            {
                if (possible_enemy_locations.size() == 0)
                {
                    possible_enemy_locations = observation->GetGameInfo().enemy_start_locations;
                }

                if (enemy_locations.size() == 0)
                {
                    // Locations of enemies spotted by the scouting agent anywhere on the map within the last 2 minutes
                    if (scout->last_seen_near(possible_enemy_locations.back(), 15, 10000).size() > 0)
                    {
                        for (auto &record : scout->last_seen_near(possible_enemy_locations.back(), 15, 10000))
                        {
                            enemy_locations.push_back(record.location);
                        }
                        possible_enemy_locations.pop_back();
                    }
                    else
                    {
                        enemy_locations.push_back(possible_enemy_locations.back());
                        possible_enemy_locations.pop_back();
                    }
                }

                all_alive(squadron_units);
                task_queue->push(Task(ATTACK, ATTACK_AGENT, 7, squadron_units, ABILITY_ID::ATTACK_ATTACK, enemy_locations.back()));
                std::cout << "Mass Attack Order sent" << std::endl;

                // Tried to limit choke points by designing an area as accepted rather than a point
                Point2D enemy_loc = enemy_locations.back();
                if (abs(unit->pos.x - enemy_loc.x) < 5 && abs(unit->pos.y - enemy_loc.y) < 5)
                {
                    enemy_locations.pop_back();
                }
            }

            // If there are more than 4 sieged tanks,
            // unsiege some of them for attacks
            if (unit->unit_type == UNIT_TYPEID::TERRAN_SIEGETANKSIEGED)
            {
                if (CountUnitType(unit->unit_type) < 4)
                {
                    return;
                }

                else
                {
                    task_queue->push(Task(ATTACK, ATTACK_AGENT, 7, unit, ABILITY_ID::MORPH_UNSIEGE, enemy_locations.back()));
                }
            }

            squadron_units.push_back(unit);
            break;
        }

            // Support Units don't attack, so gotta give them a differnt ABILITY_ID
            // case UNIT_TYPEID::TERRAN_MEDIVAC:
            // case UNIT_TYPEID::TERRAN_RAVEN:
            // {
            //     if (observation->GetArmyCount() > squadron_size)
            //     {
            //         if (possible_enemy_locations.size() == 0)
            //         {
            //             possible_enemy_locations = observation->GetGameInfo().enemy_start_locations;
            //         }

            //         if (enemy_locations.size() == 0)
            //         {
            //             // Locations of enemies spotted by the scouting agent anywhere on the map within the last 2 minutes
            //             if (scout->last_seen_near(possible_enemy_locations.back(), 15, 10000).size() > 0)
            //             {
            //                 for (auto &record : scout->last_seen_near(possible_enemy_locations.back(), 15, 10000))
            //                 {
            //                     enemy_locations.push_back(record.location);
            //                 }
            //                 possible_enemy_locations.pop_back();
            //             }
            //             else
            //             {
            //                 enemy_locations.push_back(possible_enemy_locations.back());
            //                 possible_enemy_locations.pop_back();
            //             }
            //         }

            //         task_queue->push(Task(ATTACK, ATTACK_AGENT, 6, unit, ABILITY_ID::MOVE_MOVE, enemy_locations.back()));

            //         // Tried to limit choke points by designing an area as accepted rather than a point
            //         Point2D enemy_loc = enemy_locations.back();
            //         if (abs(unit->pos.x - enemy_loc.x) < 4 && abs(unit->pos.y - enemy_loc.y) < 4)
            //         {
            //             if (unit->unit_type != UNIT_TYPEID::TERRAN_VIKINGFIGHTER)
            //             {
            //                 enemy_locations.pop_back();
            //             }
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
        return observation->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
    }

    int getSquadronSize()
    {
        return squadron_size;
    }

    int incSquadronSize()
    {
        squadron_size += 5;
    }

    void all_alive(Units army)
    {
        for (int i = 0; i < army.size(); ++i)
        {
            if (!army[i]->is_alive)
            {
                if (i > 1)
                {
                    army.erase(army.begin() + i - 1);
                }

                else if (i = 1)
                {
                    army.erase(army.begin() + i);
                }

                else
                {
                    army.erase(army.begin());
                }
            }
        }
    }

private:
    threadsafe_priority_queue<Task> *task_queue;
    const ObservationInterface *observation;
    TF_Agent *scout;
    std::vector<Point2D> possible_enemy_locations;
    std::vector<Point2D> enemy_locations;
    Units squadron_units;
    Units squadron_supp_units;
    int squadron_size = 20;
};

#endif