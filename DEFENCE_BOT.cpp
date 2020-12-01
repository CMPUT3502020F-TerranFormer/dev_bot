//
// Created by Carter Sabadash on 2020-10-24
//
#include "DEFENCE_BOT.hpp"

DEFENCE_BOT::DEFENCE_BOT(TF_Bot* bot)
    : TF_Agent(bot)
{
    scout = nullptr;
    attack = nullptr;
    resource = nullptr;
}

DEFENCE_BOT::~DEFENCE_BOT() {

}

void DEFENCE_BOT::step() {
    // build engineering bay once a barracks is built
    if (!hasBarracks) {
        check_for_barracks();
    } else {
        if (!orderedEngBay) {
            buildEngineeringBay();
            orderedEngBay = true;
        }
    }

    // build armoury bay once a factory is built
    if (!hasFactory) {
        check_for_factory();
    } else {
        if (!orderedArmoury) {
            buildArmory();
            orderedArmoury = true;
        }
    }

    if (hasEngineeringBay) {
        for (auto &p : base_needs_defence) {
            std::sort(poi.begin(), poi.end(), [p](const Point2D& p1, const Point2D& p2) {
                return distance(p, p1) < distance(p, p2);
            });
            // build missile turret on the 2 nearest poi
            for (int i = 0; i < 2; ++i) {
                buildMissileTurret(poi[i]);
            }
        }

        base_needs_defence.clear();
    }

    auto mCount = observation->GetMinerals();
    if (mCount > 1000 && hasFactory) {
        orderThor(1);
    } else if (mCount > 600 && hasFactory) {
        if (distribution(generator)) {
            orderSiegeTank(1);
        } else {
            orderCyclone(1);
        }

        if (hasBarracks) {
            orderMarine(2);
            orderMarauder(1);
        }
        if (hasStarport) {
            orderBanshee(2);
        } else {
            if (observation->GetGameLoop()/16 > 500 && !orderedStarport) {
                buildStarport();
                orderedStarport = true;
            }
        }
    } else if (mCount > 400) {
        orderMarauder(1);
    } else if (mCount > 200 && hasBarracks && All_Attack_Units.size() < 30) {
        orderMarine(1);
    }
}

void DEFENCE_BOT::addTask(Task t) {

}

void DEFENCE_BOT::addUnit(TF_unit u) {

}

void DEFENCE_BOT::buildingConstructionComplete(const sc2::Unit* u) {
    if (u->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER && u->alliance == Unit::Self) { // new command center added
        base_needs_defence.emplace_back(u->pos);
        bases.emplace_back(u->pos);


        // find the 2 closest defence points
        std::sort(poi.begin(), poi.end(), [u](const Point2D& p1, const Point2D& p2) {
            return distance(u->pos, p1) < distance(u->pos, p2);
        });
        defence_point.emplace_back(poi[0]);
        defence_point.emplace_back(poi[1]);

        for (auto unit : All_Attack_Units) {
            if (distribution(generator)){
                Point2D pos;
                if (distribution(generator)) {
                    pos = poi[0];
                } else {
                    pos = poi[1];
                }
                if (unit.second->unit_type == UNIT_TYPEID::TERRAN_SIEGETANKSIEGED || unit.second->unit_type == UNIT_TYPEID::TERRAN_SIEGETANK) {
                    action->UnitCommand(unit.second, ABILITY_ID::MORPH_UNSIEGE);
                    action->SendActions();
                }
                action->UnitCommand(unit.second, ABILITY_ID::MOVE_MOVE, pos, true);
                action->UnitCommand(unit.second, ABILITY_ID::MORPH_SIEGEMODE, true);
            }
        }

        return;
    }

    switch((int) u->unit_type) {
        case (int) UNIT_TYPEID::TERRAN_SUPPLYDEPOT: // supply depot auto lower to save space
            action->UnitCommand(u, ABILITY_ID::MORPH_SUPPLYDEPOT_LOWER);
            break;
        case (int) UNIT_TYPEID::TERRAN_FACTORY:
            hasFactory = true;
            factories.push_back(const_cast<Unit*>(u));
            break;
        case (int) UNIT_TYPEID::TERRAN_BARRACKS:
            hasBarracks = true;
            barracks.push_back(const_cast<Unit*>(u));
            break;
        case (int) UNIT_TYPEID::TERRAN_STARPORT:
            hasStarport = true;
            starports.push_back(const_cast<Unit*>(u));
            break;
    }
}

void DEFENCE_BOT::unitDestroyed(const sc2::Unit* u) {
    if (u->unit_type == UNIT_TYPEID::TERRAN_ENGINEERINGBAY) {
        buildEngineeringBay();
        return;
    } else if (u->unit_type == UNIT_TYPEID::TERRAN_ARMORY) {
        buildArmory();
        return;
    }

    switch ((int) u->unit_type) {
        case (int) UNIT_TYPEID::TERRAN_STARPORT:
        case (int) UNIT_TYPEID::TERRAN_STARPORTREACTOR:
        case (int) UNIT_TYPEID::TERRAN_STARPORTTECHLAB:
            for (auto it = starports.begin(); it != starports.end(); ++it) {
                if ((*it)->tag == u->tag) {
                    starports.erase(it);
                    break;
                }
            }
            break;
        case (int) UNIT_TYPEID::TERRAN_BARRACKS:
        case (int) UNIT_TYPEID::TERRAN_BARRACKSREACTOR:
        case (int) UNIT_TYPEID::TERRAN_BARRACKSTECHLAB:
            for (auto it = barracks.begin(); it != barracks.end(); ++it) {
                if ((*it)->tag == u->tag) {
                    barracks.erase(it);
                    break;
                }
            }
            break;
        case (int) UNIT_TYPEID::TERRAN_FACTORY:
        case (int) UNIT_TYPEID::TERRAN_FACTORYREACTOR:
        case (int) UNIT_TYPEID::TERRAN_FACTORYTECHLAB:
            for (auto it = factories.begin(); it != factories.end(); ++it) {
                if ((*it)->tag == u->tag) {
                    factories.erase(it);
                    break;
                }
            }
            break;
    }

    All_Attack_Units.erase(u->tag);
}

void DEFENCE_BOT::unitCreated(const sc2::Unit* u) {
    // find the closes defence point
    std::sort(defence_point.begin(), defence_point.end(), [u](const Point2D& p1, const Point2D& p2) {
        return distance(u->pos, p1) < distance(u->pos, p2);
    });

    switch ((int) u->unit_type) {
        // Siege tank when created, will move to a choke point and morph to siege mode
        case (int) UNIT_TYPEID::TERRAN_SIEGETANK:
            All_Attack_Units.emplace(u->tag,const_cast<Unit*>(u)); // add to a list of all attacking troops
            action->UnitCommand(u, ABILITY_ID::MOVE_MOVE, defence_point[0]);
            action->UnitCommand(u, ABILITY_ID::MORPH_SIEGEMODE, true);
            break;
        case (int) UNIT_TYPEID::TERRAN_MARAUDER:
        case (int) UNIT_TYPEID::TERRAN_BANSHEE:
        case (int) UNIT_TYPEID::TERRAN_MARINE:
            All_Attack_Units.emplace(u->tag,const_cast<Unit*>(u)); // add to a list of all attacking troops
            action->UnitCommand(u, ABILITY_ID::MOVE_MOVE, defence_point[0]);
            break;
        default:
            break;
    }
}

void DEFENCE_BOT::unitEnterVision(const sc2::Unit* u) {
    /*
    if (u->alliance != sc2::Unit::Self) {
        for (auto &b : bases) {
            if (distance(u->pos, b) < 30) {
                std::cout << "Defence Activated" << std::endl;
                for (auto unit : All_Attack_Units) {
                    if (unit.second->unit_type == UNIT_TYPEID::TERRAN_SIEGETANKSIEGED || unit.second->unit_type == UNIT_TYPEID::TERRAN_SIEGETANK) {
                        action->UnitCommand(unit.second, ABILITY_ID::MORPH_UNSIEGE);
                        action->SendActions();
                    }
                    action->UnitCommand(unit.second, ABILITY_ID::MOVE_MOVE, u->pos, true);
                    action->UnitCommand(unit.second, ABILITY_ID::MORPH_SIEGEMODE, true);
                    action->UnitCommand(unit.second, ABILITY_ID::ATTACK_ATTACK, u->pos, true);
                }
            }
        }
    }
     */
}

void DEFENCE_BOT::unitIdle(const sc2::Unit* u) {
    /**
     * TODO WAITING FOR API FROM ATTACK TO GET TROOP COUNT
     */

    // TODO use behavior tree to replace the following logic
    // engineering bay upgrade tree
    if (u->unit_type == UNIT_TYPEID::TERRAN_ENGINEERINGBAY) {
        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL1);
        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL1);
        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL2);
        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL2);
        if (sAndVUpgradePhase1Complete) {
            action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL3);
            action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL3);
            if (sAndVUpgradePhase2Complete) {
                action->UnitCommand(u, ABILITY_ID::RESEARCH_HISECAUTOTRACKING);
                action->UnitCommand(u, ABILITY_ID::RESEARCH_NEOSTEELFRAME);
            }
        }
    }

    // TODO use behavior tree to replace the following logic
    // armoury upgrade tree
    if (u->unit_type == UNIT_TYPEID::TERRAN_ARMORY) {
        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL1, true);
        if (infantryUpgradePhase1Complete){
            action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL1, true);
            action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL1, true);
            if (infantryUpgradePhase2Complete) {
                action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL2, true);
                action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL2, true);
                action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL2, true);
                if (infantryUpgradePhase3Complete){
                    action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL3, true);
                    action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL3, true);
                    action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL3, true);
                }
            }
        }
    }

    if (u->unit_type == UNIT_TYPEID::TERRAN_TECHLAB) {
        action->UnitCommand(u, ABILITY_ID::RESEARCH_COMBATSHIELD);
    }
}

void DEFENCE_BOT::upgradeCompleted(sc2::UpgradeID uid) {
    switch((int) uid){
        case (int) UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL2:
            infantryUpgradePhase1Complete = true;
            break;
        case (int) UPGRADE_ID::TERRANINFANTRYARMORSLEVEL2:
            infantryUpgradePhase2Complete = true;
            break;
        case (int) UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL3:
            infantryUpgradePhase3Complete = true;
            break;
        case (int) UPGRADE_ID::TERRANSHIPWEAPONSLEVEL1:
            infantryUpgradePhase1Complete = true;
            break;
        case (int) UPGRADE_ID::TERRANSHIPWEAPONSLEVEL2:
            infantryUpgradePhase2Complete = true;
            break;
        case (int) UPGRADE_ID::TERRANSHIPWEAPONSLEVEL3:
            infantryUpgradePhase3Complete = true;
            break;
    }
}

void DEFENCE_BOT::setAgents(TF_Agent* attackb, TF_Agent* resourceb, TF_Agent* scoutb) {
    this->attack = attackb;
    this->resource = resourceb;
    this->scout = scoutb;
}

std::vector<Spotted_Enemy> DEFENCE_BOT::last_seen_near(sc2::Point2D location, int radius, int since) {
    return scout->last_seen_near(location, radius, since);
}

void DEFENCE_BOT::init() {
    distribution = std::uniform_int_distribution<int>(0,2);

    buildingPlacementManager = new BuildingPlacementManager(observation, query);

    // base on map name, get all point of interest
    auto gi = observation->GetGameInfo();
    try {
        SQLITE3 scout_POI_db("TF_bot.db"); // open db

        // form query
        SQLITE3_QUERY q("SELECT x, y FROM DEFENCE_POI WHERE map = ?;");
        q.add_binding(gi.map_name);

        // execute query
        int ret = scout_POI_db.execute(q); // execute query
        if (ret || scout_POI_db.get_result_row_count() == 0) { // check query success
            throw std::runtime_error("DEFENCE POI Query Failed");
        }

        // load query result to poi
        auto r = scout_POI_db.copy_result();
        for (auto& xy_vec : *r) {
            auto x = std::stod(xy_vec.at(0));
            auto y = std::stod(xy_vec.at(1));
            poi.emplace_back(x, y);
        }
    }
    catch (std::runtime_error& err) {
        std::cerr << "FATAL ERROR: " << err.what() << std::endl;
        exit(1);
    }

    auto ret = observation->GetUnits([](const Unit& unit){
        if (unit.unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER && unit.alliance == sc2::Unit::Self && unit.build_progress == 1.0){
            return true;
        } else {
            return false;
        }
    });
    for (auto cmd : ret) {
        bases.emplace_back(cmd->pos);
        base_needs_defence.emplace_back(cmd->pos);

        // find the closes defence point
        std::sort(poi.begin(), poi.end(), [cmd](const Point2D& p1, const Point2D& p2) {
            return distance(cmd->pos, p1) < distance(cmd->pos, p2);
        });
        defence_point.emplace_back(poi[0]);
    }
}

double DEFENCE_BOT::distance(const Point2D &p1, const Point2D &p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

void DEFENCE_BOT::buildMissileTurret(Point2D pos) {
    Task dp(BUILD,
            DEFENCE_AGENT,
            8,
            UNIT_TYPEID::TERRAN_MISSILETURRET,
            ABILITY_ID::BUILD_MISSILETURRET,
            buildingPlacementManager->getNextMissileTurretLocation(pos));
    resource->addTask(dp);
}

void DEFENCE_BOT::buildArmory() {
    Task dp(BUILD,
            DEFENCE_AGENT,
            8,
            UNIT_TYPEID::TERRAN_ARMORY,
            ABILITY_ID::BUILD_ARMORY,
            buildingPlacementManager->getNextArmoryLocation());
    resource->addTask(dp);
}

void DEFENCE_BOT::buildEngineeringBay() {
    Task buildEB(BUILD,
            DEFENCE_AGENT,
            10,
            UNIT_TYPEID::TERRAN_ENGINEERINGBAY,
            ABILITY_ID::BUILD_ENGINEERINGBAY,
            buildingPlacementManager->getNextEngineeringBayLocation());
    resource->addTask(buildEB);
}

void DEFENCE_BOT::buildStarport() {
    Task buildSP(BUILD,
                 DEFENCE_AGENT,
                 10,
                 UNIT_TYPEID::TERRAN_STARPORT,
                 ABILITY_ID::BUILD_STARPORT,
                 buildingPlacementManager->getNextStarportLocation());
    resource->addTask(buildSP);
}

void DEFENCE_BOT::buildBarracks() {
    Task buildBR(BUILD,
                 DEFENCE_AGENT,
                 10,
                 UNIT_TYPEID::TERRAN_BARRACKS,
                 ABILITY_ID::BUILD_BARRACKS,
                 buildingPlacementManager->getNextBarracksLocation());
    resource->addTask(buildBR);
}

void DEFENCE_BOT::buildFactory() {
    Task buildFT(BUILD,
                 DEFENCE_AGENT,
                 10,
                 UNIT_TYPEID::TERRAN_FACTORY,
                 ABILITY_ID::BUILD_FACTORY,
                 buildingPlacementManager->getNextFactoryLocation());
    resource->addTask(buildFT);
}

void DEFENCE_BOT::check_for_engineering_bay() {
    auto ret = observation->GetUnits([](const Unit& unit){
        if (unit.unit_type == UNIT_TYPEID::TERRAN_ENGINEERINGBAY && unit.alliance == sc2::Unit::Self && unit.build_progress == 1.0){
            return true;
        } else {
            return false;
        }
    });

    if(!ret.empty()) {
        hasEngineeringBay = true;
    }
}

void DEFENCE_BOT::check_for_factory() {
    auto ret = observation->GetUnits([](const Unit& unit){
        if (unit.unit_type == UNIT_TYPEID::TERRAN_FACTORY && unit.alliance == sc2::Unit::Self && unit.build_progress == 1.0){
            return true;
        } else {
            return false;
        }
    });

    for (auto f : ret) {
        factories.push_back(const_cast<Unit*>(f));
    }

    if(!ret.empty()) {
        hasFactory = true;
    }
}

void DEFENCE_BOT::check_for_armoury() {
    auto ret = observation->GetUnits([](const Unit& unit){
        if (unit.unit_type == UNIT_TYPEID::TERRAN_ARMORY && unit.alliance == sc2::Unit::Self && unit.build_progress == 1.0){
            return true;
        } else {
            return false;
        }
    });

    if(!ret.empty()) {
        hasArmoury = true;
    }
}

void DEFENCE_BOT::check_for_barracks() {
    auto ret = observation->GetUnits([](const Unit& unit){
        if (unit.unit_type == UNIT_TYPEID::TERRAN_BARRACKS && unit.alliance == sc2::Unit::Self && unit.build_progress == 1.0){
            return true;
        } else {
            return false;
        }
    });

    if(!ret.empty()) {
        hasBarracks = true;
    }
}

void DEFENCE_BOT::orderSiegeTank(int count) {
    assert(count < 3);
    assert(count > 0);

    if (factories.empty()) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               7,
                               ABILITY_ID::TRAIN_SIEGETANK,
                               UNIT_TYPEID::TERRAN_SIEGETANK,
                               UNIT_TYPEID::TERRAN_FACTORY,
                               factories[last_factory_used]->tag
        ));
    }
    last_factory_used = (last_factory_used + 1) % factories.size();
}

void DEFENCE_BOT::orderThor(int count) {
    assert(count < 3);
    assert(count > 0);

    if (factories.empty()) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               7,
                               ABILITY_ID::TRAIN_THOR,
                               UNIT_TYPEID::TERRAN_THOR,
                               UNIT_TYPEID::TERRAN_FACTORY,
                               factories[last_factory_used]->tag
        ));
    }
    last_factory_used = (last_factory_used + 1) % factories.size();
}

void DEFENCE_BOT::orderMarine(int count) {
    if (barracks.empty()) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               7,
                               ABILITY_ID::TRAIN_MARINE,
                               UNIT_TYPEID::TERRAN_MARINE,
                               UNIT_TYPEID::TERRAN_BARRACKS,
                               barracks[last_barracks_used]->tag
        ));
    }

    last_barracks_used = (last_barracks_used + 1) % barracks.size();
}

void DEFENCE_BOT::orderMarauder(int count) {
    if (barracks.empty()) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               7,
                               ABILITY_ID::TRAIN_MARAUDER,
                               UNIT_TYPEID::TERRAN_MARAUDER,
                               UNIT_TYPEID::TERRAN_BARRACKS,
                               barracks[last_barracks_used]->tag
        ));
    }
    last_barracks_used = (last_barracks_used + 1) % barracks.size();
}

void DEFENCE_BOT::orderBanshee(int count) {
    if (starports.empty()) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               7,
                               ABILITY_ID::TRAIN_BANSHEE,
                               UNIT_TYPEID::TERRAN_BANSHEE,
                               UNIT_TYPEID::TERRAN_STARPORT,
                               starports[last_starport_used]->tag
        ));
    }
    last_starport_used = (last_starport_used + 1) % starports.size();
}

void DEFENCE_BOT::orderCyclone(int count) {
    assert(count < 3);
    assert(count > 0);

    if (factories.empty()) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               7,
                               ABILITY_ID::TRAIN_CYCLONE,
                               UNIT_TYPEID::TERRAN_CYCLONE,
                               UNIT_TYPEID::TERRAN_FACTORY,
                               factories[last_factory_used]->tag
        ));
    }
    last_factory_used = (last_factory_used + 1) % factories.size();
}




