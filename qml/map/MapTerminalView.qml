import Ubuntu.Components 1.3
import Ubuntu.Components 1.3 as UT
import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtLocation 5.12

import "../map"

MapItemView {
   z: 100
   property string viewName: "terminalView"
   model: []
   visible: true
   delegate: Component {
      MapTerminalNextbike {}
   }
}