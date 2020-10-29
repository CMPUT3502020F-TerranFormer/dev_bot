//
// Created by Kerry Cao on 2020-10-23.
//

/*
 * The resource agent accepts tasks of type BUILD, TRAIN, REPAIR, UPGRADE, MOVE, TRANSFER
 * BUILD specifies the unit to be built, the position
 * TRAIN specifies the building to do the training, AID specifies the unit to make, count = 1, to train many, issue multiple tasks
 * REPAIR specifies the target unit
 * UPGRADE specifies the unit to be upgraded, AID specifies the upgrade
 * MOVE specifies the unit to be move, position to (should only be used by Defence to escort, or prevent them from being killed)
 * TRANSFER specifies the unit to be transfered
 * 
 * This agent usually operates with priority 4-6, ensure that commands interfere minally
 */

#ifndef CPP_SC2_RESOURCE_BOT_HPP
#define CPP_SC2_RESOURCE_BOT_HPP

#include "TF_Agent.hpp"
#include "Task.hpp"

class RESOURCE_BOT final : public TF_Agent {
public:
    RESOURCE_BOT(TSqueue<BasicCommand>* a_queue);

    ~RESOURCE_BOT();

    /**
     * Intializes units with the initial scv's and command center
     */
    void gameStart(const sc2::Units alliedUnits);

    /**
     * Do actions base on game info provided
     * @param gi sc2::GameInfo
     */
    void step(const sc2::GameInfo& gi) final;

    /**
     * Cross agent communication
     * Add a task to the task_queue
     * @param t Task
     */
    void addTask(Task t) final;

    /**
     * Cross agent communication
     * Assign a TF_unit to the agent
     * @param u TF_unit
     */
    void addUnit(TF_unit u) final;

    /**
     * Called when a building is completed
     * @param u The constructed unit
     */
    void buildingConstructionComplete(const sc2::Unit* u) final;

    /**
     * Called when a unit is destroyed
     * @param u The destroyed unit
     */
    void unitDestroyed(const sc2::Unit* u);

    /**
     * Communication with the bot
     * Create a TF_unit for the agent
     * @param u A pointer to the unit created
     */
    void unitCreated(const sc2::Unit* u) final;

    /**
     * Called from the bot when an enemy unit enters vision from FOW
     * @param u The unit entering vision
     */
    void unitEnterVision(const sc2::Unit* u) final;
    /**
     * Called from the bot when a unit is idle
     * @param u The unit idleing
     */
    void unitIdle(const sc2::Unit* u) final;

    /**
     * Called from the bot when an upgrade is completed
     * @param uid The UpgradeID of the upgrade
     */
    void upgradeCompleted(sc2::UpgradeID uid) final;

    void setAgents(TF_Agent* defenceb, TF_Agent* attackb, TF_Agent* scoutb);

private:
    TF_Agent *defence;
    TF_Agent *attack;
    TF_Agent *scout;
    std::vector<std::pair<SourceAgent, sc2::UNIT_TYPEID>> training;
    int supply_float = 2; // 2/4/6 with 1/2/3+ command centers
};

#endif //CPP_SC2_RESOURCE_BOT_HPP
