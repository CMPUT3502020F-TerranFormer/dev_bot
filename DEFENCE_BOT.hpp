//
// Created by Kerry Cao on 2020-10-23.
//

#ifndef CPP_SC2_DEFENCE_BOT_HPP
#define CPP_SC2_DEFENCE_BOT_HPP

#include "TF_Bot.hpp"
#include "Task.hpp"

#include "SQLITE3.hpp"
#include "SQLITE3_QUERY.hpp"

#include <thread>
#include <atomic>

class DEFENCE_BOT final : public TF_Agent {
public:
    DEFENCE_BOT(TF_Bot* bot);

    ~DEFENCE_BOT();

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

    void setAgents(TF_Agent* attackb, TF_Agent* resourceb, TF_Agent* scoutb);

    std::vector<Spotted_Enemy> last_seen_near(sc2::Point2D location, int radius, int since);

    void init();

private:
    std::vector<TF_unit> units;
    TF_Agent *attack;
    TF_Agent *resource;
    TF_Agent *scout;

    std::vector<Point2D> poi;
    std::vector<Point2D> bases;
    std::vector<Point2D> base_needs_defence;

    std::vector<Point2D> defence_points;

    std::vector<Unit*> bunkers;

    std::vector<Unit *> factories;
    int last_factory_used = 0;
    std::vector<Unit *> barracks;
    int last_barracks_used = 0;
    std::vector<Unit *> starports;
    int last_starport_used = 0;

    int tankCount = 0;
    int cycloneCount = 0;
    int marineCount = 0;
    int marauderCount = 0;
    int thorCount = 0;
    int bansheeCount = 0;
    int battleCruiserCount = 0;

    int tankMaxCount = 5;
    int cycloneMaxCount = 5;
    int marineMaxCount = 30;
    int marauderMaxCount = 20;
    int thorMaxCount = 2;
    int bansheeMaxCount = 20;
    int battleCruiserMaxCount = 1;

    double troopMaxCountMultiplier = 1.5;
    int multiplierCounter = 0;

    bool hasBarracks = false;
    bool hasEngineeringBay = false;
    bool hasFactory = false;
    bool hasArmoury = false;
    bool hasStarport = false;
    bool hasFusion = false;

    bool orderedArmoury = false;
    bool orderedEngBay = false;
    bool orderedStarport = false;
    bool orderedFactory = false;
    bool orderedBarrack = false;
    bool orderedFusion = false;

    bool infantryUpgradePhase1Complete = false;
    bool infantryUpgradePhase2Complete = false;
    bool infantryUpgradePhase3Complete = false;

    bool sAndVUpgradePhase1Complete = false;
    bool sAndVUpgradePhase2Complete = false;
    bool sAndVUpgradePhase3Complete = false;

    int balance_step = 0;

    static double distance(const Point2D &p1, const Point2D &p2);

    void buildMissileTurret(Point2D pos);
    void buildEngineeringBay();
    void buildArmory();
    void buildStarport();
    void buildStarport(Point2D pos);
    void buildBarracks();
    void buildBarracks(Point2D pos);
    void buildFactory();
    void buildFactory(Point2D pos);
    void buildFusion();
    void buildBunker(Point2D pos);

    void orderSiegeTank(int count);
    void orderThor(int count);
    void orderMarine(int count);
    void orderMarauder(int count);
    void orderBanshee(int count);
    void orderCyclone(int count);
    void orderBattleCruiser(int count);

    void check_for_engineering_bay();
    void check_for_factory();
    void check_for_armoury();
    void check_for_barracks();
    void check_for_starport();
    void check_for_fusion();

    std::tuple<std::vector<const Unit *>, int, Point2D> assess_defence_point(Point2D pos);

    static int get_defence_score(UNIT_TYPEID id);

    void defence_balance();
};

#endif //CPP_SC2_DEFENCE_BOT_HPP
