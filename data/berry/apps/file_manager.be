# app: File Manager
# startMenu: /System/File Manager
import json

class FileManagerApp
  var name
  var filelist
  var path_lbl
  var current_path
  var app_w
  var app_h
  var last_list

  def init()
    self.name = 'File Manager'
    self.current_path = '/'
    self.last_list = []
  end

  def setup(content, w, h)
    self.app_w = w
    self.app_h = h
    var y = 0

    # path label
    self.path_lbl = ui.label(content, self.current_path, 0, y, w, 16)
    ui.set_text_color(self.path_lbl, ui.TEXT_COLOR, ui.WINDOW_BG)
    ui.set_text_size(self.path_lbl, 1)
    ui.set_align(self.path_lbl, ui.LEFT)
    y += 18

    # toolbar: back, view mode buttons, storage info
    var btn_w = 36
    var bx = 0

    var back_btn = ui.button(content, '<-', bx, y, btn_w, 18)
    ui.on_click(back_btn, / -> self.navigate_up())
    bx += btn_w + 2

    var list_btn = ui.button(content, 'List', bx, y, btn_w, 18)
    ui.on_click(list_btn, def ()
      ui.filelist_set_view(self.filelist, 'list')
      ui.mark_dirty()
    end)
    bx += btn_w + 2

    var icon_btn = ui.button(content, 'Icon', bx, y, btn_w, 18)
    ui.on_click(icon_btn, def ()
      ui.filelist_set_view(self.filelist, 'icons')
      ui.mark_dirty()
    end)
    bx += btn_w + 2

    var det_btn = ui.button(content, 'Det', bx, y, btn_w, 18)
    ui.on_click(det_btn, def ()
      ui.filelist_set_view(self.filelist, 'details')
      ui.mark_dirty()
    end)
    bx += btn_w + 4

    # storage info
    var info_str = action('info')
    var info = json.load(info_str)
    if info != nil && info.find('fs') != nil
      var fs = info['fs']
      var used_k = fs['usedBytes'] / 1024
      var total_k = fs['totalBytes'] / 1024
      var info_text = str(used_k) + 'K/' + str(total_k) + 'K'
      var info_lbl = ui.label(content, info_text, bx, y, w - bx, 18)
      ui.set_text_color(info_lbl, ui.TEXT_COLOR, ui.WINDOW_BG)
      ui.set_text_size(info_lbl, 1)
      ui.set_align(info_lbl, ui.RIGHT)
    end
    y += 20

    # file list view
    var list_h = h - y
    self.filelist = ui.filelist(content, 0, y, w, list_h)
    ui.filelist_set_view(self.filelist, 'list')

    ui.on_item_selected(self.filelist, / idx -> self.on_select(idx))
    ui.on_item_activated(self.filelist, / idx -> self.on_activate(idx))

    self.refresh()
  end

  def refresh()
    var list_str = action('list ' + self.current_path)
    if list_str == nil return end

    self.last_list = json.load(list_str)
    if self.last_list == nil self.last_list = [] end

    ui.filelist_set_items(self.filelist, list_str)
    ui.set_text(self.path_lbl, self.current_path)
    ui.mark_dirty()
  end

  def on_select(idx)
    ui.mark_dirty()
  end

  def on_activate(idx)
    if self.last_list == nil || idx < 0 || idx >= size(self.last_list) return end
    var item = self.last_list[idx]
    if item == nil return end
    if item.find('isDir') != nil && item['isDir']
      var name = item['name']
      if name != nil
        if self.current_path == '/'
          self.current_path = '/' + name
        else
          self.current_path = self.current_path + '/' + name
        end
        self.refresh()
      end
    end
    ui.mark_dirty()
  end

  def navigate_up()
    if self.current_path == '/' return end
    var last_slash = -1
    var len = bytes(self.current_path)
    for i : 0 .. len - 1
      if self.current_path[i] == '/' last_slash = i end
    end
    if last_slash <= 0
      self.current_path = '/'
    else
      self.current_path = self.current_path[0..last_slash - 1]
      if self.current_path == '' self.current_path = '/' end
    end
    self.refresh()
  end

  def teardown()
  end
end

return FileManagerApp
