/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "sleep_api.h"
#include "power.h"
#include "tc.h"
#include "tc_interrupt.h"
#if DEVICE_SLEEP

#include "cmsis.h"

void mbed_enter_sleep(sleep_t *obj)
{
#if (SAMD21) || (SAMR21)
    system_set_sleepmode(SYSTEM_SLEEPMODE_IDLE_2);
#elif (SAML21)
    system_set_sleepmode(SYSTEM_SLEEPMODE_IDLE);
#endif
    system_sleep();
}

void mbed_exit_sleep(sleep_t *obj)
{
    (void)obj;
}

#endif
