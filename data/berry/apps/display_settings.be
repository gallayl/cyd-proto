# app: Display
# startMenu: /Settings/Display
# icon: builtin/display

class DisplaySettingsApp
  var name
  var brightness
  var brightness_lbl

  def init()
    self.name = 'Display'
    self.brightness = 128
  end

  def setup(content, w, h)
    var y = 4

    # Brightness group box
    var bright_gb = ui.groupbox(content, 'Brightness', 2, y, w - 4, 56)
    var gb_bounds = ui.bounds(bright_gb)
    var gx = gb_bounds[0]
    var gy = gb_bounds[1]

    self.brightness_lbl = ui.label(bright_gb, str(self.brightness), 2, 14, 40, 22)
    ui.set_text_color(self.brightness_lbl, ui.TEXT_COLOR, ui.WINDOW_BG)

    var minus = ui.button(bright_gb, '-', 46, 14, 36, 22)
    ui.on_click(minus, def ()
      if self.brightness >= 25
        self.brightness -= 25
      else
        self.brightness = 0
      end
      self.apply()
    end)

    var plus = ui.button(bright_gb, '+', 86, 14, 36, 22)
    ui.on_click(plus, def ()
      if self.brightness <= 230
        self.brightness += 25
      else
        self.brightness = 255
      end
      self.apply()
    end)

    var mx = ui.button(bright_gb, 'Max', 126, 14, 40, 22)
    ui.on_click(mx, def ()
      self.brightness = 255
      self.apply()
    end)

    y += 62

    # Calibration group box
    var cal_gb = ui.groupbox(content, 'Touch Calibration', 2, y, w - 4, 48)

    var cal_btn = ui.button(cal_gb, 'Calibrate', 2, 14, 80, 22)
    ui.on_click(cal_btn, / -> action('screen calibrate'))

    y += 54

    # Rotation group box
    var rot_gb = ui.groupbox(content, 'Screen Rotation', 2, y, w - 4, 48)

    var rot0 = ui.button(rot_gb, '0°', 2, 14, 40, 22)
    ui.on_click(rot0, / -> action('screen rotate 0'))

    var rot90 = ui.button(rot_gb, '90°', 46, 14, 40, 22)
    ui.on_click(rot90, / -> action('screen rotate 1'))

    var rot180 = ui.button(rot_gb, '180°', 90, 14, 40, 22)
    ui.on_click(rot180, / -> action('screen rotate 2'))

    var rot270 = ui.button(rot_gb, '270°', 134, 14, 40, 22)
    ui.on_click(rot270, / -> action('screen rotate 3'))
  end

  def apply()
    action('screen brightness ' + str(self.brightness))
    ui.set_text(self.brightness_lbl, str(self.brightness))
  end

  def teardown()
  end
end

return DisplaySettingsApp
