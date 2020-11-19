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
    auto now = std::chrono::steady_clock::now();
    auto time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - game_epoch).count();

    // at 2 minutes time point, request the rest of the required SCV at a higher priority
    if (time_elapsed > 120 && time_elapsed < 500) {
        auto t = Task(TRAIN,
                      SCOUT_AGENT,
                      5,
                      ABILITY_ID::TRAIN_SCV,
                      UNIT_TYPEID::TERRAN_SCV,
                      UNIT_TYPEID::TERRAN_COMMANDCENTER);
        for (int i = 0; i < MAX_SCOUT_COUNT/2; ++i) {
            resource->addTask(t);
        }
    }
}

void SCOUT_BOT::addTask(Task t) {

}

void SCOUT_BOT::addUnit(TF_unit u) {
    units.emplace_back(u.type, u.tag);
}

void SCOUT_BOT::buildingConstructionComplete(const sc2::Unit* u) {

}

void SCOUT_BOT::unitDestroyed(const sc2::Unit* u) {
    // get unit identifiers
    auto tag = u->tag;
    auto type = u->unit_type;

    // remove unit from list
    auto val = TF_unit(type, tag);
    for (auto it = units.begin(); it != units.end(); ++it) {
        if (*it == val){
            units.erase(it);
            break;
        }
    }

    // request new unit
    switch ((int) type) {
        case (int) UNIT_TYPEID::TERRAN_SCV:
            resource->addTask(Task(TRAIN,
                                   SCOUT_AGENT,
                                   5,
                                   ABILITY_ID::TRAIN_SCV,
                                   UNIT_TYPEID::TERRAN_SCV,
                                   UNIT_TYPEID::TERRAN_COMMANDCENTER));
            break;
    }
}

void SCOUT_BOT::unitCreated(const sc2::Unit* u) {

}

void SCOUT_BOT::unitEnterVision(const sc2::Unit* u) {
    // if unit is enemy, record spotting
    if (u->alliance == Unit::Alliance::Enemy) {
        auto now = std::chrono::steady_clock::now();
        detection_record.emplace_back(*u, Point2D(u->pos), now);
    }
}

void SCOUT_BOT::unitIdle(const sc2::Unit* u) {
    const std::deque<Task> task_queue_iterable(task_queue.get_container());
    if (task_queue_iterable.empty()) {
        action->UnitCommand(u, MOVE, poi_close_to_enemy[0]);
        return;
    }

    // Get highest priority task in queue
    Task taskToDo = task_queue_iterable[0];
    for (auto& task : task_queue_iterable) {
        if (task.priority > taskToDo.priority) {
            taskToDo = task;
        }
    }

    if (taskToDo.priority != 11) {
        // Determine closest highest priority task to the unit

        // Calculate distances to each task
        std::vector<std::pair<Task, float>> task_distances;
        float furthest_distance = 0;
        for (auto& task : task_queue_iterable) {
            float distance = sc2::Distance2D(u->pos, task.position);
            task_distances.push_back({task, distance});
            furthest_distance = std::max(furthest_distance, distance);
        }
        // Find the task with the highest score (priority minus distance)
        int highest_score = -11;
        for (auto& [task, distance] : task_distances) {
            int normalized_distance = 10 * distance / furthest_distance;
            int score = task.priority - normalized_distance;
            if (score > highest_score) {
                highest_score = score;
                taskToDo = task;
            }
        }
    }

    // Command unit to perform task
    action->UnitCommand(u, taskToDo.ability_id, taskToDo.position);
}

void SCOUT_BOT::upgradeCompleted(sc2::UpgradeID uid) {
    // do nothing
}

void SCOUT_BOT::setAgents(TF_Agent* defenceb, TF_Agent* attackb, TF_Agent* resourceb) {
    this->defence = defenceb;
    this->attack = attackb;
    this->resource = resourceb;
}

std::vector<Unit> SCOUT_BOT::last_seen_near(Point2D location, int radius, int since) {
    std::vector<Unit> ret;

    // run query
    for (auto &record : detection_record) {
        if (record.distance(location) < radius) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - record.time).count() < since) {
                ret.push_back(record.u);
            }
        }
    }

    return ret;
}

void SCOUT_BOT::init() {
    game_epoch = std::chrono::steady_clock::now();

    // order half of the max # of scv at lower priority
    auto t = Task(TRAIN,
                  SCOUT_AGENT,
                  3,
                  ABILITY_ID::TRAIN_SCV,
                  UNIT_TYPEID::TERRAN_SCV,
                  UNIT_TYPEID::TERRAN_COMMANDCENTER);
    for (int i = 0; i < MAX_SCOUT_COUNT/2; ++i) {
        resource->addTask(t);
    }

    gi = observation->GetGameInfo();
    main_base = gi.start_locations.at(0);
    enemy_main_base = gi.start_locations.at(0);

    // base on map name, get all point of interest
    try{
        SQLITE3 scout_POI_db("TF_bot.db"); // open db

        // form query
        SQLITE3_QUERY q("SELECT x, y FROM SCOUT_POI WHERE map = ?;");
        q.add_binding(gi.map_name);

        // execute query
        int ret = scout_POI_db.execute(q); // execute query
        if (ret || scout_POI_db.get_result_row_count() == 0) { // check query success
            throw std::runtime_error("Scout POI Query Failed");
        }

        // load query result to poi
        auto r = scout_POI_db.copy_result();
        for (auto &xy_vec : *r) {
            auto x = std::stod(xy_vec.at(0));
            auto y = std::stod(xy_vec.at(1));
            poi.emplace_back(x,y);
        }
    } catch (std::runtime_error &err) {
        std::cerr << "FATAL ERROR: " << err.what() << std::endl;
        exit(1);
    }

    std::copy(poi.begin(), poi.end(), std::back_inserter(poi_close_to_base));
    std::sort(poi_close_to_base.begin(), poi_close_to_base.end(), [this](const Point2D &p1, const Point2D &p2){
        return distance(this->main_base, p1) < distance(this->main_base, p2);
    });
    std::copy(poi.begin(), poi.end(), std::back_inserter(poi_close_to_enemy));
    std::sort(poi_close_to_base.begin(), poi_close_to_base.end(), [this](const Point2D &p1, const Point2D &p2){
        return distance(this->enemy_main_base, p1) < distance(this->enemy_main_base, p2);
    });
}

double SCOUT_BOT::distance(const Point2D &p1, const Point2D &p2) {
    return sqrt(pow(p1.x-p2.x,2)+pow(p1.y-p2.y,2));
}
