//
// Created by Carter Sabadash on 2020-10-24
// Modified by Kerry Cao on 2020-11-10
//
#include "SCOUT_BOT.hpp"

SCOUT_BOT::SCOUT_BOT(TF_Bot* bot)
    : TF_Agent(bot) {
    defence = nullptr;
    attack = nullptr;
    resource = nullptr;
}

SCOUT_BOT::~SCOUT_BOT() = default;

/**
 * MainTask Generate new tasked to be executed when unitIdle is called
 */
void SCOUT_BOT::step() {
    // order scv 120sec into game time
    if (!ordered_scv) {
        if (steps / 16 > 280) {
            // order MAX_SCOUT_COUNT scvs
            auto t = Task(TRAIN,
                           SCOUT_AGENT,
                           6,
                           ABILITY_ID::TRAIN_SCV,
                           UNIT_TYPEID::TERRAN_SCV,
                           UNIT_TYPEID::TERRAN_COMMANDCENTER);

            for (int i = 0; i < MAX_SCOUT_COUNT; ++i) {
                resource->addTask(t);
            }
            ordered_scv = true;
            // scout possible enemy START bases (1 to 3 locations depending on map)
            for (auto& loc : observation->GetGameInfo().enemy_start_locations) {
                addTask(Task(BASIC_SCOUT, 7, loc));
            }
        } else {
            steps += 1;
        }
    }

    // if not busy
    if (task_queue.size() < 5) {
        Task t1(BASIC_SCOUT, 5, poi_close_to_enemy.second.at(poi_close_to_enemy.first % poi_close_to_enemy.second.size()));
        poi_close_to_enemy.first += 1;

        addTask(t1);
    }
}

void SCOUT_BOT::addTask(Task t) {
    task_queue.push(t);
}

void SCOUT_BOT::addUnit(TF_unit u) {
    //std::cout << "scout unit added" << std::endl;

    // add unit to units
    units.emplace_back(u.type, u.tag);

    // give unit a task
    while (task_queue.empty()) { std::this_thread::sleep_for(std::chrono::milliseconds(20)); }

    auto t = task_queue.pop();
    // std::cout << "going to scout: (" << t.position.x << ", " << t.position.y << ")\n";
    action->UnitCommand(observation->GetUnit(u.tag), ABILITY_ID::MOVE_MOVE, t.position);
}

void SCOUT_BOT::buildingConstructionComplete(const sc2::Unit * u) {

}

void SCOUT_BOT::unitDestroyed(const sc2::Unit * u) {
    // get unit identifiers
    auto tag = u->tag;
    auto type = u->unit_type;

    // remove unit from list
    auto val = TF_unit(type, tag);
    for (auto it = units.begin(); it != units.end(); ++it) {
        if (*it == val) { // unit belongs to scout
            if (u->unit_type.ToType() == UNIT_TYPEID::TERRAN_SCV) {
                resource->addTask(Task(TRAIN,
                    SCOUT_AGENT,
                    6,
                    ABILITY_ID::TRAIN_SCV,
                    UNIT_TYPEID::TERRAN_SCV,
                    UNIT_TYPEID::TERRAN_COMMANDCENTER));
            }
            units.erase(it);
            break;
        }
    }

    //std::cout << "scout destroyed, order new ones" << std::endl;
}

void SCOUT_BOT::unitCreated(const sc2::Unit * u) {

}

// Reference: https://liquipedia.net/starcraft2/Buildings
std::unordered_set<UNIT_TYPEID> building_types {
    // Townhall
    UNIT_TYPEID::TERRAN_COMMANDCENTER,
    UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING,
    UNIT_TYPEID::TERRAN_ORBITALCOMMAND,
    UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING,
    UNIT_TYPEID::TERRAN_PLANETARYFORTRESS,
    UNIT_TYPEID::PROTOSS_NEXUS,
    UNIT_TYPEID::ZERG_HATCHERY,
    UNIT_TYPEID::ZERG_LAIR,
    UNIT_TYPEID::ZERG_HIVE,
    
    // Gas Buildings
    UNIT_TYPEID::TERRAN_REFINERY,
    UNIT_TYPEID::TERRAN_REFINERYRICH,
    UNIT_TYPEID::PROTOSS_ASSIMILATOR,
    UNIT_TYPEID::PROTOSS_ASSIMILATORRICH,
    UNIT_TYPEID::ZERG_EXTRACTOR,
    UNIT_TYPEID::ZERG_EXTRACTORRICH,
    
    // Supply buildings
    UNIT_TYPEID::PROTOSS_PYLON,
    UNIT_TYPEID::PROTOSS_PYLONOVERCHARGED,
    UNIT_TYPEID::TERRAN_SUPPLYDEPOT,
    UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED,
    UNIT_TYPEID::ZERG_OVERLORD,
    UNIT_TYPEID::ZERG_OVERLORDCOCOON,
    UNIT_TYPEID::ZERG_OVERLORDTRANSPORT,

    // Static defence
    UNIT_TYPEID::TERRAN_BUNKER,
    UNIT_TYPEID::TERRAN_MISSILETURRET,
    UNIT_TYPEID::TERRAN_PLANETARYFORTRESS,
    UNIT_TYPEID::PROTOSS_PHOTONCANNON,
    UNIT_TYPEID::PROTOSS_SHIELDBATTERY,
    UNIT_TYPEID::ZERG_SPINECRAWLER,
    UNIT_TYPEID::ZERG_SPINECRAWLERUPROOTED,
    UNIT_TYPEID::ZERG_SPORECRAWLER,
    UNIT_TYPEID::ZERG_SPORECRAWLERUPROOTED,
    
    // Production buildings
    UNIT_TYPEID::TERRAN_BARRACKS,
    UNIT_TYPEID::TERRAN_BARRACKSFLYING,
    UNIT_TYPEID::TERRAN_BARRACKSREACTOR,
    UNIT_TYPEID::TERRAN_BARRACKSTECHLAB,
    UNIT_TYPEID::TERRAN_FACTORY,
    UNIT_TYPEID::TERRAN_FACTORYFLYING,
    UNIT_TYPEID::TERRAN_FACTORYREACTOR,
    UNIT_TYPEID::TERRAN_FACTORYTECHLAB,
    UNIT_TYPEID::TERRAN_STARPORT,
    UNIT_TYPEID::TERRAN_STARPORTFLYING,
    UNIT_TYPEID::TERRAN_STARPORTREACTOR,
    UNIT_TYPEID::TERRAN_STARPORTTECHLAB,
    UNIT_TYPEID::PROTOSS_GATEWAY,
    UNIT_TYPEID::PROTOSS_STARGATE,
    UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY,

    // Upgrade buildings
    UNIT_TYPEID::TERRAN_ENGINEERINGBAY,
    UNIT_TYPEID::TERRAN_ARMORY,
    UNIT_TYPEID::PROTOSS_FORGE,
    UNIT_TYPEID::PROTOSS_CYBERNETICSCORE,
    UNIT_TYPEID::ZERG_EVOLUTIONCHAMBER,
    UNIT_TYPEID::ZERG_SPIRE,

    // Technology-only buildings
    UNIT_TYPEID::TERRAN_ENGINEERINGBAY,
    UNIT_TYPEID::TERRAN_GHOSTACADEMY,
    UNIT_TYPEID::TERRAN_FUSIONCORE,
    UNIT_TYPEID::TERRAN_ARMORY,
    UNIT_TYPEID::PROTOSS_FORGE,
    UNIT_TYPEID::PROTOSS_CYBERNETICSCORE,
    UNIT_TYPEID::PROTOSS_ROBOTICSBAY,
    UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL,
    UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE,
    UNIT_TYPEID::PROTOSS_DARKSHRINE,
    UNIT_TYPEID::PROTOSS_DARKTEMPLAR,
    UNIT_TYPEID::PROTOSS_FLEETBEACON,
    UNIT_TYPEID::ZERG_SPAWNINGPOOL,
    UNIT_TYPEID::ZERG_BANELINGNEST,
    UNIT_TYPEID::ZERG_ROACHWARREN,
    UNIT_TYPEID::ZERG_HYDRALISKDEN,
    UNIT_TYPEID::LURKERDEN,
    UNIT_TYPEID::ZERG_SPIRE,
    UNIT_TYPEID::ZERG_GREATERSPIRE,
    UNIT_TYPEID::ZERG_ULTRALISKCAVERN,
};

void SCOUT_BOT::unitEnterVision(const sc2::Unit * u) {
    // if unit is enemy, record spotting
    if (u->alliance == Unit::Alliance::Enemy) {
        auto now = (float) observation->GetGameLoop() / 16;

        // For building types record location; Else record nearest base location
        if (building_types.find(u->unit_type.ToType()) != building_types.end()) {
            detection_record.emplace_back(*u, Point2D(u->pos), now);
        } else {
            Point2D nearest_point = poi[0];
            float smallest_dist = 100000;
            for (const auto& point : poi) {
                float dist = Distance2D(nearest_point, point);
                if (dist < smallest_dist) {
                    nearest_point = point;
                    smallest_dist = dist;
                }
            }
            bool seen = false;
            for (const auto& record : detection_record) {
                if (record.location == nearest_point) {
                    seen = true;
                    break;
                }
            }
            if (!seen) detection_record.emplace_back(Unit(), nearest_point, now);
        }
    }
}

void SCOUT_BOT::unitIdle(const sc2::Unit * u) {
    auto ret = std::find(units.begin(), units.end(), TF_unit(u->unit_type, u->tag));
    if (ret == units.end()) {
        return;
    }

    if (!task_queue.empty()) {
        // const auto task_queue_container(task_queue.get_container());
        // Get highest priority task in queue
        // Task taskToDo = task_queue_container[0];
        // for (auto& task : task_queue_container) {
        //     if (task.priority > taskToDo.priority) {
        //         taskToDo = task;
        //     }
        // }

        // if (taskToDo.priority != 11) {
        //     // Determine closest highest priority task to the unit

        //     // Calculate distances to each task
        //     std::vector<std::pair<Task, float>> task_distances;
        //     float furthest_distance = 0;
        //     for (auto& task : task_queue_container) {
        //         float distance = sc2::Distance2D(u->pos, task.position);
        //         task_distances.push_back({task, distance});
        //         furthest_distance = std::max(furthest_distance, distance);
        //     }
        //     // Find the task with the highest score (priority minus distance)
        //     int highest_score = -11;
        //     for (auto& task_distance : task_distances) {
        //         auto task = task_distance.first;
        //         auto distance = task_distance.second;
        //         int normalized_distance = 10 * distance / furthest_distance;
        //         int score = task.priority - normalized_distance;
        //         if (score > highest_score) {
        //             highest_score = score;
        //             taskToDo = task;
        //         }
        //     }
        // }

        // task_queue.remove(taskToDo);
        Task taskToDo = task_queue.pop();
        switch (taskToDo.action) {
            case BASIC_SCOUT:
                // std::cout << "going to scout: (" << taskToDo.position.x << ", " << taskToDo.position.y << ")\n";
                action->UnitCommand(u, ABILITY_ID::ATTACK_ATTACK, taskToDo.position);
                break;
            case ORBIT_SCOUT:
                break;
        }
    }
}

void SCOUT_BOT::upgradeCompleted(sc2::UpgradeID uid) {
    // do nothing
}

void SCOUT_BOT::setAgents(TF_Agent * defenceb, TF_Agent * attackb, TF_Agent * resourceb) {
    this->defence = defenceb;
    this->attack = attackb;
    this->resource = resourceb;
}

std::vector<Spotted_Enemy> SCOUT_BOT::last_seen_near(Point2D location, int radius, int since) {
    std::vector<Spotted_Enemy> ret;

    // run query
    for (auto& record : detection_record) {
        if (record.distance(location) < radius) {
            auto now = observation->GetGameLoop() / 16;
            if (now - record.time < since) {
                ret.push_back(record);
            }
        }
    }

    return ret;
}

void SCOUT_BOT::init() {
    gi = observation->GetGameInfo();
    main_base = gi.start_locations.at(0);
    enemy_main_base = gi.enemy_start_locations.at(0);

    // base on map name, get all point of interest
    try {
        SQLITE3 scout_POI_db("TF_bot.db"); // open db

        // form query
        SQLITE3_QUERY q("SELECT x, y FROM SCOUT_POI WHERE map = ?;");
        q.add_binding(std::regex_replace(gi.map_name, std::regex("'"), "''"));

        // execute query
        int ret = scout_POI_db.execute(q); // execute query
        if (ret || scout_POI_db.get_result_row_count() == 0) { // check query success
            throw std::runtime_error("Scout POI Query Failed");
        }

        // load query result to poi
        auto r = scout_POI_db.copy_result();
        for (auto& xy_vec : *r) {
            float x = std::stod(xy_vec.at(0));
            float y = std::stod(xy_vec.at(1));
            poi.emplace_back(x, y);
        }
    }
    catch (std::runtime_error& err) {
        std::cerr << "FATAL ERROR: " << err.what() << std::endl;
        exit(1);
    }

    std::copy(poi.begin(), poi.end(), std::back_inserter(poi_close_to_base.second));
    std::sort(poi_close_to_base.second.begin(), poi_close_to_base.second.end(), [this](const Point2D& p1, const Point2D& p2) {
        return distance(this->main_base, p1) < distance(this->main_base, p2);
        });
    poi_close_to_base.first = 0;
    std::copy(poi.begin(), poi.end(), std::back_inserter(poi_close_to_enemy.second));
    std::sort(poi_close_to_enemy.second.begin(), poi_close_to_enemy.second.end(), [this](const Point2D& p1, const Point2D& p2) {
        return distance(this->enemy_main_base, p1) < distance(this->enemy_main_base, p2);
        });
    poi_close_to_enemy.first = 0;
}

double SCOUT_BOT::distance(const Point2D & p1, const Point2D & p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}