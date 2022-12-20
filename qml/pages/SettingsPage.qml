import QtQuick 2.7
import Ubuntu.Components 1.3
import Ubuntu.Components.Themes 1.3
import QtQuick.Layouts 1.3
import Ubuntu.Components.ListItems 1.3
import Qt.labs.settings 1.0

import "../util"

Page {
   id: settingsPage
   anchors.fill: parent

   property var positionSource
   property var scooters
   property var logs
   property int padding: parent.width * 0.05

   Settings {
      id: settings
      property string accessToken
      property string mapType: "osm"
   }

   header: PageHeader {
      id: header
      title: i18n.tr("Settings")
   }

   Flickable {
      anchors.top: header.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottom: parent.bottom

      contentWidth: parent.width
      contentHeight: childrenRect.height

      Column {
         anchors.left: parent.left
         anchors.right: parent.right
         anchors.top: parent.top

         ListItem {
            height: l1.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: l1
               mainSlot: Label {
                  text: i18n.tr("Provider")
                  font.bold: true
               }
            }
         }

         Repeater {
            model: scooters.getSupportedProviders()

            ListItem {
               height: l2.height + (divider.visible ? divider.height : 0)

               SlotsLayout {
                  id: l2
                  mainSlot: Label {
                     text: HelperFunctions.capitalize(modelData)
                  }
                  Icon {
                     name: "toolkit_chevron-ltr_3gu"
                     SlotsLayout.position: SlotsLayout.Trailing;
                     width: units.gu(2)
                  }
               }

               onClicked: {
                  pageStack.push(Qt.resolvedUrl("./SettingsPageProvider.qml"), {
                     provider: modelData,
                     account: scooters.getAccountId(modelData),
                     scooters: scooters,
                     positionSource: positionSource })
               }
            }
         }

         ListItem {
            height: l3.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: l3
               mainSlot: Label {
                  text: i18n.tr("Support")
                  font.bold: true
               }
            }
         }

         ListItem {
            height: l4.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: l4
               mainSlot: Label {
                  text: i18n.tr("Logs")
               }
               Icon {
                  name: "toolkit_chevron-ltr_3gu"
                  SlotsLayout.position: SlotsLayout.Trailing;
                  width: units.gu(2)
               }
            }

            onClicked: {
               pageStack.push(Qt.resolvedUrl("./SettingsPageLogs.qml"), { logs: logs })
            }
         }
      }
   }
}
