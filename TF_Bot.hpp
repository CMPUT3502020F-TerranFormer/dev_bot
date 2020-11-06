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

#include "ATTACK_BOT.hpp"
#include "DEFENCE_BOT.hpp"
#include "RESOURCE_BOT.hpp"
#include "SCOUT_BOT.hpp"
#include "Task.hpp"
#include "Action.hpp"

using namespace sc2;

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
    ATTACK_BOT* attack;
    DEFENCE_BOT* defence;
    RESOURCE_BOT* resource;
    SCOUT_BOT* scout;
    TSqueue<Action> action_queue;
};