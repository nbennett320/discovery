/* discovery
 * License: GPLv2
 * See LICENSE.txt for full license text
 * 
 * FILE: timer.h
 * DATE: November 4th, 2020
 * DESCRIPTION: timer definition
 */

#pragma once

#include "common.h"

struct Timer
{
    public:
        Timer()
        {
            // zero time
            data = 0;
            start_data;
            freq = 0;
            cascade = 0;
            irq = 0;
            enable = 0;
            actual_freq = 0;
        }

        ~Timer() { }

        u16 data;
        u16 start_data;
        u8 freq;
        u8 cascade;
        u8 irq;
        u8 enable;
        u16 actual_freq;

};