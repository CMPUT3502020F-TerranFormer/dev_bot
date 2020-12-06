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
    // if (ordered_scv) {
    //     for (auto& record : last_seen_near(Point2D(0, 0), 100000000, 100000000)) {
    //         std::cout << "(" << record.location.x << ", " << record.location.y << ") ";
    //     }
    // }

    // order scv 120sec into game time
    if (!ordered_scv) {
        if (steps / 16 > -1) {
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

void SCOUT_BOT::unitEnterVision(const sc2::Unit * u) {
    // if unit is enemy, record spotting
    if (u->alliance == Unit::Alliance::Enemy) {
        auto now = observation->GetGameLoop() / 16;
        detection_record.insert(Spotted_Enemy(*u, Point2D(u->pos), now));
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

std::vector<Spotted_Enemy> SCOUT_BOT::last_seen_near(Point2D location, float radius, int since) {
    std::cout << "last seen near called\n";
    auto now = observation->GetGameLoop() / 16;
    
    std::vector<Spotted_Enemy> ret = detection_record.search(location, radius);
    
    std::cout << "ret before time filter = " << ret.size() << "\n";
    // Filter out results older than "since"
    auto new_end = std::remove_if(ret.begin(), ret.end(), [now, since](const Spotted_Enemy& e) { return now - e.time < since; });
    ret.erase(new_end, ret.end());
    
    std::cout << "ret after time filter = " << ret.size() << "\n";

    for (auto& record : ret) {
        std::cout << "(" << record.location.x << ", " << record.location.y << ") ";
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
            auto x = std::stod(xy_vec.at(0));
            auto y = std::stod(xy_vec.at(1));
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