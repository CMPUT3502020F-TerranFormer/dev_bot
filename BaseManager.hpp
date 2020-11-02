#ifndef BASEMANAGER_HPP
#define BASEMANAGER_HPP
//
// Created by Carter Sabadash October 29
//


#include "Task.hpp"
#include "threadsafe_priority_queue.h"
#include <vector>
#include "TF_unit.hpp"
#include <iostream>

/**
 * The purpose of this class is to manage command centers, scv's and minerals/vespene
 * Currently this will build 3 command centers and mine all resources -> must implement vespene & center building
 * Issues to fix: building >3 centers; moving scv's When we can't build anymore command centers/move them
 * Associating scv's with a specific resource
 */

// these are for CactusValleyLE, for testing purposes; I've only included the first 4 base locations for now (x, y)
static const std::pair<int, int> BASE_LOCATIONS[] = { std::pair<int, int>(156, 32), std::pair<int, int>(124, 28), std::pair<int, int>(136, 56), std::pair<int, int>(96, 32) };
using namespace sc2;

struct Base {
	TF_unit NoUnit = TF_unit((UNIT_TYPEID) 0, (Tag) -1);

	Base() {}
	TF_unit command;
	Point2D location;

	std::vector<TF_unit> scvs;
	std::vector<TF_unit> minerals;
	std::vector<TF_unit> vespene;

	void findResources(const Units units) {
		// must be called after a command center is added, updates with the surrounding resources
		for (auto& p : units) {
			if (p->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD
				&& DistanceSquared2D(location, p->pos) < 225) { // just add minerals close to command center
				minerals.push_back(TF_unit(p->unit_type, p->tag));
			}
			if (p->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER
				&& DistanceSquared2D(location, p->pos) < 225) {
				vespene.push_back(TF_unit(p->unit_type, p->tag));
			}
		}
	}

	bool buildNewCommand() {
		// true if more than half the resources units have been destroyed, and command is a planetary fortress
		if (command.type == UNIT_TYPEID::TERRAN_PLANETARYFORTRESS) {
			if ((vespene.size() + minerals.size()) < 5) { return true; }
		}
		return false;
	}

	bool startTransfer() {
		// start transfer process (move units if planetary; else move units + command) to new location
		// implement this later
		return false;
	}
};

class BaseManager {
public:
	BaseManager(threadsafe_priority_queue<Task>* t_queue, const ObservationInterface* obs)
		: task_queue(t_queue), observation(obs)
	{
		// template
		active_bases.push_back(Base()); // account for if scv's are added before first command center
		Point3D start = observation->GetStartLocation();
		active_bases.front().location = Point2D(start.x, start.y);
		active_bases.front().findResources(observation->GetUnits(Unit::Alliance::Neutral));
		active_bases.front().command = Base().NoUnit;
		scv_count = 0;
	}

	void addUnit(const Unit* u) {
		// add scv's to bases until they are maximally saturated, then add to idleSCVs
		switch (u->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_SCV: {
			++scv_count;
			assignSCV(u);
			break;
		}
		case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
			if (active_bases.size() == 1) { // check for initial case where scv's were added before command center
				if (active_bases.front().command.tag == -1) {
					active_bases.front().command = TF_unit(u->unit_type, u->tag);
				}
				else {
					Base base = Base();
					base.command = TF_unit(u->unit_type, u->tag);
					base.location = u->pos;
					base.findResources(observation->GetUnits(Unit::Alliance::Neutral));
				}
			}
			else {
				Base base = Base();
				base.command = TF_unit(u->unit_type, u->tag);
				base.location = u->pos;
				base.findResources(observation->GetUnits(Unit::Alliance::Neutral));
			}
		}
			// not yet sure how upgrades are handled -> maybe have to add ORBITAL_COMMAND, etc
		}

		// and build command centers at 20/40 scv's
		// PROBLEM: must first determine optimal places for command centers (likely store as a constant) (this should be as close as possible to a mineral group)
		// then choose one following a placement policy (for now, look at the map editor for coords of closes placement to resources, then go through and use the first non-used one
		for (auto& base : active_bases) {
			if (base.buildNewCommand() || scv_count == 20 || scv_count == 40) {
				for (auto& point : BASE_LOCATIONS) {
					bool used = false;
					for (auto& command : active_bases) {
						if (command.location == Point2D(point.first, point.second)) { used = true; }
					}
					if (!used) {
						task_queue->push(Task(BUILD, RESOURCE_AGENT, 6, UNIT_TYPEID::TERRAN_COMMANDCENTER, ABILITY_ID::BUILD_COMMANDCENTER));
						return;
					}
				}
			}
		}
	}

	void assignSCV(const Unit* u) {
		for (auto& p : active_bases) { // saturate bases with scv's
			if (p.command.tag == -1) { return; }
			const Unit* base = observation->GetUnit(p.command.tag);
			if (base->assigned_harvesters < base->ideal_harvesters) {
				task_queue->push(Task(HARVEST, 11, u->tag, ABILITY_ID::SMART, p.minerals.back().tag));
				return;
			}
			// if minerals are saturated, make sure refineries are built
			task_queue->push(Task(BUILD, RESOURCE_AGENT, 6, UNIT_TYPEID::TERRAN_REFINERY,
				ABILITY_ID::BUILD_REFINERY, p.vespene.front().tag));
			task_queue->push(Task(BUILD, RESOURCE_AGENT, 6, UNIT_TYPEID::TERRAN_REFINERY,
				ABILITY_ID::BUILD_REFINERY, p.vespene.back().tag));
			try {
				Units vespene = observation->GetUnits(IsVespeneRefinery());
				for (auto& v : vespene) {
					if (DistanceSquared2D(base->pos, v->pos) <= 225) { // 15**2
						if (v->assigned_harvesters < v->ideal_harvesters) {
							task_queue->push(Task(HARVEST, 11, u->tag, ABILITY_ID::SMART, v->tag));
							return;
						}
					}
				}
			}
			catch (std::exception e) {} // no forseeable reason this would happen, but...
		}
		// build a new command center
	}

	void deleteUnit(const Unit* u) {
		// template
		// if it's a COMMAND CENTER -> build a new one?
		// also have to reassign all scv's (if not building a enw one)
		// for now just decrease scv_count and delete from bases
		if (u->unit_type.ToType() == UNIT_TYPEID::TERRAN_SCV) { --scv_count; }
	}

	TF_unit getSCV(Point2D point = Point2D(0, 0)) {
		// get's an scv that is idle or mining
		// should check the closest scv, for now just gets idle scv
		// should have better selection than just this
		Units scvs = observation->GetUnits(Unit::Alliance::Self, IsSCV());
		for (auto& p : scvs) {
			if (p->orders.empty()) { return TF_unit(p->unit_type, p->tag); }
		}
		for (auto& p : scvs) {
			for (auto& order : p->orders) {
				if (order.ability_id == ABILITY_ID::SMART
					|| order.ability_id == ABILITY_ID::HARVEST_GATHER) {
					return TF_unit(p->unit_type, p->tag);
				}
			}
		}
		return TF_unit(scvs.back()->unit_type, scvs.back()->tag);
	}

	void unitIdle(const Unit* u) {
		// make scv's if not enough; (do orbital_scan if requested)
		switch (u->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_SCV: {
			assignSCV(u);
			break;
		}
		case UNIT_TYPEID::TERRAN_PLANETARYFORTRESS: {
			if (scv_count <= 70) {
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 5, ABILITY_ID::TRAIN_SCV, UNIT_TYPEID::TERRAN_SCV, UNIT_TYPEID::TERRAN_PLANETARYFORTRESS, u->tag));
			}
			break;
		}
		case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
			if (scv_count <= 70) {
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 5, ABILITY_ID::TRAIN_SCV, UNIT_TYPEID::TERRAN_SCV, UNIT_TYPEID::TERRAN_COMMANDCENTER, u->tag));
			}
			break;
		}
		case UNIT_TYPEID::TERRAN_ORBITALCOMMAND: {
			if (scv_count <= 70) {
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 5, ABILITY_ID::TRAIN_SCV, UNIT_TYPEID::TERRAN_SCV, UNIT_TYPEID::TERRAN_ORBITALCOMMAND, u->tag));
			}
			break;
		}
		}
	}

	int getSupplyFloat() {
		// try to keep 2/4/6 supply ahead (1/2/3+ command centers)
		int command_centers = isolated_bases.size() + active_bases.size();
		if (command_centers == 1) { return 2; }
		else if (command_centers == 2) { return 4; }
		else { return 6; }
	}

private:
	threadsafe_priority_queue<Task>* task_queue;
	const ObservationInterface* observation;
	std::vector<TF_unit> isolated_bases; // pretty much empty bases except for (planetary fortress)
	std::vector<Base> active_bases; // should have 3 bases -> potentially 4-6 when transferring to new location
	std::vector<TF_unit> idleSCVs; // leftover scv's when bases are maximally saturated; prefer to take from this
	int scv_count; // total active scv count; aim for 70??

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
};
#endif