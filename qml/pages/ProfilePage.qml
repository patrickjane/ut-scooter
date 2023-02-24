import QtQuick 2.7
import Ubuntu.Components 1.3
import Ubuntu.Components.Themes 1.3
import QtQuick.Layouts 1.3
import Ubuntu.Components.ListItems 1.3

import "../util"

Page {
   property string provider
   property var profile

   id: profilePage
   anchors.fill: parent

   header: PageHeader {
      id: header
      title: HelperFunctions.capitalize(provider) + " " + i18n.tr("profile")
   }

   Flickable {
      anchors.top: header.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottom: parent.bottom

      contentWidth: parent.width
      contentHeight: childrenRect.height

      Column {
         anchors.left: parent.left
         anchors.right: parent.right
         anchors.top: parent.top

         Repeater {
            model: profile

            ListItem {
               height: layout1.height + (divider.visible ? divider.height : 0)

               SlotsLayout {
                  id: layout1
                  mainSlot: Label {
                     text: modelData.id
                  }
                  Label {
                     text: !!modelData.currency
                           ? HelperFunctions.formatCurrency(modelData)
                           : modelData.value
                     SlotsLayout.position: SlotsLayout.Trailing;
                  }
               }
            }
         }
      }
   }
}