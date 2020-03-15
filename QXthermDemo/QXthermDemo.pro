QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    UI/mainwindow.cpp \
    main.cpp

HEADERS += \
    Includes/Amvideo.h \
    Includes/CyAdapterID.h \
    Includes/CyAssert.h \
    Includes/CyBayerRGB10.h \
    Includes/CyBayerRGB12.h \
    Includes/CyBayerRGB14.h \
    Includes/CyBayerRGB8.h \
    Includes/CyBuffer.h \
    Includes/CyBufferConstants.h \
    Includes/CyBufferQueue.h \
    Includes/CyCamLib.h \
    Includes/CyCamLibCDef.h \
    Includes/CyCameraInterface.h \
    Includes/CyCameraInterfaceBackwardCompatibility.h \
    Includes/CyCameraInterfaceConstants.h \
    Includes/CyCameraLVDS.h \
    Includes/CyCameraLimits.h \
    Includes/CyCameraLink.h \
    Includes/CyCameraLinkExtension.h \
    Includes/CyCameraLinkExtensionBackwardCompatibility.h \
    Includes/CyCameraRegistry.h \
    Includes/CyChannel.h \
    Includes/CyComLib.h \
    Includes/CyComLibCDef.h \
    Includes/CyConfig.h \
    Includes/CyConfigBackwardCompatibility.h \
    Includes/CyConfigConstants.h \
    Includes/CyCurrentThread.h \
    Includes/CyDevice.h \
    Includes/CyDeviceBackwardCompatibility.h \
    Includes/CyDeviceConstants.h \
    Includes/CyDeviceExtension.h \
    Includes/CyDeviceExtensionConstants.h \
    Includes/CyDeviceFinder.h \
    Includes/CyDeviceList.h \
    Includes/CyDispLib.h \
    Includes/CyDisplayExConstants.h \
    Includes/CyEngine.h \
    Includes/CyEngineDriver.h \
    Includes/CyEngineExtension.h \
    Includes/CyEngineGrabber.h \
    Includes/CyEngineLib.h \
    Includes/CyEngineSharedMemory.h \
    Includes/CyErrorInterface.h \
    Includes/CyEvent.h \
    Includes/CyExtension.h \
    Includes/CyGate.h \
    Includes/CyGigEVisionDevice.h \
    Includes/CyGigEVisionDeviceConstants.h \
    Includes/CyGigEVisionLib.h \
    Includes/CyGigEVisionPort.h \
    Includes/CyGrabber.h \
    Includes/CyGrabberBackwardCompatibility.h \
    Includes/CyGrabberConstants.h \
    Includes/CyGrabberExtension.h \
    Includes/CyGrabberExtensionConstants.h \
    Includes/CyGrabberMedium.h \
    Includes/CyGrabberMediumConstants.h \
    Includes/CyGrayscale10.h \
    Includes/CyGrayscale12.h \
    Includes/CyGrayscale14.h \
    Includes/CyGrayscale16.h \
    Includes/CyGrayscale4.h \
    Includes/CyGrayscale8.h \
    Includes/CyHighMemoryManager.h \
    Includes/CyI2CUtility.h \
    Includes/CyImageBuffer.h \
    Includes/CyImageBufferConstants.h \
    Includes/CyImageInfo.h \
    Includes/CyImgLib.h \
    Includes/CyLockScope.h \
    Includes/CyMediumLib.h \
    Includes/CyMemoryManager.h \
    Includes/CyMutex.h \
    Includes/CyObject.h \
    Includes/CyParameterRepository.h \
    Includes/CyParameterRepositoryUtilities.h \
    Includes/CyPerfMonitor.h \
    Includes/CyPersistentRepository.h \
    Includes/CyPixelConverter.h \
    Includes/CyPixelConverterConstants.h \
    Includes/CyPixelTypeConstants.h \
    Includes/CyPixelTypeFactory.h \
    Includes/CyRGB16.h \
    Includes/CyRGB24.h \
    Includes/CyRGB32.h \
    Includes/CyRGB48.h \
    Includes/CyRGB64.h \
    Includes/CyRGBFilter.h \
    Includes/CyResultEvent.h \
    Includes/CySemaphore.h \
    Includes/CySharedMemory.h \
    Includes/CySimpleBuffer.h \
    Includes/CySimpleMemoryManager.h \
    Includes/CySocketUDP.h \
    Includes/CyString.h \
    Includes/CySynchro.h \
    Includes/CyTWUtility.h \
    Includes/CyThread.h \
    Includes/CyTypes.h \
    Includes/CyUserBuffer.h \
    Includes/CyUtilsLib.h \
    Includes/CyUtilsLibCDef.h \
    Includes/CyVersion.h \
    Includes/CyVideoControllerExtension.h \
    Includes/CyXMLDocument.h \
    Includes/CyYUV10.h \
    Includes/CyYUV8.h \
    Includes/DShow.h \
    Includes/EbDriver.h \
    Includes/EbDriverNames.h \
    Includes/EbInstaller.h \
    Includes/EbNetworkAdapter.h \
    Includes/EbSetupLib.h \
    Includes/PtPublic.h \
    Includes/PtResult.h \
    Includes/PtString.h \
    Includes/PtTypes.h \
    Includes/amaudio.h \
    Includes/amstream.h \
    Includes/audevcod.h \
    Includes/austream.h \
    Includes/cydisplayex.h \
    Includes/cypixeltype.h \
    Includes/d3d.h \
    Includes/d3dcaps.h \
    Includes/d3drm.h \
    Includes/d3drmdef.h \
    Includes/d3drmobj.h \
    Includes/d3dtypes.h \
    Includes/ddraw.h \
    Includes/ddstream.h \
    Includes/dsound.h \
    Includes/dvdevcod.h \
    Includes/dxtrans.h \
    Includes/edevdefs.h \
    Includes/errors.h \
    Includes/evcode.h \
    Includes/mmstream.h \
    Includes/mtype.h \
    Includes/niimaq.h \
    Includes/qedit.h \
    Includes/uuids.h \
    Includes/vfwmsgs.h \
    Includes/winapifamily.h \
    Includes/xthermdll.h \
    UI/mainwindow.h

FORMS += \
    UI/mainwindow.ui

TRANSLATIONS += \
    QXthermDemo_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
