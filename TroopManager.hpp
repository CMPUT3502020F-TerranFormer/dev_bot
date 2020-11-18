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
    TroopManager(threadsafe_priority_queue<Task> *t_queue, const ObservationInterface *obs)
        : task_queue(t_queue), observation(obs)
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
        case UNIT_TYPEID::TERRAN_BARRACKSREACTOR: {
            if (CountUnitType(UNIT_TYPEID::TERRAN_MARINE) < 25)
            {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 7, ABILITY_ID::TRAIN_MARINE, UNIT_TYPEID::TERRAN_MARINE,
                    UNIT_TYPEID::TERRAN_BARRACKS, unit->tag));
            }
            break;
        }
        case UNIT_TYPEID::TERRAN_BARRACKSTECHLAB: {
            if (CountUnitType(UNIT_TYPEID::TERRAN_MARINE) < 25)
            {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 7, ABILITY_ID::TRAIN_MARINE, UNIT_TYPEID::TERRAN_MARINE,
                    UNIT_TYPEID::TERRAN_BARRACKS, unit->tag));
            }
            if (CountUnitType(UNIT_TYPEID::TERRAN_MARAUDER) < 30)
            {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 7, ABILITY_ID::TRAIN_MARAUDER, UNIT_TYPEID::TERRAN_MARAUDER,
                    UNIT_TYPEID::TERRAN_BARRACKS, unit->tag));
            }
            break;
        }
        case UNIT_TYPEID::TERRAN_BARRACKS:
        {
            if (CountUnitType(UNIT_TYPEID::TERRAN_MARINE) < 25)
            {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 7, ABILITY_ID::TRAIN_MARINE, UNIT_TYPEID::TERRAN_MARINE,
                                      UNIT_TYPEID::TERRAN_BARRACKS, unit->tag));
            }
            break;
        }
        case UNIT_TYPEID::TERRAN_STARPORTREACTOR:
        case UNIT_TYPEID::TERRAN_STARPORTTECHLAB:
        case UNIT_TYPEID::TERRAN_STARPORT:
        {
            if (CountUnitType(UNIT_TYPEID::TERRAN_BANSHEE) < 10) {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 5, ABILITY_ID::TRAIN_BANSHEE, UNIT_TYPEID::TERRAN_BANSHEE,
                    UNIT_TYPEID::TERRAN_STARPORT, unit->tag));
            }
            if (CountUnitType(UNIT_TYPEID::TERRAN_MEDIVAC) < 10) {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 5, ABILITY_ID::TRAIN_MEDIVAC, UNIT_TYPEID::TERRAN_MEDIVAC,
                    UNIT_TYPEID::TERRAN_STARPORT, unit->tag));
            }
            break;
        }

        case UNIT_TYPEID::TERRAN_MARINE:
        case UNIT_TYPEID::TERRAN_MEDIVAC:
        {
            // TODO: Problems with the implementation; One unit for one location
            //                task_queue->push(
            //                        Task(ATTACK, ATTACK_AGENT, 7, unit, ABILITY_ID::ATTACK_ATTACK, enemy_locations.back()));
            //                enemy_locations.pop_back();
            break;
        }
        }
    }

    // helps count the number of units present in the current game state
    // note that it does not account for variations caused by add-ons
    int CountUnitType(UnitTypeID unit_type) {
        return observation->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
    }

private:
    threadsafe_priority_queue<Task> *task_queue;
    const ObservationInterface *observation;
    std::vector<Point2D> enemy_locations;
};

#endif