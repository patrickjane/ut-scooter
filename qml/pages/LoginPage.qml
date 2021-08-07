import QtQuick 2.7
import Ubuntu.Components 1.3
import Ubuntu.Components.Themes 1.3
import QtQuick.Layouts 1.3
import Ubuntu.Components.ListItems 1.3

import "../util"
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
                  text: i18n.tr("E-Mail") + ":"
               }
            }
         }

         ListItem {
            height: layout2.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout2
               mainSlot: TextField {
                  id: loginField
                  placeholderText: placeholderForProvider(provider)
                  SlotsLayout.position: SlotsLayout.Trailing;
                  inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhEmailCharactersOnly
               }
               Button {
                  id: buttonLogin
                  text: i18n.tr("Login")
                  enabled: loginField.text.length && validateInput(loginField.text) && !confirmNeeded

                  SlotsLayout.position: SlotsLayout.Trailing;
                  onClicked: {
                     scooters.onConfirmLoginNeeded.connect(handleConfirmNeeded)
                     scooters.login(provider, loginField.text)
                  }
               }
            }
         }

         ListItem {
            height: layout3.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout3
               mainSlot: Label {
                  text: i18n.tr("Confirmation code") + ":"
               }
            }
         }

         ListItem {
            visible: providerHasConfirmation
            height: layout4.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout4
               mainSlot: TextField {
                  id: confirmationField
                  enabled: confirmNeeded
                  placeholderText: i18n.tr("Enter confirmation")
                  SlotsLayout.position: SlotsLayout.Trailing;
                  inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
               }
               Button {
                  id: buttonConfirm
                  enabled: confirmationField.text.length && confirmationField.text
                  text: i18n.tr("Confirm")
                  SlotsLayout.position: SlotsLayout.Trailing;
                  onClicked: {
                     scooters.onLoginStatusChanged.connect(handleLoginChanged)
                     scooters.confirmLogin(provider, confirmationField.text)
                  }
               }
            }
         }
      }
   }

   function handleConfirmNeeded(aProvider) {
      scooters.onConfirmLoginNeeded.disconnect(handleConfirmNeeded)
      confirmNeeded = true
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

   function placeholderForProvider(provider) {
      if (provider === "bird")
         return i18n.tr("Enter email")

      return i18n.tr("Enter username/email")
   }

   function titleForProvider(provider) {
      if (provider === "bird")
         return i18n.tr("Email")

      return i18n.tr("Username/email")
   }

   function validateInput(input) {
      if (provider === "bird") {
         return input.match(regExMail)
      }

      return true;
   }
}
