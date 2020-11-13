//
// Created by Kerry Cao on 2020-11-02.
//

#ifndef TFBOT_UTILITY_HPP
#define TFBOT_UTILITY_HPP

#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "Task.hpp"

/*
SourceAgent findOwner(const Unit* unit) {
	UNIT_TYPEID type = unit->unit_type;
	Tag tag = unit->tag;
}
*/

// Filters for Observation()->GetUnits()
struct IsSCV {
	bool operator() (const Unit& u) {
		switch (u.unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_SCV: return true;
		default: return false;
		}
	}
};

struct IsVespeneRefinery {
	bool operator() (const Unit& u) {
		switch (u.unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_REFINERY: return true;
		default: return false;
		}
	}
};

struct IsCommandCenter {
	bool operator() (const Unit& u) {
		switch (u.unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_COMMANDCENTER: return true;
		case UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING: return true;
		case UNIT_TYPEID::TERRAN_ORBITALCOMMAND: return true;
		case UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING: return true;
		case UNIT_TYPEID::TERRAN_PLANETARYFORTRESS: return true;
		default: return false;
		}
	}
};

struct IsMinerals {
	bool operator() (const Unit& u) {
		switch (u.unit_type.ToType()) {
		case UNIT_TYPEID::NEUTRAL_MINERALFIELD: return true;
		case UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD: return true;
		case UNIT_TYPEID::NEUTRAL_MINERALFIELD450: return true;
		case UNIT_TYPEID::NEUTRAL_MINERALFIELD750: return true;
		case UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750: return true;
		default: return false;
		}
	}
};
struct IsVespeneGeyser {
	bool operator() (const Unit& u) {
		switch (u.unit_type.ToType()) {
		case UNIT_TYPEID::NEUTRAL_VESPENEGEYSER: return true;
		case UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER: return true;
		case UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER: return true;
		default: return false;
		}
	}
};

struct IsUnit {
	IsUnit(UNIT_TYPEID id)
		: uid(id) {}
	UNIT_TYPEID uid;
	bool operator() (const Unit& u) {
		if (u.unit_type == uid) { return true; }
		return false;
	}
};

struct IsClose {
	IsClose(Point2D location, int distanceSquared)
		: p(location), d2(distanceSquared) {}
	int d2;
	Point2D p;
	bool operator() (const Unit& u) {
		if (DistanceSquared2D(u.pos, p) < d2) { return true; }
		return false;
	}
};

#endif //TFBOT_UTILITY_HPP