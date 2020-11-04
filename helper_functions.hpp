// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "TF_Bot.hpp"

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