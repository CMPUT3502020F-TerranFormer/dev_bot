//
// Created by Kerry Cao on 2020-10-22.
//

#ifndef CPP_SC2_TF_AGENT_HPP
#define CPP_SC2_TF_AGENT_HPP

#include <sc2api/sc2_api.h>

#include <vector>
#include "threadsafe_priority_queue.h"
#include "TS_Queue.hpp"
#include "TF_unit.hpp"
#include "Task.hpp"
#include "Action.hpp"
#include "sc2api/sc2_api.h"

/**
 * Base TF_Agent class
 */
using namespace sc2;

class TF_Agent {
public:
    TF_Agent(const ObservationInterface* obs, TSqueue<Action>* act, const QueryInterface* query)
        : observation(obs), action_queue(act), query(query)
    {}
    /**
     * Virtual Destructor
     */
    virtual ~TF_Agent() {}

    /**
     * Do actions base on game info provided
     * @param gi sc2::GameInfo
     */
    virtual void step() = 0;

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

    /**
     * Called when a building is completed
     * @param u The constructed unit
     */
    virtual void buildingConstructionComplete(const sc2::Unit* u) = 0;

    /**
     * Update the units associated with the agent
     * When OnUnitDestroyed() is called by the Agent,
     * this function is called, and if the unit deleted
     * is contained by units, then remove it
     * @param u pointer to unit destroyed
     */
    virtual void unitDestroyed(const sc2::Unit* u) = 0;

    /**
     * Communication with the bot
     * Create a TF_unit for the agent
     * @param u A pointer to the unit created
     */
    virtual void unitCreated(const sc2::Unit* u) = 0;

    /**
     * Called from the bot when an enemy unit enters vision from FOW
     * @param u The unit entering vision
     */
    virtual void unitEnterVision(const sc2::Unit* u) = 0;

    /**
     * Called from the bot when a unit is idle
     * @param u The unit idleing
     */
    virtual void unitIdle(const sc2::Unit* u) = 0;

    /**
     * Called from the bot when an upgrade is completed
     * @param uid The UpgradeID of the upgrade
     */
    virtual void upgradeCompleted(sc2::UpgradeID uid) = 0;

protected:
    threadsafe_priority_queue<Task> task_queue;
    const ObservationInterface* observation;
    TSqueue<Action> action_queue;
    const QueryInterface* query;
};

#endif //CPP_SC2_TF_AGENT_HPP
