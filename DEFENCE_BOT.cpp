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

DEFENCE_BOT::~DEFENCE_BOT() {

}

void DEFENCE_BOT::step() {
    if (hasEngineeringBay) {
        for (auto &p : base_needs_defence) {
            std::sort(poi.begin(), poi.end(), [p](const Point2D &p1, const Point2D &p2) {
                return distance(p, p1) < distance(p, p2);
            });
            // build missile turret on the 2 nearest poi
            for (int i = 0; i < 2; ++i) {
                buildMissileTurret(poi[i]);
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
        orderSiegeTank(3);
        orderCyclone(2);
        orderMarine(10);
        orderMarauder(5);
    } else if (mCount > 1000 && hasFactory) {
        orderThor(1);
        orderSiegeTank(1);
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
            if (observation->GetGameLoop() / 16 > 500 && !orderedStarport) {
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
            if (distribution(generator)) {
                buildBarracks(u->pos);
            } else {
                buildFactory(u->pos);
            }
        }

        // find the 2 closest defence points
        std::sort(poi.begin(), poi.end(), [u](const Point2D &p1, const Point2D &p2) {
            return distance(u->pos, p1) < distance(u->pos, p2);
        });
        defence_point.emplace_back(poi[0]);
        defence_point.emplace_back(poi[1]);

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
        return;
    }

    switch ((int) u->unit_type) {
        case (int) UNIT_TYPEID::TERRAN_SUPPLYDEPOT: // supply depot auto lower to save space
            action->UnitCommand(u, ABILITY_ID::MORPH_SUPPLYDEPOT_LOWER);
            break;
        case (int) UNIT_TYPEID::TERRAN_FACTORY:
            hasFactory = true;

            // build armoury if no armoury is built
            if (!orderedArmoury) {
                buildArmory();
                orderedArmoury = true;
            }
            factories.push_back(const_cast<Unit *>(u));
            orderSiegeTank(2);
            break;
        case (int) UNIT_TYPEID::TERRAN_BARRACKS:
            hasBarracks = true;

            // build engineering bay if no engineering bay is built
            if (!orderedEngBay) {
                buildEngineeringBay();
                orderedEngBay = true;
                buildFactory(u->pos); // build a factory after we have a barrack
            }
            barracks.push_back(const_cast<Unit *>(u));
            break;
        case (int) UNIT_TYPEID::TERRAN_STARPORT:
            hasStarport = true;
            starports.push_back(const_cast<Unit *>(u));
            break;
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
            tankCount -= 5*multiplierCounter;
            break;
        case (int) UNIT_TYPEID::TERRAN_MARAUDER:
            marauderCount -= 5*multiplierCounter;
            break;
        case (int) UNIT_TYPEID::TERRAN_BANSHEE:
            bansheeCount -= 5*multiplierCounter;
            break;
        case (int) UNIT_TYPEID::TERRAN_MARINE:
            marineCount -= 5*multiplierCounter;
            break;
        case (int) UNIT_TYPEID::TERRAN_CYCLONE:
            cycloneCount -= 5*multiplierCounter;
            break;
        case (int) UNIT_TYPEID::TERRAN_THOR:
            thorCount -= 5*multiplierCounter;
            break;
        case (int) UNIT_TYPEID::TERRAN_BATTLECRUISER:
            battleCruiserCount -= 5*multiplierCounter;
            break;
    }

    All_Attack_Units.erase(u->tag);
}

void DEFENCE_BOT::unitCreated(const sc2::Unit *u) {
    // find the closes defence point
    std::sort(defence_point.begin(), defence_point.end(), [u](const Point2D &p1, const Point2D &p2) {
        return distance(u->pos, p1) < distance(u->pos, p2);
    });

    switch ((int) u->unit_type) {
        // Siege tank when created, will move to a choke point and morph to siege mode
        case (int) UNIT_TYPEID::TERRAN_SIEGETANK:
            All_Attack_Units.emplace(u->tag, const_cast<Unit *>(u)); // add to a list of all attacking troops
            action->UnitCommand(u, ABILITY_ID::MORPH_UNSIEGE);
            action->SendActions();
            action->UnitCommand(u, ABILITY_ID::MOVE_MOVE, defence_point[0]);
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
            All_Attack_Units.emplace(u->tag, const_cast<Unit *>(u)); // add to a list of all attacking troops
            action->UnitCommand(u, ABILITY_ID::MOVE_MOVE, defence_point[0]);
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

    switch ((int) u->unit_type) {
        // TODO use behavior tree to replace the following logic
        // engineering bay upgrade tree
        case (int) UNIT_TYPEID::TERRAN_ENGINEERINGBAY:
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
            break;
        // TODO use behavior tree to replace the following logic
        // armoury upgrade tree
        case (int) UNIT_TYPEID::TERRAN_ARMORY:
            action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL1, true);
            if (infantryUpgradePhase1Complete) {
                action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL1, true);
                action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL1, true);
                if (infantryUpgradePhase2Complete) {
                    action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL2, true);
                    action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL2, true);
                    action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL2, true);
                    if (infantryUpgradePhase3Complete) {
                        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL3, true);
                        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL3, true);
                        action->UnitCommand(u, ABILITY_ID::RESEARCH_TERRANSHIPWEAPONSLEVEL3, true);
                    }
                }
            }
            break;
        case (int) UNIT_TYPEID::TERRAN_TECHLAB:
            action->UnitCommand(u, ABILITY_ID::RESEARCH_COMBATSHIELD);
            break;
        case (int) UNIT_TYPEID::TERRAN_SIEGETANK:
            action->UnitCommand(u, ABILITY_ID::MORPH_SIEGEMODE, true);
            action->SendActions();
            break;
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
    distribution = std::uniform_int_distribution<int>(0, 1);

    buildingPlacementManager = new BuildingPlacementManager(observation, query);

    // base on map name, get all point of interest
    auto gi = observation->GetGameInfo();
    try {
        SQLITE3 scout_POI_db("TF_bot.db"); // open db

        // form query
        SQLITE3_QUERY q("SELECT x, y FROM DEFENCE_POI WHERE map = ?;");
        q.add_binding(std::regex_replace(gi.map_name, std::regex("'"), "''"));

        // execute query
        int ret = scout_POI_db.execute(q); // execute query
        if (ret || scout_POI_db.get_result_row_count() == 0) { // check query success
            throw std::runtime_error("DEFENCE POI Query Failed");
        }

        // load query result to poi
        auto r = scout_POI_db.copy_result();
        for (auto &xy_vec : *r) {
            auto x = std::stod(xy_vec.at(0));
            auto y = std::stod(xy_vec.at(1));
            poi.emplace_back(x, y);
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
        std::sort(poi.begin(), poi.end(), [cmd](const Point2D &p1, const Point2D &p2) {
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
            4,
            UNIT_TYPEID::TERRAN_ARMORY,
            ABILITY_ID::BUILD_ARMORY,
            buildingPlacementManager->getNextArmoryLocation());
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

void DEFENCE_BOT::buildFusion() {
    Task buildEB(BUILD,
                 DEFENCE_AGENT,
                 8,
                 UNIT_TYPEID::TERRAN_FUSIONCORE,
                 ABILITY_ID::BUILD_FUSIONCORE,
                 buildingPlacementManager->getNextFusionCoreLocation());
    resource->addTask(buildEB);
}

void DEFENCE_BOT::buildStarport() {
    Task buildSP(BUILD,
                 ATTACK_AGENT,
                 8,
                 UNIT_TYPEID::TERRAN_STARPORT,
                 ABILITY_ID::BUILD_STARPORT,
                 buildingPlacementManager->getNextStarportLocation());
    resource->addTask(buildSP);
}

void DEFENCE_BOT::buildStarport(Point2D pos) {
    Task buildSP(BUILD,
                 ATTACK_AGENT,
                 8,
                 UNIT_TYPEID::TERRAN_STARPORT,
                 ABILITY_ID::BUILD_STARPORT,
                 buildingPlacementManager->getNextStarportLocation(pos));
    resource->addTask(buildSP);
}

void DEFENCE_BOT::buildBarracks() {
    Task buildBR(BUILD,
                 ATTACK_AGENT,
                 7,
                 UNIT_TYPEID::TERRAN_BARRACKS,
                 ABILITY_ID::BUILD_BARRACKS,
                 buildingPlacementManager->getNextBarracksLocation());
    resource->addTask(buildBR);
}

void DEFENCE_BOT::buildFactory() {
    Task buildFT(BUILD,
                 ATTACK_AGENT,
                 8,
                 UNIT_TYPEID::TERRAN_FACTORY,
                 ABILITY_ID::BUILD_FACTORY,
                 buildingPlacementManager->getNextFactoryLocation());
    resource->addTask(buildFT);
}

void DEFENCE_BOT::buildBarracks(Point2D pos) {
    Task buildBR(BUILD,
                 ATTACK_AGENT,
                 6,
                 UNIT_TYPEID::TERRAN_BARRACKS,
                 ABILITY_ID::BUILD_BARRACKS,
                 buildingPlacementManager->getNextBarracksLocation(pos));
    resource->addTask(buildBR);
}

void DEFENCE_BOT::buildFactory(Point2D pos) {
    Task buildFT(BUILD,
                 ATTACK_AGENT,
                 8,
                 UNIT_TYPEID::TERRAN_FACTORY,
                 ABILITY_ID::BUILD_FACTORY,
                 buildingPlacementManager->getNextFactoryLocation(pos));
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
        } else {
            return false;
        }
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

        }else {
            return false;
        }
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
        } else {
            return false;
        }
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
    last_factory_used = (last_factory_used + 1) % factories.size();
}

void DEFENCE_BOT::orderBattleCruiser(int count) {
    if (starports.empty() || battleCruiserCount >= battleCruiserMaxCount) {
        return;
    }

    check_for_fusion();
    if (!hasFusion) {
        buildFusion();
    }

    for (auto port : starports) {
        if (port->unit_type == UNIT_TYPEID::TERRAN_STARPORTTECHLAB) {
            for (int i = 0; i < count; ++i) {
                resource->addTask(Task(TRAIN,
                                       ATTACK_AGENT,
                                       10,
                                       ABILITY_ID::TRAIN_BATTLECRUISER,
                                       UNIT_TYPEID::TERRAN_BATTLECRUISER,
                                       starports[last_factory_used]->unit_type,
                                       starports[last_factory_used]->tag
                ));
            }
            battleCruiserCount += 1;
            last_starport_used = (last_starport_used + 1) % starports.size();
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
    last_factory_used = (last_factory_used + 1) % factories.size();
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
    last_barracks_used = (last_barracks_used + 1) % barracks.size();
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
    last_barracks_used = (last_barracks_used + 1) % barracks.size();
}

void DEFENCE_BOT::orderBanshee(int count) {
    if (starports.empty() || bansheeCount >= bansheeMaxCount) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               ATTACK_AGENT,
                               7,
                               ABILITY_ID::TRAIN_BANSHEE,
                               UNIT_TYPEID::TERRAN_BANSHEE,
                               starports[last_starport_used]->unit_type,
                               starports[last_starport_used]->tag
        ));
    }
    bansheeCount += 1;
    last_starport_used = (last_starport_used + 1) % starports.size();
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
    last_factory_used = (last_factory_used + 1) % factories.size();
}




