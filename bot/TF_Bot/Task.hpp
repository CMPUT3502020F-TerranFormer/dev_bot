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
enum {BUILD, TRAIN, BASIC_SCOUT, ORBIT_SCOUT, DEFEND, ATTACK, REPAIR, MOVE, UPGRADE, TRANSFER};

/**
 * for who_it_belong
 */
enum {DEFENCE_AGENT, ATTACK_AGENT, RESOURCE_AGENT, SCOUT_AGENT};

/**
 * Task Class
 *
 */
struct Task {
    /**
     * enum {BUILD, TRAIN, BASIC_SCOUT, ORBIT_SCOUT, DEFEND, ATTACK, REPAIR, MOVE, UPGRADE};
     */
    unsigned short action;

    /**
     * enum {DEFENCE_AGENT, ATTACK_AGENT, RESOURCE_AGENT, SCOUT_AGENT};
     */
    unsigned short who_it_belong;

    /**
     * from 0 - 10
     * defence agent have max of 10
     * attack agent have max of 8
     * resource agent have max of 6
     * scout agent have max of 6
     */
    unsigned short priority;

    sc2::UNIT_TYPEID unit_typeid;
    sc2::ABILITY_ID ability_id;
    sc2::Point2D position;

    int count;
};


#endif //TF_TASK_HPP
