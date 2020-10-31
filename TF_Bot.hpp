//
// Created by Carter Sabadash on 2020-10-24
//
#pragma once

#include <sc2api/sc2_api.h>
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include <iostream>
#include "threadsafe_priority_queue.h"
#include <vector>

#include "Task.hpp"

using namespace sc2;

/**
 * Data class for a unit belonging to a agent
 */
struct TF_unit {
    TF_unit(UNIT_TYPEID type, Tag tag)
        : type(type), tag(tag)
    {}
    UNIT_TYPEID type;
    Tag tag;
};

class TF_Bot : public Agent 
{
public:
    TF_Bot();

    ~TF_Bot();

    virtual void OnGameStart() final;

    virtual void OnGameEnd() final;

    virtual void OnStep() final;

    virtual void OnUnitDestroyed(const Unit* unit) final;

    virtual void OnUnitCreated(const Unit* unit) final;

    virtual void OnUnitIdle(const Unit* unit) final;

    virtual void OnUpgradeCompleted(UpgradeID uid) final;

    virtual void OnBuildingConstructionComplete(const Unit* unit) final;

    virtual void OnUnitEnterVision(const Unit* unit) final;

private:
    threadsafe_priority_queue<Task> resource_queue;
    std::vector<TF_unit> resource_units;
    threadsafe_priority_queue<Task> attack_queue;
    std::vector<TF_unit> attack_units;
    threadsafe_priority_queue<Task> scout_queue;
    std::vector<TF_unit> scout_units;
    threadsafe_priority_queue<Task> defence_queue;
    std::vector<TF_unit> defence_units;

    #include "resource.hpp"
    #include "defence.hpp"
    #include "attack.hpp"
    #include "scout.hpp"
};