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
 * for who_it_belong
 */
enum TargetAgent {DEFENCE_AGENT, ATTACK_AGENT, RESOURCE_AGENT, SCOUT_AGENT};

/**
 * Task Class
 *
 */
struct Task {

    enum AgentActions action;

    enum TargetAgent target;

    /**
     * from 0 - 10
     * defence agent have max of 10
     * attack agent have max of 8
     * resource agent have max of 6
     * scout agent have max of 6
     * ** actions that do not comsume resources should have a priority of 11
     * ** that way they are guaranteed to be seen
     */
    unsigned short priority;

    sc2::UNIT_TYPEID unit_typeid;
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
