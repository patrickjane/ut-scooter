pragma Singleton

import QtQuick 2.0

import "../notify"
import "../provider"

Item {
   signal mapUpdated(var provider, var places, var bikes)

   function init() {
      NextBike.mapUpdated.connect(function(places, bikes) {
         emit: mapUpdated("nextbike", places, bikes)
      })

      NextBike.init();
   }

   function getCountryList(provider) {
      switch (provider) {
         case 'nextbike': return NextBike.getCountryList()
      }

      return null
   }

   function getCityList(provider, country) {
      switch (provider) {
         case 'nextbike': return NextBike.getCityList(country)
      }

      return null
   }

   function update(provider, country, city, domain) {
      switch (provider) {
         case 'nextbike': return NextBike.update(country, city, domain)
      }
   }
}