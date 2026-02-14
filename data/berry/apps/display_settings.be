# app: Display

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

    # Brightness section
    var header = ui.label(content, 'Brightness', 4, y, w - 8, 14)
    ui.set_text_color(header, ui.TEXT_COLOR, ui.WINDOW_BG)
    ui.set_text_size(header, 1)
    ui.set_align(header, ui.LEFT)
    y += 18

    self.brightness_lbl = ui.label(content, str(self.brightness), 4, y, 40, 22)
    ui.set_text_color(self.brightness_lbl, ui.TEXT_COLOR, ui.WINDOW_BG)

    var minus = ui.button(content, '-', 48, y, 36, 22)
    ui.on_click(minus, def ()
      if self.brightness >= 25
        self.brightness -= 25
      else
        self.brightness = 0
      end
      self.apply()
    end)

    var plus = ui.button(content, '+', 88, y, 36, 22)
    ui.on_click(plus, def ()
      if self.brightness <= 230
        self.brightness += 25
      else
        self.brightness = 255
      end
      self.apply()
    end)

    var mx = ui.button(content, 'Max', 128, y, 40, 22)
    ui.on_click(mx, def ()
      self.brightness = 255
      self.apply()
    end)
    y += 34

    # Calibration section
    var cal_header = ui.label(content, 'Touch Calibration', 4, y, w - 8, 14)
    ui.set_text_color(cal_header, ui.TEXT_COLOR, ui.WINDOW_BG)
    ui.set_text_size(cal_header, 1)
    ui.set_align(cal_header, ui.LEFT)
    y += 18

    var cal_btn = ui.button(content, 'Calibrate', 4, y, 80, 22)
    ui.on_click(cal_btn, / -> action('screen calibrate'))
  end

  def apply()
    action('screen brightness ' + str(self.brightness))
    ui.set_text(self.brightness_lbl, str(self.brightness))
  end

  def teardown()
  end
end

return DisplaySettingsApp
