import QtQuick 2.7
import Ubuntu.Components 1.3
import Ubuntu.Components.Themes 1.3
import QtQuick.Layouts 1.3
import Ubuntu.Components.ListItems 1.3
import Qt.labs.settings 1.0

import "../util"

Page {
   id: listSelectorPage
   anchors.fill: parent

   property var title
   property var items

   signal itemSelected(var index, var value)

   header: PageHeader {
      id: header
      title: listSelectorPage.title
   }

   Flickable {
      anchors.top: header.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottom: parent.bottom

      contentWidth: parent.width
      contentHeight: rows.height

      Column {
         id: rows
         anchors.left: parent.left
         anchors.right: parent.right
         anchors.top: parent.top

         Repeater {
            model: items

            ListItem {
               height: layout.height + (divider.visible ? divider.height : 0)
               ListItemLayout {
                  id: layout
                  title.text: modelData
               }

               onClicked: {
                  pageStack.pop()
                  emit: itemSelected(index, modelData)
               }
            }
         }
      }
   }
}
