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

#include "cmsis.h"
#include "device.h"

#if DEVICE_LOWPOWERTIMER

#include "lp_ticker_api.h"
#include "sleep_api.h"
#include "objects.h"
#include "mbed_assert.h"
#include "ins_gclk.h"
#include "compiler.h"
#include "system.h"
#include "tc.h"
#include "tc_interrupt.h"
#include "us_ticker_api.h"

extern struct tc_module us_ticker_module;

#if (SAMD21) || (SAMR21)
#define TICKER_COUNTER_uS       TC4
#define TICKER_COUNTER_IRQn     TC4_IRQn
#define TICKER_COUNTER_Handlr   TC4_Handler
#elif (SAML21) /*SAML21 TCC4 does not support 32 bit counter operations*/
#define TICKER_COUNTER_uS       TC0
#define TICKER_COUNTER_IRQn     TC0_IRQn
#define TICKER_COUNTER_Handlr   TC0_Handler
#endif

int lp_ticker_inited = 0;
extern uint8_t g_sys_init;
extern int us_ticker_inited;
static volatile uint32_t lpoverflow = 0;

void lp_ticker_irq_handler_internal(struct tc_module* us_tc_module)
{
    uint32_t status_flags, cur_flag;
    cur_flag = tc_get_status(&us_ticker_module);
    if (cur_flag & TC_STATUS_COUNT_OVERFLOW)
        lpoverflow++;
    /* Clear TC capture overflow and TC count overflow */
    status_flags = TC_STATUS_CAPTURE_OVERFLOW | TC_STATUS_COUNT_OVERFLOW | TC_STATUS_CHANNEL_1_MATCH ; /*Channel one is configured for compare in lp ticker*/
    tc_clear_status(&us_ticker_module, status_flags);
}

void lp_configure_channel ( void)
{
    NVIC_DisableIRQ(TICKER_COUNTER_IRQn);
    NVIC_SetVector(TICKER_COUNTER_IRQn, (uint32_t)TICKER_COUNTER_Handlr);
    tc_disable(&us_ticker_module);

    /* Register callback function */
    tc_register_callback(&us_ticker_module, (tc_callback_t)lp_ticker_irq_handler_internal, (TC_CALLBACK_CC_CHANNEL1|TC_CALLBACK_OVERFLOW));

    /* Enable the timer module */
    tc_enable(&us_ticker_module);
    NVIC_EnableIRQ(TICKER_COUNTER_IRQn);

}

void lp_ticker_init(void)
{
    if (g_sys_init == 0) {
        system_init();
        g_sys_init = 1;
    }

    if(lp_ticker_inited)
        return;

    if (us_ticker_inited) {
        lp_configure_channel();
    } else {
        us_ticker_init();
        lp_configure_channel();
    }
    lp_ticker_inited = 1;
}

uint32_t lp_ticker_read()
{
    if (!lp_ticker_inited)
        lp_ticker_init();

    return tc_get_count_value(&us_ticker_module);
}

uint32_t lp_ticker_get_overflows_counter(void)
{
    return lpoverflow;
}

uint32_t lp_ticker_get_compare_match(void)
{
    if (!lp_ticker_inited)
        lp_ticker_init();

    return tc_get_capture_value(&us_ticker_module, TC_COMPARE_CAPTURE_CHANNEL_1);
}

void lp_ticker_set_interrupt(uint32_t now, uint32_t time)
{
    (void)now;
    NVIC_DisableIRQ(TICKER_COUNTER_IRQn);
    NVIC_SetVector(TICKER_COUNTER_IRQn, (uint32_t)TICKER_COUNTER_Handlr);

    /* Enable the callback */
    tc_enable_callback(&us_ticker_module, TC_CALLBACK_CC_CHANNEL1);
    tc_set_compare_value(&us_ticker_module, TC_COMPARE_CAPTURE_CHANNEL_1, (uint32_t)time);

    NVIC_EnableIRQ(TICKER_COUNTER_IRQn);
}

void lp_ticker_sleep_until(uint32_t now, uint32_t time)
{
    sleep_t sleep_obj;
    lp_ticker_set_interrupt(now, time);
    mbed_enter_sleep(&sleep_obj);
}

#endif
