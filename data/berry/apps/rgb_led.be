# app: RGB LED

class RgbLedApp
  var name
  var r
  var g
  var b
  var preview
  var r_lbl
  var g_lbl
  var b_lbl

  def init()
    self.name = 'RGB LED'
    self.r = 0
    self.g = 0
    self.b = 0
  end

  def setup(content, w, h)
    var y = 4

    # color preview canvas
    self.preview = ui.canvas(content, 4, y, w - 8, 30)
    ui.canvas_fill(self.preview, 0)
    y += 38

    # Red row
    self.r_lbl = self.add_color_row(content, y, w, 'R', / v -> self.set_r(v))
    y += 34

    # Green row
    self.g_lbl = self.add_color_row(content, y, w, 'G', / v -> self.set_g(v))
    y += 34

    # Blue row
    self.b_lbl = self.add_color_row(content, y, w, 'B', / v -> self.set_b(v))
    y += 34

    # Off button
    var off_btn = ui.button(content, 'Off', 4, y, w - 8, 22)
    ui.on_click(off_btn, / -> self.off())
  end

  def set_r(val)
    self.r = val
    ui.set_text(self.r_lbl, str(self.r))
    self.apply_color()
  end

  def set_g(val)
    self.g = val
    ui.set_text(self.g_lbl, str(self.g))
    self.apply_color()
  end

  def set_b(val)
    self.b = val
    ui.set_text(self.b_lbl, str(self.b))
    self.apply_color()
  end

  def apply_color()
    action('rgbLed setColor ' + str(self.r) + ' ' + str(self.g) + ' ' + str(self.b))
    # update preview
    var c = ui.color565(self.r, self.g, self.b)
    ui.canvas_fill(self.preview, c)
    ui.canvas_draw_rect(self.preview, 0, 0, 232, 30, ui.BUTTON_SHADOW)
  end

  def off()
    self.r = 0
    self.g = 0
    self.b = 0
    ui.set_text(self.r_lbl, '0')
    ui.set_text(self.g_lbl, '0')
    ui.set_text(self.b_lbl, '0')
    self.apply_color()
  end

  def add_color_row(content, y, w, label, set_fn)
    var lbl = ui.label(content, label, 4, y, 16, 22)
    ui.set_text_color(lbl, ui.TEXT_COLOR, ui.WINDOW_BG)

    var val_lbl = ui.label(content, '0', 22, y, 30, 22)
    ui.set_text_color(val_lbl, ui.TEXT_COLOR, ui.WINDOW_BG)

    # minus
    var minus = ui.button(content, '-', 54, y, 28, 22)
    ui.on_click(minus, def ()
      var cur = self.get_val(label)
      if cur >= 25 cur -= 25 else cur = 0 end
      set_fn(cur)
    end)

    # plus
    var plus = ui.button(content, '+', 86, y, 28, 22)
    ui.on_click(plus, def ()
      var cur = self.get_val(label)
      if cur <= 230 cur += 25 else cur = 255 end
      set_fn(cur)
    end)

    # max
    var mx = ui.button(content, 'Max', 118, y, 40, 22)
    ui.on_click(mx, / -> set_fn(255))

    return val_lbl
  end

  def get_val(label)
    if label == 'R' return self.r end
    if label == 'G' return self.g end
    if label == 'B' return self.b end
    return 0
  end

  def teardown()
  end
end

return RgbLedApp
