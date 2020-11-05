#ifndef BASEMANAGER_HPP
#define BASEMANAGER_HPP

//
// Created by Carter Sabadash October 29
//

/**
 * The purpose of this class is to manage command centers, scv's and minerals/vespene
 * Currently it's purpose is to try and have 3 Bases which are always mining resources
 * Issues to fix: When we can't build anymore command centers/move them
 */

using namespace sc2;

struct Base {

};

class BaseManager {
public:
	BaseManager(threadsafe_priority_queue<Task>* task_queue, const ObservationInterface* obs)
		: task_queue(task_queue), observation(obs)
	{}


private:
	threadsafe_priority_queue<Task> *task_queue;
	std::vector<TF_unit> isolated_bases; // pretty much empty bases except for (planetary fortress)
	std::vector<Base> active_bases; // should have 3 bases -> potentially 4-6 when transferring to new location
	const ObservationInterface* observation;
};

#endif