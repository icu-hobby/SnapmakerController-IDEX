#pragma once

#include "planner.h"
#include "shaper/AxisInputShaper.h"
#include "shaper/FuncManager.h"
#include "shaper/MoveQueue.h"
#include "../../../../snapmaker/debug/debug.h"

class AxisStepper {
  public:
    int8_t axis = -1;
    int8_t dir = 0;
    time_double_t print_time = 0;
    float delta_time = 0;
};

class Axis {
  public:
    float step;

    bool is_consumed = true;
    time_double_t print_time = 0;
    int8_t dir = 0;

    bool is_get_next_step_null = false;

    bool is_shaped = false;

    AxisInputShaper* axis_input_shaper = nullptr;

    FuncManager func_manager;

  private:
    int8_t axis;

    float half_step;

    int generated_block_index = -1;

    int generated_move_index = -1;

  public:
    Axis() {};

    void init(int8_t axis, float step) {
        this->axis = axis;
        this->step = step;
        this->half_step = step / 2;
        this->func_manager.init(axis);

        if (axis <= 1) {
            if (axis_input_shaper == nullptr) {
                axis_input_shaper = new AxisInputShaper();
            }
            axis_input_shaper->setAxis(axis);
            axis_input_shaper->init();

            is_shaped = true;
        }

        func_manager.print_pos = 0;
    }

    bool generateFuncParams(uint8_t block_index, block_t& block, uint8_t move_start, uint8_t move_end);

    bool getNextStep();

    void abort() {
        is_consumed = true;
        print_time = 0;
        dir = 0;
        is_get_next_step_null = false;

        generated_block_index = -1;
        generated_move_index = -1;

        func_manager.abort();
    }

  private:
    bool generateFuncParams(FuncManager& func_manager, uint8_t move_start, uint8_t move_end);
};

class AxisManager {
  public:
    int counts[20] = {0};

    Axis axis[AXIS_SIZE] = {Axis()};

    float shaped_right_delta = 0;
    float shaped_delta = 0;

    time_double_t print_time = 0;

    time_double_t min_last_time = 0;

  private:
    bool need_add_move_start = true;

    bool is_consumed = true;

    bool is_shaped = false;

    int8_t print_axis;
    int8_t print_dir;

  public:
    AxisManager() {};

    void init() {
        for (int i = 0; i < AXIS_SIZE; ++i) {
            axis[i].init(i, planner.steps_to_mm[i]);
        }

        bool shaped = false;
        for (int i = 0; i < AXIS_SIZE; ++i) {
            if (axis[i].is_shaped) {
                shaped = true;
            }
        }
        is_shaped = shaped;
        shaped_right_delta = 0;
        shaped_delta = 0;
        if (is_shaped) {
            for (int i = 0; i < AXIS_SIZE; ++i) {
                if (axis[i].axis_input_shaper != nullptr && axis[i].axis_input_shaper->delta_window > shaped_delta) {
                    shaped_delta = axis[i].axis_input_shaper->delta_window;
                }
                if (axis[i].axis_input_shaper != nullptr && axis[i].axis_input_shaper->delta_window > shaped_delta) {
                    shaped_delta = axis[i].axis_input_shaper->delta_window;
                }
            }
        }
    };

    bool isShaped() {
        return is_shaped;
    }

    bool generateAllAxisFuncParams(uint8_t block_index, block_t& block);

    float getRemainingConsumeTime() {
        return min_last_time - print_time;
    }

    void abort() {
        for (size_t i = 0; i < AXIS_SIZE; i++)
        {
            axis[i].abort();
        }

        print_time = 0;

        min_last_time = 0;

        need_add_move_start = true;

        is_consumed = true;

        is_shaped = false;

        print_axis = -1;
        print_dir = 0;
    }

    bool getCurrentAxisStepper(AxisStepper* axis_stepper);

    bool getNextAxisStepper();

    bool tryAddMoveStart() {
        if (!isShaped() || !need_add_move_start) {
            return false;
        }
        moveQueue.addMoveStart();
        need_add_move_start = false;
        return true;
    }

    void updateMinLastTime() {
        time_double_t new_min_last_time = axis[0].func_manager.last_time;
        for (int i = 1; i < AXIS_SIZE; ++i) {
            if (axis[i].func_manager.last_time < new_min_last_time) {
                new_min_last_time = axis[i].func_manager.last_time;
            }
        }
        min_last_time = new_min_last_time;
    }

    bool tryAddMoveEnd() {
        if (!isShaped() || need_add_move_start) {
            return false;
        }
        moveQueue.addMoveEnd();
        need_add_move_start = true;
        return true;
    }

    void addEmptyMove() {
        if (!isShaped()) {
            return;
        }
        moveQueue.addEmptyMove(shaped_delta + 0.001f);
    }
};

extern AxisManager axisManager;

