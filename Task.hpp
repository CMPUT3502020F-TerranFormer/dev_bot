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

enum AgentActions { HARVEST, BUILD, TRAIN, BASIC_SCOUT, ORBIT_SCOUT, DEFEND, ATTACK, REPAIR, MOVE, UPGRADE, TRANSFER };


/**
 * the source agent
 */
enum SourceAgent { DEFENCE_AGENT, ATTACK_AGENT, RESOURCE_AGENT, SCOUT_AGENT };


/**
 * Task Class
 */
struct Task {
    /**HARVEST - RESOURCES -- for internal use of resources
     * @param action: HARVEST
     * @param priority: priority -> 11+
     * @param source_unit: The scv
     * @param aid: The ability id
     * @param target: The mineral field/refinery
     */
    Task(enum AgentActions action, int priority, sc2::Tag source, sc2::ABILITY_ID aid, sc2::Tag target)
            : action(action), priority(priority), self(source), ability_id(aid), target(target) {}

    /** BUILD - RESOURCES;
     * @param Action : BUILD
     * @param source : The source agent
     * @param priority: The priority
     * @param utype: The UNIT_TYPEID of the unit to build
     * @param aid: The ability id for an scv to produce the structure
     * @param point: The point to place the structure; The task will be removed if it cannot be placed
     */
    Task(enum AgentActions action, enum SourceAgent source, int priority, sc2::UNIT_TYPEID utype, sc2::ABILITY_ID aid, sc2::Point2D point)
        : action(action), source(source), priority(priority), unit_typeid(utype), ability_id(aid), position(point)
    {
        target = -1; // for RESOURCE functionality
    }

    /** BUILD - RESOURCES;
     * @param Action : BUILD
     * @param source : The source agent
     * @param priority: The priority
     * @param utype: The UNIT_TYPEID of the unit to build
     * @param aid: The ability id for an scv to produce the structure
     * @param target: The unit to build a structure on (ie. building refineries)
     */
    Task(enum AgentActions action, enum SourceAgent source, int priority, sc2::UNIT_TYPEID utype, sc2::ABILITY_ID aid,
         sc2::Tag target = -1)
            : action(action), source(source), priority(priority), unit_typeid(utype), ability_id(aid), target(target) {
        position = sc2::Point2D(0, 0); // for RESOURCE functionality
    }

    /** TRAIN - RESOURCES; can specify a specific unit to do the training (aid tells what unit to train), utype must be specified either way
     * @param action : TRAIN
     * @param source : The source agent
     * @param priority: The priority
     * @param aid: The AbilityID that will produce the required unit (eg. Train SCV)
     * @param utype: The type of unit being produced (eg. scv)
     * @param source_unit: The type of unit producing the unit (eg. Command Center)
     * @param target:   The exact unit that will be used to produce the desired unit (not required)
     *                  It is preferred to specify a target so that an action isn't delayed because of queuing
     */
    Task(enum AgentActions action, enum SourceAgent source, int priority, sc2::ABILITY_ID aid, sc2::UNIT_TYPEID utype,
         sc2::UNIT_TYPEID source_unit, sc2::Tag target = -1)
            : action(action), source(source), priority(priority), ability_id(aid), unit_typeid(utype),
              source_unit(source_unit), target(target) {}

    /** REPAIR - RESOURCES; specify which unit needs repairing -- it should be in a safe location
     * (like by an active command center) When checking unit health, be sure to also check build progress!
     * @param action: REPAIR
     * @param source: The source agent
     * @param priority: The priority
     * @param target: The unit to repair
     * @param aid: The ABILITY_ID of the repair type that needs to be performed
     * @param count: The maximum number of scvs that are allowed to repair the target at once (default 1)
     */
    Task(enum AgentActions action, enum SourceAgent source, int priority, sc2::Tag target, sc2::ABILITY_ID aid, int count = 1)
        : action(action), source(source), priority(priority), target(target), ability_id(aid), count(count)
    {}

    /** MOVE - RESOURCES; the type of movement (ABILITY_ID) must be specified
     * @param Action : MOVE
     * @param source : The source agent
     * @param priority: The priority (should probably be > 10)
     * @param target : The unit that will be ordered to move
     * @param aid: The ABILITY_ID of the type of movement
     * @param position: The position to move to
     */
    Task(enum AgentActions action, enum SourceAgent source, int priority, sc2::Tag target, sc2::ABILITY_ID aid,
         sc2::Point2D position)
            : action(action), source(source), priority(priority), ability_id(aid), position(position) {}

    /** ATTACK - ATTACK_AGENT; Sending troops to attack a given location
     * Takes a vector of tags rather than a single tag
     * @param action : ATTACK
     * @param source : Source agent
     * @param priority : The priority
     * @param units : The units to move
     * @param aid : The ABILITY_ID of the type of attack
     * @param position : The position to move to (will be enemy location for now)
     */
    Task(enum AgentActions action, enum SourceAgent source, int priority, std::vector<sc2::Unit*> units,
         sc2::ABILITY_ID aid, sc2::Point2D position)
            : action(action), source(source), priority(priority), units(std::move(units)), ability_id(aid), position(position) {}

    /** UPGRADE - RESOURCES This is for the upgrade task
     * @param action : UPGRADE
     * @param source : The source agent
     * @param priority: The priority
     * @param source_unit: The unit that will be doing the upgrade
     * @param uid: The UpGradeID of the upgrade
     * @param aid: The AbilityID the source_unit will use to perform the upgrade
     */
    Task(enum AgentActions action, enum SourceAgent source, int priority, sc2::Tag source_unit, sc2::UPGRADE_ID uid, sc2::ABILITY_ID aid)
        : action(action), source(source), priority(priority), self(source_unit), upgrade_id(uid), ability_id(aid)
    {}

    /** TRANSFER - RESOURCES, ATTACK
     * @param action : TRANSFER
     * @param source : The source agent
     * @param priority: The priority
     * @param source_unit : The Tag of the source unit, -1 for any scv (must be valid for ATTACK)
     */
    Task(enum AgentActions action, enum SourceAgent source, int priority, sc2::Tag source_unit)
        : action(action), source(source), priority(priority), self(source_unit)
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

    sc2::Tag target;                
    sc2::UNIT_TYPEID unit_typeid;  
    sc2::ABILITY_ID ability_id;
    sc2::Point2D position;
    sc2::UNIT_TYPEID source_unit;
    sc2::Tag self;
    sc2::UPGRADE_ID upgrade_id;

    std::vector<sc2::Unit*> units;

    int count;

    /**
     * Compares the priority of tasks;
     * implemented for priority queue
     */
    bool operator<(const Task &r) const {
        return priority < r.priority;
    }
};


#endif //TF_TASK_HPP