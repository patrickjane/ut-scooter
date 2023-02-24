import QtQuick 2.0
import Ubuntu.Components 1.3

import "../util"

Rectangle {
   property var rideInfo
   property int padding: parent.width * 0.05
   property int paddingMedium: parent.width * 0.025

   id: rideSummary
   anchors.verticalCenter: parent.verticalCenter
   anchors.horizontalCenter: parent.horizontalCenter
   width: parent.width * 0.8
   height: childrenRect.height
   visible: !!rideInfo

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
         spacing: padding

         Image {
            width: units.gu(3)
            height: units.gu(3)
            source: rideSummary.rideInfo && (rideSummary.rideInfo.provider == "nextbike"
                  ? "qrc:///graphics/bike.png"
                  : "qrc:///graphics/scooter.png") || "qrc:///graphics/scooter.png"

            anchors.verticalCenter: parent.verticalCenter
         }

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(2)
            color: "#232323"
            text: rideInfo && HelperFunctions.capitalize(rideInfo.provider) || ""
         }
      }

      Text {
         anchors.horizontalCenter: parent.horizontalCenter
         font.pointSize: units.gu(1.5)
         color: "#232323"
         text: rideInfo && rideInfo.cancelledAt
               ? i18n.tr("Ride has been cancelled")
               : i18n.tr("Ride completed!")
      }

      Rectangle {
         color: "transparent"
         width: 50
         height: 15
      }

      Row {
         anchors.horizontalCenter: parent.horizontalCenter
         spacing: paddingMedium
         visible: !rideInfo || !rideInfo.cancelledAt

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(1.5)
            font.bold: true
            color: "#232323"
            text: i18n.tr("Duration") + ": "
         }

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(1.5)
            color: "#232323"
            text: rideInfo && rideInfo.duration || ""
         }
      }

      Row {
         anchors.horizontalCenter: parent.horizontalCenter
         spacing: paddingMedium
         visible: !rideInfo || !rideInfo.cancelledAt

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(1.5)
            font.bold: true
            color: "#232323"
            text: i18n.tr("Distance") + ": "
         }

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(1.5)
            color: "#232323"
            text: rideInfo && rideInfo.distance || ""
         }
      }

      Row {
         anchors.horizontalCenter: parent.horizontalCenter
         spacing: paddingMedium
         visible: !rideInfo || !rideInfo.cancelledAt

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(1.5)
            font.bold: true
            color: "#232323"
            text: i18n.tr("Cost") + ": "
         }

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(1.5)
            font.strikeout: true
            color: "#232323"
            text: rideInfo && rideInfo.costRegular || ""
            visible: rideInfo && rideInfo.costRegular || false
         }

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(1.5)
            color: "#232323"
            text: rideInfo
                  && (rideInfo.provider == "nextbike" ? HelperFunctions.formatCurrency(rideInfo) : rideInfo.cost)
                  || ""
         }
      }

      Text {
         anchors.horizontalCenter: parent.horizontalCenter
         font.pointSize: units.gu(1.5)
         color: "#232323"
         width: parent.width
         wrapMode: Text.WordWrap
         text: rideInfo && rideInfo.hint || ""
         visible: rideInfo && rideInfo.hint || false
      }

      Button {
         anchors.horizontalCenter: parent.horizontalCenter
         color: "black"
         text: i18n.tr("Close")
         width: (scooterInfo.width -2*padding) / 2 - paddingMedium
         onClicked: {
            rideSummary.rideInfo = null
         }
      }

      Rectangle {
         id: spacer
         color: "transparent"
         width: parent.width
         height: padding
      }
   }
}
