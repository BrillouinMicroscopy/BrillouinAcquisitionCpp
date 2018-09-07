#ifndef NIDAQ_H
#define NIDAQ_H

#include <QSerialPort>
#include <vector>
#include "NIDAQmx.h"
#include "scancontrol.h"
#include "simplemath.h"
#include <gsl/gsl>
namespace Thorlabs_TIM {
	#include <Thorlabs.MotionControl.TCube.InertialMotor.h>
}
namespace Thorlabs_FF {
	#include <Thorlabs.MotionControl.FilterFlipper.h>
}

#include "H5Cpp.h"
#include "filesystem"

struct ELEMENTPOSITION {
	Thorlabs_FF::FF_Positions CalFlipMirror{ Thorlabs_FF::FF_Positions::Position1 };	// Brillouin measurement
	Thorlabs_FF::FF_Positions BeamBlock{ Thorlabs_FF::FF_Positions::Position1 };		// Beam path blocked
};

class NIDAQ: public ScanControl {
	Q_OBJECT

private:
	TaskHandle taskHandle = 0;
	
	struct Calibration {
		std::string date{ "" };
		POINT2 translation{ -3.8008e-6, 1.1829e-6 };	// [m]	translation
		double rho{ -0.2528 };		// [rad]	rotation
		COEFFICIANTS5 coef{
			-6.9185e-4, // [1/m�]	coefficient of fourth order
			6.7076e-4,	// [1/m�]	coefficient of third order
			-1.1797e-4,	// [1/m]	coefficient of second order
			4.1544e-4,	// [1]		coefficient of first order
			0			// [m]		offset term
		};
		BOUNDS bounds = {
			-53,	// [�m] minimal x-value
			 53,	// [�m] maximal x-value
			-43,	// [�m] minimal y-value
			 43,	// [�m] maximal y-value
			 -1000,	// [�m] minimal z-value
			  1000	// [�m] maximal z-value
		};
		bool valid = false;
	} m_calibration;

	VOLTAGE2 m_voltages{ 0, 0 };	// current voltage
	POINT3 m_position{ 0, 0, 0 };	// current position
	ELEMENTPOSITION m_elementPositions{ Thorlabs_FF::FF_Positions::Position1, Thorlabs_FF::FF_Positions::Position1 };
	
	
	// TODO: make the following parameters changeable:
	char const *m_serialNo_TIM = "65864438";	// serial number of the TCube Inertial motor controller device (can be found in Kinesis)
	Thorlabs_TIM::TIM_Channels m_channelPosZ{ Thorlabs_TIM::Channel1 };
	int m_PiezoIncPerMum{ 50 };

	char const *m_serialNo_FF1 = "37000784";
	char const *m_serialNo_FF2 = "37000251";

	enum class DEVICE_ELEMENT {
		CALFLIPMIRROR,
		BEAMBLOCK,
		DEVICE_ELEMENT_COUNT
	};

public:
	NIDAQ() noexcept;
	~NIDAQ();

	VOLTAGE2 positionToVoltage(POINT2 position);
	POINT2 voltageToPosition(VOLTAGE2 position);

	void applyScanPosition();

	void setPosition(POINT3 position);
	POINT3 getPosition();

	// NIDAQ specific function to move position to center of field of view
	void centerPosition();
	
	std::vector<std::string> m_groupLabels = { "Flip Mirror", "Beam Block" };
	std::vector<int> m_maxOptions = { 2, 2 };

public slots:
	void init();
	bool connectDevice();
	bool disconnectDevice();
	void setElement(DeviceElement element, int position);
	void getElement(DeviceElement element);
	void setElements(ScanControl::SCAN_PRESET preset);
	void getElements();
	void setCalFlipMirror(int position);
	void setBeamBlock(int position);
	// sets the position relative to the home position m_homePosition
	void setPositionRelativeX(double position);
	void setPositionRelativeY(double position);
	void setPositionRelativeZ(double position);
	void loadVoltagePositionCalibration(std::string filepath) override;
	double getCalibrationValue(H5::H5File file, std::string datasetName);
};

#endif // NIDAQMX_H