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

    // The following are taken from sc2_interfaces.h They will be used by the agents to access units
    //! Get a list of all known units in the game.
    //!< \return List of all ally and visible enemy and neutral units.
    virtual Units GetUnits() const final;

    //! Get all units belonging to a certain alliance and meet the conditions provided by the filter. The unit structure is const data only.
    //! Therefore editing that data will not change any in game state. See the ActionInterface for changing Unit state.
    //!< \param alliance The faction the units belong to.
    //!< \param filter A functor or lambda used to filter out any unneeded units in the list.
    //!< \return A list of units that meet the conditions provided by alliance and filter.
    virtual Units GetUnits(Unit::Alliance alliance, Filter filter = {}) const final;

    //! Get all units belonging to self that meet the conditions provided by the filter. The unit structure is const data only.
    //! Therefore editing that data will not change any in game state. See the ActionInterface for changing Unit state.
    //!< \param filter A functor or lambda used to filter out any unneeded units in the list.
    //!< \return A list of units that meet the conditions provided by the filter.
    virtual Units GetUnits(Filter filter) const final;

    //! Get the unit state as represented by the last call to GetObservation.
    //!< \param tag Unique tag of the unit.
    //!< \return Pointer to the Unit object.
    virtual const Unit* GetUnit(Tag tag) const final;

private:
    ATTACK_BOT* attack;
    DEFENCE_BOT* defence;
    RESOURCE_BOT* resource;
    SCOUT_BOT* scout;
    TSqueue<BasicCommand>* a_queue; // where actions from the bots go
};