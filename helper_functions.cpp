// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "helper_functions.hpp"

bool IsDamaged::operator()(const Unit *unit_) const
{
    return unit_->health < unit_->health_max;
}

bool IsCombatUnit::operator()(const Unit *unit_) const
{
    switch (unit_->unit_type.ToType())
    {
    case sc2::UNIT_TYPEID::TERRAN_BANSHEE:
    case sc2::UNIT_TYPEID::TERRAN_CYCLONE:
    case sc2::UNIT_TYPEID::TERRAN_GHOST:
    case sc2::UNIT_TYPEID::TERRAN_HELLION:
    case sc2::UNIT_TYPEID::TERRAN_HELLIONTANK:
    case sc2::UNIT_TYPEID::TERRAN_LIBERATOR:
    case sc2::UNIT_TYPEID::TERRAN_LIBERATORAG:
    case sc2::UNIT_TYPEID::TERRAN_MARAUDER:
    case sc2::UNIT_TYPEID::TERRAN_MARINE:
    case sc2::UNIT_TYPEID::TERRAN_MEDIVAC:
    case sc2::UNIT_TYPEID::TERRAN_RAVEN:
    case sc2::UNIT_TYPEID::TERRAN_REAPER:
    case sc2::UNIT_TYPEID::TERRAN_SIEGETANK:
    case sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED:
    case sc2::UNIT_TYPEID::TERRAN_THOR:
    case sc2::UNIT_TYPEID::TERRAN_THORAP:
    case sc2::UNIT_TYPEID::TERRAN_VIKINGASSAULT:
    case sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
    case sc2::UNIT_TYPEID::TERRAN_WIDOWMINE:
    case sc2::UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED:
        return true;

    default:
        return false;
    }
}

bool IsBuildingWithSupportForAddon::operator()(sc2::UNIT_TYPEID type_) const {
    switch (type_) {
        case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
        case sc2::UNIT_TYPEID::TERRAN_BARRACKSFLYING:
        case sc2::UNIT_TYPEID::TERRAN_FACTORY:
        case sc2::UNIT_TYPEID::TERRAN_FACTORYFLYING:
        case sc2::UNIT_TYPEID::TERRAN_STARPORT:
        case sc2::UNIT_TYPEID::TERRAN_STARPORTFLYING:
            return true;

        default:
            return false;
    }
}
