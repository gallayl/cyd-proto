# app: File Manager
import json

class FileManagerApp
  var name

  def init()
    self.name = 'File Manager'
  end

  def setup(content, w, h)
    var scroll = ui.scrollable(content)
    var row_h = 14
    var y = 4

    self.add_row(scroll, y, w, '-- Files --')
    y += row_h

    var list_str = action('list')
    var files = json.load(list_str)

    if files != nil
      for i : 0 .. size(files) - 1
        var f = files[i]
        var name = str(f['name'])
        var is_dir = false
        if f.find('isDir') != nil is_dir = f['isDir'] end

        var line
        if is_dir
          line = '[' + name + ']'
        else
          var sz = 0
          if f.find('size') != nil sz = f['size'] end
          line = name + '  (' + str(sz) + ' B)'
        end

        self.add_row(scroll, y, w, line)
        y += row_h
      end
    end

    y += 4
    self.add_row(scroll, y, w, '-- Storage --')
    y += row_h

    var info_str = action('info')
    var info = json.load(info_str)
    if info != nil && info.find('fs') != nil
      var fs = info['fs']
      self.add_row(scroll, y, w, 'Total: ' + str(fs['totalBytes']) + ' B')
      y += row_h
      self.add_row(scroll, y, w, 'Used: ' + str(fs['usedBytes']) + ' B')
      y += row_h
    end

    ui.set_content_height(scroll, y)
  end

  def add_row(scroll, y, w, text)
    var lbl = ui.label(scroll, text, 4, y, w - 8, 12)
    ui.set_text_color(lbl, ui.TEXT_COLOR, ui.WINDOW_BG)
    ui.set_text_size(lbl, 1)
    ui.set_align(lbl, ui.LEFT)
  end

  def teardown()
  end
end

return FileManagerApp
