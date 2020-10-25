//
// Created by Kerry Cao on 2020-10-22.
//

#ifndef CPP_SC2_TF_AGENT_HPP
#define CPP_SC2_TF_AGENT_HPP

#include <sc2api/sc2_api.h>

#include <vector>
#include "threadsafe_priority_queue.h"
#include "TS_Queue.hpp"

#include "Task.hpp"

/**
 * Data class for a unit belonging to a agent
 */
struct TF_unit {
    TF_unit(sc2::UNIT_TYPEID type, sc2::Tag tag)
        : type(type), tag(tag)
    {}
    sc2::UNIT_TYPEID type;
    sc2::Tag tag;
};

/**
 * Data classes for unit commands, we do not use queued_commands
 */

struct BasicCommand // data for a command to itself
{
    BasicCommand(const sc2::Unit* u, sc2::AbilityID aid)
        : unit(u), ability(aid)
    {}
    const sc2::Unit* unit;
    sc2::AbilityID ability;
};

struct MoveCommand : public BasicCommand // data for a command to move
{
    MoveCommand(const sc2::Unit* u, sc2::AbilityID aid, const sc2::Point2D& p)
        : BasicCommand(u, aid), point(p)
    {}
    const sc2::Point2D& point;
};

struct AttackCommand : public BasicCommand // data for a command to attack
{
    AttackCommand(const sc2::Unit* u, sc2::AbilityID aid, const sc2::Unit* target)
        : BasicCommand(u, aid), target(target)
    {}
    const sc2::Unit* target;
};

/**
 * Base TF_Agent class
 */
class TF_Agent {
public:
    TF_Agent(TSqueue<BasicCommand>* a_queue) 
        : action_queue(a_queue)
    {}
    /**
     * Virtual Destructor
     */
    virtual ~TF_Agent() {}

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
    std::vector<TF_unit> units;
    threadsafe_priority_queue<Task> task_queue;
    TSqueue<BasicCommand> *action_queue;
};

#endif //CPP_SC2_TF_AGENT_HPP
