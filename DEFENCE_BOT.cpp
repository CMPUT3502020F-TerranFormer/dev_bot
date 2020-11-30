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
        } else if (!hasEngineeringBay){
            stepsEng += 1;
            check_for_engineering_bay();
            if (!hasEngineeringBay && stepsEng / 16 > 10) {
                buildEngineeringBay();
                stepsEng = 0;
            }
        }
    }

    // build armoury bay once a factory is built
    if (!hasFactory) {
        check_for_factory();
    } else {
        if (!orderedArmoury) {
            buildArmory();
            orderedArmoury = true;
        } else if (!hasArmoury){
            stepsArm += 1;
            check_for_armoury();
            if (!hasArmoury && stepsArm / 16 > 10) {
                buildArmory();
                stepsArm = 0;
            }
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
}

void DEFENCE_BOT::addTask(Task t) {

}

void DEFENCE_BOT::addUnit(TF_unit u) {

}

void DEFENCE_BOT::buildingConstructionComplete(const sc2::Unit* u) {
    if (u->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER) { // new command center added
        base_needs_defence.emplace_back(u->pos);
    } else if (u->unit_type == UNIT_TYPEID::TERRAN_SUPPLYDEPOT) { // supply depot auto lower to save space
        action->UnitCommand(u, ABILITY_ID::MORPH_SUPPLYDEPOT_LOWER);
    }
}

void DEFENCE_BOT::unitDestroyed(const sc2::Unit* u) {
    if (u->unit_type == UNIT_TYPEID::TERRAN_ENGINEERINGBAY) {
        buildEngineeringBay();
    } else if (u->unit_type == UNIT_TYPEID::TERRAN_ARMORY) {
        buildArmory();
    }
}

void DEFENCE_BOT::unitCreated(const sc2::Unit* u) {
    std::sort(poi.begin(), poi.end(), [u](const Point2D& p1, const Point2D& p2) {
        return distance(u->pos, p1) < distance(u->pos, p2);
    });

    switch ((int) u->unit_type) {
        // Siege tank when created, will move to a choke point and morph to siege mode
        case (int) UNIT_TYPEID::TERRAN_SIEGETANK:
            action->UnitCommand(u, ABILITY_ID::MOVE_MOVE, poi[0]);
            action->UnitCommand(u, ABILITY_ID::MORPH_SIEGEMODE, true);
            break;
        case (int) UNIT_TYPEID::TERRAN_MARAUDER:
        case (int) UNIT_TYPEID::TERRAN_BANSHEE:
        case (int) UNIT_TYPEID::TERRAN_MARINE:
            action->UnitCommand(u, ABILITY_ID::MOVE_MOVE, poi[0]);
            break;
        default:
            break;
    }
}

void DEFENCE_BOT::unitEnterVision(const sc2::Unit* u) {
    if (u->alliance == sc2::Unit::Enemy) {
        for (auto &b : bases) {
            if (distance(u->pos, b) < 5) {
                // TODO: wait for api from attack
            }
        }
    }
}

void DEFENCE_BOT::unitIdle(const sc2::Unit* u) {
    /**
     * TODO WAITING FOR API FROM ATTACK TO GET TROOP COUNT
     */

    // if refinery idle, meaning gas exhausted, salvage building
    if (u->unit_type == UNIT_TYPEID::TERRAN_REFINERY) {
        action->UnitCommand(u, ABILITY_ID::EFFECT_SALVAGE);
        return;
    }

    // TODO use behavior tree to replace the following logic
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
  // do nothing
}

void DEFENCE_BOT::init() {
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

    bases.push_back(gi.start_locations[0]);
    base_needs_defence.push_back(gi.start_locations[0]);
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
            buildingPlacementManager->getNextMissileTurretLocation());
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

void DEFENCE_BOT::NewBaseBuilt(Point2D pos) {
    base_needs_defence.push_back(pos);
    bases.push_back(pos);
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
