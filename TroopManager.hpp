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
#include "utility.hpp"

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
            // barracks train marine
            // TODO: possibly switch to Marauders if we already have a sufficient amount of Marines
            //
            // to train marauders, check for the presence of a tech lab first
        case UNIT_TYPEID::TERRAN_BARRACKS:
        {
            if (unit->add_on_tag == 0) {
                if (observation->GetUnits(Unit::Alliance::Self, IsBarracks()).size() - 1 % 4 == 0) { // build a reactor first
                    task_queue->push(Task(TRAIN, ATTACK_AGENT, 8, UNIT_TYPEID::TERRAN_BARRACKS,
                        ABILITY_ID::BUILD_REACTOR_BARRACKS, unit->tag));
                }
                else {
                    task_queue->push(Task(TRAIN, ATTACK_AGENT, 7, UNIT_TYPEID::TERRAN_BARRACKS,
                        ABILITY_ID::BUILD_TECHLAB_BARRACKS, unit->tag));
                }
            }

            if (CountUnitType(UNIT_TYPEID::TERRAN_MARINE) < 30)
            {
                // if there is a reactor it will train both at once
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 6, ABILITY_ID::TRAIN_MARINE, UNIT_TYPEID::TERRAN_MARINE,
                    UNIT_TYPEID::TERRAN_BARRACKS, unit->tag));
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
        case UNIT_TYPEID::TERRAN_STARPORT:
        {
            if (unit->add_on_tag == 0) {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 7, UNIT_TYPEID::TERRAN_STARPORT, 
                    ABILITY_ID::BUILD_TECHLAB_STARPORT, unit->tag));
            }
            // Anti Marines and Tanks
            if (CountUnitType(UNIT_TYPEID::TERRAN_BANSHEE) < 4)
            {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 7, ABILITY_ID::TRAIN_BANSHEE, UNIT_TYPEID::TERRAN_BANSHEE,
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
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 6, ABILITY_ID::TRAIN_RAVEN, UNIT_TYPEID::TERRAN_RAVEN,
                                      UNIT_TYPEID::TERRAN_STARPORT, unit->tag));
            }

            // Healer
            if (CountUnitType(UNIT_TYPEID::TERRAN_MEDIVAC) < 2)
            {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 6, ABILITY_ID::TRAIN_MEDIVAC, UNIT_TYPEID::TERRAN_MEDIVAC,
                                      UNIT_TYPEID::TERRAN_STARPORT, unit->tag));
            }

            break;
        }
        case UNIT_TYPEID::TERRAN_FACTORY:
        {
            if (unit->add_on_tag == 0) {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 8, UNIT_TYPEID::TERRAN_FACTORY, 
                    ABILITY_ID::BUILD_TECHLAB_FACTORY, unit->tag));
            }
            if (CountUnitType(UNIT_TYPEID::TERRAN_SIEGETANK) < 5)
            {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 6, ABILITY_ID::TRAIN_SIEGETANK, UNIT_TYPEID::TERRAN_SIEGETANK, UNIT_TYPEID::TERRAN_FACTORY, unit->tag));
            }
            break;
        }
        case UNIT_TYPEID::TERRAN_MARINE:
        case UNIT_TYPEID::TERRAN_SIEGETANK:
        case UNIT_TYPEID::TERRAN_SIEGETANKSIEGED:
        case UNIT_TYPEID::TERRAN_BANSHEE:
        case UNIT_TYPEID::TERRAN_MARAUDER:
        case UNIT_TYPEID::TERRAN_THOR:
        case UNIT_TYPEID::TERRAN_BATTLECRUISER:
        case UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
        {
            updateEnemyLocations();
            task_queue->push(Task(ATTACK, ATTACK_AGENT, 6, unit, ABILITY_ID::ATTACK_ATTACK, enemy_locations.back()));
            mark_location_visited();
        }

        // Support Units don't attack, so gotta give them a different ABILITY_ID
        case UNIT_TYPEID::TERRAN_MEDIVAC:
        case UNIT_TYPEID::TERRAN_RAVEN:
        {
            updateEnemyLocations();
            task_queue->push(Task(ATTACK, ATTACK_AGENT, 6, unit, ABILITY_ID::MOVE_MOVE, enemy_locations.back()));
            break;
        }
        }
    }

    void updateEnemyLocations() {
        if (possible_enemy_locations.size() == 0)
        {
            auto enemies = observation->GetUnits(Unit::Alliance::Enemy, IsBuilding(observation->GetUnitTypeData()));
            if (!enemies.empty()) { 
                for (auto& e : enemies) {
                    possible_enemy_locations.emplace_back(e->pos.x, e->pos.y);
                }
            }
            else { possible_enemy_locations = observation->GetGameInfo().enemy_start_locations; }
        }

        if (enemy_locations.size() == 0)
        {
            // Locations of enemies spotted by the scouting agent anywhere on the map within the last 2 minutes
            auto enemy_record = scout->last_seen_near(possible_enemy_locations.back(), 15, 150);
            if (enemy_record.size() > 0)
            {
                for (auto& record : enemy_record)
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
    }

    // helps count the number of units present in the current game state
    // note that it does not account for variations caused by add-ons
    int CountUnitType(UnitTypeID unit_type)
    {
        return observation->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
    }

    int getSquadronSize()
    {
        // grows at 2.5 units /min, max 50, because later on units take more supply
        auto size = 2.5f * (observation->GetGameLoop() / 16 / 60);
        if (size > 50) { return 50; }
        return size;
    }

    void mark_location_visited()
    {
        if (enemy_locations.empty()) { return; }
        Point2D current_location = enemy_locations.back();
        IsClose enemy_nearby(current_location, 100);
        std::vector<const Unit *> enemy_units = observation->GetUnits(Unit::Alliance::Enemy, enemy_nearby);

        if (enemy_units.size() == 0)
        {
            enemy_locations.pop_back();
        }
        // else
        // {
        //     int rndInt = GetRandomInteger(0, enemy_units.size() - 1);
        //     const Unit *rndEnemy = enemy_units[rndInt];
        //     Point2D enemy_pos(rndEnemy->pos.x, rndEnemy->pos.y);
        //     enemy_locations.push_back(enemy_pos);
        // }
    }

private:
    threadsafe_priority_queue<Task> *task_queue;
    const ObservationInterface *observation;
    TF_Agent *scout;
    std::vector<Point2D> possible_enemy_locations;
    std::vector<Point2D> enemy_locations;
    int counter = 1;
};

#endif