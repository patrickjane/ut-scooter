import QtQuick 2.7
import Ubuntu.Components 1.3
import Ubuntu.Components.Themes 1.3
import QtQuick.Layouts 1.3
import Ubuntu.Components.ListItems 1.3

import "../util"

Page {
   property string provider
   property var history

   id: historyPage
   anchors.fill: parent

   header: PageHeader {
      id: header
      title: HelperFunctions.capitalize(provider) + " " + i18n.tr("account history")
   }

   Flickable {
      anchors.top: header.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottom: parent.bottom

      contentWidth: parent.width
      contentHeight: col.height

      Column {
         id: col
         anchors.left: parent.left
         anchors.right: parent.right
         anchors.top: parent.top

         Repeater {
            model: history

            ListItem {
               height: layout1.height + (divider.visible ? divider.height : 0)

               SlotsLayout {
                  id: layout1
                  mainSlot: Column {
                     spacing: units.gu(1)

                     Text {
                        anchors.left: parent.left
                        anchors.leftMargin: units.gu(1)
                        anchors.right: parent.right
                        anchors.rightMargin: units.gu(1)

                        text: modelData.type
                        font.bold: true
                     }

                     Text {
                        anchors.left: parent.left
                        anchors.leftMargin: units.gu(1)
                        anchors.right: parent.right
                        anchors.rightMargin: units.gu(1)
                        text: modelData.text
                        width: parent.width - units.gu(2)
                        wrapMode: Text.WordWrap
                     }

                     Row {
                        anchors.left: parent.left
                        anchors.leftMargin: units.gu(1)
                        anchors.right: parent.right
                        anchors.rightMargin: units.gu(1)
                        spacing: units.gu(1)

                        Text {
                           text: HelperFunctions.formatCurrency(modelData)
                        }

                        Text {
                           visible: !!modelData.duration
                           text: HelperFunctions.toElapsed(modelData.duration)
                        }
                     }
                  }
                  Label {
                     text: new Date(modelData.date*1000).toLocaleString(Qt.locale(), Locale.ShortFormat)
                     SlotsLayout.position: SlotsLayout.Trailing;
                  }
               }
            }
         }
      }
   }
}