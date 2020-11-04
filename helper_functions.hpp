// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once 

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2api/sc2_unit.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

using namespace sc2;

struct IsDamaged {
    bool operator()(const Unit* unit_) const;
};

// Does not include "combat buildings" (cannon, turrets etc.)
struct IsCombatUnit {
    bool operator()(const Unit* unit_) const;
};

// is Building a Barracks, Factory or Starport?
struct IsBuildingWithSupportForAddon {
    bool operator()(sc2::UNIT_TYPEID type_) const;
};

UNIT_TYPEID whichAddon(ABILITY_ID build_cmd);

bool isAddOnConnected();