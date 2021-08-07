import QtQuick 2.0
import Ubuntu.Components 1.3
import Qt.labs.settings 1.0

import "../util"

Rectangle {
   property int padding: parent.width * 0.05
   property int paddingMedium: units.gu(2)

   Settings {
      id: settings
      property bool couponWarningAccepted: false
   }

   id: appCouponWarning
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
            width: units.gu(3)
            height: units.gu(3)
            source: "qrc:///graphics/scooter.png"
            anchors.verticalCenter: parent.verticalCenter
         }

         Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: units.gu(2)
            color: "#232323"
            text: "Bird"
         }
      }

      Text {
         anchors.horizontalCenter: parent.horizontalCenter
         font.pointSize: units.gu(1.5)
         color: "#232323"
         text: i18n.tr("Coupons")
      }

      Text {
         anchors.horizontalCenter: parent.horizontalCenter
         font.pointSize: units.gu(1)
         wrapMode: Text.WordWrap
         width: parent.width

         color: "#232323"
         text: i18n.tr("Due to a recent change in the Bird API it is no longer possible to use coupons/ride passes which have been purchased using the official apps. This means, that even though unlocking should be free, using this app will still charge the unlock price.")
      }

      Button {
         anchors.horizontalCenter: parent.horizontalCenter

         color: "black"
         text: i18n.tr("Accept")
         width: (scooterInfo.width -2*padding) / 2 - paddingMedium
         onClicked: {
            settings.couponWarningAccepted = true
            appCouponWarning.visible = false
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
