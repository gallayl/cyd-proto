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

    # Red channel group box
    var r_gb = ui.groupbox(content, 'Red', 2, y, w - 4, 42)
    self.r_lbl = self.add_color_controls(r_gb, / v -> self.set_r(v))
    y += 48

    # Green channel group box
    var g_gb = ui.groupbox(content, 'Green', 2, y, w - 4, 42)
    self.g_lbl = self.add_color_controls(g_gb, / v -> self.set_g(v))
    y += 48

    # Blue channel group box
    var b_gb = ui.groupbox(content, 'Blue', 2, y, w - 4, 42)
    self.b_lbl = self.add_color_controls(b_gb, / v -> self.set_b(v))
    y += 52

    # Off button
    var off_btn = ui.button(content, 'Off', 4, y, w - 8, 22)
    ui.on_click(off_btn, / -> self.off())
  end

  def add_color_controls(gb, set_fn)
    var cy = 14
    var val_lbl = ui.label(gb, '0', 2, cy, 30, 20)
    ui.set_text_color(val_lbl, ui.TEXT_COLOR, ui.WINDOW_BG)

    var minus = ui.button(gb, '-', 36, cy, 28, 20)
    ui.on_click(minus, def ()
      var cur = self.get_val_for(val_lbl)
      if cur >= 25 cur -= 25 else cur = 0 end
      set_fn(cur)
    end)

    var plus = ui.button(gb, '+', 68, cy, 28, 20)
    ui.on_click(plus, def ()
      var cur = self.get_val_for(val_lbl)
      if cur <= 230 cur += 25 else cur = 255 end
      set_fn(cur)
    end)

    var mx = ui.button(gb, 'Max', 100, cy, 40, 20)
    ui.on_click(mx, / -> set_fn(255))

    return val_lbl
  end

  def get_val_for(lbl)
    if lbl == self.r_lbl return self.r end
    if lbl == self.g_lbl return self.g end
    if lbl == self.b_lbl return self.b end
    return 0
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

  def teardown()
  end
end

return RgbLedApp
