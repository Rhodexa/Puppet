#ifndef MOTION_H
#define MOTION_H

#include <Arduino.h>

/*
    The concept of a line path
    State Machine: Max Speed, Acceleration

    Line: XYZWS

       mid point
           |
           v

           C    <-- acceleration reverses here
          . .
         .   .
        .     .
       .       .
      ._________.    <--- speed cap
     /           \
    /             \
   A               B
*/

const int AXIS_COUNT = 4;

typedef struct
{
    int speed;
    int accumulator;
    const int pin_dir;
    const int pin_step;
} Axis;

typedef struct
{
    int elapsed_ticks;
    int target_ticks;
    int speed;
    int acceleration;
    int speed_cap;
} Clock;

Axis axis[AXIS_COUNT];
clock clock = {0, 0, 0, 0, 0};

/* Axes functions */
void axis_init(Axis &axis, int step, int direction)
{
    axis.speed = 0;
    axis.accumulator = 0;
    axis.pin_step = step;
    axis.pin_dir = direction;
}

void axis_writePinDirection(Axis &axis, int direction)
{
    digitalWrite(axis.pin_dir, direction);
}

void axis_writePinStep(Axis &axis, int direction)
{
    digitalWrite(axis.pin_step, direction);
}

/* Actual motion control magic */
void motion_setup(int *step_pins, int *dir_pins)
{
    for (int i = 0; i < AXIS_COUNT; i++)
    {
        axis_init(axis[i], step_pins[i], dir_pins[i]);
    }
}

void motion_initMove()
{    
    clock.target_ticks = 0;
    for (int i = 0; i < AXIS_COUNT; i++)
    {
        axis_writeDirectionPin(axis[i], axis[i].speed < 0 ? 0 : 1);
        if (axis[i].speed > clock.target_ticks) clock.target_ticks = axis[i].speed;
        axis[i].accumulator = 0;
    }
    clock.elapsed_ticks = 0;
}

void motion_tick()
{
    if (clock.elapsed_ticks < clock.target_ticks)
    {
        clock.elapsed_ticks ++;
        if((clock.elapsed_ticks << 1) < clock.target_ticks) clock.speed -= clock.acceleration;
        else clock.speed += clock.acceleration;
        clock_setTimerspeed(min(clock.speed, clock.speed_cap));

        /* line logic */
        for (int i = 0; i < AXIS_COUNT; i++)
        {
            axis[i].accumulator += axis[i].speed;
            if (axis[i].accumulator > clock.target_ticks)
            {
                axis[i].accumulator -= clock.target_ticks;
                axis_writeStepPin(axis[i], 0);
            }
        }

        // delay() perhaps?
        for (int i = 0; i < AXIS_COUNT; i++)
        {
            axis_writeStepPin(axis[i], 1);
        }
    }
}

#endif