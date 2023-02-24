import QtQuick 2.7
import Ubuntu.Components 1.3
import Ubuntu.Components.Themes 1.3
import QtQuick.Layouts 1.3
import Ubuntu.Components.ListItems 1.3

import "../util"

Page {
   property var positionSource
   property string provider
   property string account
   property string city
   property string country
   property var scooters

   id: settingsPageProvider
   anchors.fill: parent

   Component.onCompleted: {
      scooters.onAccountHistoryLoaded.connect(function(err, history) {
         if (err) {
            Notify.error(i18n.tr("Failed to load account history"), err)
            return
         }

         var items = []

         try {
            items = JSON.parse(history)
         } catch (e) {
            Notify.error(i18n.tr("Failed to load account history"), e)
            return
         }

         items.sort(function(a, b) {
            if (a.date < b.date) return 1;
            if (a.date > b.date) return -1;
            return 0;
         });

         pageStack.push(Qt.resolvedUrl("./AccountHistory.qml"), { provider: provider, history: items })
      })

      scooters.onProfile.connect(function(provider, profile) {
         console.log("profile loaded")

         if (profile)
            pageStack.push(Qt.resolvedUrl("./ProfilePage.qml"), { provider: provider, profile: profile })
      })

      scooters.onLoginStatusChanged.connect(function(providerName, loggedIn, account) {
         console.log("Login status changed", providerName, loggedIn, account)

         if (providerName === provider) {
            settingsPageProvider.account = !loggedIn ? "" : (account && account.length ? account : "")
         }
      })
   }

   header: PageHeader {
      id: header
      title: i18n.tr("Settings") + " "  + HelperFunctions.capitalize(provider)
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
            height: layout.height + (divider.visible ? divider.height : 0)
            ListItemLayout {
               id: layout
               title.text: i18n.tr("General")
               title.font.bold: true
            }
         }

         ListItem {
            height: layout4.height + (divider.visible ? divider.height : 0)
            SlotsLayout {
               id: layout4
               mainSlot: Label {
                  text: descriptionForProvider(provider)
                  wrapMode: Text.WordWrap
               }
            }
         }

         ListItem {
            height: layout3.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout3
               mainSlot: Label {
                  text: !!account ? (i18n.tr("Account") + ": " + account) : i18n.tr("Not logged in")
               }
               Button {
                  text: account && i18n.tr("Logout") || i18n.tr("Login")
                  SlotsLayout.position: SlotsLayout.Trailing;
                  onClicked: {
                     if (account) {
                        scooters.logout(provider)
                     } else {

                        var pageName = "./LoginPage.qml"

                        if (provider == "nextbike")
                           pageName = "./LoginPageNextbike.qml"

                        var dialog = pageStack.push(Qt.resolvedUrl(pageName), {
                                                      provider: provider,
                                                      scooters: scooters,
                                                      providerHasConfirmation: provider == "bird"
                                                   })
                     }
                  }
               }
            }
         }

         ListItem {
            id: profileRow
            property bool shouldEnable: !needsLocationFor("profile") || settingsPageProvider.positionSource.hasValidPosition()
            height: layout2.height + (divider.visible ? divider.height : 0)
            enabled: !!account && shouldEnable

            SlotsLayout {
               id: layout2
               mainSlot: Label {
                  text: i18n.tr("Profile")
                  + (profileRow.shouldEnable ? "" : (" " + i18n.tr("(GPS needed to show profile)")))
                  color: profileRow.shouldEnable ? "black" : "gray"
               }
               Icon {
                  name: "toolkit_chevron-ltr_3gu"
                  SlotsLayout.position: SlotsLayout.Trailing;
                  width: units.gu(2)
               }
            }

            onClicked: {
               scooters.getProfile(provider)
            }
         }

         ListItem {
            height: layout5.height + (divider.visible ? divider.height : 0)
            visible: provider === "nextbike"

            SlotsLayout {
               id: layout5
               mainSlot: Label {
                  text: i18n.tr("Account history")
               }
               Icon {
                  name: "toolkit_chevron-ltr_3gu"
                  SlotsLayout.position: SlotsLayout.Trailing;
                  width: units.gu(2)
               }
            }

            onClicked: {
               scooters.getAccountHistory(provider)
            }
         }
      }
   }

   function descriptionForProvider(provider) {
      switch (provider) {
         case "bird":
            return i18n.tr("Access to electric scooters provided by Bird (see https://www.bird.co). Existing account with associated payment information is needed. This app does not support creating accounts or changing payment information.")
         case "nextbike":
            return i18n.tr("Access to bikes provided by NextBike (see https://www.nextbike.de/de/). Existing account with associated payment information is needed. This app does not support creating accounts or changing payment information.")
      }

      return i18n.tr("No description available")
   }

   function needsLocationFor(what) {
      switch (settingsPageProvider.provider) {
      case "bird":
         if (what === "profile")
            return true
         break;

      default: break
      }

      return false
   }
}
