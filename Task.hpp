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
    /** BUILD - RESOURCES; 
     * @param Action : BUILD
     * @param source : The source agent
     * @param priority: The priority
     * @param utype: The UNIT_TYPEID of the unit to build
     * @param aid: The ability id for an scv to produce the structure
     * @param point: The point to place the structure; The task will be removed if it cannot be placed
     */
    Task (enum AgentActions action, enum SourceAgent source, int priority, sc2::UNIT_TYPEID utype, sc2::ABILITY_ID aid, sc2::Point2D point)
        : action(action), source(source), priority(priority), unit_typeid(utype), ability_id(aid), position(point)
    {}

    /** TRAIN - RESOURCES; can specify a specific unit to do the training (aid tells what unit to train), utype must be specified either way
     * @param Action : TRAIN
     * @param source : The source agent
     * @param priority: The priority
     * @param aid: The AbilityID that will produce the required unit (eg. Train SCV)
     * @param utype: The type of unit being produced (eg. scv)
     * @param source_unit: The type of unit producing the unit (eg. Command Center)
     * @param target:   The exact unit that will be used to produce the desired unit (not required)
     *                  It is preferred to specify a target so that an action isn't interrupted when randomly choosing a unit
     */
    Task(enum AgentActions action, enum SourceAgent source, int priority, sc2::ABILITY_ID aid, sc2::UNIT_TYPEID utype,
        sc2::UNIT_TYPEID source_unit, sc2::Tag target = -1)
        : action(action), source(source), priority(priority), ability_id(aid), unit_typeid(utype), source_unit(source_unit), target(target)
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
    int priority;

    sc2::Tag target;        // a specific unit to use, sometimes this is required, otherwise if the exact unit doesn't matter (use nullptr)
    sc2::UNIT_TYPEID unit_typeid;   // the unit_typeid is enough
    sc2::ABILITY_ID ability_id;
    sc2::Point2D position;
    sc2::UNIT_TYPEID source_unit;

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