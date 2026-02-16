# app: Log Viewer
# startMenu: /System/Log Viewer
# icon: builtin/log_viewer
import json

class LogViewerApp
  var name
  var scroll
  var log_labels
  var last_count
  var app_w

  def init()
    self.name = 'Log Viewer'
    self.log_labels = []
    self.last_count = 0
  end

  def setup(content, w, h)
    self.app_w = w
    var scroll = ui.scrollable(content)
    self.scroll = scroll

    self.populate_log()

    ui.timer(3000, / -> self.refresh_log())
  end

  def populate_log()
    var log_str = action('log')
    var entries = json.load(log_str)
    if entries == nil return end

    var count = size(entries)
    var row_h = 12
    var y = 2
    var w = self.app_w

    var shown = 0
    var i = count - 1
    while i >= 0 && shown < 50
      var entry = entries[i]
      var sev = '?'
      if entry.find('severity') != nil sev = str(entry['severity']) end
      var msg = ''
      if entry.find('message') != nil msg = str(entry['message']) end
      var line = '[' + sev + '] ' + msg

      var lbl = ui.label(self.scroll, line, 2, y, w - 4, 10)
      ui.set_text_color(lbl, ui.TEXT_COLOR, ui.WINDOW_BG)
      ui.set_text_size(lbl, 1)
      ui.set_align(lbl, ui.LEFT)
      self.log_labels.push(lbl)
      y += row_h
      shown += 1
      i -= 1
    end

    self.last_count = count
    ui.set_content_height(self.scroll, y)
  end

  def refresh_log()
    var log_str = action('log')
    var entries = json.load(log_str)
    if entries == nil return end

    var count = size(entries)
    if count == self.last_count return end

    # clear and rebuild
    ui.clear(self.scroll)
    self.log_labels = []

    var row_h = 12
    var y = 2
    var w = self.app_w

    var shown = 0
    var i = count - 1
    while i >= 0 && shown < 50
      var entry = entries[i]
      var sev = '?'
      if entry.find('severity') != nil sev = str(entry['severity']) end
      var msg = ''
      if entry.find('message') != nil msg = str(entry['message']) end
      var line = '[' + sev + '] ' + msg

      var lbl = ui.label(self.scroll, line, 2, y, w - 4, 10)
      ui.set_text_color(lbl, ui.TEXT_COLOR, ui.WINDOW_BG)
      ui.set_text_size(lbl, 1)
      ui.set_align(lbl, ui.LEFT)
      self.log_labels.push(lbl)
      y += row_h
      shown += 1
      i -= 1
    end

    self.last_count = count
    ui.set_content_height(self.scroll, y)
  end

  def teardown()
  end
end

return LogViewerApp
