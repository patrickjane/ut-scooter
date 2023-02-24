pragma Singleton

import QtQuick 2.0
import Qt.labs.settings 1.0
import QtPositioning 5.12

import "../notify"

Item {
   id: providerNextBike
   property var countryInfoList

   property var currentCity
   property var currentPlaces
   property var currentBikes
   property var currentZones

   signal mapUpdated(var places, var bikes)

   Settings {
      id: settingsNextbike
      category: "nextbike"

      property string city
      property string country
      property string domain
   }

   // *************************************************************************
   // getCountryList
   // *************************************************************************

   function getCountryList() {
      return countryInfoList;
   }

   // *************************************************************************
   // getCityList
   // *************************************************************************

   function getCityList(country) {
      for (var i = 0; i < countryInfoList.length; i++) {
         if (countryInfoList[i].name == country)
            return countryInfoList[i].cities;
      }

      return []
   }
}
