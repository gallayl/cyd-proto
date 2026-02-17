#pragma once

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ILI9341_2 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;
    lgfx::Touch_XPT2046 _touch_instance;

public:
    LGFX(void)
    {
        {
            auto cfg = _bus_instance.config();

            // SPI2_HOST (was HSPI_HOST) for display bus
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0; // Sets the SPI communication mode (0 to 3) SPI通信モードを設定 (0 ~ 3)
            // SPI clock during transmission (Maximum 80MHz, rounded to an integer value of 80MHz)
            // cfg.freq_write = 40000000;  // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
            // cfg.freq_write = 80000000;    // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
            cfg.freq_write = 55000000; // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
            cfg.freq_read = 16000000;  // SPI clock when receiving 受信時のSPIクロック
            // cfg.spi_3wire = false;     // Set to true if receiving is done on the MOSI pin 受信をMOSIピンで行う場合はtrueを設定
            // cfg.use_lock = true;       // Set to true if you want to use transaction locking. トランザクションロックを使用する場合はtrueを設定
            // Set the DMA channel to use (0 = no DMA / 1 = 1ch / 2 = ch / SPI_DMA_CH_AUTO = automatic setting)
            // cfg.dma_channel = SPI_DMA_CH_AUTO; // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
            // * With the ESP-IDF version upgrade, SPI_DMA_CH_AUTO (automatic setting) is now recommended for the DMA channel. Specifying 1ch or 2ch is no longer recommended.
            // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
            cfg.pin_sclk = 14; // set SPI SCLK pin number SCK
            cfg.pin_mosi = 13; // set MOSI pin number of SPI SDI
            cfg.pin_miso = 12; // set SPI's MISO pin number (-1 = disable) SDO
            cfg.pin_dc = 2;    // set SPI D/C pin number (-1 = disable) RS
            // If you use the same SPI bus as the SD card, be sure to set MISO without omitting it.
            // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
            //*/
            /*
            // I2C bus configuration I2Cバスの設定
                  cfg.i2c_port    = 0;          // Select the I2C port to use (0 or 1) 使用するI2Cポートを選択 (0 or 1)
                  cfg.freq_write  = 400000;     // Clock when transmitting 送信時のクロック
                  cfg.freq_read   = 400000;     // Clock when receiving 受信時のクロック
                  cfg.pin_sda     = 21;         // Pin number to which SDA is connected SDAを接続しているピン番号
                  cfg.pin_scl     = 22;         // Pin number to which SCL is connected SCLを接続しているピン番号
                  cfg.i2c_addr    = 0x3C;       // I2C device address I2Cデバイスのアドレス
            //*/
            /*
            // 8-bit parallel bus settings 8ビットパラレルバスの設定
                  // Select the I2S port to use (I2S_NUM_0 or I2S_NUM_1) (Uses ESP32's I2S LCD mode)
                  cfg.i2s_port = I2S_NUM_0;     // 使用するI2Sポートを選択 (I2S_NUM_0 or I2S_NUM_1) (ESP32のI2S LCDモードを使用します)
                  cfg.freq_write = 20000000;    // Transmit clock (maximum 20MHz, rounded to an integer value of 80MHz) 送信クロック (最大20MHz, 80MHzを整数で割った値に丸められます)
                  cfg.pin_wr =  4;              // Pin number to which WR is connected WR を接続しているピン番号
                  cfg.pin_rd =  2;              // Pin number to which RD is connected RD を接続しているピン番号
                  cfg.pin_rs = 15;              // Pin number to which RS is connected RS(D/C)を接続しているピン番号
                  cfg.pin_d0 = 12;              // Pin number to which D0 is connected D0を接続しているピン番号
                  cfg.pin_d1 = 13;              // Pin number to which D1 is connected D1を接続しているピン番号
                  cfg.pin_d2 = 26;              // Pin number to which D2 is connected D2を接続しているピン番号
                  cfg.pin_d3 = 25;              // Pin number to which D3 is connected D3を接続しているピン番号
                  cfg.pin_d4 = 17;              // Pin number to which D4 is connected D4を接続しているピン番号
                  cfg.pin_d5 = 16;              // Pin number to which D5 is connected D5を接続しているピン番号
                  cfg.pin_d6 = 27;              // Pin number to which D6 is connected D6を接続しているピン番号
                  cfg.pin_d7 = 14;              // Pin number to which D7 is connected D7を接続しているピン番号
            //*/

            _bus_instance.config(cfg); // The setting value is reflected in the bus. 設定値をバスに反映します。
            _panel_instance.setBus(&_bus_instance); // Place the bus on the panel. バスをパネルにセットします。
        }

        { // Configure the display panel control settings.
            // 表示パネル制御の設定を行います。
            auto cfg =
                _panel_instance
                    .config(); // Gets the structure for display panel settings. 表示パネル設定用の構造体を取得します。

            cfg.pin_cs = 15;   // pin number to which CS is connected (-1 = disable)
            cfg.pin_rst = -1;  // pin number to which RST is connected (-1 = disable) — pin 4 is shared with RGB LED red
            cfg.pin_busy = -1; // pin number to which BUSY is connected (-1 = disable)

            // * The following settings are set to general default values ​​for each panel, so please try commenting out any items you are unsure about.
            // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

            cfg.panel_width = 240;  // actual displayable width
            cfg.panel_height = 320; // actual displayable height
            cfg.offset_x = 0;       // Panel offset in the X direction パネルのX方向オフセット量
            cfg.offset_y = 0;       // Panel offset in the Y direction パネルのY方向オフセット量
            cfg.offset_rotation =
                6; // Rotation direction offset 0~7 (4~7 are upside down) 回転方向の値のオフセット 0~7 (4~7は上下反転)
            cfg.dummy_read_pixel =
                8; // Number of dummy read bits before pixel readout ピクセル読出し前のダミーリードのビット数
            cfg.dummy_read_bits =
                1; // Number of dummy read bits before reading non-pixel data ピクセル以外のデータ読出し前のダミーリードのビット数
            cfg.readable = true; // Set to true if data can be read データ読出しが可能な場合 trueに設定
            cfg.invert =
                true; // Set to true if the panel's brightness is reversed パネルの明暗が反転してしまう場合 trueに設定
            cfg.rgb_order =
                false; // Set to true if the red and blue of the panel are swapped. パネルの赤と青が入れ替わってしまう場合 trueに設定
            cfg.dlen_16bit =
                false; // Set to true for panels that transmit data in 16-bit units via 16-bit parallel or SPI. 16bitパラレルやSPIでデータ長を16bit単位で送信するパネルの場合 trueに設定
            // If the bus is shared with the SD card, set it to true (bus control is performed using drawJpgFile, etc.)
            cfg.bus_shared = true; // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

            // The following settings should only be used if the display is misaligned when using a driver with a variable pixel count, such as ST7735 or ILI9163.
            // 以下はST7735やILI9163のようにピクセル数が可変のドライバで表示がずれる場合にのみ設定してください。
            // cfg.memory_width = 240;  // Maximum width supported by the driver IC ドライバICがサポートしている最大の幅
            // cfg.memory_height = 320; // Maximum height supported by the driver IC ドライバICがサポートしている最大の高さ

            _panel_instance.config(cfg);
        }

        //*
        { // Set the backlight control (delete if not required).
            // バックライト制御の設定を行います。（必要なければ削除）
            auto cfg = _light_instance
                           .config(); // Gets the backlight settings structure. バックライト設定用の構造体を取得します。

            cfg.pin_bl = 21; // The pin number to which the backlight is connected バックライトが接続されているピン番号
            cfg.invert = false;  // true if the backlight brightness is inverted バックライトの輝度を反転させる場合 true
            cfg.freq = 44100;    // Backlight PWM frequency バックライトのPWM周波数
            cfg.pwm_channel = 7; // PWM channel number to use 使用するPWMのチャンネル番号

            _light_instance.config(cfg);
            _panel_instance.setLight(
                &_light_instance); // Place the backlight on the panel. バックライトをパネルにセットします。
        }
        //*/

        //*
        { // Configure touchscreen control (delete if not needed).
            // タッチスクリーン制御の設定を行います。（必要なければ削除）
            auto cfg = _touch_instance.config();

            cfg.x_min =
                0; // The minimum X value (raw value) obtained from the touchscreen タッチスクリーンから得られる最小のX値(生の値)
            cfg.x_max =
                239; // Maximum X value (raw value) obtained from the touchscreen タッチスクリーンから得られる最大のX値(生の値)
            cfg.y_min =
                0; // The minimum Y value (raw value) obtained from the touchscreen タッチスクリーンから得られる最小のY値(生の値)
            cfg.y_max =
                319; // Maximum Y value (raw value) obtained from the touchscreen タッチスクリーンから得られる最大のY値(生の値)
            cfg.pin_int = -1; // The pin number to which INT is connected INTが接続されているピン番号
            cfg.bus_shared =
                true; // If you are using a bus that is common with the screen, set it to true. 画面と共通のバスを使用している場合 trueを設定
            cfg.offset_rotation =
                0; // Adjust if display and touch direction do not match. Set a value between 0 and 7. 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定 // tft_espi 1=swap xy, 2=invert x, 4=inverty

            // For SPI connection SPI接続の場合
            cfg.spi_host = SPI3_HOST; // SPI3_HOST (was VSPI_HOST) for touch controller
            cfg.freq = 2500000;       // Set SPI clockSPIクロックを設定
            cfg.pin_sclk = 25;        // pin number where SCLK is connected, TP CLK
            cfg.pin_mosi = 32;        // pin number where MOSI is connected, TP DIN
            cfg.pin_miso = 39;        // pin number Vwhere MISO is connected, TP DOUT
            cfg.pin_cs = 33;          // pin number where CS is connected, TP CS

            // I2C接続の場合
            // cfg.i2c_port = 1;      // 使用するI2Cを選択 (0 or 1)
            // cfg.i2c_addr = 0x38;   // I2Cデバイスアドレス番号
            // cfg.pin_sda  = 23;     // SDAが接続されているピン番号
            // cfg.pin_scl  = 32;     // SCLが接続されているピン番号
            // cfg.freq = 400000;     // I2Cクロックを設定

            _touch_instance.config(cfg);
            _panel_instance.setTouch(
                &_touch_instance); // Place the touchscreen on the panel. タッチスクリーンをパネルにセットします。
        }
        //*/

        setPanel(&_panel_instance); // Set the panel to be used. 使用するパネルをセットします。
    }
};