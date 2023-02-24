import QtQuick 2.7
import Ubuntu.Components 1.3
import QtMultimedia 5.12
import QZXing 3.3

import "../util"
import "../notify"

Page {
   property bool popped: false
   property var scooters
   property var positionSource

   id: dialog
   anchors.fill: parent

   header: PageHeader {
      id: header
      title: i18n.tr("Scan QR code")
   }

   Camera {
      id: camera
      focus {
         focusMode: Camera.FocusContinuous | Camera.FocusMacro
         focusPointMode: Camera.FocusPointAuto
      }
      captureMode: Camera.CaptureViewfinder
      exposure.exposureMode: Camera.ExposureBarcode
   }

   VideoOutput {
      id: videoOutput
      source: camera
      width: parent.width
      height: parent.width
      anchors.verticalCenter: parent.verticalCenter

      focus: true
      //         filters: [ videoFilter ]
      autoOrientation: true
      fillMode: VideoOutput.PreserveAspectCrop
   }

   MouseArea {
      anchors.fill: parent
      onClicked: camera.searchAndLock()
   }

   //   QZXingFilter {
   //      id: videoFilter
   //      decoder {
   //         imageSourceFilter: QZXing.SourceFilter_ImageNormal
   //         enabledDecoders: QZXing.DecoderFormat_QR_CODE
   //         tryHarder: true
   //         tryHarderType: QZXing.TryHarderBehaviour_Rotate | QZXing.TryHarderBehaviour_ThoroughScanning

   //         onTagFound: {
   //            if (popped)
   //               return;

   //            popped = true
   //            pageStack.pop()
   //            handleScannedCode(tag)
   //         }
   //      }
   //   }

   QZXing {
      id: qzxing
      enabledDecoders: QZXing.DecoderFormat_QR_CODE
      tryHarder: true
      tryHarderType: QZXing.TryHarderBehaviour_Rotate | QZXing.TryHarderBehaviour_ThoroughScanning

      onTagFound: {
         if (popped)
            return;

         popped = true
         pageStack.pop()
         handleScannedCode(tag)
      }
   }

   Timer {
      id: scanTimer
      interval: 250
      repeat: true
      running: true
      onTriggered: {
         videoOutput.grabToImage(function(result) {
            console.log("Decoging image ....")
            qzxing.decodeImage(result.image);
         });
      }
   }

   function handleScannedCode(code) {
      var idAndProvider= extractidAndProvider(code)

      if (!idAndProvider) {
         Notify.warning(i18n.tr("Ride"), i18n.tr("Code not recognized/provider not supported"))
         return
      }

      scooters.scanScooter(idAndProvider[0], idAndProvider[1], positionSource.position.coordinate)
   }

   function extractidAndProvider(code) {
      if (code.indexOf("https://ride.bird.co/") === 0)
         return ["bird", code.replace("https://ride.bird.co/", "")]

      if (code.indexOf("http://nxtb.it/") === 0)
         return ["nextbike", code.replace("http://nxtb.it/", "")]

      return
   }
}
