import Ubuntu.Components 1.3
import Ubuntu.Components 1.3 as UT
import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtLocation 5.12

import "../map"

MapItemView {
   property string viewName: "areaView"
   model: []
   visible: false
   delegate: Component {
      MapPolygon {
         property var myArea: modelData
         color: myArea.color || "red"
         opacity: myArea.color ? 0.3 : 0.4
         border.width: 2
         border.color: "#676767"

         path: myArea.path
      }
   }
}