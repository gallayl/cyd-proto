# app: Paint

class PaintApp
  var name
  var canvas_h
  var cv
  var tool_btns
  var tool       # 0=draw, 1=erase, 2=rect, 3=circle, 4=fill
  var color_idx
  var touching
  var start_x
  var start_y
  var last_x
  var last_y
  var palette
  var palette_h
  var pal_w
  var pal_cv

  def init()
    self.name = 'Paint'
    self.tool = 0
    self.color_idx = 0
    self.touching = false
    self.tool_btns = []
    self.palette = [
      0x0000, 0xFFFF, 0x8410, 0xF800, 0xFD20, 0xFFE0,
      0x07E0, 0x07FF, 0x001F, 0x8010, 0xF81F, 0x8200
    ]
  end

  def setup(content, w, h)
    var toolbar_h = 20
    var pal_h = 20
    self.palette_h = pal_h
    self.pal_w = w

    var cv_h = h - toolbar_h - pal_h
    self.canvas_h = cv_h

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

    # canvas (4-bit palette for memory savings)
    self.cv = ui.canvas(content, 0, toolbar_h, w, cv_h, 4)

    # set up palette colors
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

    # fill white (palette index 1)
    ui.canvas_fill(self.cv, 1)

    # palette display canvas (16-bit)
    self.pal_cv = ui.canvas(content, 0, toolbar_h + cv_h, w, pal_h)
    self.draw_palette(self.pal_cv, w, pal_h)

    # canvas touch: gives correct local coordinates via BerryCanvasElement::onTouch
    ui.on_touch(self.cv, / lx ly -> self.on_canvas_touch(lx, ly))
    # palette touch: gives correct local coordinates
    ui.on_touch(self.pal_cv, / lx ly -> self.on_palette_touch(lx, ly))
    # content touch_end: always fires (even if finger lifts outside canvas)
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

  def draw_palette(pal_cv, w, h)
    var n = size(self.palette)
    var sw = w / n
    for i : 0 .. n - 1
      var sx = i * sw
      var cw = (i < n - 1) ? sw : (w - i * sw)
      ui.canvas_fill_rect(pal_cv, sx + 1, 1, cw - 2, h - 2, self.palette[i])
      if i == self.color_idx
        ui.canvas_draw_rect(pal_cv, sx, 0, cw, h, 0xFFFF)
      else
        ui.canvas_draw_rect(pal_cv, sx, 0, cw, h, ui.BUTTON_SHADOW)
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
        self.draw_thick_line(self.last_x, self.last_y, lx, ly, self.color_idx, 1)
      elif self.tool == 1
        self.draw_thick_line(self.last_x, self.last_y, lx, ly, 1, 3)
      end
      self.last_x = lx
      self.last_y = ly
    end
  end

  def on_palette_touch(lx, ly)
    if self.touching return end
    var n = size(self.palette)
    var sw = self.pal_w / n
    var idx = lx / sw
    if idx >= 0 && idx < n
      self.color_idx = idx
      self.draw_palette(self.pal_cv, self.pal_w, self.palette_h)
      ui.mark_dirty()
    end
  end

  def on_touch_end()
    if !self.touching return end

    if self.tool == 2
      # rect: use last known canvas position
      var lx = self.last_x
      var ly = self.last_y
      var rx = (self.start_x < lx) ? self.start_x : lx
      var ry = (self.start_y < ly) ? self.start_y : ly
      var rw = self.abs(lx - self.start_x) + 1
      var rh = self.abs(ly - self.start_y) + 1
      ui.canvas_draw_rect(self.cv, rx, ry, rw, rh, self.color_idx)
    elif self.tool == 3
      # ellipse: use last known canvas position
      var lx = self.last_x
      var ly = self.last_y
      var cx = (self.start_x + lx) / 2
      var cy = (self.start_y + ly) / 2
      var erx = self.abs(lx - self.start_x) / 2
      var ery = self.abs(ly - self.start_y) / 2
      if erx > 0 && ery > 0
        ui.canvas_draw_ellipse(self.cv, cx, cy, erx, ery, self.color_idx)
      end
    end

    self.touching = false
  end

  def draw_thick_line(x0, y0, x1, y1, col, r)
    var dx = self.abs(x1 - x0)
    var dy = self.abs(y1 - y0)
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

  def abs(v)
    return (v < 0) ? -v : v
  end

  def teardown()
  end
end

return PaintApp
