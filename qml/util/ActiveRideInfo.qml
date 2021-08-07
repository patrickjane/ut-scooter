import QtQuick 2.0
import Ubuntu.Components 1.3

import "../util"

Rectangle {
   property int padding: parent.width * 0.05
   property bool needConfirm: true
   property var onEndRide
   property var ride

   function reset() {
      needConfirm = true

      if (ride)
         textRideDuration.text = HelperFunctions.toElapsed((new Date().getTime() - ride.started.getTime())/1000)
      else
         textRideDuration.text = ""
   }

   id: activeRideInfo
   anchors.top: menuButton.bottom
   anchors.topMargin: padding
   anchors.horizontalCenter: parent.horizontalCenter
   width: parent.width * 0.8
   height: childrenRect.height
   visible: !!ride
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
            source: "qrc:///graphics/scooter.png"
            anchors.verticalCenter: parent.verticalCenter
         }

         Text {
            id: textRideProvider
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(2)
            color: "#232323"
            text: !ride ? "?" : HelperFunctions.capitalize(ride.provider)
         }

         Text {
            id: textRideDuration
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(2)
            color: "#232323"
            text: "00:00:00"
         }
      }

      Row {
         spacing: padding
         anchors.horizontalCenter: parent.horizontalCenter

         Button {
            color: "black"
            text: needConfirm
                  ? i18n.tr("End ride")
                  : i18n.tr("Confirm end")
            onClicked: {
               if (needConfirm){
                  needConfirm = false
                  timerResetEndRide.start()
               } else {
                  onEndRide()
               }
            }
         }
      }

      Rectangle {
         id: spacer
         color: "transparent"
         width: parent.width
         height: padding
      }
   }

   Timer {
      id: timerRideDisplay
      interval: 1000
      repeat: true
      running: !!ride

      onTriggered: {
         if (ride)
            textRideDuration.text = HelperFunctions.toElapsed((new Date().getTime() - ride.started.getTime())/1000)
         else
            textRideDuration.text = ""
      }
   }

   Timer {
      id: timerResetEndRide
      interval: 7000
      repeat: false
      running: false

      onTriggered: {
         needConfirm = true
      }
   }
}
