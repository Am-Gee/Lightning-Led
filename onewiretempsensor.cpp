#include <Arduino.h>
#include "onewiretempsensor.h"

COneWireTempSensor::COneWireTempSensor(int id, OneWire* pDS, byte* adress, long interval) : CSensor(interval)
{
    m_id = id;
    m_arrSensorAdress = adress;
    m_pDS = pDS;
    m_fLastValue = 0.0f;
    m_bWrite = true;
}

COneWireTempSensor::~COneWireTempSensor()
{
    //dtor
}

int COneWireTempSensor::GetId()
{
    return m_id;
}
void COneWireTempSensor::Setup()
{
    CSensor::Setup();

    if (OneWire::crc8(m_arrSensorAdress, 7) != m_arrSensorAdress[7])
    {
      Serial.println("CRC is not valid!");
      return;
    }
}

float COneWireTempSensor::GetSensorValue()
{
    return m_fLastValue;
}

void COneWireTempSensor::OnUpdate()
{
    if(m_bWrite)
    {
        WriteTimeToScratchpad();
        m_bWrite = false;
        return;
    }
    m_bWrite = true;

    byte data[12];

    ReadTimeFromScratchpad(data);

    m_fLastValue = ReadTimeFromScratchpad(data);
}

void COneWireTempSensor::WriteTimeToScratchpad()
{
  //reset the bus
  m_pDS->reset();
  //select our sensor
  m_pDS->select(m_arrSensorAdress);
  //CONVERT T function call (44h) which puts the temperature into the scratchpad
  m_pDS->write(0x44,0);
  //sleep a second for the write to take place
  //delay(1000);
}

float COneWireTempSensor::ReadTimeFromScratchpad(byte* data)
{
  int tr;

  //reset the bus
  m_pDS->reset();
  //select our sensor
  m_pDS->select(m_arrSensorAdress);
  //read the scratchpad (BEh)
  m_pDS->write(0xBE);
  for (byte i=0;i<9;i++)
    {
    data[i] = m_pDS->read();
  }

   //put in temp all the 8 bits of LSB (least significant byte)
  tr = data[0];

  //check for negative temperature
  if (data[1] > 0x80)
  {
    tr = !tr + 1; //two's complement adjustment
    tr = tr * -1; //flip value negative.
  }

  //COUNT PER Celsius degree (10h)
  int cpc = data[7];
  //COUNT REMAIN (0Ch)
  int cr = data[6];

  //drop bit 0
  tr = tr >> 1;

  return tr - (float)0.25 + (cpc - cr)/(float)cpc;
}
