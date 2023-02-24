import QtQuick 2.7
import Ubuntu.Components 1.3
import Ubuntu.Components.Themes 1.3
import QtQuick.Layouts 1.3
import Ubuntu.Components.ListItems 1.3

import "../util"
import "../provider"
import "../notify"

Page {
   property string provider
   property var scooters
   property bool providerHasConfirmation
   property bool confirmNeeded: false
   readonly property var regExMail: /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/

   id: loginPage
   anchors.fill: parent

   header: PageHeader {
      id: header
      title: HelperFunctions.capitalize(provider) + " " + i18n.tr("login")
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

         ListItem {
            height: layout1.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout1
               mainSlot: Label {
                  text: i18n.tr("Phone number / PIN") + ":"
               }
            }
         }

         ListItem {
            height: layout2.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout2
               mainSlot: TextField {
                  id: loginField
                  placeholderText: i18n.tr("Enter phone number")
                  SlotsLayout.position: SlotsLayout.Trailing;
                  inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhDialableCharactersOnly
               }
            }
         }

         ListItem {
            height: layout4.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout4
               mainSlot: TextField {
                  id: pinField
                  placeholderText: i18n.tr("Enter PIN")
                  SlotsLayout.position: SlotsLayout.Trailing;
                  inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhDialableCharactersOnly
               }
               Button {
                  id: buttonConfirm
                  enabled: !!pinField.text && !!loginField.text
                  text: i18n.tr("Login")
                  SlotsLayout.position: SlotsLayout.Trailing;
                  onClicked: {
                     scooters.onLoginStatusChanged.connect(handleLoginChanged)
                     scooters.login(provider, loginField.text, pinField.text)
                  }
               }
            }
         }
      }
   }


   function handleLoginChanged(aProvider, loggedIn) {
      scooters.onLoginStatusChanged.disconnect(handleLoginChanged)

      if (loggedIn)
         Notify.info(HelperFunctions.capitalize(aProvider), i18n.tr("Login successful"))
      else
         Notify.error(HelperFunctions.capitalize(aProvider), i18n.tr("Login failed"))

      if (!loggedIn || aProvider != provider)
         return;

      pageStack.pop()
   }
}
