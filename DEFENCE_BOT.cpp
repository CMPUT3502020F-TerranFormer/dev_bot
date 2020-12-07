//
// Created by Carter Sabadash on 2020-10-24
//
#include "DEFENCE_BOT.hpp"

DEFENCE_BOT::DEFENCE_BOT(TF_Bot *bot)
        : TF_Agent(bot) {
    scout = nullptr;
    attack = nullptr;
    resource = nullptr;
}

DEFENCE_BOT::~DEFENCE_BOT() = default;

void DEFENCE_BOT::step() {
    auto gl = observation->GetGameLoop();

    if (gl % 4 != 0) { return; } // only about ever 1/4 second

    if (gl % 8 == 0) { check_active_defence(); }

    if (balance_step / 16 > 20) {
        defence_balance();
        balance_step = 0;
    } else {
        ++balance_step;
    }

    check_for_engineering_bay();
    check_for_factory();

    if (!orderedEngBay && !hasEngineeringBay && gl / 16 > 500) {
        buildEngineeringBay();
        orderedEngBay = true;
    }

    if (!orderedArmoury && !hasArmoury && gl / 16 > 500) {
        buildArmory();
        orderedArmoury = true;
    }

    if (!orderedStarport && !hasStarport && gl / 16 > 600) {
        buildStarport();
        orderedStarport = true;
    }

    if (hasEngineeringBay) {
        for (auto &p : base_needs_defence) {
            std::sort(poi.begin(), poi.end(), [p](const DEFENCE_POI &p1, const DEFENCE_POI &p2) {
                return distance(p, p1.pos) < distance(p, p2.pos);
            });
            // build 2 missile turret on each base
            for (int i = 0; i < 2; ++i) {
                buildMissileTurret(poi[i].pos);
            }
        }

        base_needs_defence.clear();
    }

    check_for_factory();
    check_for_starport();
    check_for_barracks();

    auto mCount = observation->GetMinerals();
    if (mCount > 4000 && multiplierCounter < 4) {
        tankMaxCount *= troopMaxCountMultiplier;
        cycloneMaxCount *= troopMaxCountMultiplier;
        marineMaxCount *= troopMaxCountMultiplier;
        marauderMaxCount *= troopMaxCountMultiplier;
        thorMaxCount *= troopMaxCountMultiplier;
        bansheeMaxCount *= troopMaxCountMultiplier;
        battleCruiserMaxCount *= troopMaxCountMultiplier;
        multiplierCounter += 0.5;
    } else if (mCount > 3000 && multiplierCounter < 2) {
        tankMaxCount *= troopMaxCountMultiplier;
        cycloneMaxCount *= troopMaxCountMultiplier;
        marineMaxCount *= troopMaxCountMultiplier;
        marauderMaxCount *= troopMaxCountMultiplier;
        thorMaxCount *= troopMaxCountMultiplier;
        bansheeMaxCount *= troopMaxCountMultiplier;
        battleCruiserMaxCount *= troopMaxCountMultiplier;
        multiplierCounter += 0.5;
    } else if (mCount > 2000 && multiplierCounter < 1) {
        tankMaxCount *= troopMaxCountMultiplier;
        cycloneMaxCount *= troopMaxCountMultiplier;
        marineMaxCount *= troopMaxCountMultiplier;
        marauderMaxCount *= troopMaxCountMultiplier;
        thorMaxCount *= troopMaxCountMultiplier;
        bansheeMaxCount *= troopMaxCountMultiplier;
        battleCruiserMaxCount *= troopMaxCountMultiplier;
        multiplierCounter += 1;
    }

    if (mCount > 2000 && hasFactory) {
        if (hasStarport) {
            orderBattleCruiser(1);
        }
        orderMarine(8);
        orderMarauder(5);
        if (hasStarport) {
            orderBanshee(2);
        }
    } else if (mCount > 1000 && hasFactory) {
        orderThor(1);
        if (hasStarport) {
            orderBanshee(2);
        }
    } else if (mCount > 600 && hasFactory) {
        //orderSiegeTank(1);

        if (hasBarracks) {
            orderMarine(2);
            orderMarauder(5);
        }
        if (hasStarport) {
            orderBanshee(2);
        }
    } else if (mCount > 400) {
        orderMarauder(1);
        orderMarine(1);
        if (hasStarport) {
            orderBanshee(2);
        }
    } else if (mCount > 200 && hasBarracks) {
        orderMarine(1);
        if (hasStarport) {
            orderBanshee(1);
        }
    }
}

void DEFENCE_BOT::addTask(Task t) {

}

void DEFENCE_BOT::addUnit(TF_unit u) {

}

void DEFENCE_BOT::buildingConstructionComplete(const sc2::Unit *u) {
    if (u->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER && u->alliance == Unit::Self) { // new command center added
        base_needs_defence.emplace_back(u->pos);
        bases.emplace_back(u->pos);

        auto mCount = observation->GetMinerals();
        if (mCount > 1000) {
            buildBarracks(u->pos);
            buildFactory(u->pos);
            buildStarport(u->pos);
        } else {
            if (barracks.size() > factories.size()) {
                buildFactory();
            } else {
                buildBarracks();
            }
        }

        // find the 2 closest defence points
        std::sort(poi.begin(), poi.end(), [u](const DEFENCE_POI &p1, const DEFENCE_POI &p2) {
            return distance(u->pos, p1.pos) < distance(u->pos, p2.pos);
        });

        for (auto & i : poi) {
            if (std::find(defence_points.begin(), defence_points.end(), i) == defence_points.end()) {
                if (distance(u->pos, i.pos) < 20) {
                    defence_points.push_back(i);
                    buildBunker(i.pos);
                    buildBunker(i.pos);
                    break;
                }
            }
        }

        /** TODO to be replaced with the new defence balancing system
        for (auto unit : All_Attack_Units) {
            if (distribution(generator)) {
                Point2D pos;
                if (distribution(generator)) {
                    pos = poi[0];
                } else {
                    pos = poi[1];
                }
                if (unit.second->unit_type == UNIT_TYPEID::TERRAN_SIEGETANKSIEGED ||
                    unit.second->unit_type == UNIT_TYPEID::TERRAN_SIEGETANK) {
                    action->UnitCommand(unit.second, ABILITY_ID::MORPH_UNSIEGE);
                    action->SendActions();
                }
                action->UnitCommand(unit.second, ABILITY_ID::MOVE_MOVE, pos, true);
                action->UnitCommand(unit.second, ABILITY_ID::MORPH_SIEGEMODE, true);
            }
        }
         */
        return;
    }

    switch ((int) u->unit_type) {
        case (int) UNIT_TYPEID::TERRAN_SUPPLYDEPOT: // supply depot auto lower to save space
            action->UnitCommand(u, ABILITY_ID::MORPH_SUPPLYDEPOT_LOWER);
            break;
        case (int) UNIT_TYPEID::TERRAN_FACTORY:
            hasFactory = true;
            action->UnitCommand(u, ABILITY_ID::BUILD_TECHLAB);
            factories.push_back(const_cast<Unit *>(u));
            orderSiegeTank(2);
            break;
        case (int) UNIT_TYPEID::TERRAN_BARRACKS:
            hasBarracks = true;
            barracks.push_back(const_cast<Unit *>(u));
            break;
        case (int) UNIT_TYPEID::TERRAN_STARPORT:
            hasStarport = true;
            starports.push_back(const_cast<Unit *>(u));
            break;
        case (int) UNIT_TYPEID::TERRAN_BUNKER:
            bunkers.push_back(const_cast<Unit *>(u));
    }
}

void DEFENCE_BOT::unitDestroyed(const sc2::Unit *u) {
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

    switch ((int) u->unit_type) {
        case (int) UNIT_TYPEID::TERRAN_SIEGETANK:
            tankCount -= 5 * multiplierCounter;
            break;
        case (int) UNIT_TYPEID::TERRAN_MARAUDER:
            marauderCount -= 5 * multiplierCounter;
            break;
        case (int) UNIT_TYPEID::TERRAN_BANSHEE:
            bansheeCount -= 5 * multiplierCounter;
            break;
        case (int) UNIT_TYPEID::TERRAN_MARINE:
            marineCount -= 5 * multiplierCounter;
            break;
        case (int) UNIT_TYPEID::TERRAN_CYCLONE:
            cycloneCount -= 5 * multiplierCounter;
            break;
        case (int) UNIT_TYPEID::TERRAN_THOR:
            thorCount -= 5 * multiplierCounter;
            break;
        case (int) UNIT_TYPEID::TERRAN_BATTLECRUISER:
            battleCruiserCount -= 5 * multiplierCounter;
            break;
    }
}

void DEFENCE_BOT::unitCreated(const sc2::Unit *u) {
    // find the closes defence point
    std::sort(defence_points.begin(), defence_points.end(), [u](const DEFENCE_POI &p1, const DEFENCE_POI &p2) {
        return distance(u->pos, p1.pos) < distance(u->pos, p2.pos);
    });

    if (defence_points.empty()) { 
        std::cout << "No Defence Points" << std::endl;
        return; 
    }
    switch ((int) u->unit_type) {
        // Siege tank when created, will move to a choke point and morph to siege mode
        case (int) UNIT_TYPEID::TERRAN_SIEGETANK:
            action->UnitCommand(u, ABILITY_ID::ATTACK_ATTACK, defence_points[0].pos, true);
            action->UnitCommand(u, ABILITY_ID::MORPH_SIEGEMODE, true);
            break;
        case (int) UNIT_TYPEID::TERRAN_MARAUDER:
        case (int) UNIT_TYPEID::TERRAN_BANSHEE:
        case (int) UNIT_TYPEID::TERRAN_MARINE:
        case (int) UNIT_TYPEID::TERRAN_CYCLONE:
        case (int) UNIT_TYPEID::TERRAN_THOR:
        case (int) UNIT_TYPEID::TERRAN_VIKINGASSAULT:
        case (int) UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
        case (int) UNIT_TYPEID::TERRAN_MEDIVAC:
        case (int) UNIT_TYPEID::TERRAN_RAVEN:
        case (int) UNIT_TYPEID::TERRAN_BATTLECRUISER:
            action->UnitCommand(u, ABILITY_ID::ATTACK_ATTACK, defence_points[0].pos, true);
            action->UnitCommand(u, ABILITY_ID::MOVE_MOVE, defence_points[0].pos, true);
            break;
        default:
            break;
    }
}

void DEFENCE_BOT::unitEnterVision(const sc2::Unit *u) {
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

void DEFENCE_BOT::unitIdle(const sc2::Unit *u) {

    switch ((int)u->unit_type) {
        // TODO use behavior tree to replace the following logic
        // engineering bay upgrade tree
    case (int)UNIT_TYPEID::TERRAN_ENGINEERINGBAY: {
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANINFANTRYARMORSLEVEL1, ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL1));
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL1, ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL1));
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 6, u->tag, UPGRADE_ID::TERRANINFANTRYARMORSLEVEL2, ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL2));
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 6, u->tag, UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL2, ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL2));
        if (sAndVUpgradePhase1Complete) {
            resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANINFANTRYARMORSLEVEL3, ABILITY_ID::RESEARCH_TERRANINFANTRYARMORLEVEL3));
            resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL3, ABILITY_ID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL3));
            if (sAndVUpgradePhase2Complete) {
                resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::HISECAUTOTRACKING, ABILITY_ID::RESEARCH_HISECAUTOTRACKING));
                resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::NEOSTEELFRAME, ABILITY_ID::RESEARCH_NEOSTEELFRAME));
            }
        }
        break;
    }
                                                // TODO use behavior tree to replace the following logic
                                                // armoury upgrade tree
    case (int)UNIT_TYPEID::TERRAN_ARMORY: {
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL1, ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL1));
        if (infantryUpgradePhase1Complete) {
            resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL1, ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL1));
            resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANSHIPWEAPONSLEVEL1, ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL1));
            if (infantryUpgradePhase2Complete) {
                resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL2, ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL2));
                resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL2, ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL2));
                resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANSHIPWEAPONSLEVEL2, ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL2));
                if (infantryUpgradePhase3Complete) {
                    resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL3, ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL3));
                    resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL3, ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL3));
                    resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::TERRANSHIPWEAPONSLEVEL3, ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL3));
                }
            }
        }
        break;
    }
    case (int)UNIT_TYPEID::TERRAN_TECHLAB: {
        break;
    }
    case (int)UNIT_TYPEID::TERRAN_SIEGETANK:
        if (u->orders.empty()) {
            action->UnitCommand(u, ABILITY_ID::MORPH_SIEGEMODE, true);
        }
        break;
    case (int)UNIT_TYPEID::TERRAN_MARINE:
    case (int)UNIT_TYPEID::TERRAN_MARAUDER:
        // load empty bunkers
        if (!bunkers.empty()) {
            action->UnitCommand(bunkers[0], ABILITY_ID::LOAD_BUNKER, u);
            action->SendActions();
            if (bunkers[0]->cargo_space_taken == bunkers[0]->cargo_space_max) {
                bunkers.erase(bunkers.begin());
            }
        }
        break;
    case (int)UNIT_TYPEID::TERRAN_BARRACKSTECHLAB: {
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::STIMPACK, ABILITY_ID::RESEARCH_STIMPACK)); // barracks
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 6, u->tag, UPGRADE_ID::COMBATSHIELD, ABILITY_ID::RESEARCH_COMBATSHIELD)); // barracks
        // not sure what the UPGRADE_ID is, neosteel frames has a higher cost so we'll use that in it's place
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 6, u->tag, UPGRADE_ID::NEOSTEELFRAME, ABILITY_ID::RESEARCH_CONCUSSIVESHELLS)); // barracks
        break;
    }
    case (int)UNIT_TYPEID::TERRAN_STARPORTTECHLAB: {
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 6, u->tag, UPGRADE_ID::BANSHEECLOAK, ABILITY_ID::RESEARCH_BANSHEECLOAKINGFIELD)); // starport
        // corvid reactor has the same cost because I couldn't figure out the actual upgrade id
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 6, u->tag, UPGRADE_ID::RAVENCORVIDREACTOR, ABILITY_ID::RESEARCH_BANSHEEHYPERFLIGHTROTORS));
        break;
    }
    }
}

void DEFENCE_BOT::upgradeCompleted(sc2::UpgradeID uid) {
    switch ((int) uid) {
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
        case (int)UPGRADE_ID::STIMPACK:
            stim_researched = true;
            break;
        case (int)UPGRADE_ID::BANSHEECLOAK:
            banshee_cloak_researched = true;
            break;
    }
}

void DEFENCE_BOT::setAgents(TF_Agent *attackb, TF_Agent *resourceb, TF_Agent *scoutb) {
    this->attack = attackb;
    this->resource = resourceb;
    this->scout = scoutb;
}

std::vector<Spotted_Enemy> DEFENCE_BOT::last_seen_near(sc2::Point2D location, int radius, int since) {
    return scout->last_seen_near(location, radius, since);
}

void DEFENCE_BOT::init() {
    // base on map name, get all point of interest
    auto gi = observation->GetGameInfo();
    try {
        SQLITE3 scout_POI_db("TF_bot.db"); // open db

        // form query
        SQLITE3_QUERY q("SELECT x, y, major FROM DEFENCE_POI WHERE map = ?;");
        q.add_binding(std::regex_replace(gi.map_name, std::regex("'"), "''"));

        // execute query
        int ret = scout_POI_db.execute(q); // execute query
        if (ret || scout_POI_db.get_result_row_count() == 0) { // check query success
            throw std::runtime_error("DEFENCE POI Query Failed");
        }

        // load query result to poi
        auto r = scout_POI_db.copy_result();
        for (auto &xy_vec : *r) {
            float x = std::stod(xy_vec.at(0));
            float y = std::stod(xy_vec.at(1));
            auto major = std::stoi(xy_vec.at(2));
            poi.emplace_back(x, y, major);
        }
    }
    catch (std::runtime_error &err) {
        std::cerr << "FATAL ERROR: " << err.what() << std::endl;
        exit(1);
    }

    auto ret = observation->GetUnits([](const Unit &unit) {
        if (unit.unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER && unit.alliance == sc2::Unit::Self &&
            unit.build_progress == 1.0) {
            return true;
        } else {
            return false;
        }
    });
    for (auto cmd : ret) {
        bases.emplace_back(cmd->pos);
        base_needs_defence.emplace_back(cmd->pos);

        // find the closes defence point
        std::sort(poi.begin(), poi.end(), [cmd](const DEFENCE_POI &p1, const DEFENCE_POI &p2) {
            return distance(cmd->pos, p1.pos) < distance(cmd->pos, p2.pos);
        });

        for (auto & i : poi) {
            if (i.major) {
                defence_points.emplace_back(i);
                break;
            }
        }
    }

    buildBunker(defence_points[0].pos);
}

double DEFENCE_BOT::distance(const Point2D &p1, const Point2D &p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

void DEFENCE_BOT::buildMissileTurret(Point2D pos) {
    Task dp(BUILD,
            DEFENCE_AGENT,
            6,
            UNIT_TYPEID::TERRAN_MISSILETURRET,
            ABILITY_ID::BUILD_MISSILETURRET,
            pos);
    resource->addTask(dp);
}

void DEFENCE_BOT::buildArmory() {
    Task dp(BUILD,
            DEFENCE_AGENT,
            4,
            UNIT_TYPEID::TERRAN_ARMORY,
            ABILITY_ID::BUILD_ARMORY);
    resource->addTask(dp);
}

void DEFENCE_BOT::buildEngineeringBay() {
    if (observation->GetGameLoop() / 16 < 400) {
        return;
    }
    Task buildEB(BUILD,
                 DEFENCE_AGENT,
                 8,
                 UNIT_TYPEID::TERRAN_ENGINEERINGBAY,
                 ABILITY_ID::BUILD_ENGINEERINGBAY);
    resource->addTask(buildEB);
}

void DEFENCE_BOT::buildFusion() {
    Task buildEB(BUILD,
                 DEFENCE_AGENT,
                 8,
                 UNIT_TYPEID::TERRAN_FUSIONCORE,
                 ABILITY_ID::BUILD_FUSIONCORE);
    resource->addTask(buildEB);
}

void DEFENCE_BOT::buildStarport() {
    Task buildSP(BUILD,
                 ATTACK_AGENT,
                 8,
                 UNIT_TYPEID::TERRAN_STARPORT,
                 ABILITY_ID::BUILD_STARPORT);
    resource->addTask(buildSP);
}

void DEFENCE_BOT::buildStarport(Point2D pos) {
    Task buildSP(BUILD,
                 ATTACK_AGENT,
                 8,
                 UNIT_TYPEID::TERRAN_STARPORT,
                 ABILITY_ID::BUILD_STARPORT,
                 pos);
    resource->addTask(buildSP);
}

void DEFENCE_BOT::buildBarracks() {
    Task buildBR(BUILD,
                 ATTACK_AGENT,
                 7,
                 UNIT_TYPEID::TERRAN_BARRACKS,
                 ABILITY_ID::BUILD_BARRACKS);
    resource->addTask(buildBR);
}

void DEFENCE_BOT::buildFactory() {
    Task buildFT(BUILD,
                 ATTACK_AGENT,
                 8,
                 UNIT_TYPEID::TERRAN_FACTORY,
                 ABILITY_ID::BUILD_FACTORY);
    resource->addTask(buildFT);
}

void DEFENCE_BOT::buildBarracks(Point2D pos) {
    Task buildBR(BUILD,
                 ATTACK_AGENT,
                 6,
                 UNIT_TYPEID::TERRAN_BARRACKS,
                 ABILITY_ID::BUILD_BARRACKS,
                 pos);
    resource->addTask(buildBR);
}

void DEFENCE_BOT::buildFactory(Point2D pos) {
    Task buildFT(BUILD,
                 ATTACK_AGENT,
                 8,
                 UNIT_TYPEID::TERRAN_FACTORY,
                 ABILITY_ID::BUILD_FACTORY,
                 pos);
    resource->addTask(buildFT);
}

void DEFENCE_BOT::buildBunker(Point2D pos) {
    Task buildFT(BUILD,
                 ATTACK_AGENT,
                 10,
                 UNIT_TYPEID::TERRAN_BUNKER,
                 ABILITY_ID::BUILD_BUNKER,
                 pos);
    resource->addTask(buildFT);
}

void DEFENCE_BOT::check_for_engineering_bay() {
    auto ret = observation->GetUnits([](const Unit &unit) {
        if (unit.unit_type == UNIT_TYPEID::TERRAN_ENGINEERINGBAY && unit.alliance == sc2::Unit::Self &&
            unit.build_progress == 1.0) {
            return true;
        } else {
            return false;
        }
    });

    if (!ret.empty()) {
        hasEngineeringBay = true;
    } else {
        hasEngineeringBay = false;
    }
}

void DEFENCE_BOT::check_for_factory() {
    auto ret = observation->GetUnits([](const Unit &unit) {
        if (unit.unit_type == UNIT_TYPEID::TERRAN_FACTORY && unit.alliance == sc2::Unit::Self &&
            unit.build_progress == 1.0) {
            return true;
        } else if (unit.unit_type == UNIT_TYPEID::TERRAN_FACTORYTECHLAB && unit.alliance == sc2::Unit::Self &&
                   unit.build_progress == 1.0) {
            return true;
        } else if (unit.unit_type == UNIT_TYPEID::TERRAN_FACTORYREACTOR && unit.alliance == sc2::Unit::Self &&
                   unit.build_progress == 1.0) {
            return true;
        }
        return false;
    });

    if (!ret.empty()) {
        hasFactory = true;
    } else {
        hasFactory = false;
        factories.clear();

        if (observation->GetGameLoop() / 16 > 120 && !orderedFactory) {
            buildFactory();
            orderedFactory = true;
        }
        return;
    }

    factories.clear();
    for (auto f : ret) {
        factories.push_back(const_cast<Unit *>(f));
    }
}

void DEFENCE_BOT::check_for_starport() {
    auto ret = observation->GetUnits([](const Unit &unit) {
        if (unit.unit_type == UNIT_TYPEID::TERRAN_STARPORTTECHLAB && unit.alliance == sc2::Unit::Self &&
            unit.build_progress == 1.0) {
            return true;
        } else if (unit.unit_type == UNIT_TYPEID::TERRAN_STARPORT && unit.alliance == sc2::Unit::Self &&
                   unit.build_progress == 1.0) {
            return true;
        } else if (unit.unit_type == UNIT_TYPEID::TERRAN_STARPORTREACTOR && unit.alliance == sc2::Unit::Self &&
                   unit.build_progress == 1.0) {
            return true;
        }
        return false;
    });

    if (!ret.empty()) {
        hasStarport = true;
    } else {
        hasStarport = false;
        starports.clear();
        return;
    }

    starports.clear();
    for (auto f : ret) {
        starports.push_back(const_cast<Unit *>(f));
    }
}

void DEFENCE_BOT::check_for_armoury() {
    auto ret = observation->GetUnits([](const Unit &unit) {
        if (unit.unit_type == UNIT_TYPEID::TERRAN_ARMORY && unit.alliance == sc2::Unit::Self &&
            unit.build_progress == 1.0) {
            return true;
        } else {
            return false;
        }
    });

    if (!ret.empty()) {
        hasArmoury = true;
    } else {
        hasArmoury = false;
    }
}

void DEFENCE_BOT::check_for_barracks() {
    auto ret = observation->GetUnits([](const Unit &unit) {
        if (unit.unit_type == UNIT_TYPEID::TERRAN_BARRACKS && unit.alliance == sc2::Unit::Self &&
            unit.build_progress == 1.0) {
            return true;
        } else if (unit.unit_type == UNIT_TYPEID::TERRAN_BARRACKSREACTOR && unit.alliance == sc2::Unit::Self &&
                   unit.build_progress == 1.0) {
            return true;
        } else if (unit.unit_type == UNIT_TYPEID::TERRAN_BARRACKSTECHLAB && unit.alliance == sc2::Unit::Self &&
                   unit.build_progress == 1.0) {
            return true;
        }
        return false;
    });

    if (!ret.empty()) {
        hasBarracks = true;
    } else {
        barracks.clear();
        hasBarracks = false;

        if (observation->GetGameLoop() / 16 > 120 && !orderedBarrack) {
            buildBarracks();
            orderedBarrack = true;
        }
        return;
    }

    barracks.clear();
    for (auto f : ret) {
        barracks.push_back(const_cast<Unit *>(f));
    }
}

void DEFENCE_BOT::check_for_fusion() {
    auto ret = observation->GetUnits([](const Unit &unit) {
        if (unit.unit_type == UNIT_TYPEID::TERRAN_FUSIONCORE && unit.alliance == sc2::Unit::Self &&
            unit.build_progress == 1.0) {
            return true;
        } else {
            return false;
        }
    });

    if (!ret.empty()) {
        hasFusion = true;
    } else {
        hasFusion = false;
    }
}

void DEFENCE_BOT::orderSiegeTank(int count) {
    if (factories.empty() || tankCount >= tankMaxCount) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               7,
                               ABILITY_ID::TRAIN_SIEGETANK,
                               UNIT_TYPEID::TERRAN_SIEGETANK,
                               factories[last_factory_used]->unit_type,
                               factories[last_factory_used]->tag
        ));
    }
    tankCount += 1;
    last_factory_used = (last_factory_used + 1) % (int) factories.size();
}

void DEFENCE_BOT::orderBattleCruiser(int count) {
    if (starports.empty() || battleCruiserCount >= battleCruiserMaxCount) {
        return;
    }

    check_for_fusion();
    if (!hasFusion) {
        if (!orderedFusion) {
            buildFusion();
            orderedFusion = true;
        }
        return;
    }

    for (auto port : starports) {
        if (port->unit_type == UNIT_TYPEID::TERRAN_STARPORTTECHLAB) {
            for (int i = 0; i < count; ++i) {
                resource->addTask(Task(TRAIN,
                                       ATTACK_AGENT,
                                       10,
                                       ABILITY_ID::TRAIN_BATTLECRUISER,
                                       UNIT_TYPEID::TERRAN_BATTLECRUISER,
                                       starports[last_starport_used]->unit_type,
                                       starports[last_starport_used]->tag
                ));
            }
            battleCruiserCount += 1;
            last_starport_used = (last_starport_used + 1) % (int) starports.size();
            return;
        }
    }

}

void DEFENCE_BOT::orderThor(int count) {
    if (factories.empty() || thorCount >= thorMaxCount) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               8,
                               ABILITY_ID::TRAIN_THOR,
                               UNIT_TYPEID::TERRAN_THOR,
                               factories[last_factory_used]->unit_type,
                               factories[last_factory_used]->tag
        ));
    }
    thorCount += 1;
    last_factory_used = (last_factory_used + 1) % (int) factories.size();
}

void DEFENCE_BOT::orderMarine(int count) {
    if (barracks.empty() || marineCount >= marineMaxCount) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               7,
                               ABILITY_ID::TRAIN_MARINE,
                               UNIT_TYPEID::TERRAN_MARINE,
                               barracks[last_barracks_used]->unit_type,
                               barracks[last_barracks_used]->tag
        ));
    }
    marineCount += 1;
    last_barracks_used = (last_barracks_used + 1) % (int) barracks.size();
}

void DEFENCE_BOT::orderMarauder(int count) {
    if (barracks.empty() || marauderCount >= marauderMaxCount) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               7,
                               ABILITY_ID::TRAIN_MARAUDER,
                               UNIT_TYPEID::TERRAN_MARAUDER,
                               barracks[last_barracks_used]->unit_type,
                               barracks[last_barracks_used]->tag
        ));
    }
    marauderCount += 1;
    last_barracks_used = (last_barracks_used + 1) % (int) barracks.size();
}

void DEFENCE_BOT::orderBanshee(int count) {
    if (starports.empty() || bansheeCount >= bansheeMaxCount) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               10,
                               ABILITY_ID::TRAIN_BANSHEE,
                               UNIT_TYPEID::TERRAN_BANSHEE,
                               starports[last_starport_used]->unit_type,
                               starports[last_starport_used]->tag
        ));
    }
    bansheeCount += 1;
    last_starport_used = (last_starport_used + 1) % (int) starports.size();
}

void DEFENCE_BOT::orderCyclone(int count) {
    if (factories.empty() || cycloneCount >= cycloneMaxCount) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               7,
                               ABILITY_ID::TRAIN_CYCLONE,
                               UNIT_TYPEID::TERRAN_CYCLONE,
                               factories[last_factory_used]->unit_type,
                               factories[last_factory_used]->tag
        ));
    }
    cycloneCount += 1;
    last_factory_used = (last_factory_used + 1) % (int) factories.size();
}

std::tuple<std::vector<const Unit *>, int, Point2D> DEFENCE_BOT::assess_defence_point(Point2D pos) {
    int defence_score = 0;

    auto all_troops_at_point = observation->GetUnits([pos, &defence_score](const Unit &unit) {
        if (unit.alliance == sc2::Unit::Self) {
            switch ((int) unit.unit_type) {
                case (int) UNIT_TYPEID::TERRAN_SIEGETANK:
                case (int) UNIT_TYPEID::TERRAN_MARAUDER:
                case (int) UNIT_TYPEID::TERRAN_BANSHEE:
                case (int) UNIT_TYPEID::TERRAN_MARINE:
                case (int) UNIT_TYPEID::TERRAN_CYCLONE:
                case (int) UNIT_TYPEID::TERRAN_THOR:
                case (int) UNIT_TYPEID::TERRAN_VIKINGASSAULT:
                case (int) UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
                case (int) UNIT_TYPEID::TERRAN_MEDIVAC:
                case (int) UNIT_TYPEID::TERRAN_RAVEN:
                case (int) UNIT_TYPEID::TERRAN_BATTLECRUISER:
                case (int) UNIT_TYPEID::TERRAN_SIEGETANKSIEGED:
                case (int) UNIT_TYPEID::TERRAN_HELLION:
                case (int) UNIT_TYPEID::TERRAN_HELLIONTANK:
                case (int) UNIT_TYPEID::TERRAN_REAPER:
                    if (distance(pos, unit.pos) <= 5) {
                        defence_score += get_defence_score(unit.unit_type);
                        return true;
                    } else {
                        return false;
                    }
            }
        }
        return false;
    });

    return std::make_tuple(all_troops_at_point, defence_score, pos);
}

int DEFENCE_BOT::get_defence_score(UNIT_TYPEID id) {
    switch ((int) id) {
        case (int) UNIT_TYPEID::TERRAN_MEDIVAC:
        case (int) UNIT_TYPEID::TERRAN_RAVEN:
            return 0;
        case (int) UNIT_TYPEID::TERRAN_HELLION:
        case (int) UNIT_TYPEID::TERRAN_REAPER:
        case (int) UNIT_TYPEID::TERRAN_MARINE:
            return 1;
        case (int) UNIT_TYPEID::TERRAN_CYCLONE:
        case (int) UNIT_TYPEID::TERRAN_HELLIONTANK:
            return 2;
        case (int) UNIT_TYPEID::TERRAN_VIKINGASSAULT:
        case (int) UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
        case (int) UNIT_TYPEID::TERRAN_MARAUDER:
        case (int) UNIT_TYPEID::TERRAN_BANSHEE:
            return 3;
        case (int) UNIT_TYPEID::TERRAN_SIEGETANK:
            return 5;
        case (int) UNIT_TYPEID::TERRAN_THOR:
            return 6;
        case (int) UNIT_TYPEID::TERRAN_BATTLECRUISER:
        case (int) UNIT_TYPEID::TERRAN_SIEGETANKSIEGED:
            return 7;
        default:
            return 0;
    }
}

void DEFENCE_BOT::defence_balance() {
    // minimum_defence_score a defence point should have, sqrt of game look so it doesn't grow to fast
    int minimum_defence_score = sqrt(observation->GetGameLoop());

    // get defence score on all current defence points
    std::vector<std::tuple<std::vector<const Unit *>, int, Point2D>> defence_point_scores;
    for (auto &d_point : defence_points) {
        auto ret = assess_defence_point(d_point.pos);
        defence_point_scores.push_back(ret);
    }

    // assess and re-balance defence points
    for (auto r1 : defence_point_scores) {
        if (std::get<1>(r1) < minimum_defence_score) { // point is low on defence
            // find the points with the highest defence score
            auto max = std::max_element(defence_point_scores.begin(), defence_point_scores.end(), [](const auto p1, const auto p2){
                return std::get<1>(p1) < std::get<1>(p2);
            });

            if (max != defence_point_scores.end() && std::get<1>(*max) > minimum_defence_score) {
                // get all the tanks in the most defended point
                std::vector<const Unit*> tanks_at_max_point;
                for (const Unit* unit : std::get<0>(*max)) {
                    if (unit->unit_type == UNIT_TYPEID::TERRAN_SIEGETANKSIEGED ||
                            unit->unit_type == UNIT_TYPEID::TERRAN_SIEGETANK) {
                        tanks_at_max_point.push_back(unit);
                    }
                }

                if (tanks_at_max_point.size() > 3) {
                    action->UnitCommand(tanks_at_max_point[0], ABILITY_ID::MORPH_UNSIEGE);
                    action->UnitCommand(tanks_at_max_point[0], ABILITY_ID::ATTACK_ATTACK, std::get<2>(r1), true);
                    action->UnitCommand(tanks_at_max_point[0], ABILITY_ID::MORPH_SIEGEMODE, std::get<2>(r1), true);
                }
            } else { // all points under defenced, exit
                break;
            }
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void DEFENCE_BOT::check_active_defence() {
    // this figures out what the groups of enemy units nears our command centers are
    // and commands our units to approach them (if not in range -- need to figure this one out)
    auto command_centers = observation->GetUnits(Unit::Alliance::Self, IsCommandCenter());
    auto enemy_units = observation->GetUnits(Unit::Alliance::Enemy, IsNearUnits(command_centers, 8));
    auto our_units = observation->GetUnits(Unit::Alliance::Self, IsArmy());

    if (enemy_units.empty() || command_centers.empty() || our_units.empty()) { return; }

    // now we need to figure out if there are separate groups of enemies (and how large they are)
    // we'll do this by going through the enemy units (taking the first unit as the initial group)
    // and checking if the other units are within range 10, if they aren't -> treat it as another group
    std::vector<std::pair<Point2D, int>> enemy_groups;

    auto first_enemy = enemy_units.back();
    enemy_units.pop_back();
    enemy_groups.emplace_back(first_enemy->pos, 1);

    for (auto& e : enemy_units) {
        for (auto& group : enemy_groups) {
            if (IsClose(group.first, 100)(*e)) {
                ++group.second;
                break;
            }
            else {
                enemy_groups.emplace_back(e->pos, 1);
                break;
            }
        }
    }

    // we'll go through the enemy groups from largest to smallest sending our units to attack 
    // (in 10% larger numbers) until we run out of units (this may result it enemy targets changing rapidly
    // during the start of the attack (maybe search a wider radius for enemy units, only pay attention 
    //      to very close groups)
    // ISSUE: consider grabbing all attacking units nearby in the future (or coordinate with attack)
    //      for now we queue commands
    std::sort(enemy_groups.begin(), enemy_groups.end(), [](auto i, auto j) { return i.second > j.second; });

    auto unit_num = 0; // access units by index
    for (auto& group : enemy_groups) {
        auto troop_count = unit_num + (group.second * 1.1);
        while (unit_num < troop_count && unit_num < our_units.size()) {
            auto unit = our_units.at(unit_num++);
            switch (unit->unit_type.ToType()) { // use abilities & attack
            case UNIT_TYPEID::TERRAN_SIEGETANKSIEGED: {
                auto close_units = observation->GetUnits(IsClose(unit->pos, 13 * 13));
                if (close_units.empty()) {
                    action->UnitCommand(unit, ABILITY_ID::MORPH_UNSIEGE);
                }
                action->UnitCommand(unit, ABILITY_ID::ATTACK, group.first, true);
                break;
            }
            case UNIT_TYPEID::TERRAN_SIEGETANK: {
                // check at 1 unit under max range in case they move away slightly
                auto close_units = observation->GetUnits(IsClose(unit->pos, 12 * 12));
                if (!close_units.empty()) {
                    action->UnitCommand(unit, ABILITY_ID::MORPH_SIEGEMODE);
                }
                action->UnitCommand(unit, ABILITY_ID::ATTACK, group.first, true);
                break;
            }
            case UNIT_TYPEID::TERRAN_MARAUDER: {
                if (IsClose(group.first, 6 * 6)(*unit) && stim_researched) {
                    bool stimmed = false;
                    for (auto& buff : unit->buffs) {
                        if (buff == BUFF_ID::STIMPACK) {
                            stimmed = true;
                        }
                    }
                    if (!stimmed) { action->UnitCommand(unit, ABILITY_ID::EFFECT_STIM); }
                }
                action->UnitCommand(unit, ABILITY_ID::ATTACK, group.first, true);
                break;
            }
            case UNIT_TYPEID::TERRAN_BANSHEE: {
                if (IsClose(group.first, 6 * 6)(*unit)
                    && banshee_cloak_researched
                    && unit->energy > 50) {
                    action->UnitCommand(unit, ABILITY_ID::BEHAVIOR_CLOAKON);
                }
                action->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, group.first, true);
                break;
            }
            case UNIT_TYPEID::TERRAN_MARINE: {
                if (IsClose(group.first, 5 * 5)(*unit) && stim_researched) {
                    bool stimmed = false;
                    for (auto& buff : unit->buffs) {
                        if (buff == BUFF_ID::STIMPACK) {
                            stimmed = true;
                        }
                    }
                    if (!stimmed) { action->UnitCommand(unit, ABILITY_ID::EFFECT_STIM); }
                }
                action->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, group.first, true);
                break;
            }
            case UNIT_TYPEID::TERRAN_CYCLONE: {
                // see examples.cc for how to use lock on with flying units
                action->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, group.first);
                break;
            }
            case UNIT_TYPEID::TERRAN_THOR: {
                // just use basic attack
                // see https://liquipedia.net/starcraft2/Thor_(Legacy_of_the_Void) for abilities to use on air
                action->UnitCommand(unit, ABILITY_ID::ATTACK, group.first, true);
                break;
            }
            case UNIT_TYPEID::TERRAN_VIKINGASSAULT: {
                // figure out when to switch from air to ground -> for now prefer air and just try to attack
                action->UnitCommand(unit, ABILITY_ID::ATTACK, group.first);
                break;
            }
            case UNIT_TYPEID::TERRAN_VIKINGFIGHTER: {
                // same as above
                action->UnitCommand(unit, ABILITY_ID::ATTACK, group.first, true);
                break;
            }
            case UNIT_TYPEID::TERRAN_MEDIVAC: {
                action->UnitCommand(unit, ABILITY_ID::EFFECT_HEAL);
                break;
            }
            case UNIT_TYPEID::TERRAN_RAVEN: {
                if (IsClose(group.first, 6 * 6)(*unit) && unit->energy > 50) {
                    // does this need a 2d point?
                    action->UnitCommand(unit, ABILITY_ID::EFFECT_AUTOTURRET, unit->pos);
                }
                action->UnitCommand(unit, ABILITY_ID::ATTACK, group.first, true);
                break;
            }
            case UNIT_TYPEID::TERRAN_BATTLECRUISER: {
                // figure out abilities later
                action->UnitCommand(unit, ABILITY_ID::ATTACK, group.first, true);
                break;
            }
            case UNIT_TYPEID::TERRAN_HELLION: {
                if (hasArmoury) {
                    action->UnitCommand(unit, ABILITY_ID::MORPH_HELLBAT);
                }
                action->UnitCommand(unit, ABILITY_ID::ATTACK, group.first, true);
                break;
            }
            case UNIT_TYPEID::TERRAN_HELLIONTANK: {
                // not sure what abilities it can use
                action->UnitCommand(unit, ABILITY_ID::ATTACK, group.first, true);
                break;
            }
            case UNIT_TYPEID::TERRAN_REAPER: {
                if (IsClose(group.first, 4)(*unit)) {
                    action->UnitCommand(unit, ABILITY_ID::EFFECT_KD8CHARGE);
                }
                action->UnitCommand(unit, ABILITY_ID::ATTACK, group.first, true);
                break;
            }
            default: action->UnitCommand(unit, ABILITY_ID::ATTACK, group.first, true);
            }
        }
    }
}