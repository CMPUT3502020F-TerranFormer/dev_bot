//
// Created by Kerry Cao on 2020-10-23.
//

#ifndef CPP_SC2_DEFENCE_BOT_HPP
#define CPP_SC2_DEFENCE_BOT_HPP

#include "TF_Agent.hpp"
#include "Task.hpp"

class DEFENCE_BOT final : public TF_Agent {
public:
    DEFENCE_BOT(TSqueue<BasicCommand>* a_queue);

    ~DEFENCE_BOT();

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

    void setAgents(const TF_Agent* attackb, const TF_Agent* resourceb, const TF_Agent* scoutb);

private:
    const TF_Agent *attack;
    const TF_Agent *resource;
    const TF_Agent *scout;
};

#endif //CPP_SC2_DEFENCE_BOT_HPP
