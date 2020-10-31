#ifndef BASEMANAGER_HPP
#define BASEMANAGER_HPP

//
// Created by Carter Sabadash October 29
//


#include "Task.hpp"
#include "threadsafe_priority_queue.h"
#include <vector>
#include "TF_unit.hpp"

/**
 * The purpose of this class is to manage command centers, scv's and minerals/vespene
 * Currently it's purpose is to try and have 3 Bases which are always mining resources
 * Issues to fix: When we can't build anymore command centers/move them
 */

using namespace sc2;

struct Base {
	bool depleted; // true when resources are all depleted -> start transferring process
	bool transferring; // in the process transferring from one location to another
	TF_unit command;

	bool buildNewCommand() { 
		// true if more than half the resources units have been destroyed, and command is a planetary fortress

	}

	bool startTransfer() {
		// start transfer process (move units if planetary; else move units + command) to new location

	}

};

class BaseManager {
public:
	BaseManager(threadsafe_priority_queue<Task>* task_queue)
		: task_queue(task_queue)
	{
		scv_count = 0;

	}

	void addUnit(const Unit* u) {
		// template
		if (u->unit_type.ToType() == UNIT_TYPEID::TERRAN_SCV) { ++scv_count;  }
	}

	void deleteUnit(const Unit* u) {
		// template
		if (u->unit_type.ToType() == UNIT_TYPEID::TERRAN_SCV) { --scv_count; }
	}

	const Unit* getSCV(Point2D point = Point2D(0, 0)) {
		// get's a free scv, or determines the best scv to use
		return nullptr;
	}

	void findResources(const Units units) {
		// must be called after a command center is added, updates with the surrounding resources
	}

	void idleUnit(const Unit* u) {
		// make scv's if not enough; (do orbital_scan if requested)
		switch (u->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_SCV:
			break;
		case UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
			if (scv_count <= 70) {
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 5, ABILITY_ID::TRAIN_SCV, UNIT_TYPEID::TERRAN_ORBITALCOMMAND, u));
			}
			break;
		case UNIT_TYPEID::TERRAN_COMMANDCENTER:
			if (scv_count <= 70) {
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 5, ABILITY_ID::TRAIN_SCV, UNIT_TYPEID::TERRAN_ORBITALCOMMAND, u));
			}
		case UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
			if (orbital_scan) {
				task_queue->push(Task(ORBIT_SCOUT, RESOURCE_AGENT, 6, u, UNIT_TYPEID::TERRAN_ORBITALCOMMAND, ABILITY_ID::EFFECT_SCAN));
				// figure out what the actual ABILITY_ID for orbital scan is
				orbital_scan = false;
			}
			else if (scv_count <= 70) {
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 5, ABILITY_ID::TRAIN_SCV, UNIT_TYPEID::TERRAN_ORBITALCOMMAND, u));
			}
			break;
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
	std::vector<TF_unit> isolated_bases; // pretty much empty bases except for (planetary fortress)
	std::vector<Base> active_bases; // should have 3 bases -> potentially 4-6 when transferring to new location
	int scv_count; // aim for 70
	bool orbital_scan; // when an orbital scan is requested, then it 
};

#endif