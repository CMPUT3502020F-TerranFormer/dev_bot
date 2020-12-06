//
// Created by Kerry Cao on 2020-10-23.
//

#ifndef CPP_SC2_ATTACK_BOT_HPP
#define CPP_SC2_ATTACK_BOT_HPP

#include "TF_Bot.hpp"
#include "Task.hpp"
#include "TroopManager.hpp"

class ATTACK_BOT final : public TF_Agent {
public:

    ATTACK_BOT(TF_Bot* bot);

    ~ATTACK_BOT();

    /**
     * Do actions base on game info provided
     * @param gi sc2::GameInfo
     */
    void step() final;

    /**
     * Cross agent communication
     * Add a task to the task_queue
     * @param t Task
     */
    void addTask(Task t);

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

    void setAgents(TF_Agent* defenceb, TF_Agent* resourceb, TF_Agent* scoutb);


    void init();

    // Same as buildStructure, except in this case, we build much closer to the building
    // template, change later
    void buildAddOn(const Unit* unit);

    void buildBarracks();

    void buildFactory();

    void buildStarport();

    // Pointer protection
    // Clear all the dead units in the vector of attack_units
    void all_alive(std::vector<const Unit*> attack_units);

    std::vector<Spotted_Enemy> last_seen_near(Point2D location, float radius, int since);

private:
    std::vector<TF_unit> units;
    std::vector<const Unit*> attack_units;
    TF_Agent* defence;
    TF_Agent* resource;
    TF_Agent* scout;
    TroopManager* troopManager;
};

#endif //CPP_SC2_ATTACK_BOT_HPP
