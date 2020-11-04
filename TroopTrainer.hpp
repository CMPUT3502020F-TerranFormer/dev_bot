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

class TroopTrainer
{
public:
    TroopTrainer(threadsafe_priority_queue<Task> *t_queue, const ObservationInterface *obs)
        : task_queue(t_queue), observation(obs)
    {
    }

private:
    threadsafe_priority_queue<Task> *task_queue;
    const ObservationInterface *observation;
}

#endif