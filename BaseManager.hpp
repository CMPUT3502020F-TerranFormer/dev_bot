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

using namespace sc2;

struct Base {
	Base() {
		depleted = false;
		transferring = false;
	}

	bool depleted; // true when resources are all depleted -> start transferring process
	bool transferring; // in the process transferring from one location to another
	TF_unit command;
	Point2D location;

	// for now, just keep a vector of scv's
	// and a vector of mineral/vespene units
	std::vector<TF_unit> scvs;
	std::vector<TF_unit> minerals;
	std::vector<TF_unit> vespene; // also need to store refineries

	// ideally, later we want to maximize scv's/resource

	void findResources(const Units units) {
		// must be called after a command center is added, updates with the surrounding resources
		for (auto& p : units) {
			if ((p->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD
				|| p->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD750)
				&& DistanceSquared2D(location, p->pos) < 400) { // just add minerals close to command center
				minerals.push_back(TF_unit(p->unit_type, p->tag));
			}
			if (p->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER
				&& DistanceSquared2D(location, p->pos) < 400) {
				vespene.push_back(TF_unit(p->unit_type, p->tag));
			}
		}
	}

	bool buildNewCommand() { 
		// true if more than half the resources units have been destroyed, and command is a planetary fortress
		return false;
	}

	bool startTransfer() {
		// start transfer process (move units if planetary; else move units + command) to new location
		return false;
	}

	void unitIdle(const Unit* u, const ObservationInterface* o, threadsafe_priority_queue<Task>* task_queue) {
		// template; this is poorly done and doesn't really work because scv's auto mine minerals --> that also need to be fixed
		// if this base has this unit -> first try to assign to vespene
		// -> then assign to minerals
		for (auto& scv : scvs) {
			if (scv.tag == u->tag) {
				const Units units = o->GetUnits(Unit::Alliance::Self);
				for (auto& p : units) {
					if (p->unit_type == UNIT_TYPEID::TERRAN_REFINERY) {
						if (p->assigned_harvesters < p->ideal_harvesters) {
							task_queue->push(Task(HARVEST, 11, scv.tag, ABILITY_ID::SMART, p->tag));
							return;
						}
					}
				}
				// otherwise assign to nearest mineral target
				// for simplicity:
				const Unit* mineral = o->GetUnit(minerals.front().tag);
				task_queue->push(Task(HARVEST, 11, scv.tag, ABILITY_ID::SMART, mineral->tag));
			}
		}
	}

	void deleteUnit(const Unit* u) {
		// if this base has this unit; delete it
		if (u->tag == command.tag) {
			command.tag = -1;
			return;
		}
		for (auto it = scvs.cbegin(); it != scvs.cend(); ++it) {
			if (it->tag == u->tag) {
				scvs.erase(it);
				return;
			}
		}
	}

};

class BaseManager {
public:
	BaseManager(threadsafe_priority_queue<Task>* t_queue, const ObservationInterface* obs)
		: task_queue(t_queue), observation(obs)
	{
		// template
		scv_count = 0;
	}

	void addUnit(const Unit* u) {
		// template, for now, there is one base
		// we need to make sure this works when scv's are added before the command center (when there is no bases yet)
		if (u->unit_type.ToType() == UNIT_TYPEID::TERRAN_COMMANDCENTER) {
			if (!active_bases.size() == 0 && active_bases.front().location == Point2D()) {
				active_bases.front().command = TF_unit(u->unit_type, u->tag);
				active_bases.front().location = u->pos;
				active_bases.front().findResources(observation->GetUnits(Unit::Alliance::Neutral));
			} else {
				Base base = Base();
				base.command = TF_unit(u->unit_type, u->tag);
				base.location = u->pos;
				base.findResources(observation->GetUnits(Unit::Alliance::Neutral));
				active_bases.push_back(base);
			}
			std::cout << "Add: " << "COMMAND CENTER" << std::endl;
		}
		if (u->unit_type.ToType() == UNIT_TYPEID::TERRAN_SCV) {
			++scv_count;
			if (active_bases.size() == 0) {
				active_bases.push_back(Base());
			}
			active_bases.back().scvs.push_back(TF_unit(UNIT_TYPEID::TERRAN_SCV, u->tag));
			std::cout << "Add: " << "SCV" << std::endl;


			for (auto& base : active_bases) {
				// for now, use 17/18 scv's as when to build on vespene (no idea if they are optimal #'s)
				if (base.scvs.size() == 17) {
					task_queue->push(Task(BUILD, RESOURCE_AGENT, 6, UNIT_TYPEID::TERRAN_REFINERY,
						ABILITY_ID::BUILD_REFINERY, observation->GetUnit(base.vespene.back().tag)->pos, base.vespene.front().tag));
				}
				else if (base.scvs.size() == 18) {
					task_queue->push(Task(BUILD, RESOURCE_AGENT, 6, UNIT_TYPEID::TERRAN_REFINERY, 
						ABILITY_ID::BUILD_REFINERY, observation->GetUnit(base.vespene.back().tag)->pos, base.vespene.back().tag));
				}

				// and build command centers at 20/40 scv's
				// PROBLEM: must first determine optimal places for command centers (likely store as a constant) (this should be as close as possible to a mineral group)
				// then choose one following a placement policy (closest for now)
			}
		}
		// must also deal with MULES, etc
	}

	void deleteUnit(const Unit* u) {
		// template
		// if it's a COMMAND CENTER -> build a new one?
		// also have to reassign all scv's (if not building a enw one)
		if (u->unit_type.ToType() == UNIT_TYPEID::TERRAN_SCV) { --scv_count; }
		for (auto& base : active_bases) { base.deleteUnit(u); }
	}

	const TF_unit getSCV(Point2D point = Point2D(0, 0)) {
		// get's a free scv, or determines the best scv to use
		// prefers scv's that are mining resources or idle
		// template
		// should check the closest base; right now only checks first
		// should have better selection than just this
		for (auto& scv : active_bases.front().scvs) {
			const Unit* unit = observation->GetUnit(scv.tag);
			if (unit == nullptr) {
				continue;
			} // this is important -> I don't know why, but sometimes no unit is returned
			for (auto &order : unit->orders) {
				if (order.ability_id == ABILITY_ID::HARVEST_GATHER) { return TF_unit(unit->unit_type, unit->tag); }
			}
		}
		// safety net
		return active_bases.front().scvs.front();
	}

	void idleUnit(const Unit* u) {
		// make scv's if not enough; (do orbital_scan if requested)
		// template
		switch (u->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_SCV:
			for (auto& base : active_bases) { base.unitIdle(u, observation, task_queue);  }
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
			if (orbital_scan) {
				//task_queue->push(Task(ORBIT_SCOUT, RESOURCE_AGENT, 6, u, UNIT_TYPEID::TERRAN_ORBITALCOMMAND, ABILITY_ID::EFFECT_SCAN));
				// figure out what the actual ABILITY_ID for orbital scan is
				orbital_scan = false;
			}
			else if (scv_count <= 70) {
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
	int scv_count; // aim for 70
	bool orbital_scan; // when an orbital scan is requested, then it
};

#endif