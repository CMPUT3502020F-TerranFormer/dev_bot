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
    while (!task_queue.empty()) {
        Task t = task_queue.pop();
        switch (t.action) {
        case TRANSFER: {
            /* Transfer a unit to another agent, and remove from resource unit list
            * The exact unit must be specified
            */
            TF_unit unit = TF_unit(observation->GetUnit(t.self)->unit_type, t.self);

            // add to correct agent
            switch (t.source) {
            case RESOURCE_AGENT: resource->addUnit(unit);
                break;
            case DEFENCE_AGENT: attack->addUnit(unit);
                break;
            case SCOUT_AGENT: scout->addUnit(unit);
                break;
            default:
                std::cerr << "TRANSFER to invalid agent requested!" << std::endl;
                return;
            }

            // and remove from resource_units
            for (auto it = units.cbegin(); it != units.cend(); ++it) {
                if (it->tag == t.self) {
                    units.erase(it);
                    break;
                }
            }
            break;
        }
        default:
            std::cerr << "DEFENCE Unrecognized Task: " << t.source << " " << t.action << std::endl;
        }
    }

    if (balance_step / 16 > 20) {
        defence_balance();
        balance_step = 0;
    } else {
        ++balance_step;
    }

    check_for_engineering_bay();
    check_for_factory();

    auto gl = observation->GetGameLoop();

    if (gl % 16 == 0) { check_active_defence(); }
    if (gl % 4 != 0) { return; } // only update ~ever 1/4 second

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

    if (gl / 16 > 200) {
        orderTroops();
       // std::cout << orderQueueSize << std::endl;
    }
}

void DEFENCE_BOT::addTask(Task t) {
    task_queue.push(t);
}

void DEFENCE_BOT::addUnit(TF_unit u) {
    units.push_back(u);
}

void DEFENCE_BOT::buildingConstructionComplete(const sc2::Unit *u) {
    if (!u->Self) { return; }
    if (u->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER) { // new command center added
        base_needs_defence.emplace_back(u->pos);
        bases.emplace_back(u->pos);

        auto mCount = observation->GetMinerals();
        if (mCount > 1000) {
            buildBarracks();
            buildFactory();
            buildStarport();
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

    for (auto it = units.begin(); it != units.end(); ++it) {
        if (it->tag == u->tag) {
            units.erase(it);
            return;
        }
    }
}

void DEFENCE_BOT::unitCreated(const sc2::Unit *u) {
    /* From ATTACK, this needs to be updated -- we build too many buildings - I already reduced the number we build from this
    I lowered the priority of these building to 6 so they would still build */
    int command_count = observation->GetUnits(Unit::Alliance::Self, IsCommandCenter()).size();
    int barracks_count = observation->GetUnits(Unit::Alliance::Self, IsBarracks()).size();
    int factory_count = observation->GetUnits(Unit::Alliance::Self, IsFactory()).size();
    int starport_count = observation->GetUnits(Unit::Alliance::Self, IsStarport()).size();
    if (barracks_count < 1 + (command_count))
    {
        buildBarracks();
    }

    if (factory_count < command_count)
    {
        buildFactory();
    }

    if (starport_count < command_count)
    {
        buildStarport();
    }
    /* End from ATTACK */


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
            orderQueueSize -= 1;
            action->UnitCommand(u, ABILITY_ID::ATTACK_ATTACK, defence_points[0].pos, true);
            action->UnitCommand(u, ABILITY_ID::MORPH_SIEGEMODE, true);
            break;
        case (int) UNIT_TYPEID::TERRAN_MARAUDER:
        case (int) UNIT_TYPEID::TERRAN_BANSHEE:
        case (int) UNIT_TYPEID::TERRAN_MARINE:
        case (int) UNIT_TYPEID::TERRAN_THOR:
        case (int) UNIT_TYPEID::TERRAN_VIKINGASSAULT:
        case (int) UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
        case (int) UNIT_TYPEID::TERRAN_MEDIVAC:
        case (int) UNIT_TYPEID::TERRAN_RAVEN:
            orderQueueSize -= 1;
            action->UnitCommand(u, ABILITY_ID::ATTACK_ATTACK, defence_points[0].pos, true);
            action->UnitCommand(u, ABILITY_ID::MOVE_MOVE, defence_points[0].pos, true);
            return;
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
    buildingIdle(u);

    if (std::find(std::begin(units), std::end(units), TF_unit(u->unit_type, u->tag)) == std::end(units)) { return; }

    switch ((int) u->unit_type) { // troop logic is here
        case (int) UNIT_TYPEID::TERRAN_SIEGETANK:
            /*
            if (u->orders.empty()) {
                action->UnitCommand(u, ABILITY_ID::MORPH_SIEGEMODE, true);
            }
             */
            break;
        case (int) UNIT_TYPEID::TERRAN_MARINE:
        case (int) UNIT_TYPEID::TERRAN_MARAUDER:
            // load empty bunkers
            if (!bunkers.empty()) {
                action->UnitCommand(bunkers[0], ABILITY_ID::LOAD_BUNKER, u);
                action->SendActions();
                if (bunkers[0]->cargo_space_taken == bunkers[0]->cargo_space_max) {
                    bunkers.erase(bunkers.begin());
                }
            }
        case (int) UNIT_TYPEID::TERRAN_RAVEN:
            action->UnitCommand(u, ABILITY_ID::EFFECT_AUTOTURRET);
    }
}

/* Should be changed to use UPGRADE task in resources -- I'm not sure what priority these should be
    This will ensure that they are all queued as queuing fails if there isn't enough resources 
    **Update the priority as needed **
*/
void DEFENCE_BOT::buildingIdle(const Unit* u) {
    // TODO use behavior tree to replace the following logic
    // engineering bay upgrade tree
    switch (u->unit_type.ToType()) {
    case UNIT_TYPEID::TERRAN_ENGINEERINGBAY: {
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
    case UNIT_TYPEID::TERRAN_ARMORY: {
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
    case UNIT_TYPEID::TERRAN_TECHLAB: {
        // do we have to put these in the **TECHLAB?
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 8, u->tag, UPGRADE_ID::STIMPACK, ABILITY_ID::RESEARCH_STIMPACK)); // barracks
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::COMBATSHIELD, ABILITY_ID::RESEARCH_COMBATSHIELD)); // barracks
        // not sure what the UPGRADE_ID is, neosteel frames has a higher cost so we'll use that in it's place
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 7, u->tag, UPGRADE_ID::NEOSTEELFRAME, ABILITY_ID::RESEARCH_CONCUSSIVESHELLS)); // barracks
        resource->addTask(Task(UPGRADE, DEFENCE_AGENT, 6, u->tag, UPGRADE_ID::BANSHEECLOAK, ABILITY_ID::RESEARCH_BANSHEECLOAKINGFIELD)); // starport
        break;
    }
    // the following is the old (modified for defence) logic from ATTACK
    case UNIT_TYPEID::TERRAN_BARRACKS: {
        if (barracks.size() - 1 % 4 == 0) { // the first barracks should have a reactor
            resource->addTask(Task(TRAIN, DEFENCE_AGENT, 8, ABILITY_ID::BUILD_REACTOR_BARRACKS,
                UNIT_TYPEID::TERRAN_BARRACKSREACTOR, u->unit_type, u->tag));
        }
        else {
            resource->addTask(Task(TRAIN, DEFENCE_AGENT, 8, ABILITY_ID::BUILD_TECHLAB_BARRACKS,
                UNIT_TYPEID::TERRAN_BARRACKSTECHLAB, u->unit_type, u->tag));
        }
        break;
    }
        // barracks train marine
        // TODO: possibly switch to Marauders if we already have a sufficient amount of Marines
        //
        // to train marauders, check for the presence of a tech lab first

    case UNIT_TYPEID::TERRAN_FACTORY: {
        resource->addTask(Task(TRAIN, DEFENCE_AGENT, 8, ABILITY_ID::BUILD_TECHLAB_FACTORY,
            UNIT_TYPEID::TERRAN_FACTORYTECHLAB, u->unit_type, u->tag));
        // build reactor factories?
        break;
    }

    case UNIT_TYPEID::TERRAN_STARPORT: {
        // build reactor starport?
        resource->addTask(Task(TRAIN, DEFENCE_AGENT, 8, ABILITY_ID::BUILD_TECHLAB_STARPORT,
            UNIT_TYPEID::TERRAN_STARPORTTECHLAB, u->unit_type, u->tag));
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
        case (int) UPGRADE_ID::STIMPACK:
            stim_researched = true;
            break;
        case (int) UPGRADE_ID::BANSHEECLOAK:
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

void DEFENCE_BOT::buildStarport() {
    Task buildSP(BUILD,
                 DEFENCE_AGENT,
                 6,
                 UNIT_TYPEID::TERRAN_STARPORT,
                 ABILITY_ID::BUILD_STARPORT);
    resource->addTask(buildSP);
}

void DEFENCE_BOT::buildBarracks() {
    Task buildBR(BUILD,
                 DEFENCE_AGENT,
                 6,
                 UNIT_TYPEID::TERRAN_BARRACKS,
                 ABILITY_ID::BUILD_BARRACKS);
    resource->addTask(buildBR);
}

void DEFENCE_BOT::buildFactory() {
    Task buildFT(BUILD,
                 DEFENCE_AGENT,
                 6,
                 UNIT_TYPEID::TERRAN_FACTORY,
                 ABILITY_ID::BUILD_FACTORY);
    resource->addTask(buildFT);
}

void DEFENCE_BOT::buildBunker(Point2D pos) {
    Task buildFT(BUILD,
                 DEFENCE_AGENT,
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
            unit.build_progress >= 1.0) {
            return true;
        } else if (unit.unit_type == UNIT_TYPEID::TERRAN_FACTORYTECHLAB && unit.alliance == sc2::Unit::Self &&
                   unit.build_progress >= 1.0) {
            return true;
        } else if (unit.unit_type == UNIT_TYPEID::TERRAN_FACTORYREACTOR && unit.alliance == sc2::Unit::Self &&
                   unit.build_progress >= 1.0) {
            return true;
        }
        return false;
    });

    if (!ret.empty()) {
        hasFactory = true;
        for (auto u : ret) {
            if (u->unit_type == UNIT_TYPEID::TERRAN_FACTORYTECHLAB) {
                hasTechFactory = true;
                break;
            }
            hasTechFactory = false;
        }
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
        if (f->unit_type == UNIT_TYPEID::TERRAN_FACTORY) {
            factories.push_back(const_cast<Unit*>(f));
        }
    }
}

void DEFENCE_BOT::check_for_starport() {
    auto ret = observation->GetUnits([](const Unit &unit) {
        if (unit.unit_type == UNIT_TYPEID::TERRAN_STARPORTTECHLAB && unit.alliance == sc2::Unit::Self &&
            unit.build_progress >= 1.0) {
            return true;
        } else if (unit.unit_type == UNIT_TYPEID::TERRAN_STARPORT && unit.alliance == sc2::Unit::Self &&
                   unit.build_progress >= 1.0) {
            return true;
        } else if (unit.unit_type == UNIT_TYPEID::TERRAN_STARPORTREACTOR && unit.alliance == sc2::Unit::Self &&
                   unit.build_progress >= 1.0) {
            return true;
        }
        return false;
    });

    if (!ret.empty()) {
        hasStarport = true;
        for (auto u : ret) {
            if (u->unit_type == UNIT_TYPEID::TERRAN_STARPORTTECHLAB) {
                hasTechStarport = true;
                break;
            }
            hasTechStarport = false;
        }
    } else {
        hasStarport = false;
        starports.clear();
        return;
    }

    starports.clear();
    for (auto f : ret) {
        if (f->unit_type == UNIT_TYPEID::TERRAN_STARPORT) {
            starports.push_back(const_cast<Unit*>(f));
        }
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
            unit.build_progress >= 1.0) {
            return true;
        } else if (unit.unit_type == UNIT_TYPEID::TERRAN_BARRACKSREACTOR && unit.alliance == sc2::Unit::Self &&
                   unit.build_progress >= 1.0) {
            return true;
        } else if (unit.unit_type == UNIT_TYPEID::TERRAN_BARRACKSTECHLAB && unit.alliance == sc2::Unit::Self &&
                   unit.build_progress >= 1.0) {
            return true;
        }
        return false;
    });

    if (!ret.empty()) {
        hasBarracks = true;
        for (auto u : ret) {
            if (u->unit_type == UNIT_TYPEID::TERRAN_BARRACKSTECHLAB) {
                hasTechBarracks = true;
                break;
            }
            hasTechBarracks = false;
        }
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
        if (f->unit_type == UNIT_TYPEID::TERRAN_BARRACKS) {
            barracks.push_back(const_cast<Unit*>(f));
        }
    }
}

void DEFENCE_BOT::orderSiegeTank(int count) {
    if (factories.empty()) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               DEFENCE_AGENT,
                               8,
                               ABILITY_ID::TRAIN_SIEGETANK,
                               UNIT_TYPEID::TERRAN_SIEGETANK,
                               factories[last_factory_used]->unit_type,
                               factories[last_factory_used]->tag
        ));
    }
    last_factory_used = (last_factory_used + 1) % (int) factories.size();
}

void DEFENCE_BOT::orderThor(int count) {
    if (factories.empty()) {
        return;
    }

    for (auto f : factories) {
        if (f->unit_type == UNIT_TYPEID::TERRAN_FACTORYTECHLAB) {
            for (int i = 0; i < count; ++i) {
                resource->addTask(Task(TRAIN,
                                       DEFENCE_AGENT,
                                       8,
                                       ABILITY_ID::TRAIN_THOR,
                                       UNIT_TYPEID::TERRAN_THOR,
                                       factories[last_factory_used]->unit_type,
                                       factories[last_factory_used]->tag
                ));
            }
            last_factory_used = (last_factory_used + 1) % (int) factories.size();
            return;
        }
    }
}

void DEFENCE_BOT::orderMarine(int count) {
    if (barracks.empty()) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               DEFENCE_AGENT,
                               8,
                               ABILITY_ID::TRAIN_MARINE,
                               UNIT_TYPEID::TERRAN_MARINE,
                               barracks[last_barracks_used]->unit_type,
                               barracks[last_barracks_used]->tag
        ));
    }
    last_barracks_used = (last_barracks_used + 1) % (int) barracks.size();
}

void DEFENCE_BOT::orderMarauder(int count) {
    if (barracks.empty()) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               DEFENCE_AGENT,
                               8,
                               ABILITY_ID::TRAIN_MARAUDER,
                               UNIT_TYPEID::TERRAN_MARAUDER,
                               barracks[last_barracks_used]->unit_type,
                               barracks[last_barracks_used]->tag
        ));
    }
    last_barracks_used = (last_barracks_used + 1) % (int) barracks.size();
}

void DEFENCE_BOT::orderBanshee(int count) {
    if (starports.empty()) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               DEFENCE_AGENT,
                               8,
                               ABILITY_ID::TRAIN_BANSHEE,
                               UNIT_TYPEID::TERRAN_BANSHEE,
                               starports[last_starport_used]->unit_type,
                               starports[last_starport_used]->tag
        ));
    }
    last_starport_used = (last_starport_used + 1) % (int) starports.size();
}

void DEFENCE_BOT::orderMedicVac(int count) {
    if (starports.empty()) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               DEFENCE_AGENT,
                               8,
                               ABILITY_ID::TRAIN_MEDIVAC,
                               UNIT_TYPEID::TERRAN_MEDIVAC,
                               starports[last_starport_used]->unit_type,
                               starports[last_starport_used]->tag
        ));
    }
    last_starport_used = (last_starport_used + 1) % (int) starports.size();
}

void DEFENCE_BOT::orderViking(int count) {
    if (starports.empty()) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        resource->addTask(Task(TRAIN,
                               DEFENCE_AGENT,
                               8,
                               ABILITY_ID::TRAIN_BATTLECRUISER,
                               UNIT_TYPEID::TERRAN_BATTLECRUISER,
                               starports[last_starport_used]->unit_type,
                               starports[last_starport_used]->tag
        ));
    }
    last_starport_used = (last_starport_used + 1) % (int) starports.size();
}

void DEFENCE_BOT::orderRaven(int count) {
    if (starports.empty()) {
        return;
    }

    for (auto port : starports) {
        if (port->unit_type == UNIT_TYPEID::TERRAN_STARPORTTECHLAB) {
            for (int i = 0; i < count; ++i) {
                resource->addTask(Task(TRAIN,
                                       DEFENCE_AGENT,
                                       8,
                                       ABILITY_ID::TRAIN_BATTLECRUISER,
                                       UNIT_TYPEID::TERRAN_BATTLECRUISER,
                                       starports[last_starport_used]->unit_type,
                                       starports[last_starport_used]->tag
                ));
            }
            last_starport_used = (last_starport_used + 1) % (int) starports.size();
            return;
        }
    }
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

    if (enemy_units.empty() || command_centers.empty()) { return; }

    // get only the offensive units we own
    std::vector<TF_unit> army;
    for (auto& u : units) {
        if (IsArmy()(u.type)) {
            army.push_back(u);
        }
    }

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
    std::sort(enemy_groups.begin(), enemy_groups.end(), [](auto i, auto j) { return i.second > j.second; });

    auto unit_num = 0; // access units by index
    for (auto& group : enemy_groups) {
        auto troop_count = unit_num + (group.second * 1.1);
        while (unit_num < troop_count && unit_num < army.size()) {
            auto unit = observation->GetUnit(army.at(unit_num++).tag);
            if (unit == nullptr) { 
                ++troop_count; 
                continue;
            }
            switch(unit->unit_type.ToType()) { // use abilities & attack
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

void DEFENCE_BOT::orderTroops() {
    if (armyToOrder.empty()) {
        armyToOrder.emplace_back();
    } else { // clean out army units that is full
        armyToOrder.erase(std::remove_if(armyToOrder.begin(), armyToOrder.end(), [](ArmyUnit au){
            if (au.full()) {
                return true;
            }
            return false;
        }), armyToOrder.end());
    }

    if (orderQueueSize < ArmyUnit::MAX) {
        for (auto &a : armyToOrder) {
            if (hasTechFactory && a.tank != ArmyUnit::SIEGETANK) {
                std::cout << "Tank Ordered" << std::endl;
                orderSiegeTank(ArmyUnit::SIEGETANK);
                a.tank += ArmyUnit::SIEGETANK;
                orderQueueSize += a.tank;
            }
            if (hasTechBarracks && a.marauder != ArmyUnit::MARAUDER) {
                std::cout << "MARAUDER Ordered" << std::endl;
                orderMarauder(ArmyUnit::MARAUDER);
                a.marauder += ArmyUnit::MARAUDER;
                orderQueueSize += a.marauder;
            }
            if (hasBarracks && a.marine != ArmyUnit::MARINE) {
                std::cout << "MARINE Ordered" << std::endl;
                orderMarine(ArmyUnit::MARINE);
                a.marine += ArmyUnit::MARINE;
                orderQueueSize += a.marine;
            }
            if (hasStarport && a.banshee != ArmyUnit::BANSHEE) {
                std::cout << "MARINBANSHEEE Ordered" << std::endl;
                std::cout << "MEDIVAC Ordered" << std::endl;
                orderBanshee(ArmyUnit::BANSHEE);
                orderMedicVac(ArmyUnit::MEDIVAC);
                a.banshee += ArmyUnit::BANSHEE;
                a.medivac += ArmyUnit::MEDIVAC;
                orderQueueSize += a.banshee + a.medivac;
            }
            if (hasTechStarport && a.raven != ArmyUnit::RAVEN) {
                std::cout << "RAVEN Ordered" << std::endl;
                orderRaven(ArmyUnit::RAVEN);
                a.raven += ArmyUnit::RAVEN;
                orderQueueSize += a.raven;
            }
            if (observation->GetMinerals() > 1000 && hasTechFactory && a.thor != ArmyUnit::THOR) {
                std::cout << "Thor Ordered" << std::endl;
                orderThor(ArmyUnit::THOR);
                a.thor += ArmyUnit::THOR;
                orderQueueSize += a.thor;
            }
            if (orderQueueSize >= ArmyUnit::MAX) {
                break;
            }
        }
    }
}


