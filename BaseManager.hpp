#ifndef BASEMANAGER_HPP
#define BASEMANAGER_HPP

//
// Created by Carter Sabadash October 29
//

#include "Task.hpp"
#include "threadsafe_priority_queue.h"
#include <vector>
#include "TF_unit.hpp"
#include "utility.hpp"
#include <algorithm>
#include "BuildingPlacementManager.hpp"
#include <iostream>
#include <random>


/**
 * The purpose of this class is to manage command centers, scv's and minerals/vespene
 * Currently this will build 3 command centers and mine all resources
 * And it heals all damaged units around an undamaged command center with radius 10
 * Issue: MULE functionality is incomplete and may not work (at all?)
 *		We are not currently using mules
 */

using namespace sc2;

static const TF_unit NoUnit = TF_unit(UNIT_TYPEID::INVALID, (Tag) -1);

struct Base {

	Base() {
		transfer = false;
	}
	TF_unit command;
	Point2D location;
	bool transfer;

	std::vector<TF_unit> minerals;
	std::vector<TF_unit> vespene;

	void findResources(const Units units) {
		// must be called after a command center is added, updates with the surrounding resources
		minerals.clear();
		vespene.clear();
		IsMinerals min;
		IsVespeneGeyser vesp;
		for (auto& p : units) {
			if (min(*p) && DistanceSquared2D(location, p->pos) < 225 // just add minerals close to command center
				&& p->mineral_contents > 0) {						 // must also have resources
				minerals.push_back(TF_unit(p->unit_type, p->tag));
			}
			if (vesp(*p) && DistanceSquared2D(location, p->pos) < 225
				&& p->vespene_contents > 0) {
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

	/* Returns if the minerals are depleted;
	 */
	bool depleted() {
		return (minerals.empty() && vespene.empty());
	}

	/** By waiting until all resources are gone we often lose scv efficiency
	 * so wait until 1/2 resources are gone
	 * findResources must be called first to ensure that we have accurate information
	 * about the state of resources
	 */
	bool startTransfer() {
		// start transfer process (move units if planetary; else move units + command) to new location
		// implement this later
		if ((minerals.size() <= 4 || vespene.size() <= 1) && transfer == false) {
			transfer = true; // we don't want to build multiple bases in place of one
			return true;
		}
		return false;
	}
};

class BaseManager {
public:
	BaseManager(threadsafe_priority_queue<Task>* t_queue, const ObservationInterface* obs,
		std::vector<Tag>* units)
		: task_queue(t_queue), observation(obs), resource_units(units)
	{
		Base base; // to account for if scv's are added before first command center
		Point3D start = observation->GetStartLocation();
		base.location = Point2D(start.x, start.y);
		base.findResources(observation->GetUnits(Unit::Alliance::Neutral));
		base.command = NoUnit;
		active_bases.push_back(base);

		scv_count = 0;
		scv_target_count = 60;

		std::random_device r;
		rand_gen = std::mt19937(r());
	}

	/**
	 * This handles the actions that will be made during a step
	 * It's easier (and more clear) to handle stuff like building refineries
	 * in here instead of when a unit is added/idle/deleted
	 * This automatically attempts to repair all units close to the command center (10 units)
	 */
	void step(Units scvs) {
		// Stuff that is particular to each base, such as refineries, handling excess scv's
		// we must check that build progress is complete when looking for units that can be repaired

		// update every 1/4 second
		if (observation->GetGameLoop() % 4 != 0) { return; }

		// when re-assigning harvesting scv's it doesn't matter if we are in control of them or not
		for (auto& base : active_bases) {
			// smaller range than 15, should still include refineries and immediate units
			Units units = observation->GetUnits(Unit::Alliance::Self, IsClose(base.location, 100)); 
			const Unit* command = observation->GetUnit(base.command.tag);
			if (command == nullptr) { return; }

			// do not build refineries or repair other units while the command center is damaged
			// it is more important to repair the command center (max 6 scvs)
			if (command->health < command->health_max
				&& command->build_progress >= 1) {
				task_queue->push(Task(REPAIR, RESOURCE_AGENT, 8, command->tag, ABILITY_ID::EFFECT_REPAIR, 6));
			}
			else {
				// if they are already built, this won't do anything; but it is simpler
				// to just try than to check if they are not built, as we don't have complete vision
				// the turn that a command center is completed (trying to build on a snapshot doesn't appear to work
				// because we have to get the unit
				if (command->build_progress >= 1) {
					buildRefineries(command);
				}

				// repair all units that are close to the command center and try to finish unfinished buildings
				for (auto& unit : units) {
					if (unit->health < unit->health_max
						&& unit->build_progress >= 1) {
						task_queue->push(Task(REPAIR, RESOURCE_AGENT, 8, unit->tag, ABILITY_ID::EFFECT_REPAIR, 1));
					}
					else if (unit->build_progress < 1) {
						/* We first need to figure out how to determine if an scv is working on a building
						* (They don't have a target tag, and only a position as far as I'm currently aware)
						task_queue->push(Task(REPAIR, RESOURCE_AGENT, 8, unit->tag, ABILITY_ID::SMART, 1));
						*/
					}
				}
			}

			// we also deal with bases that have too many scvs mining (1 scv / base / step)
			if (command->assigned_harvesters > command->ideal_harvesters) {
				for (auto& unit : units) {
					if (IsCarryingMinerals(*unit)) {
						assignSCV(unit);
						break;
					}
				}
			}

			// we must also make sure the base still has resources, if not move it to isolated_bases
			// or start the transfer process --> implement later
			base.findResources(observation->GetUnits(Unit::Alliance::Neutral));

			// deal with bases that are running out of resources
			if (command->build_progress == 1) {
				// when a command center is just built not all resources are in vision, so check the # ideal harvesters is not max
				if (base.startTransfer() && command->ideal_harvesters <= 8) {
					task_queue->push(Task(BUILD, RESOURCE_AGENT, 6,
						UNIT_TYPEID::TERRAN_COMMANDCENTER, ABILITY_ID::BUILD_COMMANDCENTER));
				}
				else if (base.depleted()) {
					isolated_bases.push_back(base.command);
					for (auto it = active_bases.cbegin(); it != active_bases.cend(); ++it) {
						if (base.command.tag == it->command.tag) {
							active_bases.erase(it);
							break;
						}
					}
				}
			}
		}

		// Isolated bases have a subset of the actions of active_bases
		// they still repair nearby units to make it easy for the other agents,
		// but it is preferred to send them to active bases
		for (auto& base : isolated_bases) {
			// smaller range than 15, should still include refineries and immediate units
			const Unit* command = observation->GetUnit(base.tag);
			if (command == nullptr) { return; }
			Units units = observation->GetUnits(Unit::Alliance::Self, IsClose(command->pos, 100));

			// center is repaired, then it is probably safe
			if (command->health < command->health_max) {
				task_queue->push(Task(REPAIR, RESOURCE_AGENT, 6, command->tag, ABILITY_ID::EFFECT_REPAIR, 6));
			}
			else {
				// repair all units that are close to the command center (with less priority, there are likely no nearby scvs
				for (auto& unit : units) {
					if (unit->health < unit->health_max
						&& unit->build_progress >= 1) {
						task_queue->push(Task(REPAIR, RESOURCE_AGENT, 6, unit->tag, ABILITY_ID::EFFECT_REPAIR, 1));
					}
				}
			}
		}

		// Build a new command center if necessary, right now, we use preset conditions
		// remember that a unit is added when it starts being build
		// build command centers at 16/36 scv's, or when a planetary fortress is running out of resources
		int num_command_centers = active_bases.size() + isolated_bases.size();
		bool build = false;

		// to stay alive as long as possible
		auto command_build_priority = 7;
		if (num_command_centers == 0) { 
			build = true;
			command_build_priority = 20;
		}
		else if (active_bases.size() < 2 && scv_count >= 16) { build = true; }
		else if (active_bases.size() < 3 && scv_count >= 36) { build = true; }

		// then check if we have a planetary fortress that is running out of resources
		// build a new command center in advance so there is less idle time
		// ISSUE: Implement base transfers with LIFT OFF
		for (auto& base : active_bases) {
			if (base.buildNewCommand()) {
				build = true;
				break;
			}
		}

		if (build) {
			task_queue->push(Task(BUILD, RESOURCE_AGENT, command_build_priority,
				UNIT_TYPEID::TERRAN_COMMANDCENTER, ABILITY_ID::BUILD_COMMANDCENTER));
		}

		// we must also deal with vespene refineries that have excess workers
		// (or we have too much vespene compared to minerals)
		Units refineries = observation->GetUnits(Unit::Alliance::Self, IsVespeneRefinery());
		for (auto& r : refineries) { // focus oversaturated refineries first
			if (r->assigned_harvesters > r->ideal_harvesters) {
				for (auto& s : scvs) {
					if (IsCarryingVespene(*s)) {
						assignSCV(s);
						return;
					}
				}
			}
		}

		// now deal with balancing working gas and minerals
		for (auto& r : refineries) { // focus oversaturated refineries first
			if ((float) observation->GetVespene() / (float) (observation->GetMinerals() + 1) > 1.5) {
				for (auto& s : scvs) {
					if (IsCarryingVespene(*s)) {
						assignSCV(s, false);
						return;
					}
				}
			}
		}
		if ((float) observation->GetVespene() / (float) (observation->GetMinerals() + 1) < 0.4) {
			for (auto& s : scvs) {
				if (IsCarryingMinerals(*s)) {
					assignSCV(s, false);
					return;
				}
			}
		}
	}

	void addUnit(const Unit* u) {
		// Add MULES?? -> should still work because they are included in unitIdle()
		switch (u->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_SCV: {
			++scv_count;
			assignSCV(u);
			break;
		}
		case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
			if (active_bases.size() == 1
				&& active_bases.front().command.tag == -1) { // check for initial case where scv's were added before command center
				active_bases.front().command = TF_unit(u->unit_type, u->tag);
			}
			else {
				Base base = Base();
				base.command = TF_unit(u->unit_type, u->tag);
				base.location = u->pos;
				base.findResources(observation->GetUnits(Unit::Alliance::Neutral));
				active_bases.push_back(base);
			}
		}
		// not yet sure how upgrades are handled -> maybe have to add ORBITAL_COMMAND, etc
		}
	}

	/**
	 * Takes appropriate actions for the type of deleted unit
	 * This does not modify resource_units; that's for the resource agent
	 * and it does not try to build replacement unit (step())
	 */
	void deleteUnit(const Unit* u) {
		if (u->alliance != Unit::Alliance::Self) { return; }

		if (u->unit_type.ToType() == UNIT_TYPEID::TERRAN_SCV) { --scv_count; }

		if (IsCommandCenter()(*u)) { // it is necessary to remove the base so scvs will be properly assigned
			for (auto it = active_bases.cbegin(); it != active_bases.cend(); ++it) {
				if (it->command.tag == u->tag) {
					active_bases.erase(it);
					return;
				}
			}
			for (auto it = isolated_bases.cbegin(); it != isolated_bases.cend(); ++it) {
				if (it->tag == u->tag) {
					isolated_bases.erase(it);
					return;
				}
			}
		}
	}

	/**
	 * Get's an scv that is idle or harvesting; preferring close scvs
	 * If there are no close scvs idle or harvesting (<15 away from point)
	 * Then a search is performed for idle or harvesting scvs disregarding distance
	 * This does not prefer idle scv's over harvesting; idle scvs will just take over harvesting
	 * This may return the same scv if called multiple times during the same step
	 * (a random available scv is selected to minimize this chance)
	 * It is recommended to call this method without a point when you know there is likely only 1 scv
	 * nearby and you want to build multiple buildings (they will all queue to the same scv
	 *	we don't know the scv queue size)
	 * 
	 * We pass in scvs because this may be called multiple times a step
	 */
	TF_unit getSCV(Units scvs, Point2D point = Point2D(0, 0)) {
		if (scvs.empty()) { return NoUnit; }
		Units possible_scvs;

		// first get all harvesting/idle (or close, if ia point is given) scv's
		// check that they are also in resource_units
		for (auto& scv : scvs) {
			if (std::find(std::begin(*resource_units), std::end(*resource_units), scv->tag) == std::end(*resource_units)) {
				continue;
			}
			if (scv->orders.empty()) {
				if (point != Point2D(0, 0)) {
					if (DistanceSquared2D(scv->pos, point) < 225) { // 15**2
						possible_scvs.push_back(scv);
					}
				}
				else {
					possible_scvs.push_back(scv);
				}
			}
			else {
				bool safe = true;
				for (auto& order : scv->orders) {
					if (order.ability_id != ABILITY_ID::SMART
						&& order.ability_id != ABILITY_ID::HARVEST_GATHER
						&& order.ability_id != ABILITY_ID::HARVEST_RETURN)
					{
						safe = false;
					}
				}
				if (safe) {
					if (point != Point2D(0, 0)) {
						if (DistanceSquared2D(scv->pos, point) < 225) { // 15**2
							possible_scvs.push_back(scv);
						}
					}
					else {
						possible_scvs.push_back(scv);
					}
				}
			}
		}

		// then search from all harvesting scvs if none are available (if no point is specified
		// this has already been done)
		if (possible_scvs.empty() && point != Point2D(0, 0)) {
			return getSCV(scvs);
		}

		// if there are no scvs that can immediately be reassigned don't return one so orders aren't cancelled
		// because the work queue is too big
		if (possible_scvs.empty()) { return NoUnit; }

		// otherwise return a random scv
		std::uniform_int_distribution<> distrib(0, possible_scvs.size() - 1);
		int index = distrib(rand_gen);
		return TF_unit(scvs.data()[index]->unit_type, scvs.data()[index]->tag);
	}

	/**
	 * Handles idle units
	 */
	void unitIdle(const Unit* u) {
		// make scv's if not enough; (do orbital_scan if requested)
		switch (u->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_SCV: {
			assignSCV(u);
			break;
		}
		case UNIT_TYPEID::TERRAN_MULE: {
			assignMULE(u);
			break;
		}
		case UNIT_TYPEID::TERRAN_PLANETARYFORTRESS: {
			if (scv_count <= scv_target_count) {
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 5, ABILITY_ID::TRAIN_SCV, UNIT_TYPEID::TERRAN_SCV, UNIT_TYPEID::TERRAN_PLANETARYFORTRESS, u->tag));
			}
			break;
		}
		case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
			if (scv_count <= scv_target_count) {
				auto priority = 6;
				if (scv_count < (active_bases.size() * 14)) { priority = 7; } // prioritize scvs when we are missing many workers
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, priority, ABILITY_ID::TRAIN_SCV, UNIT_TYPEID::TERRAN_SCV, UNIT_TYPEID::TERRAN_COMMANDCENTER, u->tag));
			}
			break;
		}
		case UNIT_TYPEID::TERRAN_ORBITALCOMMAND: {
			if (scv_count <= scv_target_count) {
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 5, ABILITY_ID::TRAIN_SCV, UNIT_TYPEID::TERRAN_SCV, UNIT_TYPEID::TERRAN_ORBITALCOMMAND, u->tag));
			}
			else if (u->energy >= 50){ // MULE energy cost
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 5, ABILITY_ID::EFFECT_CALLDOWNMULE, UNIT_TYPEID::TERRAN_MULE, UNIT_TYPEID::TERRAN_ORBITALCOMMAND, u->tag));
			}
			break;
		}
		}
	}

	/**
	 * Depending on how many command centers we have, we want to have so much extra supply
	 * available; try to keep 3/5/8 supply ahead (1/2/3+ command centers)
	 */
	int getSupplyFloat() {
		int command_centers = isolated_bases.size() + active_bases.size();
		if (command_centers <= 1) { return 3; }
		else if (command_centers == 2) { return 5; }
		else { return 8; }
	}

private:
	threadsafe_priority_queue<Task>* task_queue;
	const ObservationInterface* observation;
	std::vector<TF_unit> isolated_bases; // pretty much empty bases except for (planetary fortress)
	std::vector<Base> active_bases; // should have 3 bases -> potentially 4-6 when transferring to new location
	
	int scv_count; // total active scv count
	int scv_target_count; // aim for 66?

	std::vector<Tag>* resource_units;

	std::mt19937 rand_gen; //Standard mersenne_twister_engine seeded with rd()

	/* Tries to assign scvs so we maintain a decent amount of both minerals & vespene
	* Assumes that when switching the scvs are already harvesting at another target
	* so if we fail to find a new target (different resource) then we let them keep
	* working the current one
	* @param retarget: If switching from another resource this should be set to false
	*		so we don't waste time switching targets
	*/
	void assignSCV(const Unit* u, bool retarget = true) {
		if ((float) observation->GetVespene() / (float) (observation->GetMinerals() + 1) < 0.4) {
			if (retarget) {
				if (!assign_vespene(u)) { assign_minerals(u); }
			}
			else { assign_vespene(u); }
		}
		else {
			if (retarget) {
				if (!assign_minerals(u)) { assign_vespene(u); }
			}
			else { assign_minerals(u); }
		}
	}

	/* Assign an scv to minerals, preferring the ones they are close to */
	bool assign_minerals(const Unit* u, bool close = true) {
		// first try to assign it to the closest base, if that fails go through them all
		for (auto& p : active_bases) { // saturate bases with scv's
			if (p.command.tag == -1) { return false; }
			const Unit* base = observation->GetUnit(p.command.tag);
			if (base == nullptr) {
				return false;
			}
			if (close && !IsClose(p.location, 25)(*u)) {
				continue;
			}
			else {
				if (base->assigned_harvesters < base->ideal_harvesters) {
					if (p.minerals.empty()) { return false; }
					task_queue->push(Task(HARVEST, 11, u->tag, ABILITY_ID::HARVEST_GATHER, p.minerals.back().tag));
					return true;
				}
			}
		}
		if (close) { assign_minerals(u, false); }
		return false;
	}

	/* Assign an scv to vespene, preferring the ones they are close to */
	bool assign_vespene(const Unit* u, bool close = true) {
		Units vespene = observation->GetUnits(Unit::Alliance::Self, IsVespeneRefinery());
		if (vespene.empty()) { return false; }
		for (auto& v : vespene) {
			if (close && !IsClose(v->pos, 100)(*v)) {
				continue;
			}
			else {
				if (v->assigned_harvesters < v->ideal_harvesters) {
					task_queue->push(Task(HARVEST, 11, u->tag, ABILITY_ID::HARVEST_GATHER, v->tag));
					return true;
				}
			}
		}
		if (close) { assign_vespene(u, false); }
		return false;
	}

	bool assignMULE(const Unit* u) {
		// similar to assign SCV -> get the nearest base with minerals
		// and make it harvest them
		Base closest_base;
		int distance2 = 1000000; // 1000**2, larger than maps
		for (auto& base : active_bases) {
			if (DistanceSquared2D(u->pos, base.location) < distance2
				&& base.minerals.size() != 0) {
				closest_base = base;
				distance2 = DistanceSquared2D(u->pos, base.location);
			}
		}
		// make sure it doesn't fail if there are no bases with minerals
		if (closest_base.minerals.empty()) { return false; }
		task_queue->push(Task(HARVEST, 11, u->tag, ABILITY_ID::HARVEST_GATHER, closest_base.minerals.front().tag));
		return true;
	}

	/**
	 * Adds Refineries for the command center to the task queue
	 * Must be called when the geysers are guaranteed to be in vision
	 * don't add to queue when refineries already exist
	 */
	void buildRefineries(const Unit* command) {
		if (observation->GetGameLoop() / 16 < 60) { return; }
		if (command->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER) {
			Units vespene = observation->GetUnits(IsVespeneGeyser());
			for (auto& p : vespene) {
				if (DistanceSquared2D(command->pos, p->pos) < 225
					&& p->vespene_contents > 0) {
					task_queue->push(Task(BUILD, RESOURCE_AGENT, 6, UNIT_TYPEID::TERRAN_REFINERY,
						ABILITY_ID::BUILD_REFINERY, p->tag));
				}
			}
		}
	}


};
#endif