//
// Created by Kerry Cao on 2020-10-21.
//

#ifndef TF_TASK_HPP
#define TF_TASK_HPP

/**
 * Possible action that can be taken
 * More can be added
 */
enum {BUILD, TRAIN, BASIC_SCOUT, ORBIT_SCOUT, DEFEND, ATTACK, REPAIR, MOVE, UPGRADE, TRANSFER};

enum {DEFENCE_AGENT, ATTACK_AGENT, RESOURCE_AGENT, SCOUT_AGENT};

/**
 * Task Class
 *
 */
class Task {
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
};


#endif //TF_TASK_HPP
