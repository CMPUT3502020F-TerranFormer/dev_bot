#pragma once

void attackGameStart();

void attackStep();

void attackIdle(const Unit* unit);

void attackUpgradeCompleted(UpgradeID uid);

void train_unit(const Unit *trainer_unit, ABILITY_ID unit_to_train);

int CountUnitType(const ObservationInterface* observation, UnitTypeID unit_type);

bool buildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV);

bool buildBarracks();

bool buildFactory();

bool buildStarport();