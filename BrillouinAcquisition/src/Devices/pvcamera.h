#ifndef PVCAMERA_H
#define PVCAMERA_H

#include "Camera.h"
#include <typeinfo>

namespace PVCam {
	#include "master.h"
	#include "pvcam.h"
}

class PVCamera : public Camera {
	Q_OBJECT

private:
	PVCam::int16 m_camera{ -1 };
	bool m_isInitialised{ false };
	bool m_isCooling{ false };
	QTimer* m_tempTimer = nullptr;
	SensorTemperature m_sensorTemperature;

	void cleanupAcquisition();
	void preparePreview();

	void acquireImage(unsigned char* buffer) override;

	/*
	 * Members and functions inherited from base class
	 */
	void readOptions();
	void readSettings();

public:
	PVCamera() noexcept {};
	~PVCamera();
	bool initialize();

	// setters/getters for sensor cooling
	bool getSensorCooling();
	const std::string getTemperatureStatus();
	double getSensorTemperature();
	void setCalibrationExposureTime(double);

private slots:
	void checkSensorTemperature();

public slots:
	/*
	* Members and functions specific to Andor class
	*/
	// setters/getters for sensor cooling
	void setSensorCooling(bool cooling);

	/*
	* Members and functions inherited from base class
	*/
	void init();
	void connectDevice();
	void disconnectDevice();

	void setSettings(CAMERA_SETTINGS);

	void startPreview();
	void stopPreview();
	void startAcquisition(CAMERA_SETTINGS);
	void stopAcquisition();
	
	void getImageForAcquisition(unsigned char* buffer, bool preview = true) override;
};

#endif // PVCAMERA_H
