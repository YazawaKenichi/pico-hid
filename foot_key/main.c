#include "main.h"

bool timing = false;

void main_init()
{
    stdio_init_all();
    adc_init();
    board_init();
    tusb_init();

    //! LED 初期設定
    for(unsigned short int i = 0; i <= 4; i++)
    {
        gpio_init(leds[i]);
        gpio_set_dir(leds[i], GPIO_OUT);
        gpio_put(leds[i], 1);
    }

    //! Switch 初期設定
    for(unsigned short int i = 0; i < 6; i++)
    {
        gpio_init(switches[i]);
        gpio_set_dir(switches[i], GPIO_IN);
    }

    timing = false;
}

int main(void)
{
    main_init();

    while (1)
    {
        tud_task();
        led_blinking_task();

        task();
        sleep_ms(1);
    }

    return 0;
}

void task(void)
{
    on_timing();
    if(!tud_suspended())
    {
        if(tud_hid_ready())
        {
            bool btn = (!gpio_get(SWL) || !gpio_get(SWR));
            bool click = timing && btn;
            gpio_put(LED0, !click);
            hid_task(click);
        }
    }
    else
    {
        gpio_put(LED0, 1);
        tud_remote_wakeup();
    }
}

int on_timing()
{
    float interval_value = 0;
    adc_select_input(VL1);
    interval_value += adc_read();
    // adc_select_input(VL2);
    // interval_value += adc_read();
    // adc_select_input(VL3);
    // interval_value += adc_read();
    // const uint32_t interval_ms = rescale(interval_value, 3 * (4096 - 1), 0, INTERVAL_MS * 50, INTERVAL_MS);
    const uint32_t interval_ms = rescale(interval_value, 4096 - 1, 0, 200, 20);
    static uint32_t start = 0;
    uint32_t now = board_millis();

    if(now - start > interval_ms)
    {
        timing = true;
        start = now;
    }
    else
    {
        timing = false;
    }
    return timing;
}

void hid_task(bool click)
{
    uint8_t button, l_button, r_button;

    l_button = click ? MOUSE_BUTTON_LEFT : 0;
    r_button = 0;
    button = l_button | r_button;

    tud_hid_mouse_report(REPORT_ID_MOUSE, button, 0, 0, 0, 0);
}

void send_hid_report(uint8_t report_id)
{
    return ;
}

float rescale(int16_t in, int16_t in_max, int16_t in_min, int16_t out_max, int16_t out_min)
{
    float out;
    out = out_min + (out_max - out_min) * (in - in_min) / (float) (in_max - in_min);
    return out;
}
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
    (void) instance;
    (void) len;
    uint8_t next_report_id = report[0] + 1;
    if (next_report_id < REPORT_ID_COUNT)
    {
        send_hid_report(next_report_id);
    }
}
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    return 0;
}
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    (void) instance;
    if (report_type == HID_REPORT_TYPE_OUTPUT)
    {
        if (report_id == REPORT_ID_KEYBOARD)
        {
            if ( bufsize < 1 ) return;
            uint8_t const kbd_leds = buffer[0];
            if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
            {
                blink_interval_ms = 0;
                board_led_write(true);
            }
            else
            {
                board_led_write(false);
                blink_interval_ms = BLINK_MOUNTED;
            }
        }
    }
}
void led_blinking_task(void)
{
    static uint32_t start_ms = 0;
    static bool led_state = false;
    if (!blink_interval_ms) return;
    if ( board_millis() - start_ms < blink_interval_ms) return;
    start_ms += blink_interval_ms;
    board_led_write(led_state);
    led_state = 1 - led_state;
}
void tud_mount_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}
void tud_umount_cb(void)
{
    blink_interval_ms = BLINK_NOT_MOUNTED;
}
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}
void tud_resume_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

