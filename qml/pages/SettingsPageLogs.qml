import QtQuick 2.7
import Ubuntu.Components 1.3
import Ubuntu.Components.Themes 1.3
import QtQuick.Layouts 1.3
import Ubuntu.Components.ListItems 1.3
import Qt.labs.settings 1.0

import "../util"

import QmlClipboard 1.0

Page {
   id: settingsPage
   anchors.fill: parent

   property var logs
   property var scooters

   Settings {
      id: settings
      property string accessToken
   }

   QmlClipboard {
      id: clipboard
   }

   header: PageHeader {
      id: header
      title: i18n.tr("Logs")
      trailingActionBar {
         actions: [
            Action {
               iconName: "share"
               onTriggered: {
                  var logFile = scooters.exportLogs(logWindow.text)

                  console.log("Exporting:", logFile)

                  if (logFile)
                     pageStack.push(Qt.resolvedUrl("SharePage.qml"), { url: "file://" + logFile })
               }
            },
            Action {
               iconName: "edit-copy"
               onTriggered: {
                  clipboard.setText(logWindow.text)
               }
            }
         ]
      }
   }

   TextArea {
      id: logWindow
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: header.bottom
      anchors.bottom: parent.bottom
      font.family: "Courier New"
      readOnly: true
      wrapMode: Text.NoWrap
   }

   Component.onCompleted: {
      var l = logs || []

      l.forEach(function(msg) {
         try {
            var message = formatDate(msg.time)
                  + ":" + msg.severity
                  + ":" + HelperFunctions.capitalize(msg.provider.padEnd(4))
                  + ": " + msg.message

            logWindow.append(message)
         } catch (e) {}
      })

      var endPos = logWindow.text.lastIndexOf("\n")

      if (endPos === -1)
         endPos = 0
      else
         endPos++

      logWindow.moveCursorSelection(endPos)
      logWindow.deselect()
   }

   function formatDate(date) {
      return date.toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm:ss")
   }
}
