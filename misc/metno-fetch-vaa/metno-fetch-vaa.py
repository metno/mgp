#!/usr/bin/python

# Copyright (C) 2014 MET Norway (met.no)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

import commands, datetime, HTMLParser, os, subprocess, sys, urllib2, urlparse

from PyQt4.QtCore import *
from PyQt4.QtGui import *

checked_dict = {False: Qt.Unchecked, True: Qt.Checked}

class Settings(QSettings):

    """Convenience class to help read values from settings files as Python datatypes.
    """

    def __init__(self, organisation, product):

        QSettings.__init__(self, organisation, product)
    
    def value(self, key, default = QVariant()):
    
        """Reads the value from the settings file that corresponds to the given key,
        with the type defined by the default value. If the key is not defined in the
        settings file then the default value is returned instead.
        """
        value = QSettings.value(self, key, default)
        if type(default) == int:
            return value.toInt()[0]
        elif type(default) == bool:
            return value.toBool()
        elif type(default) == float:
            return value.toFloat()[0]
        elif type(default) == str:
            return str(value.toString())
        elif type(default) == unicode:
            return unicode(value.toString())
        elif type(default) == list:
            if type(default[0]) == int:
                return map(lambda v: v.toInt()[0], value.toList())
        else:
            return value
        
        raise ValueError, "Unknown value type for key '%s': %s" % (key, type(default))


class Parser(HTMLParser.HTMLParser):

    def __init__(self):
    
        HTMLParser.HTMLParser.__init__(self)
        self.href = ""
        self.text = ""
        self.table_row = []
        self.tags = []
    
    def feed(self, data):

        self.anchors = []
        HTMLParser.HTMLParser.feed(self, data)
    
    def handle_starttag(self, tag, attrs):
    
        self.tags.append(tag)
        
        if tag == "a":
            d = dict(attrs)
            try:
                self.href = d["href"]
            except KeyError:
                pass
        elif tag == "tr":
            self.table_row = []
        
        self.text = ""

    def handle_data(self, data):
    
        self.text += data
    
    def handle_endtag(self, tag):

        if tag == "a":
            self.anchors.append((self.href, self.text, self.table_row))
            self.href = ""
        elif tag == "td":
            self.table_row.append(self.text)
        
        # Discard any non-matching end tags.
        while self.tags:
            if self.tags.pop() == tag:
                break


class Fetcher:

    showBusy = True
    showInMenu = True

    def hasExistingFile(self, output_dir, href):

        file_name = href.split("/")[-1]
         
        vaa_file = os.path.join(output_dir, file_name)
        if vaa_file.endswith(".html"):
            kml_file = file_name.replace(".html", ".kml")
        else:
            kml_file = file_name + ".kml"
        
        return os.path.exists(os.path.join(output_dir, kml_file))


class ToulouseFetcher(Fetcher):

    url = "http://www.meteo.fr/vaac/evaa.html"
    number_to_fetch = 10
    returns_html = True

    def fetch(self, vaaList, output_dir):
    
        "Reads the messages available from the URL for the current VAA centre."

        html = urllib2.urlopen(self.url).read()
        p = Parser()
        p.feed(html)
        p.close()
        
        count = 0

        for href, text, table_text in p.anchors:

            if text.endswith("UTC"):
            
                # The date is encoded in the URL for the advisory.
                info = href.split(".")
                date = datetime.datetime.strptime(info[-2], "%Y%m%d%H%M").strftime("%Y-%m-%d %H:%M")
                volcano = info[2].replace("_", " ")
                item = QListWidgetItem("%s (%s)" % (date, volcano))
                item.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
                item.href = href
                item.url = urlparse.urljoin(self.url, href)
                item.content = None
                item.setCheckState(checked_dict[self.hasExistingFile(output_dir, href)])
                vaaList.addItem(item)

                count += 1
                if count == self.number_to_fetch:
                    break


class AnchorageFetcher(Fetcher):

    url = "http://vaac.arh.noaa.gov/list_vaas.php"
    number_to_fetch = 10
    returns_html = True

    def fetch(self, vaaList, output_dir):
    
        "Reads the messages available from the URL for the current VAA centre."

        html = urllib2.urlopen(self.url).read()
        p = Parser()
        p.feed(html)
        p.close()
        
        count = 0

        for href, text, table_text in p.anchors:

            if text == "X" and href.split("/")[-2] == "VAA":
            
                # The date is encoded in the associated table text.
                date = datetime.datetime.strptime(table_text[0], "%Y-%m-%d %H:%M:%S").strftime("%Y-%m-%d %H:%M:%S")
                volcano = table_text[1].replace("_", " ")
                item = QListWidgetItem("%s (%s)" % (date, volcano))
                item.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
                item.href = href
                item.url = urlparse.urljoin(self.url, href)
                item.content = None
                item.setCheckState(checked_dict[self.hasExistingFile(output_dir, href)])
                vaaList.addItem(item)

                count += 1
                if count == self.number_to_fetch:
                    break


class LocalFileFetcher(Fetcher):

    returns_html = False
    showBusy = False
    showInMenu = False

    def fetch(self, vaaList, output_dir):
    
        fileName = QFileDialog.getOpenFileName(None, QApplication.translate("LocalFileFetcher", "Open VAA File"))
        
        if fileName.isEmpty():
            return
        
        fileName = unicode(fileName)
        
        vaaList.clear()
        item = QListWidgetItem(fileName)
        item.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
        item.href = fileName
        item.url = urlparse.urljoin("file://", fileName)
        item.content = None
        item.setCheckState(checked_dict[False])
        vaaList.addItem(item)


class EditDialog(QDialog):

    def __init__(self, content, parent = None):

        QDialog.__init__(self, parent)

        self.textEdit = QPlainTextEdit()
        self.textEdit.setPlainText(content)
        
        buttonBox = QDialogButtonBox()
        buttonBox.addButton(QDialogButtonBox.Ok)
        buttonBox.addButton(QDialogButtonBox.Cancel)
        
        buttonBox.accepted.connect(self.accept)
        buttonBox.rejected.connect(self.reject)

        layout = QVBoxLayout(self)
        layout.addWidget(self.textEdit)
        layout.addWidget(buttonBox)

        self.setWindowTitle(self.tr("Edit Message"))


class Window(QMainWindow):

    def __init__(self, fetchers):

        QMainWindow.__init__(self)

        self.fetchers = fetchers
        self.settings = Settings("met.no", "metno-fetch-vaa")
        
        self.output_dir = self.settings.value("work directory",
                          os.path.join(os.getenv("HOME"), ".diana", "work"))
        self.workLog = []
        
        contentWidget = QWidget()
        layout = QGridLayout(contentWidget)
        
        fileMenu = self.menuBar().addMenu(self.tr("&File"))
        openFileAction = fileMenu.addAction(self.tr("&Open File..."), self.fetchAdvisories,
            QKeySequence.Open)
        openFileAction.name = u"Local file"

        fileMenu.addSeparator()

        # Create a list of available VAA centres in the menu.
        names = self.fetchers.keys()
        names.sort()

        for name in names:
            if self.fetchers[name].showInMenu:
                action = fileMenu.addAction(name, self.fetchAdvisories)
                action.name = name
        
        fileMenu.addSeparator()
        fileMenu.addAction(self.tr("E&xit"), self.close, QKeySequence(QKeySequence.Quit))

        # Create a list of downloaded advisories.
        self.vaaList = QListWidget()
        layout.addWidget(self.vaaList, 0, 0)
        
        # Add a panel of buttons.
        buttonLayout = QHBoxLayout()

        self.editButton = QPushButton(self.tr("&Edit message"))
        self.editButton.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Fixed)
        buttonLayout.addWidget(self.editButton)
        buttonLayout.setAlignment(self.editButton, Qt.AlignHCenter)

        self.convertButton = QPushButton(self.tr("&Convert messages"))
        self.convertButton.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Fixed)
        buttonLayout.addWidget(self.convertButton)
        buttonLayout.setAlignment(self.convertButton, Qt.AlignHCenter)

        layout.addLayout(buttonLayout, 1, 0)
        
        # Ensure that the list widgets are given enough space.
        layout.setRowStretch(1, 1)

        # Add a log viewer.
        self.logViewer = QPlainTextEdit()
        self.logViewer.setReadOnly(True)
        self.showHideLogButton = QPushButton(self.tr("&Hide log"))
        self.showHideLogButton.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Fixed)

        layout.addWidget(self.logViewer, 3, 0)
        layout.addWidget(self.showHideLogButton, 4, 0)
        self.showHideLogViewer(self.settings.value("window/log", False))

        # Make connections.
        self.vaaList.itemSelectionChanged.connect(self.showLog)
        self.vaaList.itemActivated.connect(self.showLog)
        self.vaaList.itemSelectionChanged.connect(self.updateButtons)
        self.vaaList.itemActivated.connect(self.updateButtons)
        self.editButton.clicked.connect(self.editMessage)
        self.convertButton.clicked.connect(self.convertAdvisories)
        self.showHideLogButton.clicked.connect(self.showHideLogViewer)
        
        self.updateButtons()

        self.setCentralWidget(contentWidget)
        self.setWindowTitle(self.tr("Fetch Volcanic Ash Advisories"))
        if self.settings.contains("window/geometry"):
            self.restoreGeometry(self.settings.value("window/geometry").toByteArray())
        else:
            self.resize(640, 480)
    
    def fetchAdvisories(self):
    
        self.vaaList.clear()
        name = self.sender().name
        fetcher = self.fetchers[name]
        
        # Create the output directory if it does not already exist.
        if not os.path.exists(self.output_dir):
            os.system("mkdir -p " + commands.mkarg(self.output_dir))
        
        if fetcher.showBusy:
            QApplication.setOverrideCursor(Qt.WaitCursor)
        
        fetcher.fetch(self.vaaList, self.output_dir)

        self.workLog = []
        self.updateButtons()
        
        if fetcher.showBusy:
            QApplication.restoreOverrideCursor()
            
            if self.vaaList.count() == 0:
                QMessageBox.information(self, self.tr("Fetching from %1").arg(name),
                    self.tr("No new messages available from %1.").arg(name))

    def updateButtons(self):
    
        yet_to_convert = len(filter(lambda i: self.vaaList.item(i).checkState() == Qt.Unchecked,
                                    range(self.vaaList.count())))
        self.convertButton.setEnabled(yet_to_convert)
        editable = self.vaaList.count() > 0 and self.vaaList.currentRow() != -1
        self.editButton.setEnabled(editable)
    
    def convertAdvisories(self):
    
        QApplication.setOverrideCursor(Qt.WaitCursor)
        
        kml_files = []
        failed_files = []
        self.workLog = []
        
        for i in range(self.vaaList.count()):
        
            item = self.vaaList.item(i)
            item.setCheckState(Qt.Unchecked)
            QApplication.processEvents()

            href = item.href
            url = item.url
            
            file_name = href.split("/")[-1]
            
            vaa_file = os.path.join(self.output_dir, file_name)
            if vaa_file.endswith(".html"):
                kml_file = file_name.replace(".html", ".kml")
            else:
                kml_file = file_name + ".kml"
            
            kml_file = os.path.join(self.output_dir, kml_file)
            if not item.content:
                vaa_content = item.content = urllib2.urlopen(url).read()
            else:
                vaa_content = item.content

            # Wrap any non-HTML content in a <pre> element.
            if not vaa_file.endswith(".html"):
                vaa_content = "<pre>\n" + vaa_content + "\n</pre>\n"
                vaa_file += ".html"

            open(vaa_file, "w").write(vaa_content)
            
            # Convert the message in the HTML file to a KML file.
            s = subprocess.Popen(["/usr/bin/metno-vaa-kml", vaa_file],
                                 stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            if s.wait() != 0:
                failed_files.append(vaa_file)
                item.setIcon(QApplication.style().standardIcon(QStyle.SP_MessageBoxWarning))
            else:
                # Remove the HTML file.
                os.remove(vaa_file)
                kml_files.append(kml_file)
                item.setCheckState(Qt.Checked)
                item.setIcon(QIcon())

            self.workLog.append(s.stdout.read())
        
        # Update the log viewer if it is already shown.
        if self.logViewer.isVisible():
            self.showLog()

        self.updateButtons()
        
        QApplication.restoreOverrideCursor()
    
    def showLog(self):
    
        # Since this is called when the selection changes, and the selection
        # changes just before the list is cleared, we can't check for the
        # number of items in the list.
        if len(self.vaaList.selectedItems()) == 0:
            self.logViewer.clear()
            return
        
        row = self.vaaList.currentRow()
        if 0 <= row < len(self.workLog):
        
            text = self.workLog[row]
            self.logViewer.setPlainText(text)
            self.showHideLogViewer(True)
    
    # Use a decorator to avoid receiving the signal that includes a boolean value.
    @pyqtSlot()
    def showHideLogViewer(self, show = None):
    
        if show is None:
            show = not self.logViewer.isVisible()
        
        self.logViewer.setShown(show)
        if show:
            self.showHideLogButton.setText(self.tr("&Hide log"))
        else:
            self.showHideLogButton.setText(self.tr("&Show log"))
    
    def editMessage(self):

        row = self.vaaList.currentRow()
        item = self.vaaList.item(row)

        if not item.content:
            item.content = urllib2.urlopen(item.url).read()
        
        oldContent = item.content

        editDialog = EditDialog(item.content[:], self)
        editDialog.restoreGeometry(self.settings.value("editdialog/geometry").toByteArray())

        if editDialog.exec_() == QDialog.Accepted:
            item.content = unicode(editDialog.textEdit.toPlainText())
            if oldContent != item.content:
                item.setCheckState(Qt.Unchecked)

        self.updateButtons()
    
    def closeEvent(self, event):

        self.settings.setValue("window/geometry", self.saveGeometry())
        self.settings.setValue("window/log", self.logViewer.isVisible())
        self.settings.setValue("editdialog/geometry", self.saveGeometry())
        self.settings.sync()


if __name__ == "__main__":

    app = QApplication(sys.argv)

    fetchers = {u"Toulouse VAAC": ToulouseFetcher(),
                u"Anchorage VAAC": AnchorageFetcher(),
                u"Local file": LocalFileFetcher()}

    window = Window(fetchers)
    window.show()
    sys.exit(app.exec_())
