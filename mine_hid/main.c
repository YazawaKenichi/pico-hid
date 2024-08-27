#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include <unistd.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define ONFOOT 1    //! 赤ランプがついてるとき長押し
#define LOWFREQ 1   //! 赤ランプがついてるとき周期押し

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

#define HIGH_FREQ_INTERVAL_MIN 10
#define HIGH_FREQ_INTERVAL_MAX 100
#define LOW_FREQ_INTERVAL_MIN 300
#define LOW_FREQ_INTERVAL_MAX 5000

float rescale(int16_t, int16_t, int16_t, int16_t, int16_t);
void hid_task(void);

//! オルタネートスイッチで使う
uint8_t alternated;
uint16_t report_count = 0;
uint8_t button_buff = 0x00;
uint8_t keybef = 0x00;

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

/*------------- MAIN -------------*/
int main(void)
{
    main_init();

    while (1)
    {
        alternated = !gpio_get(SW6);
        gpio_put(LED1, !alternated);

        tud_task(); // tinyusb デバイスタスク

        hid_task();
        gpio_put(LED2, 1);
        sleep_ms(1);
    }

    return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// デバイスがマウントされたときに呼び出される
void tud_mount_cb(void)
{
}

// デバイスがマウント解除されたときに呼び出される
void tud_umount_cb(void)
{
}

// USB バスがサスペンドされたときに呼び出される
// remote_wakeup_en : ホストがリモートウェイクアップの実行を許可する場合
// 7 ms 以内に、デバイスはバスから平均 2.5 mA 未満の電流を引き出す必要がある
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
}

// USB バスが再開されたときに呼び出される
void tud_resume_cb(void)
{
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

/**
 * @brief 
 * 
 * @param report_id 
 * REPORT_ID_KEYBOARD : キーボード
 * REPORT_ID_MOUSE : マウス
 * REPORT_ID_COSUMER_CONTROL : ナニコレ（ボリュームが使えるなにか）
 * REPORT_ID_GAMEPAD : ゲームパッド
 * @param btn 
 */
static void send_hid_report(uint8_t report_id, uint32_t btn)
{
    // HID の準備ができていない場合はスキップ
    if(!tud_hid_ready())
        return;

    switch(report_id)
    {
        case REPORT_ID_KEYBOARD:
        {
            uint8_t keycode[6] = { 0 };
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        }
        break;

        case REPORT_ID_MOUSE:
        {
            float xaxis, yaxis;
            uint8_t button, l_button, r_button;
            float dpi;
            bool reverse;

            dpi = 1 / (float) 50;

            adc_select_input(1);
            xaxis = adc_read();
            xaxis = rescale(xaxis, 4096 - 1, 0, 500, -500);
            if(!reverse)
              xaxis *= -1;
            xaxis *= (float) dpi;

            adc_select_input(0);
            yaxis = adc_read();
            yaxis = rescale(yaxis, 4096 - 1, 0, 500 + 100, -500);
            if(!reverse)
              yaxis *= -1;
            yaxis *= (float) dpi;

            r_button = !gpio_get(SW4) ? MOUSE_BUTTON_RIGHT : 0;

            if(!gpio_get(SW3))
            {
                adc_select_input(2);
                float interval_value = adc_read();
                uint16_t interval_max = alternated ? HIGH_FREQ_INTERVAL_MAX : LOW_FREQ_INTERVAL_MAX;
                uint16_t interval_min = alternated ? HIGH_FREQ_INTERVAL_MIN : LOW_FREQ_INTERVAL_MIN;
                const uint32_t attack = rescale(interval_value, 4096 - 1, 0, interval_max, interval_min);
                if(report_count <= 0)
                {
                    l_button = MOUSE_BUTTON_LEFT;
                    report_count = attack - 1;
                }
                else
                {
                    l_button = 0x00;
                    report_count--;
                }
            }
            else
            {
                l_button = 0x00;
                report_count = 0;
            }

            button = l_button | r_button;
            gpio_put(LED2, !l_button);

            //! tud_hid_mouse_report(REPORT_ID_MOUSE, ボタン, 右移動量, 下移動量, スクロール量, パン量(パンって何))
            xaxis = 0;
            yaxis = 0;
            tud_hid_mouse_report(REPORT_ID_MOUSE, button, xaxis, yaxis, 0, 0);
            //printf("xaxis = %4d, yaxis = %4d\r", xaxis, yaxis);
        }
        break;
        default:
        break;
    }
}

// 1 ms ごとに、HID プロファイル（キーボード、マウスなど）ごとに１つのレポートが送信される
// tud_hid_report_complete_cb() は、前のレポートが完了した後に次のレポートを送信するために使用される
void hid_task(void)
{
  static uint32_t start_ms = 0;

  /*
  if ( board_millis() - start_ms < 1)
  {
      return;
  }
  start_ms += 1;
  */

  uint32_t const btn = board_button_read();

  // リモートウェイクアップ
  if(tud_suspended())
  {
    // サスペンドモードの場合はホストをウェイクアップする
    // ホストによって REMOTE_WAKEUP 機能が有効になっている
    tud_remote_wakeup();
  }
  else
  {
    // レポートチェーンの最初のものを送信し、残りは tud_hid_report_complete_cb() によって送信される
    send_hid_report(REPORT_ID_KEYBOARD, btn);
  }
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
    send_hid_report(next_report_id, board_button_read());
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
        board_led_write(true);
      }else
      {
        // Capslock オフ：通常の点滅に戻る
        board_led_write(false);
      }
    }
  }
}

float rescale(int16_t in, int16_t in_max, int16_t in_min, int16_t out_max, int16_t out_min)
{
    float out;
    out = out_min + (out_max - out_min) * (in - in_min) / (float) (in_max - in_min);
    return out;
}

