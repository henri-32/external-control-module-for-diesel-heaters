#pragma once
#include "interfaces.h"

class InputDevices : public IInputDevices {
//Diese Klasse ist eine Abstraktionsschicht zwischen den Treibern und stellen 
//für den SystemController den zugriff auf init() und update() Funktionen der Treiber 
//an einer einfachen Schnittstelle zur Verfügung. 
//Die Modellierung, dass der SystemController über die Treiber iteriert, um deren Methoden 
//polymorph aufzurufen war in der Umsetzung für mich deutlich komplexer und hatte keinen 
//direkten Mehrwert, da diese Klasse bestand. 
//Aufgrund der feststehenden Treiberklassen, erscheint es akzeptabel, dieses generische 
//Interface zwischen SystemController und Treibern bestehen zu lassen. 
public:
  explicit InputDevices(InputDevicesDataSet &inputData, IToggleSwitch &ps,
                        IToggleSwitch &ms, IPushButton &db,
                        IEncoderDriver &encoderDriver,
                        ITempSensorDriver &tempSensorDriver);

  void init() override;
  void update() override;

private:
  IToggleSwitch &m_powerSwitch;
  IToggleSwitch &m_modeSwitch;
  IPushButton &m_displayButton;
  IEncoderDriver &m_encoderDriver;
  ITempSensorDriver &m_tempSensorDriver;
};

class OutputDevices : public IOutputDevices {
public:
  OutputDevices(OutputDevicesIntent &outputIntent,
                IDisplayDriver &displayDriver, IRelais &relais);

  void init() override;
  void update() override;

private:
  IRelais &m_relais;
  IDisplayDriver &m_displayDriver;
};
