import QtQuick 2.0
import Ubuntu.Components 1.3

import "../util"

Rectangle {
   property int padding: parent.width * 0.05
   property int paddingMedium: units.gu(2)

   property var scooter
   property string mode: "unlock"
   property var onRing
   property var onMissing
   property var onUnlock

   id: scooterInfo
   anchors.verticalCenter: parent.verticalCenter
   anchors.horizontalCenter: parent.horizontalCenter
   width: parent.width * 0.8
   height: childrenRect.height

   visible: !!scooter
   radius: 8
   z: 200

   border.width: 1
   border.color: "#cdcdcd"

   MouseArea {
      anchors.fill: parent
   }

   Column {
      anchors.top: parent.top
      anchors.topMargin: padding
      anchors.horizontalCenter: parent.horizontalCenter
      width: parent.width - 2*padding
      spacing: paddingMedium

      Row {
         anchors.horizontalCenter: parent.horizontalCenter
         spacing: paddingMedium

         Image {
            width: units.gu(3)
            height: units.gu(3)
            source: "qrc:///graphics/scooter.png"
            anchors.verticalCenter: parent.verticalCenter
         }

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(2)
            color: "#232323"
            text: scooterInfo.scooter && HelperFunctions.capitalize(scooterInfo.scooter.provider) || ""
         }
      }

      Text {
         anchors.horizontalCenter: parent.horizontalCenter

         font.pointSize: units.gu(1)
         wrapMode: Text.WordWrap
         width: parent.width

         color: "#232323"
         text: scooterInfo.scooter && scooterInfo.scooter.priceString || ""
      }

      Row {
         spacing: paddingMedium
         anchors.horizontalCenter: parent.horizontalCenter

         Battery {
            batteryLevel: scooterInfo.scooter && (scooterInfo.scooter.battery / 100) || 0
         }

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(1)
            color: "#232323"
            text: scooterInfo.scooter && (Math.floor(scooterInfo.scooter.range / 1000) + "km") || ""
         }

         Icon {
            width: units.gu(3)
            height: units.gu(3)
            name: "lock"
            visible: scooterInfo.scooter ? (scooterInfo.scooter.captive || !scooterInfo.scooter.available) : false
         }
      }

      Row {
         spacing: paddingMedium
         anchors.horizontalCenter: parent.horizontalCenter

         Button {
            color: "black"
            text: i18n.tr("Ring")
            width: (scooterInfo.width -2*padding) / 2 - paddingMedium
            visible: scooterInfo.mode == "show"
            onClicked: onRing(scooterInfo.scooter)
         }
         Button {
            color: "black"
            text: scooterInfo.scooter && (!scooterInfo.scooter.available || scooterInfo.scooter.captive)
                  ? (scooterInfo.scooter.captive ? i18n.tr("Captive") : i18n.tr("Not available"))
                  : i18n.tr("Unlock")
            width: (scooterInfo.width -2*padding) / 2 - paddingMedium
            visible: scooterInfo.mode == "unlock"
            enabled: scooterInfo.scooter && (!scooterInfo.scooter.captive && scooterInfo.scooter.available) || false
            onClicked: onUnlock(scooterInfo.scooter)
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
