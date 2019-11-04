#ifndef ZEISSMTB_H
#define ZEISSMTB_H

#include <atlutil.h>
#include "scancontrol.h"

namespace Thorlabs_FF {
	#include <Thorlabs.MotionControl.FilterFlipper.h>
}

#import "MTBApi.tlb" named_guids
using namespace MTBApi;

class ZeissMTB: public ScanControl {
	Q_OBJECT

private:
	/*
	 * Zeiss MTB handles
	 */
	// MTB interface pointer to the connection class
	IMTBConnectionPtr m_MTBConnection = nullptr;
	// MTB interface ptr to the root of the tree of devices of the microscope
	IMTBRootPtr m_Root = nullptr;
	// my ID received from MTB
	CComBSTR m_ID = _T("");
	// MTB interface pointer to the halogen lamp
	IMTBContinualPtr m_Lamp = nullptr;
	// MTB interface pointer to the halogen lamp mirror
	IMTBChangerPtr m_Mirror = nullptr;
	// MTB interface pointer to the objective
	IMTBChangerPtr m_Objective = nullptr;
	// MTB interface pointer to the reflector
	IMTBChangerPtr m_Reflector = nullptr;
	// MTB interface pointer to the tubelens
	IMTBChangerPtr m_Tubelens = nullptr;
	// MTB interface pointer to the baseport
	IMTBChangerPtr m_Baseport = nullptr;
	// MTB interface pointer to the sideport
	IMTBChangerPtr m_Sideport = nullptr;
	// MTB interface pointer to the RL shutter
	IMTBChangerPtr m_RLShutter = nullptr;
	// MTB interface pointer to the focus
	IMTBContinualPtr m_ObjectiveFocus = nullptr;
	// MTB interface pointer to the stage axis x
	IMTBContinualPtr m_stageX = nullptr;
	// MTB interface pointer to the stage axis y
	IMTBContinualPtr m_stageY = nullptr;

	bool m_isMTBConnected{ false };

	char const *m_serialNo_FF2 = "37000251";

	enum class DEVICE_ELEMENT {
		BEAMBLOCK,
		OBJECTIVE,
		REFLECTOR,
		TUBELENS,
		BASEPORT,
		SIDEPORT,
		RLSHUTTER,
		MIRROR,
		LAMP,
		COUNT
	};

	POINT2 pixToMicroMeter(POINT2);

public:
	ZeissMTB() noexcept;
	~ZeissMTB();

	void setPosition(POINT3 position);
	void setPosition(POINT2 position);
	POINT3 getPosition();

public slots:
	void init();
	void connectDevice();
	void disconnectDevice();
	int getElement(DeviceElement element);
	void setElement(DeviceElement element, double position);
	int getElement(IMTBChangerPtr element);
	bool setElement(IMTBChangerPtr element, int position);
	void setPreset(ScanPreset preset);
	void getElements();
	int getBeamBlock();
	void setBeamBlock(int position);
	int getReflector();
	void setReflector(int value, bool check = false);
	int getObjective();
	void setObjective(int value, bool check = false);
	int getTubelens();
	void setTubelens(int value, bool check = false);
	int getBaseport();
	void setBaseport(int value, bool check = false);
	int getSideport();
	void setSideport(int value, bool check = false);
	int getRLShutter();
	void setRLShutter(int value, bool check = false);
	int getMirror();
	void setMirror(int value, bool check = false);
	double getLamp();
	void setLamp(int value, bool check = false);
	// sets the position relative to the home position m_homePosition
	void setPositionRelativeX(double position);
	void setPositionRelativeY(double position);
	void setPositionRelativeZ(double position);
	void setPositionInPix(POINT2);
};

#endif // ZEISSMTB_H