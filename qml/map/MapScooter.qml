import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtLocation 5.12

MapQuickItem {
   property var myScooter: modelData
   anchorPoint.x: image.width/4
   anchorPoint.y: image.height

   coordinate: myScooter.coordinate

   sourceItem: Image {
      id: image
      source: myScooter && (myScooter.provider == "nextbike"
               ? "qrc:///graphics/bike_marker_128.png"
               : "qrc:///graphics/scooter_marker_128.png") || "qrc:///graphics/scooter_marker_128.png"
      width: units.gu(4)
      height: units.gu(4)

      MouseArea {
         anchors.fill: parent
         onClicked: {
            if (mapPage.activeRide || !myScooter || myScooter.provider === "nextbike")
               return

            mapPage.selectedScooter = myScooter
            scooterInfo.mode = "show"
         }
      }
   }
}