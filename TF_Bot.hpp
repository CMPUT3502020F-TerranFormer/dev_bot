//
// Created by Carter Sabadash on 2020-10-24
//
<<<<<<< HEAD
#ifndef TF_Bot_HPP
#define TF_Bot_HPP
=======
#ifndef TF_BOT_HPP
#define TF_BOT_HPP
>>>>>>> origin

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include <iostream>
#include <vector>
#include "threadsafe_priority_queue.h"

<<<<<<< HEAD
#include <vector>
#include "threadsafe_priority_queue.h"
#include "TS_Queue.hpp"
=======
#include "BaseManager.hpp"
>>>>>>> origin
#include "TF_unit.hpp"
#include "Task.hpp"
#include "BuildingPlacementManager.hpp"

/**
 * Base TF_Agent class, forward declaration is needed and they are declared together
 * as in the implementation they both reference each other's methods
 */
using namespace sc2;

class RESOURCE_BOT;
class ATTACK_BOT;
class SCOUT_BOT;
class DEFENCE_BOT;

class TF_Bot : public Agent 
{
public:
    TF_Bot();

    ~TF_Bot();

    virtual void OnGameStart() final;

    virtual void OnGameEnd() final;

    virtual void OnStep() final;

    virtual void OnUnitDestroyed(const Unit* unit) final;

    virtual void OnUnitCreated(const Unit* unit) final;

    virtual void OnUnitIdle(const Unit* unit) final;

    virtual void OnUpgradeCompleted(UpgradeID uid) final;

    virtual void OnBuildingConstructionComplete(const Unit* unit) final;

    virtual void OnUnitEnterVision(const Unit* unit) final;

private:
<<<<<<< HEAD
    ATTACK_BOT* attack;
    DEFENCE_BOT* defence;
    RESOURCE_BOT* resource;
    SCOUT_BOT* scout;

};

class TF_Agent {
public:
    TF_Agent(TF_Bot* bot)
        : bot(bot)
    {
        observation = bot->Observation();
        action = bot->Actions();
        query = bot->Query();
    }
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
    TF_Bot* bot;
    const ObservationInterface* observation;
    ActionInterface* action;
    QueryInterface* query;
    BuildingPlacementManager* buildingPlacementManager;
=======
    #include "resource.hpp"
    #include "defence.hpp"
    #include "attack.hpp"
    #include "scout.hpp"
    #include "utility.hpp"
>>>>>>> origin
};

#endif