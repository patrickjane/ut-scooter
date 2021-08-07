import QtQuick 2.0

Rectangle {
   property double batteryLevel: 0.7

   id: battery
   anchors.verticalCenter: parent.verticalCenter
   width: units.gu(3)
   height: units.gu(1.5)
   color: "transparent"

   Rectangle {
      id: batteryFrame
      color: "transparent"
      border.width: 1
      border.color: "#232323"

      anchors.left: parent.left
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      width: parent.width-5

      Rectangle {
         anchors.left: parent.left
         anchors.leftMargin: 1
         anchors.top: parent.top
         anchors.topMargin: 1
         anchors.bottom: parent.bottom
         anchors.bottomMargin: 1

         width: (parent.width-2) * batteryLevel
         height: battery.height
         color: batteryLevel > 0.5 ? "green" : (batteryLevel > 0.3 ? "orange" : "red")
      }
   }

   Rectangle {
      anchors.left: batteryFrame.right
      anchors.verticalCenter: parent.verticalCenter
      height: units.gu(0.5)
      width: units.gu(0.1)
      color: "#232323"
   }
}
