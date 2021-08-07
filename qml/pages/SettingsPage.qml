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

            ListItemLayout {
               id: l1
               title.text: i18n.tr("Maps")
               title.font.bold: true
               subtitle.text: i18n.tr("Restart the app after changing the map type")
               subtitle.color: "red"
               subtitle.visible: false
            }
         }

         ListItem {
            height: l2.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: l2
               mainSlot: Text {
                  text: i18n.tr("Use OpenStreetMap")
                  wrapMode: Text.WordWrap
               }
               Switch {
                  id: switchOSM
                  checked: settings.mapType === "osm"
                  SlotsLayout.position: SlotsLayout.Trailing;

                  onClicked: {
                     l1.subtitle.visible = true

                     if (checked) settings.mapType = "osm"
                     else         settings.mapType = "mapboxgl"
                  }
               }
            }
         }

         ListItem {
            height: l3.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: l3
               mainSlot: Text {
                  text: i18n.tr("Use Mapbox GL")
                  wrapMode: Text.WordWrap
               }
               Switch {
                  id: switchMapbox
                  checked: settings.mapType === "mapboxgl"
                  SlotsLayout.position: SlotsLayout.Trailing;

                  onClicked: {
                     l1.subtitle.visible = true

                     if (checked) settings.mapType = "mapboxgl"
                     else         settings.mapType = "osm"
                  }
               }
            }
         }

         ListItem {
            height: l4.height + (divider.visible ? divider.height : 0)

            ListItemLayout {
               id: l4
               title.text: i18n.tr("Mapbox Access Token")
               title.color: switchMapbox.checked ? "black" : "gray"
               subtitle.text: i18n.tr("Map won't work unless a valid access token is supplied")
               subtitle.color: "red"
               subtitle.visible: !settings.accessToken && switchMapbox.checked
            }
         }

         ListItem {
            divider.visible: false
            height: l5.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: l5
               mainSlot: Text {
                  text: i18n.tr("Mapbox requires a valid account and associated API key. After generating the key, please add it in the field below and restart the app.\nTo create an API key, consult the following article:\nhttps://docs.mylistingtheme.com/article/how-to-generate-a-mapbox-api-key")
                  color: switchMapbox.checked ? "black" : "gray"
                  wrapMode: Text.WordWrap
               }
            }
         }

         ListItem {
            height: l6.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: l6
               mainSlot: TextField {
                  id: accessTokenField
                  placeholderText: i18n.tr("Enter mapbox access token")
                  inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                  text: settings.accessToken || ""
                  enabled: switchMapbox.checked
               }
               Button {
                  text: i18n.tr("Save")
                  enabled: !!accessTokenField.text.length && switchMapbox.checked

                  SlotsLayout.position: SlotsLayout.Trailing;
                  onClicked: {
                     settings.accessToken = accessTokenField.text
                     l2.subtitle.text = i18n.tr("Please restart app")
                     l2.subtitle.visible = true
                  }
               }
            }
         }

         ListItem {
            height: l7.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: l7
               mainSlot: Label {
                  text: i18n.tr("Provider")
                  font.bold: true
               }
            }
         }

         Repeater {
            model: scooters.getSupportedProviders()

            ListItem {
               height: layoutProviderEntry.height + (divider.visible ? divider.height : 0)

               SlotsLayout {
                  id: layoutProviderEntry
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
                  pageStack.push(Qt.resolvedUrl("./SettingsPageProvider.qml"), { provider: modelData, account: scooters.getAccountId(modelData), scooters: scooters })
               }
            }
         }


         ListItem {
            height: l8.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: l8
               mainSlot: Label {
                  text: i18n.tr("Support")
                  font.bold: true
               }
            }
         }

         ListItem {
            height: l9.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: l9
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
