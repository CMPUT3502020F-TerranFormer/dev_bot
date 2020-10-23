//
// Created by Kerry Cao on 2020-10-23.
//

#ifndef CPP_SC2_RESOURCE_BOT_HPP
#define CPP_SC2_RESOURCE_BOT_HPP

#include "TF_Agent.hpp"
#include "Task.hpp"

class RESOURCE_BOT final : public TF_Agent {
public:
    /**
     * Virtual Destructor
     */
    ~RESOURCE_BOT() final {

    }

    /**
     * Do actions base on game info provided
     * @param gi sc2::GameInfo
     */
    void step(const sc2::GameInfo &gi) final {

    }

    /**
     * Cross agent communication
     * Add a task to the task_queue
     * @param t Task
     */
    void addTask(Task t) final {

    }

    /**
     * Cross agent communication
     * Assign a TF_unit to the agent
     * @param u TF_unit
     */
    void addUnit(TF_unit u) final {

    }

    void setAgents(const TF_Agent *defenceb, const TF_Agent *attackb, const TF_Agent *scoutb) {
        this->defence = defenceb;
        this->attack = attackb;
        this->scout = scoutb;
    }

private:
    const TF_Agent *defence;
    const TF_Agent *attack;
    const TF_Agent *scout;
};

#endif //CPP_SC2_RESOURCE_BOT_HPP
