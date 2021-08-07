import Ubuntu.Components 1.3
import Ubuntu.Components 1.3 as UT
import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtLocation 5.12
import QtPositioning 5.12
import Qt.labs.settings 1.0

import Scooter 1.0
import MapboxMap 1.0

import "../util"
import "../notify"

Item {
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

   MapboxMap {
      id: map
      anchors.fill: parent
      zoomLevel: 16.0
      minimumZoomLevel: 10
      maximumZoomLevel: 20
      pixelRatio: units.gridUnit / 8.0 * 1.5
      accessToken: settings.accessToken || "invalidkey"
      cacheDatabaseDefaultPath: true
      cacheDatabaseStoreSettings: false
      styleUrl: "mapbox://styles/mapbox/outdoors-v10"
      center: QtPositioning.coordinate(settings.lastLatitude, settings.lastLongitude)

      Component.onCompleted: {
         initLayers();

         if (positionSource.position.latitudeValid && positionSource.position.longitudeValid)
            center = positionSource.position.coordinate
         else
            center = QtPositioning.coordinate(settings.lastLatitude, settings.lastLongitude)
      }

      MapboxMapGestureArea {
         id: area
         map: map
         anchors.fill: parent
         activeClickedGeo: true

         onClicked: {
            mapPage.selectedScooter = null
         }

         onClickedGeo: {
            var selectedScooter = null;

            if (mapPage.activeRide)
               return

            scooters.scooters.forEach(function(scooter) {
               var dlon = (geocoordinate.longitude - scooter.coordinate.longitude) / degLonPerPixel;
               var dlat = (geocoordinate.latitude - scooter.coordinate.latitude) / degLatPerPixel;
               var dist2 = dlon*dlon + dlat*dlat;

               if (dist2 < map.pixelRatio*map.pixelRatio*900 && (!selectedScooter || selectedScooter.dist2 > dist2)) {
                  selectedScooter = { 'scooter': scooter, 'dist2': dist2 };
               }
            });

            if (selectedScooter) {
               mapPage.selectedScooter = selectedScooter.scooter
               scooterInfo.mode = "show"
            }
         }
      }
   }

   PositionSource {
      id: positionSource
      updateInterval: 3000
      active: true

      property double lastUpdate: -1
      property var lastScooterReloadCoordinate: undefined
      property var lastAreaReloadCoordinate: undefined
      property var lastCoordinate: undefined
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

         updateLocation(positionSource.position.coordinate, accuracy || 400)

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
            map.zoomLevel = 16
         }
      }
   }

   Component.onCompleted: {
      if (!settings.accessToken || !settings.accessToken.length)
         openSettings.start()
   }

   Timer {
      id: openSettings
      interval: 200
      onTriggered: pageStack.push(Qt.resolvedUrl("./SettingsPage.qml"), { scooters: scooters })
   }

   function onScooterClicked(scooter) {
      scooterInfo.mode = "show"
      mapPage.selectedScooter = scooter
   }

   function updateScooters(scooters) {
      var coords = scooters.map(function(c) { return c.coordinate })
      var names = scooters.map(function(c) { return HelperFunctions.capitalize(c.provider) })

      map.updateSourcePoints("scooter-source", coords, names);
   }

   function updateLocation(coord, accuracy) {
      map.updateSource("location",
                       {"type": "geojson",
                          "data": {
                             "type": "Feature",
                             "geometry": {
                                "type": "Point",
                                "coordinates": [coord.longitude, coord.latitude]
                             }
                          }
                       })
   }

   function updateAreas(provider, areas) {
      var coordinates = []
      var what = areas || []

      what.forEach(function(newArea) {
         var coords = newArea.polygon.map(function(p) { return p.coordinate })
         coordinates.push(coords)
      })

      map.updateSource("no-parking-zone", { 'type': 'geojson', 'data': { 'type': 'Feature', 'geometry': { 'type': 'Polygon', 'coordinates': coordinates }}});
   }

   function initLayers(lon, lat) {

      // position

      map.addSource("location", {"type": "geojson", "data": { "type": "Feature", "geometry": { "type": "Point", "coordinates": [8.6841700, 50.1155200] }}})
      map.addLayer("location-uncertainty", {"type": "circle", "source": "location"}, "waterway-label")
      map.setPaintProperty("location-uncertainty", "circle-radius", 10)
      map.setPaintProperty("location-uncertainty", "circle-color", "#87cefa")
      map.setPaintProperty("location-uncertainty", "circle-opacity", 0.25)

      map.addLayer("location-case", {"type": "circle", "source": "location"}, "waterway-label")
      map.setPaintProperty("location-case", "circle-radius", 7)
      map.setPaintProperty("location-case", "circle-color", "white")
      map.addLayer("location", {"type": "circle", "source": "location"}, "waterway-label")
      map.setPaintProperty("location", "circle-radius", 3)
      map.setPaintProperty("location", "circle-color", "blue")

      // scooter POIs

      map.addSourcePoints("scooter-source", []);
      map.addImagePath("scooter-image", ":/graphics/scooter_marker_128.png");
      map.addLayer("scooter-layer", {"type": "symbol", "source": "scooter-source"}, "waterway-label");
      map.setLayoutProperty("scooter-layer", "icon-allow-overlap", true);
      map.setLayoutProperty("scooter-layer", "icon-anchor", "bottom");
      map.setLayoutProperty("scooter-layer", "icon-image", "scooter-image");
      map.setLayoutProperty("scooter-layer", "icon-size", 0.25);
      map.setLayoutProperty("scooter-layer", "text-anchor", "top");
      map.setLayoutProperty("scooter-layer", "text-optional", true);
      map.setLayoutProperty("scooter-layer", "text-size", 10);
      map.setLayoutProperty("scooter-layer", "text-field", "{name}");

      // no-parking zones (currently NOT provider-dependent)

      map.addSource('no-parking-zone', { 'type': 'geojson', 'data': { 'type': 'Feature', 'geometry': { 'type': 'Polygon', 'coordinates': [] }}});
      map.addLayer('no-parking-zone', {"type": "fill", "source": 'no-parking-zone'}, "scooter-layer")
      map.setPaintProperty('no-parking-zone', "fill-color", '#FF0000')
      map.setPaintProperty('no-parking-zone', "fill-opacity", 0.5)
   }
}
