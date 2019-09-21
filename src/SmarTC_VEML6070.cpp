/*
* SmarTC VEML6070 Module
* See COPYRIGHT file at the top of the source tree.
*
* This product includes software developed by the SmarTC team.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, at version 3 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the SmarTC and the GNU General
* Public License along with this program. If not,
* see <http://www.lsstcorp.org/LegalNotices/>.
*/

/**
 * @file SmarTC_VEML6070.h
 * 
 * @brief Source code for VEML6070 UV Sensor management.
 * @ingroup SmarTC_VEML6070
 * 
 * @author Clément TOCHE
 * Contact: ad@toche.fr
 */

#include "SmarTC_VEML6070.h"

#include <Arduino.h>
#include <Wire.h>

SmarTC_VEML6070::SmarTC_VEML6070()
{
    cmd_buffer.buf = 0;

    i_it = VEML6070_4_T;
    i_rset = 300;
}

SmarTC_VEML6070::SmarTC_VEML6070(veml6070_it_e it, int rset)
{
    cmd_buffer.buf = 0;

    i_it = it;
    i_rset = rset;
    i_init = false;
}

SmarTC_VEML6070::~SmarTC_VEML6070()
{
    if (!i_init)
        return;

    clearInt();
    shutDown(true);
}

bool SmarTC_VEML6070::launch()
{
    if (!i_init)
    {
        Wire.begin(); // Join bus as a master

        cmd_buffer.bitfield.RES = true; // Reserved bit to be always 1

        i_init = clearInt();
        if (!i_init)
        {
            Serial.println("Fail to clearInt");
            return i_init;
        }
    }
    else
    {
        Serial.println("Instance already initialized");
        return i_init;
    }

    // Init Integration Time
    cmd_buffer.bitfield.IT = i_it;
    if (!write())
    {
        Serial.println("Fail to write Integration Time");
        i_init = false;
    }

    return i_init;
}

bool SmarTC_VEML6070::clearInt()
{
    if (0 == Wire.requestFrom(VEML6070_ADDR_ARA, 1))
    {
        Serial.println("Unable to request from ARA address");
        return false;
    }

    unsigned long start = millis();
    while (!Wire.available())
    {
        delay(1);
        if (millis() - start > 5000)
        {
            Serial.println("Wire timeout");
            return false;
        }
    }

    // Ensure we have unstacked the value
    Wire.read();

    return true;
}

bool SmarTC_VEML6070::write()
{
    Serial.print("Write buffer : ");
    Serial.println(cmd_buffer.buf, BIN);

    Wire.beginTransmission(VEML6070_ADDR_CMD);
    Wire.write(cmd_buffer.buf);
    byte ret = Wire.endTransmission();

    switch (ret)
    {
    case 0:
        return true;
    case 1:
        Serial.println("Data too long to fit in transmit buffer");
        return false;
    case 2:
        Serial.println("received NACK on transmit of address");
        return false;
    case 3:
        Serial.println("received NACK on transmit of data");
        return false;
    case 4:
        Serial.println("other error");
        return false;
    default:
        return false;
    }

    return false;
}

bool SmarTC_VEML6070::shutDown(bool enable)
{
    cmd_buffer.bitfield.SD = enable;
    if (!write())
    {
        Serial.println("Fail to write SD command");
        return false;
    }

    return true;
}

uint16_t SmarTC_VEML6070::readUV()
{
    if (Wire.requestFrom(VEML6070_ADDR_MSB, 1) != 1)
    {
        Serial.println("Fail to request MSB UV Value");
        return -1;
    }

    uint16_t uv = Wire.read();
    uv <<= 8;

    if (Wire.requestFrom(VEML6070_ADDR_LSB, 1) != 1)
    {
        Serial.println("Fail to request LSB UV Value");
        return -1;
    }

    uv |= Wire.read();

    return uv;
}

bool SmarTC_VEML6070::setACK(bool active, bool steps)
{
    cmd_buffer.bitfield.ACK = active;
    cmd_buffer.bitfield.ACK_THD = steps;

    clearInt();
    if (!write())
    {
        Serial.println("Fail to write ACK settings");
        return false;
    }

    return true;
}
