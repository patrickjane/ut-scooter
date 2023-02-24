import QtQuick 2.0
import Ubuntu.Components 1.3

import "../util"

Rectangle {
   property int padding: parent.width * 0.05
   property int paddingMedium: units.gu(2)

   property var terminal
   property string mode: "unlock"
   property var onRing
   property var onMissing
   property var onUnlock

   id: terminalInfo
   anchors.verticalCenter: parent.verticalCenter
   anchors.horizontalCenter: parent.horizontalCenter
   width: parent.width * 0.8
   height: childrenRect.height

   visible: !!terminal
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
            source: "qrc:///graphics/bike_marker_128.png"
            anchors.verticalCenter: parent.verticalCenter
         }

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(2)
            color: "#232323"
            text: terminalInfo.terminal && terminalInfo.terminal.provider
               && (HelperFunctions.capitalize(terminalInfo.terminal.provider) + " " + i18n.tr("station"))
               || ""
         }
      }

      Row {
         spacing: paddingMedium
         anchors.horizontalCenter: parent.horizontalCenter

         Image {
            visible: terminalInfo.terminal && terminalInfo.terminal.maintenance || false
            width: units.gu(3)
            height: units.gu(3)
            source: "qrc:///graphics/tool_128.png"
            anchors.verticalCenter: parent.verticalCenter
         }

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(1)
            color: "#232323"
            text: terminalInfo.terminal && terminalInfo.terminal.name || ""
         }
      }

      Text {
         anchors.horizontalCenter: parent.horizontalCenter

         font.pointSize: units.gu(1)
         wrapMode: Text.WordWrap
         width: parent.width

         color: "#232323"
         text: terminalInfo.terminal &&
            (i18n.tr("Available bikes") + ": " + terminalInfo.terminal.ridesAvailable + " / " + terminalInfo.terminal.ridesTotal)
            || ""
      }

      Text {
         anchors.horizontalCenter: parent.horizontalCenter

         font.pointSize: units.gu(1)
         wrapMode: Text.WordWrap
         width: parent.width

         color: "#232323"
         text: terminalInfo.terminal
            && (i18n.tr("Available racks") + ": " + terminalInfo.terminal.racksAvailable + " / " + terminalInfo.terminal.racksTotal)
            || ""
      }

      // Row {
      //    spacing: paddingMedium
      //    anchors.horizontalCenter: parent.horizontalCenter

      //    Button {
      //       color: "black"
      //       text: i18n.tr("Ring")
      //       width: (terminalInfo.width -2*padding) / 2 - paddingMedium
      //       visible: terminalInfo.mode == "show"
      //       onClicked: onRing(terminalInfo.scooter)
      //    }
      //    Button {
      //       color: "black"
      //       text: terminalInfo.scooter && (!terminalInfo.scooter.available || terminalInfo.scooter.captive)
      //             ? (terminalInfo.scooter.captive ? i18n.tr("Captive") : i18n.tr("Not available"))
      //             : i18n.tr("Unlock")
      //       width: (terminalInfo.width -2*padding) / 2 - paddingMedium
      //       visible: terminalInfo.mode == "unlock"
      //       enabled: terminalInfo.scooter && (!terminalInfo.scooter.captive && terminalInfo.scooter.available) || false
      //       onClicked: onUnlock(terminalInfo.scooter)
      //    }
      // }

      Rectangle {
         id: spacer
         color: "transparent"
         width: parent.width
         height: units.gu(2)
      }
   }
}
