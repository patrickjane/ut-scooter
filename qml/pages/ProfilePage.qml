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

         ListItem {
            height: layout1.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout1
               mainSlot: Label {
                  text: i18n.tr("E-Mail")
               }
               Label {
                  text: profile.email
                  SlotsLayout.position: SlotsLayout.Trailing;
               }
            }
         }
         ListItem {
            height: layout2.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout2
               mainSlot: Label {
                  text: i18n.tr("Registered")
               }
               Label {
                  text: new Date(profile.created_at).toLocaleString(Qt.locale(profile.locale), Locale.ShortFormat)
                  SlotsLayout.position: SlotsLayout.Trailing;
               }
            }
         }
         ListItem {
            height: layout3.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout3
               mainSlot: Label {
                  text: i18n.tr("Last ride")
               }
               Label {
                  text: new Date(profile.last_ride_at).toLocaleString(Qt.locale(profile.locale), Locale.ShortFormat)
                  SlotsLayout.position: SlotsLayout.Trailing;
               }
            }
         }
         ListItem {
            height: layout4.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout4
               mainSlot: Label {
                  text: i18n.tr("Ride count")
               }
               Label {
                  text: profile.ride_count
                  SlotsLayout.position: SlotsLayout.Trailing;
               }
            }
         }
         ListItem {
            height: layout5.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout5
               mainSlot: Label {
                  text: i18n.tr("Free rides")
               }
               Label {
                  text: profile.free_rides
                  SlotsLayout.position: SlotsLayout.Trailing;
               }
            }
         }
         ListItem {
            height: layout6.height + (divider.visible ? divider.height : 0)

            SlotsLayout {
               id: layout6
               mainSlot: Label {
                  text: i18n.tr("Balance")
               }
               Label {
                  text: HelperFunctions.formatCurrency(profile.locale, profile.balances)
                  SlotsLayout.position: SlotsLayout.Trailing;
               }
            }
         }
      }
   }
}