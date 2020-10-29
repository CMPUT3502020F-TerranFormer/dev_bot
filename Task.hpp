//
// Created by Kerry Cao on 2020-10-21.
//

#ifndef TF_TASK_HPP
#define TF_TASK_HPP

#include <sc2api/sc2_api.h>

/**
 * Possible action that can be taken
 * More can be added
 */
enum AgentActions {BUILD, TRAIN, BASIC_SCOUT, ORBIT_SCOUT, DEFEND, ATTACK, REPAIR, MOVE, UPGRADE, TRANSFER};

/**
 * the source agent
 */
enum SourceAgent {DEFENCE_AGENT, ATTACK_AGENT, RESOURCE_AGENT, SCOUT_AGENT};

/**
 * Task Class
 */
struct Task {
    // see individual agent header files for instructions about what type of tasks they accept
    Task(enum AgentActions action, enum SourceAgent source, unsigned short priority, sc2::Tag target,
        sc2::UNIT_TYPEID unit_typeid, sc2::ABILITY_ID aid, sc2::Point2D pos = sc2::Point2D(), int count = 1)
        : action(action), source(source), priority(priority), target(target), unit_typeid(unit_typeid),
        ability_id(aid), position(pos), count(count)
    {}

    enum AgentActions action;

    enum SourceAgent source;

    /**
     * from 0 - 10
     * defence agent have max of 10
     * attack agent have max of 8
     * resource agent have max of 6
     * scout agent have max of 
     * Tasks that do not require resources should use priority 11
     * to guarantee that they will be seen
     */
    unsigned short priority;

    sc2::Tag target;        // a specific unit to use, sometimes this is required, otherwise if the exact unit doesn't matter, use 0 for NULL
    sc2::UNIT_TYPEID unit_typeid;   // the unit_typeid is enough
    sc2::ABILITY_ID ability_id;
    sc2::Point2D position;

    int count;

    /**
     * Compares the priority of tasks;
     * implemented for priority queue
     */
    bool operator<(const Task& r) const
    {
        return priority < r.priority;
    }
};


#endif //TF_TASK_HPP
