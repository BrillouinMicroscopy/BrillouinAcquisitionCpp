#ifndef ZEISSMTB_ERLANGEN_H
#define ZEISSMTB_ERLANGEN_H

#include <atlutil.h>
#include "ODTControl.h"

#import "MTBApi.tlb" named_guids
using namespace MTBApi;

class ZeissMTB_Erlangen: public ODTControl {
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
	// MTB interface pointer to the objective
	IMTBChangerPtr m_Objective = nullptr;
	// MTB interface pointer to the reflector
	IMTBChangerPtr m_Reflector = nullptr;
	// MTB interface pointer to the sideport
	IMTBChangerPtr m_Sideport = nullptr;
	// MTB interface pointer to the RL shutter
	IMTBChangerPtr m_RLShutter = nullptr;
	// MTB interface pointer to the RL/TL switch
	IMTBChangerPtr m_RLTLSwitch = nullptr;
	// MTB interface pointer to the focus
	IMTBContinualPtr m_ObjectiveFocus = nullptr;
	// MTB interface pointer to the stage axis x
	IMTBContinualPtr m_stageX = nullptr;
	// MTB interface pointer to the stage axis y
	IMTBContinualPtr m_stageY = nullptr;

	bool m_isMTBConnected{ false };

	enum class DEVICE_ELEMENT {
		OBJECTIVE,
		REFLECTOR,
		SIDEPORT,
		RLSHUTTER,
		COUNT
	};

	POINT2 pixToMicroMeter(POINT2);

public:
	ZeissMTB_Erlangen() noexcept;
	~ZeissMTB_Erlangen();

	void setPosition(POINT3 position);
	void setPosition(POINT2 position);
	POINT3 getPosition();

	void setVoltage(VOLTAGE2 voltage);

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
	int getReflector();
	void setReflector(int value, bool check = false);
	int getObjective();
	void setObjective(int value, bool check = false);
	int getSideport();
	void setSideport(int value, bool check = false);
	int getRLShutter();
	void setRLShutter(int value, bool check = false);
	// sets the position relative to the home position m_homePosition
	void setPositionRelativeX(double position);
	void setPositionRelativeY(double position);
	void setPositionRelativeZ(double position);
	void setPositionInPix(POINT2);
	void setLEDLamp(bool enabled);
	void setAcquisitionVoltages(ACQ_VOLTAGES voltages);
};

#endif // ZEISSMTB_ERLANGEN_H