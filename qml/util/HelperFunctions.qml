pragma Singleton

import QtQuick 2.0
import Ubuntu.Components.Popups 1.3

QtObject {
   property string title: "bla"
   property var capitalize: function(str) {
      if (!str)
         return str;

      return (str + '').replace(/^\w/, function(c){ return c.toUpperCase()})
   }

   function toElapsed(secs) {
      var hours   = Math.floor(secs / 3600);
      var minutes = Math.floor((secs - (hours * 3600)) / 60);
      var seconds = Math.floor(secs - (hours * 3600) - (minutes * 60));
      if (hours   < 10) {hours   = "0"+hours;}
      if (minutes < 10) {minutes = "0"+minutes;}
      if (seconds < 10) {seconds = "0"+seconds;}
      return hours+':'+minutes+':'+seconds;
   }

   function formatCurrency(item) {
      if ((!item.value && !item.cost) || !item.locale || !item.currency)
         return;

      var cost = item.cost || item.value

      return Number(cost / 100.0).toLocaleCurrencyString(Qt.locale(item.locale), currencyToSymbol(item.currency))
   }

   function currencyToSymbol(currency) {
      var currency_symbols = {
          'usd': '$', // US Dollar
          'eur': '€', // Euro
          'crc': '₡', // Costa Rican Colón
          'gbp': '£', // British Pound Sterling
          'ils': '₪', // Israeli New Sheqel
          'inr': '₹', // Indian Rupee
          'jpy': '¥', // Japanese Yen
          'krw': '₩', // South Korean Won
          'ngn': '₦', // Nigerian Naira
          'php': '₱', // Philippine Peso
          'pln': 'zł', // Polish Zloty
          'pyg': '₲', // Paraguayan Guarani
          'thb': '฿', // Thai Baht
          'uah': '₴', // Ukrainian Hryvnia
          'vnd': '₫', // Vietnamese Dong
      };

      return currency && currency_symbols[currency.toLowerCase()] || currency;
   }
}
