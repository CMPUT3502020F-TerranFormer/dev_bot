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
	// for now, just keep a vector of scv's
	// and a vector of mineral/vespene units
	std::vector<TF_unit> scvs;
	std::vector<TF_unit> minerals;
	std::vector<TF_unit> vespene;

	// ideally, later we want to maximize scv's/resource

	bool buildNewCommand() { 
		// true if more than half the resources units have been destroyed, and command is a planetary fortress
		return false;
	}

	bool startTransfer() {
		// start transfer process (move units if planetary; else move units + command) to new location
		return false;
	}

};

class BaseManager {
public:
	BaseManager(threadsafe_priority_queue<Task>* t_queue, const ObservationInterface* obs)
		: task_queue(t_queue), observation(obs)
	{
		// template
		scv_count = 0;
		active_bases.push_back(Base());
	}

	void addUnit(const Unit* u) {
		// template, for now, there is one base
		if (u->unit_type.ToType() == UNIT_TYPEID::TERRAN_COMMANDCENTER) { 
			active_bases.data()[0].command = TF_unit(UNIT_TYPEID::TERRAN_COMMANDCENTER, u->tag);
			std::cout << "Add: " << "Command Center" << std::endl;
		}
		if (u->unit_type.ToType() == UNIT_TYPEID::TERRAN_SCV) {
			++scv_count;
			active_bases.data()[0].scvs.push_back(TF_unit(UNIT_TYPEID::TERRAN_SCV, u->tag));
			std::cout << "Add: " << "SCV" << std::endl;
		}
	}

	void deleteUnit(const Unit* u) {
		// template
		if (u->unit_type.ToType() == UNIT_TYPEID::TERRAN_SCV) { --scv_count; }
	}

	const TF_unit getSCV(Point2D point = Point2D(0, 0)) {
		// get's a free scv, or determines the best scv to use
		// prefers scv's that are mining resources or idle
		//template
		return active_bases.data()[0].scvs.back();
	}

	void findResources(const Units units) {
		// must be called after a command center is added, updates with the surrounding resources
	}

	void idleUnit(const Unit* u) {
		// make scv's if not enough; (do orbital_scan if requested)
		// template
		switch (u->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_SCV:
			break;
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