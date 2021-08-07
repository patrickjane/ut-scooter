import Ubuntu.Components 1.3
import Ubuntu.Components 1.3 as UT
import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtLocation 5.12
import QtPositioning 5.12
import Qt.labs.settings 1.0

import Scooter 1.0

import "../util"
import "../notify"

Item {
   property var areaList: []
   property var scooterList: []
   property var activeRide: scooters.activeRide
   property var selectedScooter
   property var logs: []

   property int padding: parent.width * 0.1
   property int buttonSize: units.gu(5)

   id: mapPage
   anchors.fill: parent

   Settings {
      id: settings
      property bool disclaimerAccepted: false
      property bool couponWarningAccepted: false
      property string accessToken
      property double lastLatitude: 50.1155200
      property double lastLongitude: 8.6841700
   }

   Plugin {
      id: plugin
      name: "osm"
   }

   Map {
      id: map
      anchors.fill: parent
      plugin: plugin
      center: QtPositioning.coordinate(settings.lastLatitude, settings.lastLongitude)
      zoomLevel: 18

      property var currentPosition: positionSource.position.latitudeValid
                                    && positionSource.position.longitudeValid
                                    && positionSource.position.coordinate || QtPositioning.coordinate()

      MouseArea {
         anchors.fill: parent
         onClicked: {
            mapPage.selectedScooter = null
         }
      }

      MapCircle {
         z: 10
         center: map.currentPosition
         radius: 4
         color: 'blue'
         border.width: 0
      }

      MapCircle {
         z: 5
         center: map.currentPosition
         radius: 8
         color: 'white'
         border.width: 0
      }

      MapCircle {
         center: map.currentPosition
         radius: 15
         color: 'blue'
         border.width: 0
         opacity: 0.1
      }

      MapItemView {
         model: scooterList
         delegate: Component {
            MapQuickItem {
               property var myScooter: modelData
               anchorPoint.x: image.width/4
               anchorPoint.y: image.height

               coordinate: myScooter.coordinate

               sourceItem: Image {
                  id: image
                  source: "qrc:///graphics/scooter_marker_128.png"
                  width: units.gu(4)
                  height: units.gu(4)

                  MouseArea {
                     anchors.fill: parent
                     onClicked: {
                        mapPage.selectedScooter = myScooter
                        scooterInfo.mode = "show"
                     }
                  }
               }
            }
         }
      }

      MapItemView {
         model: areaList
         delegate: Component {
            MapPolygon {
               property var myArea: modelData
               color: "red"
               opacity: 0.4
               border.width: 2
               border.color: "#676767"

               path: modelData
            }
         }
      }
   }

   PositionSource {
      id: positionSource
      updateInterval: 3000
      active: true

      property double lastUpdate: -1
      property var lastCoordinate: undefined
      property var lastScooterReloadCoordinate: undefined
      property var lastAreaReloadCoordinate: undefined
      property int accuracy: position.verticalAccuracyValid && position.horizontalAccuracyValid && (Math.round(Math.max(position.verticalAccuracy, position.horizontalAccuracy))) || 0

      onPositionChanged: {
         if (!positionSource.position.latitudeValid || !positionSource.position.longitudeValid)
            return

         var now = new Date().getTime()

         if (lastUpdate !== -1 && (new Date().getTime()) - lastUpdate < 3000)
            return

         lastUpdate = now
         settings.lastLatitude = positionSource.position.coordinate.latitude
         settings.lastLongitude = positionSource.position.coordinate.longitude

         var distance = lastCoordinate && lastCoordinate.distanceTo(position.coordinate) || -1

         if (accuracy > 20 || lastCoordinate === undefined || !!mapPage.activeRide) {
            lastCoordinate = QtPositioning.coordinate(positionSource.position.coordinate.latitude, positionSource.position.coordinate.longitude)
            map.center = position.coordinate
         }

         var reloadDistanceScooter = lastScooterReloadCoordinate && lastScooterReloadCoordinate.distanceTo(position.coordinate) || -1

         if (scooters.ready || reloadDistanceScooter >= 100 || lastScooterReloadCoordinate === undefined) {
            scooters.reloadScooters(positionSource.position.coordinate, 300)
            lastScooterReloadCoordinate = QtPositioning.coordinate(positionSource.position.coordinate.latitude, positionSource.position.coordinate.longitude)
         }

         var reloadDistanceAreas = lastAreaReloadCoordinate && lastAreaReloadCoordinate.distanceTo(position.coordinate) || -1

         if (scooters.ready || reloadDistanceAreas >= 300 || lastAreaReloadCoordinate === undefined) {
            scooters.reloadAreas(positionSource.position.coordinate, 300)
            lastAreaReloadCoordinate = QtPositioning.coordinate(positionSource.position.coordinate.latitude, positionSource.position.coordinate.longitude)
         }

         scooters.ready = false
      }

      Component.onCompleted: {
         if (positionSource.position.latitudeValid && positionSource.position.longitudeValid)
            lastCoordinate = QtPositioning.coordinate(positionSource.position.coordinate.latitude, positionSource.position.coordinate.longitude)
      }
   }

   Disclaimer {
      id: disclaimer
      visible: !settings.disclaimerAccepted
   }

   CouponWarning {
      visible: !disclaimer.visible && !settings.couponWarningAccepted
   }

   ActiveRideInfo {
      id: activeRideInfo
      ride: mapPage.activeRide

      onEndRide: function() {
         scooters.stopRide(positionSource.position.coordinate)
      }
   }

   ScooterInfo {
      id: scooterInfo
      scooter: mapPage.selectedScooter

      onRing: function(scooter) {
         scooters.ringScooter(scooter.provider, scooter.mapId, positionSource.position.coordinate)
      }

      onUnlock: function(scooter) {
         scooters.startRide(scooter.provider, scooter.unlockId, positionSource.position.coordinate)
      }
   }

   RideSummary {
      id: rideSummary
   }

   Scooters {
      id: scooters
      property bool ready: false

      onScootersChanged: updateScooters(scooters)
      onAreasChanged: updateAreas(provider, areas)
      onNetworkError: Notify.error(i18n.tr("Network error"), error)
      onError: Notify.error(HelperFunctions.capitalize(providerName) + " " + i18n.tr("error"), error)
      onNotify: Notify.info(title + " (" + HelperFunctions.capitalize(providerName) + ")", message)
      onReady: ready = true

      onActiveRideChanged: {
         mapPage.activeRide = ride
         activeRideInfo.reset()

         if (ride) {
            mapPage.selectedScooter = null
         } else {
            if (!error || !error.length)
               Notify.info(i18n.tr("Ride"), i18n.tr("Ride stopped"))
            else
               Notify.error(i18n.tr("Ride") + " " + i18n.tr("error"), error)

            if (rideInfo.length) {
               var rideInfoObj;

               try {
                  rideInfoObj = JSON.parse(rideInfo)
               } catch (e) {
                  console.log("Failed to parse rideInfo JSON", e)
               }

               rideSummary.rideInfo = rideInfoObj
            }
         }
      }

      onScooterScanned: {
         if (!scooter)
            return

         mapPage.selectedScooter = scooter
         scooterInfo.mode = "unlock"
      }

      onLogMessage2: mapPage.logs.push({ time: new Date(), provider: provider, severity: severity, message: message })
   }

   Rectangle {
      anchors.top: parent.top
      anchors.topMargin: padding
      anchors.left: parent.left
      anchors.leftMargin: padding
      color: "transparent"
      visible: !positionSource.position.latitudeValid || !positionSource.position.longitudeValid

      width: units.gu(5)
      height: units.gu(6)

      UT.Icon {
         width: units.gu(5)
         height: units.gu(5)
         anchors.top: parent.top
         anchors.horizontalCenter: parent.horizontalCenter
         name: "gps"
      }

      Text {
         anchors.horizontalCenter: parent.horizontalCenter
         anchors.bottom: parent.bottom
         text: "No GPS signal"
         font.pointSize: 12
      }
   }

   Button {
      id: menuButton
      width: buttonSize
      height: buttonSize
      color: "white"

      anchors.top: parent.top
      anchors.topMargin: padding
      anchors.right: parent.right
      anchors.rightMargin: padding

      iconName: "navigation-menu"
      onClicked: {
         pageStack.push(Qt.resolvedUrl("./SettingsPage.qml"), { scooters: scooters, logs: logs })
      }
   }

   Button {
      width: buttonSize
      height: buttonSize
      color: "white"

      anchors.horizontalCenter: parent.horizontalCenter
      anchors.bottom: parent.bottom
      anchors.bottomMargin: padding

      iconSource: "qrc:///graphics/qrcode_bk.png"
      onClicked: {
         pageStack.push(Qt.resolvedUrl("./ScanPage.qml"), { scooters: scooters, positionSource: positionSource })
      }
   }

   Button {
      width: buttonSize
      height: buttonSize
      color: "white"

      anchors.left: parent.left
      anchors.leftMargin: padding
      anchors.bottom: parent.bottom
      anchors.bottomMargin: padding

      iconName: "location-active"
      onClicked: {
         if (positionSource.position.latitudeValid && positionSource.position.longitudeValid) {
            map.center = positionSource.position.coordinate
            map.zoomLevel = 18
         }
      }
   }

   function onScooterClicked(scooter) {
      scooterInfo.mode = "show"
      mapPage.selectedScooter = scooter
   }

   function updateScooters(scooters) {
      mapPage.scooterList = scooters
   }

   function updateAreas(provider, areas) {
      var a = areas || []
      var res = []

      mapPage.areaList = []

      a.forEach(function(area) {
         try {
            var path = area.polygon.map(function(poly) {
               return { latitude: poly.coordinate[1], longitude: poly.coordinate[0] }
            })
         } catch (e) {}

         res.push(path)
      })

      mapPage.areaList = res
   }
}
