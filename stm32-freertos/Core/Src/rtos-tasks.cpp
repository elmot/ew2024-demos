#include "main.h"
#include "cmsis_os.h"
#include "queue.h"

extern osMessageQueueId_t myDisplayQHandle;


extern "C" [[noreturn]] void StartLCD([[maybe_unused]]void *argument) {
    /* Clear the LCD device */

    BSP_LCD_GLASS_Init();
    BSP_LCD_GLASS_Clear();
    BSP_LCD_GLASS_DisplayString((uint8_t *) "START");
    static int bar_count = 0;
    static const BarId_Typedef bars[4] ={LCD_BAR_3,LCD_BAR_0, LCD_BAR_1, LCD_BAR_2};

    /* Infinite loop */
    for (;;) {
        char *text = nullptr;
        osMessageQueueGet(myDisplayQHandle, &text, nullptr, 300);
        if (text != nullptr) {
            BSP_LCD_GLASS_Clear();
            BSP_LCD_GLASS_DisplayString((uint8_t *) text);
            BSP_LCD_GLASS_ClearBar(bars[bar_count]);
            bar_count = 0;
        } else {
            BSP_LCD_GLASS_ClearBar(bars[bar_count]);
            bar_count = (bar_count + 1) % 4;
            BSP_LCD_GLASS_DisplayBar(bars[bar_count]);
        }
    }
}
volatile JOYState_TypeDef joistick_state = JOY_NONE;

extern "C" [[noreturn]] void StartJoystick([[maybe_unused]]void *argument) {
    BSP_JOY_Init(JOY_MODE_GPIO);
    auto oldState = (JOYState_TypeDef) -1;
    while (true) {
        joistick_state = BSP_JOY_GetState();
        vTaskDelay(pdMS_TO_TICKS(30));
        if (joistick_state != oldState) {
            joistick_state = BSP_JOY_GetState();
            const char *lcdData;
            switch (joistick_state) {
                case JOY_DOWN:
                    lcdData = "DOWN";
                    break;
                case JOY_UP:
                    lcdData = "UP";
                    break;
                case JOY_LEFT:
                    lcdData = "LEFT";
                    break;
                case JOY_RIGHT:
                    lcdData = "RIGHT";
                    break;
                case JOY_SEL:
                    lcdData = "PRESS";
                    break;
                case JOY_NONE:
                    lcdData = "---";
                    break;
            }
            oldState = joistick_state;
            osMessageQueuePut(myDisplayQHandle, (const void *) &lcdData, 0, 0);
        }
    }
}
extern "C" [[noreturn]] void StartLED([[maybe_unused]]void *argument) {
    BSP_LED_Init(LED_RED);
    BSP_LED_Init(LED_GREEN);
    while (true) {
        switch (joistick_state) {
            case JOY_DOWN:
                BSP_LED_On(LED_GREEN);
                osDelay(pdMS_TO_TICKS(300));
                BSP_LED_Off(LED_GREEN);
                osDelay(pdMS_TO_TICKS(300));
                break;
            case JOY_UP:
                BSP_LED_On(LED_GREEN);
                osDelay(pdMS_TO_TICKS(600));
                BSP_LED_Off(LED_GREEN);
                osDelay(pdMS_TO_TICKS(600));
                break;
            case JOY_LEFT:
                BSP_LED_On(LED_RED);
                osDelay(pdMS_TO_TICKS(300));
                BSP_LED_Off(LED_RED);
                osDelay(pdMS_TO_TICKS(300));
                break;
            case JOY_RIGHT:
                BSP_LED_On(LED_RED);
                osDelay(pdMS_TO_TICKS(600));
                BSP_LED_Off(LED_RED);
                osDelay(pdMS_TO_TICKS(600));
                break;
            case JOY_SEL:
                BSP_LED_On(LED_RED);
                BSP_LED_On(LED_GREEN);
                osDelay(pdMS_TO_TICKS(100));
                BSP_LED_Off(LED_RED);
                BSP_LED_Off(LED_GREEN);
                break;
            default:
                BSP_LED_Off(LED_RED);
                BSP_LED_Off(LED_GREEN);
                osDelay(pdMS_TO_TICKS(100));

        }
    }

}
