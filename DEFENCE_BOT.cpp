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
    if (!hasEngineeringBay) {
        check_for_engineering_bay();
    }

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
            for (int i = 0; i < 3; ++i) {
                buildMissileTurret();
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
    units.emplace_back(u->unit_type,u->tag);
}

void DEFENCE_BOT::unitDestroyed(const sc2::Unit* u) {

}

void DEFENCE_BOT::unitCreated(const sc2::Unit* u) {

}

void DEFENCE_BOT::unitEnterVision(const sc2::Unit* u) {

}

void DEFENCE_BOT::unitIdle(const sc2::Unit* u) {
    if (u->unit_type == UNIT_TYPEID::TERRAN_ENGINEERINGBAY) {
        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL1);
        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL1);
        action->UnitCommand(u, ABILITY_ID::RESEARCH_HISECAUTOTRACKING);
        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL2);
        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL2);
        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL3);
        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL3);
    }
}

void DEFENCE_BOT::upgradeCompleted(sc2::UpgradeID uid) {

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

    buildEngineeringBay();
}

double DEFENCE_BOT::distance(const Point2D &p1, const Point2D &p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

void DEFENCE_BOT::buildMissileTurret() {
    Task dp(BUILD,
            DEFENCE_AGENT,
            8,
            UNIT_TYPEID::TERRAN_MISSILETURRET,
            ABILITY_ID::BUILD_MISSILETURRET,
            buildingPlacementManager->getNextEngineeringBayLocation());
    resource->addTask(dp);
}

void DEFENCE_BOT::buildArmory() {
    Task dp(BUILD,
            DEFENCE_AGENT,
            8,
            UNIT_TYPEID::TERRAN_ARMORY,
            ABILITY_ID::BUILD_ARMORY,
            buildingPlacementManager->getNextSupplyDepotLocation());
    resource->addTask(dp);
}

void DEFENCE_BOT::buildEngineeringBay() {
    Task buildEB(BUILD,
            DEFENCE_AGENT,
            8,
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

void DEFENCE_BOT::NewBaseBuilt(Point2D pos) {
    base_needs_defence.push_back(pos);
    bases.push_back(pos);
}
