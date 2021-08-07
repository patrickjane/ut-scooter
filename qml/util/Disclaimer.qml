import QtQuick 2.0
import Ubuntu.Components 1.3
import Qt.labs.settings 1.0

import "../util"

Rectangle {
   property int padding: parent.width * 0.05
   property int paddingMedium: units.gu(2)

   Settings {
      id: settings
      property bool disclaimerAccepted: false
   }

   id: appDisclaimer
   anchors.verticalCenter: parent.verticalCenter
   anchors.horizontalCenter: parent.horizontalCenter
   width: parent.width * 0.8
   height: childrenRect.height

   visible: false
   radius: 8
   z: 200

   border.width: 1
   border.color: "#cdcdcd"

   Column {
      anchors.top: parent.top
      anchors.topMargin: padding
      anchors.horizontalCenter: parent.horizontalCenter
      width: parent.width - 2*padding
      spacing: padding

      Row {
         anchors.horizontalCenter: parent.horizontalCenter
         spacing: padding * 2

         Image {
            width: units.gu(2)
            height: units.gu(2)
            source: "qrc:///graphics/scooter.png"
            anchors.verticalCenter: parent.verticalCenter
         }

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(2)
            color: "#232323"
            text: "Scooters"
         }
      }

      Text {
         anchors.horizontalCenter: parent.horizontalCenter
         font.pointSize: units.gu(1.5)
         wrapMode: Text.WordWrap
         width: parent.width

         color: "#232323"
         text: i18n.tr("USE AT YOUR OWN RISK")
      }

      Text {
         anchors.horizontalCenter: parent.horizontalCenter
         font.pointSize: units.gu(1)
         wrapMode: Text.WordWrap
         width: parent.width

         color: "#232323"
         text: i18n.tr("This app is provided 'AS-IS' and with NO WARRANTIES. The app makes use of undocumented APIs of scooter providers, errors are therefore to be expected. The author of the app can not be held responsible for costs caused by the usage of the app.")
      }

      Button {
         anchors.horizontalCenter: parent.horizontalCenter

         color: "black"
         text: i18n.tr("Accept")
         width: (scooterInfo.width -2*padding) / 2 - paddingMedium
         onClicked: {
            settings.disclaimerAccepted = true
            appDisclaimer.visible = false
         }
      }

      Rectangle {
         id: spacer
         color: "transparent"
         width: parent.width
         height: units.gu(2)
      }
   }
}
