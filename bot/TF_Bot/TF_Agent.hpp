//
// Created by Kerry Cao on 2020-10-22.
//

#ifndef CPP_SC2_TF_AGENT_HPP
#define CPP_SC2_TF_AGENT_HPP

#include <sc2api/sc2_api.h>

#include <vector>
#include <queue>

#include "Task.hpp"

/**
 * Data class for a unit belonging to a agent
 */
class TF_unit {
    sc2::UNIT_TYPEID type;
    sc2::Tag tag;
};

/**
 * Base TF_Agent class
 */
class TF_Agent {
public:
    /**
     * Virtual Destructor
     */
    virtual ~TF_Agent() {

    }

    /**
     * Do actions base on game info provided
     * @param gi sc2::GameInfo
     */
    virtual void step(const sc2::GameInfo &gi) = 0;

    /**
     * Cross agent communication
     * Add a task to the task_queue
     * @param t Task
     */
    virtual void addTask(Task t) = 0;

    /**
     * Cross agent communication
     * Assign a TF_unit to the agent
     * @param u TF_unit
     */
    virtual void addUnit(TF_unit u) = 0;



protected:
    std::vector<TF_unit> units;
    std::priority_queue<Task> task_queue;
};

#endif //CPP_SC2_TF_AGENT_HPP
