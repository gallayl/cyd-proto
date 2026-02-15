# app: Paint

class PaintApp
  var name
  var cv
  var cv_w
  var cv_h
  var tool_btns
  var tool
  var color_idx
  var touching
  var start_x
  var start_y
  var last_x
  var last_y
  var palette
  var pal_btns

  def init()
    self.name = 'Paint'
    self.tool = 0
    self.color_idx = 0
    self.touching = false
    self.tool_btns = []
    self.pal_btns = []
    self.palette = [
      0x0000, 0xFFFF, 0x8410, 0xF800, 0xFD20, 0xFFE0,
      0x07E0, 0x07FF, 0x001F, 0x8010, 0xF81F, 0x8200
    ]
  end

  def setup(content, w, h)
    var toolbar_h = 18
    var pal_h = 16
    var cv_h = h - toolbar_h - pal_h
    self.cv_w = w
    self.cv_h = cv_h

    # toolbar buttons
    var btn_w = w / 5
    var tools = ['Draw', 'Erase', 'Rect', 'Circ', 'Fill']
    self.tool_btns = []
    for i : 0 .. 4
      var bx = i * btn_w
      var bw = (i < 4) ? btn_w : (w - i * btn_w)
      var btn = ui.button(content, tools[i], bx, 0, bw, toolbar_h)
      ui.set_text_size(btn, 1)
      self.tool_btns.push(btn)
      var idx = i
      ui.on_click(btn, def ()
        self.tool = idx
        self.update_tool_visual()
      end)
    end
    self.update_tool_visual()

    # canvas — 4-bit palette, uses ~1/4 the memory of 16-bit
    self.cv = ui.canvas(content, 0, toolbar_h, w, cv_h, 4)

    for i : 0 .. size(self.palette) - 1
      var c = self.palette[i]
      var r = ((c >> 11) & 0x1F) << 3
      var g = ((c >> 5) & 0x3F) << 2
      var b = (c & 0x1F) << 3
      ui.canvas_set_palette(self.cv, i, r, g, b)
    end
    for i : size(self.palette) .. 15
      ui.canvas_set_palette(self.cv, i, 255, 255, 255)
    end
    ui.canvas_fill(self.cv, 1)

    # palette row — small buttons instead of a canvas to save memory
    var n = size(self.palette)
    var sw = w / n
    self.pal_btns = []
    for i : 0 .. n - 1
      var bx = i * sw
      var bw = (i < n - 1) ? sw : (w - i * sw)
      var btn = ui.button(content, '', bx, toolbar_h + cv_h, bw, pal_h)
      ui.set_bg_color(btn, self.palette[i])
      self.pal_btns.push(btn)
      var idx = i
      ui.on_click(btn, def ()
        self.color_idx = idx
        self.update_pal_visual()
      end)
    end
    self.update_pal_visual()

    # canvas touch for drawing; content touch_end for state cleanup
    ui.on_touch(self.cv, / lx ly -> self.on_canvas_touch(lx, ly))
    ui.on_touch_end(content, / x y -> self.on_touch_end())
  end

  def update_tool_visual()
    for i : 0 .. 4
      if i == self.tool
        ui.set_border_colors(self.tool_btns[i], ui.BUTTON_SHADOW, ui.BUTTON_HIGHLIGHT)
      else
        ui.set_border_colors(self.tool_btns[i], ui.BUTTON_HIGHLIGHT, ui.BUTTON_SHADOW)
      end
    end
  end

  def update_pal_visual()
    for i : 0 .. size(self.palette) - 1
      if i == self.color_idx
        ui.set_border_colors(self.pal_btns[i], ui.BUTTON_SHADOW, ui.BUTTON_HIGHLIGHT)
      else
        ui.set_border_colors(self.pal_btns[i], ui.BUTTON_HIGHLIGHT, ui.BUTTON_SHADOW)
      end
    end
  end

  def on_canvas_touch(lx, ly)
    if !self.touching
      self.touching = true
      self.start_x = lx
      self.start_y = ly
      self.last_x = lx
      self.last_y = ly

      if self.tool == 0
        ui.canvas_fill_circle(self.cv, lx, ly, 1, self.color_idx)
      elif self.tool == 1
        ui.canvas_fill_circle(self.cv, lx, ly, 3, 1)
      elif self.tool == 4
        ui.canvas_flood_fill(self.cv, lx, ly, self.color_idx)
      end
    else
      if self.tool == 0
        self.draw_line(self.last_x, self.last_y, lx, ly, self.color_idx, 1)
      elif self.tool == 1
        self.draw_line(self.last_x, self.last_y, lx, ly, 1, 3)
      end
      self.last_x = lx
      self.last_y = ly
    end
  end

  def on_touch_end()
    if !self.touching return end

    if self.tool == 2
      var lx = self.last_x
      var ly = self.last_y
      var rx = (self.start_x < lx) ? self.start_x : lx
      var ry = (self.start_y < ly) ? self.start_y : ly
      var rw = self.iabs(lx - self.start_x) + 1
      var rh = self.iabs(ly - self.start_y) + 1
      ui.canvas_draw_rect(self.cv, rx, ry, rw, rh, self.color_idx)
    elif self.tool == 3
      var lx = self.last_x
      var ly = self.last_y
      var cx = (self.start_x + lx) / 2
      var cy = (self.start_y + ly) / 2
      var erx = self.iabs(lx - self.start_x) / 2
      var ery = self.iabs(ly - self.start_y) / 2
      if erx > 0 && ery > 0
        ui.canvas_draw_ellipse(self.cv, cx, cy, erx, ery, self.color_idx)
      end
    end

    self.touching = false
  end

  def draw_line(x0, y0, x1, y1, col, r)
    var dx = self.iabs(x1 - x0)
    var dy = self.iabs(y1 - y0)
    var steps = (dx > dy) ? dx : dy
    if steps == 0
      ui.canvas_fill_circle(self.cv, x0, y0, r, col)
      return
    end
    for i : 0 .. steps
      var px = x0 + (x1 - x0) * i / steps
      var py = y0 + (y1 - y0) * i / steps
      ui.canvas_fill_circle(self.cv, px, py, r, col)
    end
  end

  def iabs(v)
    return (v < 0) ? -v : v
  end

  def teardown()
  end
end

return PaintApp
