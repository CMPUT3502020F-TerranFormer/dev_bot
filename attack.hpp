#pragma once

void attackGameStart();

void attackStep();

void attackIdle(const Unit* unit);

int CountUnitType(const ObservationInterface* observation, UnitTypeID unit_type);

void buildBarracks();

void buildFactory();

void buildStarport();