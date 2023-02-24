import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtLocation 5.12

MapQuickItem {
   property var myTerminal: modelData
   anchorPoint.x: image.width/4
   anchorPoint.y: image.height

   coordinate: myTerminal.coordinate

   sourceItem: Image {
      id: image
      source: "qrc:///graphics/station_marker_128.png"
      width: units.gu(5)
      height: units.gu(5)

      MouseArea {
         anchors.fill: parent
         onClicked: {
            if (mapPage.activeRide)
               return

            mapPage.selectedTerminal = myTerminal
         }
      }

      Rectangle {
         z: image.z+1
         width: scooterCount.width + units.gu(0.1)
         height: units.gu(1.5)
         anchors.right: parent.right
         anchors.top: parent.top
         anchors.topMargin: units.gu(0.2)
         anchors.rightMargin: units.gu(0.2)
         border.width: 1
         border.color: "#676767"
         radius: units.gu(0.5)

         color: myTerminal && (myTerminal.ridesAvailable > 0 ? "green" : "red") || "red"

         Text {
            id: scooterCount
            anchors.centerIn: parent
            text: myTerminal && (myTerminal.ridesAvailable + '') || ''
            color: "white"
            font.pixelSize: units.gu(1.5)
         }
      }

      Rectangle {
         visible: myTerminal && myTerminal.maintenance || false
         anchors.left: parent.left
         anchors.top: parent.top
         anchors.topMargin: units.gu(0.2)
         anchors.leftMargin: units.gu(0.2)
         width: units.gu(1)
         height: units.gu(1)
         radius: units.gu(1)
         color: "white"

         Image {
            source: "qrc:///graphics/tool_128.png"
            width: units.gu(1)
            height: units.gu(1)
            anchors.centerIn: parent
         }
      }
   }
}