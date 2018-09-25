#include "stdafx.h"
#include "andor.h"

Andor::~Andor() {
	m_tempTimer->stop();
	if (m_isConnected) {
		AT_Close(m_camera);
	}
	AT_FinaliseLibrary();
	disconnectDevice();
}

bool Andor::initialize() {
	if (!m_isInitialised) {
		int i_retCode = AT_InitialiseLibrary();
		if (i_retCode != AT_SUCCESS) {
			//error condition, check atdebug.log file
			m_isInitialised = false;
		} else {
			// when AT_InitialiseLibrary is called when the camera is still disabled, it will succeed,
			// but the camera is never found even if switched on later
			AT_64 i_numberOfDevices = 0;
			// Use system handle as inidivdual handle to the camera hasn't been opened. 
			int i_errorCode = AT_GetInt(AT_HANDLE_SYSTEM, L"DeviceCount", &i_numberOfDevices);
			if (i_numberOfDevices > 0) {
				m_isInitialised = true;
			} else {
				// if no camera is found and it was attempted to initialise the library, reinitializing will not help (wtf?)
				// the program has to be restarted
				emit(noCameraFound());
				AT_FinaliseLibrary();
				m_isInitialised = false;
			}
		}
	}
	return m_isInitialised;
}

void Andor::init() {
	// create timers and connect their signals
	// after moving andor to another thread
	m_tempTimer = new QTimer();
	QMetaObject::Connection connection = QWidget::connect(m_tempTimer, SIGNAL(timeout()), this, SLOT(checkSensorTemperature()));
}

void Andor::connectDevice() {
	// initialize library
	initialize();
	if (!m_isConnected && m_isInitialised) {
		int i_retCode = AT_Open(0, &m_camera);
		if (i_retCode == AT_SUCCESS) {
			m_isConnected = true;
			readOptions();
			setSettings(m_settings);
			readSettings();
			if (!m_tempTimer->isActive()) {
				m_tempTimer->start(1000);
			}
		}
	}
	emit(connectedDevice(m_isConnected));
}

void Andor::disconnectDevice() {
	if (m_isConnected) {
		if (m_tempTimer->isActive()) {
			m_tempTimer->stop();
		}
		int i_retCode = AT_Close(m_camera);
		if (i_retCode == AT_SUCCESS) {
			m_isConnected = false;
		}
	}
	emit(connectedDevice(m_isConnected));
}

void Andor::readOptions() {

	AT_GetFloatMin(m_camera, L"ExposureTime", &m_options.exposureTimeLimits[0]);
	AT_GetFloatMax(m_camera, L"ExposureTime", &m_options.exposureTimeLimits[1]);
	AT_GetIntMin(m_camera, L"FrameCount", &m_options.frameCountLimits[0]);
	AT_GetIntMax(m_camera, L"FrameCount", &m_options.frameCountLimits[1]);

	AT_GetIntMin(m_camera, L"AOIHeight", &m_options.ROIHeightLimits[0]);
	AT_GetIntMax(m_camera, L"AOIHeight", &m_options.ROIHeightLimits[1]);

	AT_GetIntMin(m_camera, L"AOIWidth", &m_options.ROIWidthLimits[0]);
	AT_GetIntMax(m_camera, L"AOIWidth", &m_options.ROIWidthLimits[1]);

	emit(optionsChanged(m_options));
}

void Andor::setSettings(CAMERA_SETTINGS settings) {
	m_settings = settings;

	// Set the pixel Encoding
	AT_SetEnumeratedString(m_camera, L"Pixel Encoding", m_settings.readout.pixelEncoding.c_str());

	// Set the pixel Readout Rate
	AT_SetEnumeratedString(m_camera, L"Pixel Readout Rate", m_settings.readout.pixelReadoutRate.c_str());

	// Set the exposure time
	AT_SetFloat(m_camera, L"ExposureTime", m_settings.exposureTime);

	// enable spurious noise filter
	AT_SetBool(m_camera, L"SpuriousNoiseFilter", m_settings.spuriousNoiseFilter);

	// Set the AOI
	AT_SetInt(m_camera, L"AOIWidth", m_settings.roi.width);
	AT_SetInt(m_camera, L"AOILeft", m_settings.roi.left);
	AT_SetInt(m_camera, L"AOIHeight", m_settings.roi.height);
	AT_SetInt(m_camera, L"AOITop", m_settings.roi.top);
	AT_SetEnumeratedString(m_camera, L"AOIBinning", m_settings.roi.binning.c_str());
	AT_SetEnumeratedString(m_camera, L"SimplePreAmpGainControl", m_settings.readout.preAmpGain.c_str());

	AT_SetEnumeratedString(m_camera, L"CycleMode", m_settings.readout.cycleMode.c_str());
	AT_SetEnumeratedString(m_camera, L"TriggerMode", m_settings.readout.triggerMode.c_str());

	// Allocate a buffer
	// Get the number of bytes required to store one frame
	AT_64 ImageSizeBytes;
	AT_GetInt(m_camera, L"ImageSizeBytes", &ImageSizeBytes);
	m_bufferSize = static_cast<int>(ImageSizeBytes);

	AT_GetInt(m_camera, L"AOIHeight", &m_settings.roi.height);
	AT_GetInt(m_camera, L"AOIWidth", &m_settings.roi.width);
	AT_GetInt(m_camera, L"AOILeft", &m_settings.roi.left);
	AT_GetInt(m_camera, L"AOITop", &m_settings.roi.top);

	// read back the settings
	readSettings();
}

void Andor::readSettings() {
	// general settings
	AT_GetFloat(m_camera, L"ExposureTime", &m_settings.exposureTime);
	//AT_GetInt(m_camera, L"FrameCount", &m_settings.frameCount);
	AT_GetBool(m_camera, L"SpuriousNoiseFilter", (int*)&m_settings.spuriousNoiseFilter);

	// ROI
	AT_GetInt(m_camera, L"AOIHeight", &m_settings.roi.height);
	AT_GetInt(m_camera, L"AOIWidth", &m_settings.roi.width);
	AT_GetInt(m_camera, L"AOILeft", &m_settings.roi.left);
	AT_GetInt(m_camera, L"AOITop", &m_settings.roi.top);
	getEnumString(L"AOIBinning", &m_settings.roi.binning[0]);

	// readout parameters
	getEnumString(L"CycleMode", &m_settings.readout.cycleMode[0]);
	getEnumString(L"Pixel Encoding", &m_settings.readout.pixelEncoding[0]);
	getEnumString(L"Pixel Readout Rate", &m_settings.readout.pixelReadoutRate[0]);
	getEnumString(L"SimplePreAmpGainControl", &m_settings.readout.preAmpGain[0]);
	getEnumString(L"TriggerMode", &m_settings.readout.triggerMode[0]);

	// emit signal that settings changed
	emit(settingsChanged(m_settings));
}

void Andor::getEnumString(AT_WC* feature, AT_WC* string) {
	int enumIndex;
	AT_GetEnumIndex(m_camera, feature, &enumIndex);
	AT_GetEnumStringByIndex(m_camera, feature, enumIndex, string, 256);
}

void Andor::setSensorCooling(bool cooling) {
	int i_retCode = AT_SetBool(m_camera, L"SensorCooling", (int)cooling);
	m_isCooling = cooling;
	emit(cameraCoolingChanged(m_isCooling));
}

bool Andor::getSensorCooling() {
	AT_BOOL szValue;
	int i_retCode = AT_GetBool(m_camera, L"SensorCooling", &szValue);
	return szValue;
}

const std::string Andor::getTemperatureStatus() {
	int i_retCode = AT_GetEnumIndex(m_camera, L"TemperatureStatus", &m_temperatureStatusIndex);
	AT_GetEnumStringByIndex(m_camera, L"TemperatureStatus", m_temperatureStatusIndex, temperatureStatus, 256);
	std::wstring ws(temperatureStatus);
	std::string m_temperatureStatus(ws.begin(), ws.end());
	return m_temperatureStatus;
}

double Andor::getSensorTemperature() {
	double szValue;
	int i_retCode = AT_GetFloat(m_camera, L"SensorTemperature", &szValue);
	return szValue;
}

void Andor::checkSensorTemperature() {
	m_sensorTemperature.temperature = getSensorTemperature();
	std::string status = getTemperatureStatus();
	if (status == "Cooler Off") {
		m_sensorTemperature.status = COOLER_OFF;
	} else if (status == "Fault") {
		m_sensorTemperature.status = FAULT;
	} else if(status == "Cooling") {
		m_sensorTemperature.status = COOLING;
	} else if (status == "Drift") {
		m_sensorTemperature.status = DRIFT;
	} else if (status == "Not Stabilised") {
		m_sensorTemperature.status = NOT_STABILISED;
	} else if (status == "Stabilised") {
		m_sensorTemperature.status = STABILISED;
	} else {
		m_sensorTemperature.status = FAULT;
	}
	emit(s_sensorTemperatureChanged(m_sensorTemperature));
}

void Andor::startPreview() {
	m_isPreviewRunning = true;
	preparePreview();
	getImageForPreview();

	emit(s_previewRunning(m_isPreviewRunning));
}

void Andor::preparePreview() {
	// always use full camera image for live preview
	m_settings.roi.width = m_options.ROIWidthLimits[1];
	m_settings.roi.left = 1;
	m_settings.roi.height = m_options.ROIHeightLimits[1];
	m_settings.roi.top = 1;

	setSettings(m_settings);

	int pixelNumber = m_settings.roi.width * m_settings.roi.height;
	BUFFER_SETTINGS bufferSettings = { 5, pixelNumber * 2, m_settings.roi };
	m_previewBuffer->initializeBuffer(bufferSettings);
	emit(s_previewBufferSettingsChanged());

	// Start acquisition
	AT_Command(m_camera, L"AcquisitionStart");
	AT_InitialiseUtilityLibrary();
}

void Andor::stopPreview() {
	cleanupAcquisition();
	m_isPreviewRunning = false;
	emit(s_previewRunning(m_isPreviewRunning));
}

void Andor::startAcquisition(CAMERA_SETTINGS settings) {
	// check if currently a preview is running and stop it in case
	if (m_isPreviewRunning) {
		stopPreview();
	}

	setSettings(settings);

	int pixelNumber = m_settings.roi.width * m_settings.roi.height;
	BUFFER_SETTINGS bufferSettings = { 4, pixelNumber * 2, m_settings.roi };
	m_previewBuffer->initializeBuffer(bufferSettings);
	emit(s_previewBufferSettingsChanged());

	// Start acquisition
	AT_Command(m_camera, L"AcquisitionStart");
	AT_InitialiseUtilityLibrary();

	m_isAcquisitionRunning = true;
	emit(s_acquisitionRunning(m_isAcquisitionRunning));
}

void Andor::stopAcquisition() {
	cleanupAcquisition();
	m_isAcquisitionRunning = false;
	emit(s_acquisitionRunning(m_isAcquisitionRunning));
}

void Andor::cleanupAcquisition() {
	AT_FinaliseUtilityLibrary();
	AT_Command(m_camera, L"AcquisitionStop");
	AT_Flush(m_camera);
}

void Andor::acquireImage(AT_U8* buffer) {
	// Pass this buffer to the SDK
	unsigned char* UserBuffer = new unsigned char[m_bufferSize];
	AT_QueueBuffer(m_camera, UserBuffer, m_bufferSize);

	// Acquire camera images
	AT_Command(m_camera, L"SoftwareTrigger");

	// Sleep in this thread until data is ready
	unsigned char* Buffer;
	int ret = AT_WaitBuffer(m_camera, &Buffer, &m_bufferSize, 1500 * m_settings.exposureTime);
	// return if AT_WaitBuffer timed out
	if (ret == AT_ERR_TIMEDOUT) {
		return;
	}

	// Process the image
	//Unpack the 12 bit packed data
	AT_GetInt(m_camera, L"AOIHeight", &m_settings.roi.height);
	AT_GetInt(m_camera, L"AOIWidth", &m_settings.roi.width);
	AT_GetInt(m_camera, L"AOIStride", &m_imageStride);

	AT_ConvertBuffer(Buffer, buffer, m_settings.roi.width, m_settings.roi.height, m_imageStride, m_settings.readout.pixelEncoding.c_str(), L"Mono16");

	delete[] Buffer;
}

void Andor::getImageForPreview() {
	if (m_isPreviewRunning) {

		m_previewBuffer->m_buffer->m_freeBuffers->acquire();
		acquireImage(m_previewBuffer->m_buffer->getWriteBuffer());
		m_previewBuffer->m_buffer->m_usedBuffers->release();

		QMetaObject::invokeMethod(this, "getImageForPreview", Qt::QueuedConnection);
	} else {
		stopPreview();
	}
}

void Andor::getImageForAcquisition(AT_U8* buffer) {
	acquireImage(buffer);

	// write image to preview buffer
	memcpy(m_previewBuffer->m_buffer->getWriteBuffer(), buffer, m_settings.roi.width * m_settings.roi.height * 2);
	m_previewBuffer->m_buffer->m_usedBuffers->release();
}

void Andor::setCalibrationExposureTime(double exposureTime) {
	m_settings.exposureTime = exposureTime;
	AT_Command(m_camera, L"AcquisitionStop");
	// Set the exposure time
	AT_SetFloat(m_camera, L"ExposureTime", m_settings.exposureTime);

	AT_Command(m_camera, L"AcquisitionStart");
}