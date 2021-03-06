#ifndef UEYECAM_H
#define UEYECAM_H

#include "Camera.h"

namespace uEye {
	#include "uEye.h"
}

class uEyeCam : public Camera {
	Q_OBJECT

public:
	uEyeCam() noexcept {};
	~uEyeCam();

public slots:
	void init() override {};
	void connectDevice() override;
	void disconnectDevice() override;

	void startPreview() override;
	void stopPreview() override;
	void startAcquisition(const CAMERA_SETTINGS&) override;
	void stopAcquisition() override;
	void getImageForAcquisition(std::byte* buffer, bool preview = true) override;

private:
	int acquireImage(std::byte* buffer) override;

	void readOptions() override;
	void readSettings() override;
	void applySettings(const CAMERA_SETTINGS& settings) override;

	void preparePreview();

	uEye::HCAM m_camera{ (uEye::HCAM)0 };

	char* m_imageBuffer{ nullptr };
	int m_imageBufferId{ 0 };
};

#endif // UEYECAM_H