import Ubuntu.Components 1.3
import Ubuntu.Components 1.3 as UT
import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtLocation 5.12
import QtPositioning 5.12
import Qt.labs.settings 1.0

import Scooter 1.0

import "../map"
import "../util"
import "../notify"

Item {
   property var activeRide: scooters.activeRide
   property var selectedScooter
   property var selectedTerminal
   property var logs: []
   property var mapViews: ({})

   property double lastUpdate: -1
   property var lastCoordinate: undefined
   property var lastScooterReloadCoordinate: undefined
   property var lastAreaReloadCoordinate: undefined

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

   // *************************************************************************
   // Map related
   // *************************************************************************

   Plugin {
      id: plugin
      name: "osm"
   }

   Map {
      id: map
      anchors.fill: parent
      plugin: plugin
      center: QtPositioning.coordinate(settings.lastLatitude, settings.lastLongitude)
      zoomLevel: 17

      property var currentPosition: positionSource.position.latitudeValid
                                    && positionSource.position.longitudeValid
                                    && positionSource.position.coordinate || QtPositioning.coordinate()

      MouseArea {
         anchors.fill: parent
         onClicked: {
            mapPage.selectedScooter = null
            mapPage.selectedTerminal = null
            layersSelector.visible = false
         }
      }

      // own position marker

      MapQuickItem {
         coordinate: map.currentPosition
         anchorPoint: Qt.point(sourceItem.width/2, sourceItem.height/2)

         sourceItem: Rectangle {
            width: units.gu(2)
            height: units.gu(2)
            color: "blue"
            border.width: units.gu(0.5)
            border.color: "white"
            smooth: true
            radius: units.gu(2)
         }
      }

      // Scooters

      MapScooterView {
         property string viewName: "scooterViewBird"
      }

      MapScooterView {
         property string viewName: "scooterViewNextbike"
      }

      // terminals

      MapTerminalView {
         property string viewName: "terminalViewNextbike"
      }

      // areas

      MapAreaView {
         property string viewName: "areaViewBird"
      }

      MapAreaView {
         property string viewName: "areaViewNextbike"
      }

      Component.onCompleted: {
         initLayersTimer.start()
      }
   }

   Timer {
      id: initLayersTimer
      interval: 2000
      repeat: false
      running: false

      onTriggered: {
         var res = [];
         var allProviders = scooters.getSupportedProviders()

         for (var i = 0; i < map.children.length; i++)
         {
            var vn = map.children[i].viewName

            if (!vn)
               continue;

            mapViews[vn] = map.children[i]

            var providerName = vn.replace("scooterView", "").replace("terminalView", "").replace("areaView", "").toLowerCase()
            var elementName = vn.startsWith("scooter") ? i18n.tr("scooters") : (vn.startsWith("terminal") ? i18n.tr("terminals") : i18n.tr("zones"))
            var loggedIn = scooters.isLoggedIn(providerName)

            if (elementName === i18n.tr("scooters") && providerName === "nextbike")
               elementName = i18n.tr("bikes")

            if (allProviders.indexOf(providerName) === -1)
               continue;

            res.push({
               checked: vn.startsWith("scooter") || vn.startsWith("terminal"),
               name: HelperFunctions.capitalize(providerName + " " + elementName),
               object: vn,
               subtitle: loggedIn ? "" : i18n.tr("not logged in"),
               enabled: loggedIn,
               provider: providerName
            })
         }

         res.sort(function(a, b) {
            if (a.name < b.name) {
               return -1;
            }
            if (a.name > b.name) {
               return 1;
            }
            return 0;
         });

         res.forEach(function(i) { layersModel.append(i); })

         scooters.init()
         positionSource.start()
      }
   }

   ListModel {
      id: layersModel
   }

   // *************************************************************************
   // GPS / positioning
   // *************************************************************************

   // Timer {
   //    id: fakePositionTimer
   //    interval: 3000
   //    repeat: false
   //    running: true
   //    property int i: 0

   //    onTriggered: {
   //       console.log("TIMER")
   //       var coords = [QtPositioning.coordinate(50.113898, 8.681466), QtPositioning.coordinate(50.112649, 8.686787)]
   //       onPositionUpdate({ latitudeValid: true, longitudeValid: true, coordinate: coords[(i++) % 2] }, 23)
   //    }
   // }

   PositionSource {
      id: positionSource
      updateInterval: 3000
      active: false

      onPositionChanged: {
         onPositionUpdate(positionSource.position, 0)
      }

      function hasValidPosition() {
         return positionSource.position.latitudeValid && positionSource.position.longitudeValid
      }
   }

   // *************************************************************************
   // GUI elements / widgets
   // *************************************************************************

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
         if (mapPage.activeRide && mapPage.activeRide.provider == "nextbike") {
            scooters.checkActiveRide(false);
            timerCheckActiveRide.stop()
         } else {
            scooters.stopRide(positionSource.position.coordinate)
         }
      }

      onReUnlock: function() {
         if (mapPage.activeRide)
            scooters.startRide(mapPage.activeRide.provider, mapPage.activeRide.scooterId, positionSource.position.coordinate, true);
      }
   }

   ScooterInfo {
      id: scooterInfo
      scooter: mapPage.selectedScooter

      onRing: function(scooter) {
         scooters.ringScooter(scooter.provider, scooter.mapId, positionSource.position.coordinate)
      }

      onUnlock: function(scooter) {
         scooters.startRide(scooter.provider, scooter.unlockId, positionSource.position.coordinate, false)
      }
   }

   NextBikeTerminalInfo {
      id: nextBikeTerminalInfo
      terminal: mapPage.selectedTerminal
   }

   RideSummary {
      id: rideSummary
   }

   // *************************************************************************
   // Backend connection
   // *************************************************************************

   Scooters {
      id: scooters
      onScootersChanged: updateScooters(provider, scooters)
      onAreasChanged: updateAreas(provider, areas)
      onTerminalsChanged: updateTerminals(provider, terminals)
      onNetworkError: Notify.error(i18n.tr("Network error"), error)
      onError: Notify.error(HelperFunctions.capitalize(providerName) + " " + i18n.tr("error"), error)
      onNotify: Notify.info(title + " (" + HelperFunctions.capitalize(providerName) + ")", message)

      onActiveRideChanged: {
         if (ride) {
            mapPage.activeRide = ride
            activeRideInfo.reset()

            mapPage.selectedScooter = null

            if (ride.provider === "nextbike")
               timerCheckActiveRide.start()

            adjustMapViewsForActiveRide(mapPage.activeRide.provider, true)
         } else {
            if (!error || !error.length)
               Notify.info(i18n.tr("Ride"), i18n.tr("Ride stopped"))
            else {
               Notify.error(i18n.tr("Ride") + " " + i18n.tr("error"), error)
            }

            if (rideInfo.length) {
               var rideInfoObj;

               adjustMapViewsForActiveRide(mapPage.activeRide.provider, false)
               mapPage.activeRide = null;

               try {
                  rideInfoObj = JSON.parse(rideInfo)
               } catch (e) {
                  console.log("Failed to parse rideInfo JSON", e)
               }

               rideSummary.rideInfo = rideInfoObj
            }
         }
      }

      onRideChecked: {
         if (!error && rideDetails) {
            Notify.info(i18n.tr("Ride"), i18n.tr("Ride stopped"))
            adjustMapViewsForActiveRide(mapPage.activeRide.provider, false)
            mapPage.activeRide = null
            activeRideInfo.reset()
            timerCheckActiveRide.stop()

            var rideInfoObj = null;

            try {
               rideInfoObj = JSON.parse(rideDetails)
            } catch (e) {
               console.log("Failed to parse rideInfo JSON", e)
            }

            rideSummary.rideInfo = rideInfoObj
         } else if (error) {
            Notify.error(i18n.tr("Ride") + " " + i18n.tr("error"), error)
            timerCheckActiveRide.stop()
         } else {
            console.log("Interval active ride check without result")
            timerCheckActiveRide.start()
         }
      }

      onScooterScanned: {
         if (!scooter)
            return

         mapPage.selectedScooter = scooter
         scooterInfo.mode = "unlock"
      }

      onLogMessage2: mapPage.logs.push({ time: new Date(), provider: provider, severity: severity, message: message })

      onLoginStatusChanged: {
         for (var i = 0; i < layersModel.count; i++) {
            var item = layersModel.get(i)

            if (item.provider === providerName) {
               layersModel.setProperty(i, "enabled", loggedIn)
               layersModel.setProperty(i, "subtitle", loggedIn ? "" : i18n.tr("not logged in"))

               var view = mapViews[item.object]

               if (view) {
                  view.visible = item.enabled && item.checked

                  if (!loggedIn)
                     view.model = []
               }
            }
         }
      }
   }

   Timer {
      id: timerCheckActiveRide
      interval: 15000
      repeat: false
      running: false

      onTriggered: {
         scooters.checkActiveRide(true);
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
         pageStack.push(Qt.resolvedUrl("./SettingsPage.qml"), {
            scooters: scooters,
            positionSource: positionSource,
            logs: logs
         })
      }
   }

   Button {
      id: layersButton
      width: buttonSize
      height: buttonSize
      color: "white"

      anchors.right: parent.right
      anchors.rightMargin: padding
      anchors.bottom: parent.bottom
      anchors.bottomMargin: padding

      iconSource: "qrc:///graphics/layers.png"
      onClicked: {
         layersSelector.visible = !layersSelector.visible
      }
   }

   // Button {
   //    id: debugButton1
   //    width: buttonSize
   //    height: buttonSize
   //    color: "white"

   //    anchors.bottom: layersButton.top
   //    anchors.bottomMargin: padding
   //    anchors.right: parent.right
   //    anchors.rightMargin: padding

   //    iconName: "qrc:///graphics/scooter.png"
   //    onClicked: {
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       scooters.scanScooter("bird", "ROFLCOPTER", QtPositioning.coordinate(0.0, 0.0))
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //    }
   // }

   // Button {
   //    id: debugButton2
   //    width: buttonSize
   //    height: buttonSize
   //    color: "white"

   //    anchors.bottom: debugButton1.top
   //    anchors.bottomMargin: padding
   //    anchors.right: parent.right
   //    anchors.rightMargin: padding

   //    iconName: "qrc:///graphics/bike.png"
   //    onClicked: {
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       scooters.scanScooter("nextbike", "ROFLCOPTER", QtPositioning.coordinate(0.0, 0.0))
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //       ///////////////////////            ########################### REMOVE ##############################
   //    }
   // }

   PopoverSelector {
      z: 100
      id: layersSelector
      anchors.bottom: layersButton.bottom
      anchors.bottomMargin: layersButton.height*0.5
      anchors.right: layersButton.right
      anchors.rightMargin: layersButton.width*0.5
      visible: false

      model: layersModel

      onItemChanged: {
         var view = mapViews[object]

         if (view) {
            view.visible = checked
         }
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
         if (positionSource.hasValidPosition()) {
            map.center = positionSource.position.coordinate
            map.zoomLevel = 18
         }
      }
   }

   // *************************************************************************
   // functions
   // *************************************************************************

   function onPositionUpdate(position, accuracy) {
      if (!position || !position.latitudeValid || !position.longitudeValid) {
         return
      }

      var now = new Date().getTime()

      if (lastUpdate !== -1 && (new Date().getTime() - lastUpdate) < 3000) {
         return
      }

      lastUpdate = now
      settings.lastLatitude = position.coordinate.latitude
      settings.lastLongitude = position.coordinate.longitude

      var distance = lastCoordinate && lastCoordinate.distanceTo(position.coordinate) || -1
      var reloadDistanceScooter = lastScooterReloadCoordinate && lastScooterReloadCoordinate.distanceTo(position.coordinate) || -1
      var reloadDistanceAreas = lastAreaReloadCoordinate && lastAreaReloadCoordinate.distanceTo(position.coordinate) || -1

      if (distance >= 50 || lastCoordinate === undefined || !!mapPage.activeRide) {
         lastCoordinate = QtPositioning.coordinate(position.coordinate.latitude, position.coordinate.longitude)
         map.center = position.coordinate
      }


      if (reloadDistanceScooter >= 60 || lastScooterReloadCoordinate === undefined) {
         scooters.reloadScooters(position.coordinate, 500)
         lastScooterReloadCoordinate = QtPositioning.coordinate(position.coordinate.latitude, position.coordinate.longitude)
      }


      if (reloadDistanceAreas >= 300 || lastAreaReloadCoordinate === undefined) {
         scooters.reloadAreas(position.coordinate, 500)
         lastAreaReloadCoordinate = QtPositioning.coordinate(position.coordinate.latitude, position.coordinate.longitude)
      }
   }

   function adjustMapViewsForActiveRide(provider, active) {
      for (var i = 0; i < layersModel.count; i++) {
         var item = layersModel.get(i)
         var view = mapViews[item.object]

         if (!view)
            continue;

         if (item.provider === provider) {
            if (item.object.startsWith("scooter")) {
               view.visible = !active
            }
            if (item.object.startsWith("terminal")) {
               view.visible = true
            }
            if (item.object.startsWith("area")) {
               view.visible = active
            }
         } else {
            if (item.object.startsWith("scooter")) {
               view.visible = !active
            }
            if (item.object.startsWith("terminal")) {
               view.visible = !active
            }
            if (item.object.startsWith("area")) {
               view.visible = false
            }
         }

         item.checked = view.visible
      }
   }

   function onScooterClicked(scooter) {
      scooterInfo.mode = "show"
      mapPage.selectedScooter = scooter
   }

   function updateScooters(provider, scooters) {
      var vn = "scooterView" + HelperFunctions.capitalize(provider)
      var view = mapViews[vn]

      if (view) {
         view.model = []
         view.model = scooters
      }
   }

   function updateAreas(provider, areas) {
      var a = areas || []
      var res = []

      a.forEach(function(area) {
         try {
            var path = area.polygon.map(function(poly) {
               return { latitude: poly.coordinate[1], longitude: poly.coordinate[0] }
            })
         } catch (e) {}

         res.push({ path, color: area.color })
      })

      var vn = "areaView" + HelperFunctions.capitalize(provider)
      var view = mapViews[vn]

      if (view) {
         view.model = []
         view.model = res
      }
   }

   function updateTerminals(provider, terminals) {
      var vn = "terminalView" + HelperFunctions.capitalize(provider)
      var view = mapViews[vn]

      if (view) {
         view.model = []
         view.model = terminals
      }
   }
}
