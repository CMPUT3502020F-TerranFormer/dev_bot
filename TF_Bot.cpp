//
// Created by Carter Sabadash on 2020-10-24
//
#include "TF_Bot.hpp"
<<<<<<< HEAD
#include "ATTACK_BOT.hpp"
#include "DEFENCE_BOT.hpp"
#include "RESOURCE_BOT.hpp"
#include "SCOUT_BOT.hpp"

TF_Bot::TF_Bot() {
    attack = new ATTACK_BOT(this);
    defence = new DEFENCE_BOT(this);
    scout = new SCOUT_BOT(this);
    resource = new RESOURCE_BOT(this);
}

TF_Bot::~TF_Bot() {

}

void TF_Bot::OnGameStart() {
    attack->setAgents(defence, resource, scout);
    defence->setAgents(attack, resource, scout);
    resource->setAgents(defence, attack, scout);
    scout->setAgents(defence, attack, resource);

    resource->gameStart();
}

void TF_Bot::OnGameEnd() {

}

void TF_Bot::OnStep() {
    resource->step();
    attack->step();
    defence->step();
    scout->step();
=======
#include "helper_functions.hpp"

TF_Bot::TF_Bot()
{
}

TF_Bot::~TF_Bot()
{
}

void TF_Bot::OnGameStart()
{
    // also need to get map name, enemy race -> create & store in TF_Bot variables
    baseManager = new BaseManager(&resource_queue, Observation(), resource_units);
    resourceGameStart();
}

void TF_Bot::OnGameEnd()
{
}

void TF_Bot::OnStep()
{
    attackStep();
    resourceStep();
>>>>>>> origin
}

void TF_Bot::OnUnitDestroyed(const Unit *unit)
{
    baseManager->deleteUnit(unit);
}

void TF_Bot::OnUnitCreated(const Unit *unit)
{
    // template
    baseManager->addUnit(unit);
    resource_units.push_back(unit->tag);
}

void TF_Bot::OnUnitIdle(const Unit *unit)
{
    resourceIdle(unit);
    attackIdle(unit);
}

void TF_Bot::OnUpgradeCompleted(UpgradeID uid)
{
}

void TF_Bot::OnBuildingConstructionComplete(const Unit *unit)
{
    resourceBuildingComplete(unit);
    // will have to refactor but trying it here first

    switch (unit->unit_type.ToType())
    {
    case (UNIT_TYPEID::TERRAN_BARRACKS):
    {
        buildAddOn(ABILITY_ID::BUILD_TECHLAB_BARRACKS, UNIT_TYPEID::TERRAN_BARRACKS);
        break;
    }

    case (UNIT_TYPEID::TERRAN_STARPORT):
    {
        buildAddOn(ABILITY_ID::BUILD_TECHLAB_STARPORT, UNIT_TYPEID::TERRAN_STARPORT);
        break;
    }
    }
}

void TF_Bot::OnUnitEnterVision(const Unit *unit)
{
}