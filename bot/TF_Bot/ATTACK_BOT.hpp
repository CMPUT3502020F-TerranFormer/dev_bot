//
// Created by Kerry Cao on 2020-10-23.
//

#ifndef CPP_SC2_ATTACK_BOT_HPP
#define CPP_SC2_ATTACK_BOT_HPP

#include "TF_Agent.hpp"
#include "Task.hpp"

class ATTACK_BOT final : public TF_Agent {
public:
    /**
     * Virtual Destructor
     */
    ~ATTACK_BOT() final {

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
    void addTask(Task t) override {

    }

    /**
     * Cross agent communication
     * Assign a TF_unit to the agent
     * @param u TF_unit
     */
    void addUnit(TF_unit u) override {

    }
};

#endif //CPP_SC2_ATTACK_BOT_HPP
