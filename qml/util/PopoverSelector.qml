import Ubuntu.Components 1.3
import Ubuntu.Components 1.3 as UT
import QtQuick 2.7
import QtQuick.Layouts 1.3

Rectangle {
   id: popoverSelector
   width: parent.width*0.7
   height: popoverSelectorFlickable.height
   radius: units.gu(1)

   signal itemChanged(var name, var object, var checked)

   property double maxHeight: parent.height*0.6
   property var model

   color: "white"

   Flickable {
      id: popoverSelectorFlickable
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right

      height: Math.min(popoverSelectorFlickableColumn.height, parent.maxHeight)

      contentWidth: parent.width
      contentHeight: popoverSelectorFlickableColumn.height
      clip: true

      Column {
         id: popoverSelectorFlickableColumn
         anchors.left: parent.left
         anchors.right: parent.right
         anchors.top: parent.top

         Repeater {
            model: popoverSelector.model

            ListItem {
               enabled: model.enabled
               property bool checked: model.checked
               height: popoverSelectorSlotLayout.height + (divider.visible ? divider.height : 0)

               SlotsLayout {
                  id: popoverSelectorSlotLayout
                  mainSlot: Item {
                     Label {
                        id: label
                        anchors.verticalCenter: parent.verticalCenter
                        text: HelperFunctions.capitalize(model.name)
                     }
                     Text {
                        visible: !!model.subtitle
                        anchors.top: label.bottom
                        anchors.topMargin: units.gu(0.1)
                        text: model.subtitle
                        color: "darkgrey"
                        font.pointSize: units.gu(0.7)
                     }
                  }
                  Rectangle {
                     width: units.gu(2)
                     height: popoverSelectorSlotLayoutIcon.height
                     SlotsLayout.position: SlotsLayout.Leading;
                     color: "transparent"

                     UT.Icon {
                        id: popoverSelectorSlotLayoutIcon
                        width: units.gu(2)
                        anchors.centerIn: parent

                        visible: model.checked
                        name: "toolkit_tick"
                     }
                  }
               }

               onClicked: {
                  model.checked = !model.checked
                  emit: itemChanged(model.name, model.object, model.checked)
               }
            }
         }
      }
   }
}