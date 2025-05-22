#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include <unistd.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define LED1 0  //! 赤色 LED
#define LED2 1  //! 緑色 LED

//! ボード上スイッチ
#define SW1 3   //! 上ボタン
#define SW2 6   //! 下ボタン
//! 左上 3 ピン GPIO 端子
#define SW3 2
//! 左側の 4 ピン GPIO 端子
#define SW4 7
#define SW5 8
//! 右側のオルタネートスイッチ
#define SW6 21

//! モニターサイズ
#define HEIGHT 3200
#define WIDTH 1400

//! レポート速度 ( s )
#define DT 0.001

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

float rescale(float, float, float, float, float);
void led_blinking_task(void);
void hid_task(uint8_t);
void move_to(float, float, float);
void drag_and_drop(float, float, float);

uint8_t alternated;
uint8_t button_buff = 0;

uint8_t state = 255;
uint16_t report_count = 1;
float lost_x, lost_y;

void main_init()
{
    stdio_init_all();
    adc_init();
    board_init();
    tusb_init();

    gpio_init(LED1);
    gpio_init(LED2);
    gpio_init(SW1);
    gpio_init(SW2);
    gpio_init(SW3);
    gpio_init(SW4);
    gpio_init(SW5);
    gpio_init(SW6);

    gpio_set_dir(LED1, GPIO_OUT);
    gpio_set_dir(LED2, GPIO_OUT);

    gpio_set_dir(SW1, GPIO_IN);
    gpio_set_dir(SW2, GPIO_IN);
    gpio_set_dir(SW3, GPIO_IN);
    gpio_set_dir(SW4, GPIO_IN);
    gpio_set_dir(SW5, GPIO_IN);
    gpio_set_dir(SW6, GPIO_IN);

    gpio_put(LED1, 1);
    gpio_put(LED2, 1);
}

int main(void)
{
    main_init();

    while (1)
    {
        alternated = !gpio_get(SW6);
        gpio_put(LED1, !alternated);

        tud_task(); // tinyusb デバイスタスク
        led_blinking_task();

        if(tud_suspended())
        {
            tud_remote_wakeup();
        }
        else
        {
            if(tud_hid_ready())
            {
                int32_t now = board_button_read();
                if(button_buff - now < 0)
                {
                    gpio_put(LED2, 0);
                    report_count = 1;
                    state = 0;
                }
                hid_task(state);
                button_buff = now;
            }
        }
        sleep_ms(DT * 0.001);
    }
    return 0;
}

void hid_task(uint8_t state)
{
    float dx, dy, dt;
    switch(state)
    {
        case 0:
            dx = - WIDTH;
            dy = - HEIGHT;
            dt = 0.1;
            move_to(dx, dy, dt);
            break;
        case 1:
            dx = 0;
            dy = 450;
            dt = 0.1;
            move_to(dx, dy, dt);
            break;
        case 2:
            dx = 470;
            dy = 0;
            dt = 0.1;
            move_to(dx, dy, dt);
            break;
        case 3:
            dx = 0;
            dy = 300;
            dt = 0.02;
            drag_and_drop(dx, dy, dt);
            break;
        default:
            gpio_put(LED2, 1);
            report_count = 1;
            break;
    }
}

void move_to(float dx, float dy, float dt)
{
    float v_x = dx / (float) dt * DT + lost_x;    // 今回の移動量
    float v_y = dy / (float) dt * DT + lost_y;    // 今回の移動量
    lost_x = v_x - (int32_t) v_x;
    lost_y = v_y - (int32_t) v_y;
    if(dt / (float) DT <= report_count)
    {
        lost_x = 0;
        lost_y = 0;
        report_count = 0;
        state ++;
        if(state == 3)
        {
            sleep_ms(500);
        }
    }
    else
    {
        report_count++;
    }
    tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, v_x, - v_y, 0, 0);
}

void drag_and_drop(float dx, float dy, float dt)
{
    float v_x = dx / (float) dt * DT + lost_x;    // 今回の移動量
    float v_y = dy / (float) dt * DT + lost_y;    // 今回の移動量
    lost_x = v_x - (int32_t) v_x;
    lost_y = v_y - (int32_t) v_y;
    uint8_t button = 0x00;
    if(dt / (float) DT <= report_count)
    {
        lost_x = 0;
        lost_y = 0;
        report_count = 0;
        state ++;
    }
    else
    {
        button = MOUSE_BUTTON_LEFT;
        report_count++;
    }
    tud_hid_mouse_report(REPORT_ID_MOUSE, button, v_x, - v_y, 0, 0);
}

float rescale(float in, float in_max, float in_min, float out_max, float out_min)
{
    float out;
    out = out_min + (out_max - out_min) * (float) (in - in_min) / (float) (in_max - in_min);
    return out;
}

// REPORT がホストに正常に送信されたときに呼び出される
// アプリケーションはこれを使用して次のレポートを送信できる
// 注：複合レポートの場合、report[0] はレポート ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
    (void) instance;
    (void) len;

    uint8_t next_report_id = report[0] + 1;

    if (next_report_id < REPORT_ID_COUNT)
    {
    }
}

// GET_REPORT 制御リクエストを受信したときに呼び出される
// アプリケーションは、バッファレポートの内容を入力し、その長さを返す必要がある
// ゼロを返すと、スタックが STALL 要求になる
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO 未実装
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// SET_REPORT 制御要求を受信したときに呼び出される
// OUT エンドポイントでデータを受信した（レポート ID = 0, タイプ = 0 ）
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Capslock, Numlock などのキーボード LED を設定する
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize は（少なくとも）１でなければならない
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock オン：点滅を無効にし、LED をオンにする
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Capslock オフ：通常の点滅に戻る
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // 点滅は無効
  if (!blink_interval_ms) return;

  // ms 間隔ごとに点滅
  if ( board_millis() - start_ms < blink_interval_ms) return; // 時間が足りない
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // トグル
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

