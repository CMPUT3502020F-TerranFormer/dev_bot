#ifndef TROOPTRAINER_HPP
#define TROOPTRAINER_HPP

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

class TroopManager {
public:
    TroopManager(threadsafe_priority_queue<Task> *t_queue, const ObservationInterface *obs)
            : task_queue(t_queue), observation(obs) {
        enemy_locations = observation->GetGameInfo().enemy_start_locations; // positions of possible enemy starting locations
    }

    void unitIdle(const Unit *unit) {
        // if unit is idle,
        switch (unit->unit_type.ToType()) {

            // barracks train marine
            // TODO: possibly switch to Marauders if we already have a sufficient amount of Marines
            //
            // to train marauders, check for the presence of a tech lab first

            case UNIT_TYPEID::TERRAN_BARRACKS: {

                if (CountUnitType(observation, UNIT_TYPEID::TERRAN_MARINE) < 25) {
                    task_queue->push(Task(TRAIN, ATTACK_AGENT, 5, ABILITY_ID::TRAIN_MARINE, UNIT_TYPEID::TERRAN_MARINE,
                                          UNIT_TYPEID::TERRAN_BARRACKS, unit->tag));
                }

                task_queue->push(Task(TRAIN, ATTACK_AGENT, 5, ABILITY_ID::TRAIN_MARAUDER, UNIT_TYPEID::TERRAN_MARAUDER,
                                      UNIT_TYPEID::TERRAN_BARRACKS, unit->tag));
                break;
            }

            case UNIT_TYPEID::TERRAN_STARPORT: {
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 5, ABILITY_ID::TRAIN_BANSHEE, UNIT_TYPEID::TERRAN_BANSHEE,
                                      UNIT_TYPEID::TERRAN_STARPORT, unit->tag));
                task_queue->push(Task(TRAIN, ATTACK_AGENT, 5, ABILITY_ID::TRAIN_MEDIVAC, UNIT_TYPEID::TERRAN_MEDIVAC,
                                      UNIT_TYPEID::TERRAN_STARPORT, unit->tag));
                break;
            }

            case UNIT_TYPEID::TERRAN_MARINE:
            case UNIT_TYPEID::TERRAN_MEDIVAC: {
                // TODO: Problems with the implementation; One unit for one location
                task_queue->push(
                        Task(ATTACK, ATTACK_AGENT, 7, unit, ABILITY_ID::ATTACK_ATTACK, enemy_locations.back()));
                enemy_locations.pop_back();
                break;
            }
        }
        if
    }

    // helps count the number of units present in the current game state
    static int CountUnitType(const ObservationInterface *obs, UnitTypeID unit_type) {
        int count = 0;
        Units my_units = obs->GetUnits(Unit::Alliance::Self);
        for (const auto unit : my_units) {
            if (unit->unit_type == unit_type)
                ++count;
        }
        return count;
    }

private:
    threadsafe_priority_queue<Task> *task_queue;
    const ObservationInterface *observation;
    std::vector<Point2D> enemy_locations;
};

#endif