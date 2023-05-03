/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * このソフトウェアおよび関連するドキュメント ファイル (「ソフトウェア」) のコピーを取得するすべての人に、
 * 使用、コピー、変更、マージする権利を含むがこれらに限定されない、制限なしにソフトウェアを扱う許可が無償で付与されます。 
 * ソフトウェアのコピーを発行、配布、サブライセンス、および/または販売すること、およびソフトウェアが提供された人にそれを許可すること。
 * ただし、以下の条件に従います。
 *
 * 上記の著作権通知およびこの許可通知は、ソフトウェアのすべてのコピーまたは実質的な部分に含まれるものとします。
 *
 * ソフトウェアは「現状有姿」で提供され、商品性、特定の目的への適合性、および非侵害の保証を含むがこれらに限定されない、
 * 明示または黙示を問わず、いかなる種類の保証もありません。
 * 作者または著作権所有者は、契約、不法行為、またはその他の行為によるものであるかにかかわらず、
 * 本ソフトウェアまたは本ソフトウェアの使用またはその他の取引に起因または関連して、
 * いかなる請求、損害、またはその他の責任に対しても責任を負わないものとします。 ソフトウェア。
 *
 * Translated by www.translate.google.com
 *
 */

/**
 * @file main.c
 * @author YAZAWA Kenichi (s21c1036hn@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2023-01-31
 * 
 * (C) 2023 YAZAWA Kenichi
 * 
 * 1. ボード上のボタンを押下する
 * 2. hid_task() 関数の中でそれを検出し、btn に格納 ( 多分真偽値 )
 * 3. send_hid_report(REPORT_ID_KEYBOARD, btn) 関数を呼び出す
 * 4. keycode に HID_KEY_A すなわち A キーを格納する
 * 5. tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode) で A キーを送信する
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//! ----/pico-sdk/lib/tinyusb/hw/bsp/board.h
#include "bsp/board.h"
//! ----/pico-sdk/lib/tinyusb/src/tusb.h
#include "tusb.h"

//! ----/pico-examples/usb/device/dev_hid_composite/usb_descriptors.h
#include "usb_descriptors.h"

#include <unistd.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

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

#define RESC(X) rescale(X, 4096, 0, 50, 0)

/* 点滅パターン
 * - 250 ms  : デバイスがマウントされていない
 * - 1000 ms : デバイス搭載
 * - 2500 ms : デバイスは停止されている
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

float rescale(int16_t, int16_t, int16_t, int16_t, int16_t);
void led_blinking_task(void);
void hid_task(void);

uint8_t flg;

//! オルタネートスイッチで使う
uint8_t alternated;
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

    flg = 0;
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
        led_blinking_task();

        flg = !gpio_get(SW1);

        hid_task();
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
  blink_interval_ms = BLINK_MOUNTED;
}

// デバイスがマウント解除されたときに呼び出される
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// USB バスがサスペンドされたときに呼び出される
// remote_wakeup_en : ホストがリモートウェイクアップの実行を許可する場合
// 7 ms 以内に、デバイスはバスから平均 2.5 mA 未満の電流を引き出す必要がある
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// USB バスが再開されたときに呼び出される
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
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

            uint8_t left_sw, right_sw;

            left_sw = flg ? HID_KEY_1 : HID_KEY_3;
            right_sw = flg ? HID_KEY_2 : HID_KEY_4;

            if(0)
            {
                keycode[0] = !gpio_get(SW4) ? left_sw : 0x00;
                keycode[1] = !gpio_get(SW5) ? right_sw : 0x00;
            }

            if(alternated && !gpio_get(SW4))
            {
                keycode[0] = !keybef ? HID_KEY_W : 0x00;
                keybef = keycode[0];
            }

            if(alternated && !gpio_get(SW5))
            {
                keycode[0] = !keybef ? HID_KEY_SPACE : 0x00;
                keybef = keycode[0];
            }

            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        }
        break;

        case REPORT_ID_MOUSE:
        {
            float xaxis, yaxis;
            uint8_t button;

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

            if(!alternated)
            {
                button = !gpio_get(SW3) ? 0x01 : 0x00;
            }
            else
            {
                button = button_buff ? 0x00 : 0x01;
                button_buff = button;
                button = button ? (!gpio_get(SW3) ? 0x01 : 0x00) : 0x00;
            }

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

// 10 ms ごとに、HID プロファイル（キーボード、マウスなど）ごとに１つのレポートが送信される
// tud_hid_report_complete_cb() は、前のレポートが完了した後に次のレポートを送信するために使用される
void hid_task(void)
{
  // 10 ms ごとにポーリングする
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // 時間が足りない
  start_ms += interval_ms;

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
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len)
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

float rescale(int16_t in, int16_t in_max, int16_t in_min, int16_t out_max, int16_t out_min)
{
    float out;
    out = out_min + (out_max - out_min) * (in - in_min) / (float) (in_max - in_min);
    return out;
}

