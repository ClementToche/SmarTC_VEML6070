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
* Public License along with this program.
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

    /* Calculation of refresh time depending on IT settings.
    * See http://www.vishay.com/docs/84277/veml6070.pdf (p8)
    * Calculation is made in ms considering linear function with 
    * 10% additionnal time (because of RSet value worst tolerance)
    * 
    * +-----+-----------+-----------+------------------+
    * | IT  | 300K Rset | 600K Rset |       Coef       |
    * +-----+-----------+-----------+------------------+
    * | H_T |      62.5 |       125 | 0.20833333333333 |
    * | 1_T |       125 |       250 | 0.41666666666667 |
    * | 2_T |       250 |       500 | 0.83333333333333 |
    * | 4_T |       500 |      1000 |  0.6666666666667 |
    * +-----+-----------+-----------+------------------+
    */
    switch (it)
    {
    case VEML6070_H_T:
        i_itv = static_cast<uint>(0.20833333333333 * rset * 1.1);
        break;
    case VEML6070_1_T:
        i_itv = static_cast<uint>(0.41666666666667 * rset * 1.1);
        break;
    case VEML6070_2_T:
        i_itv = static_cast<uint>(0.83333333333333 * rset * 1.1);
        break;
    case VEML6070_4_T:
        i_itv = static_cast<uint>(1.6666666666667 * rset * 1.1);
        break;
    }
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

    Serial.printf("Integration time : %ims\n", i_itv);

    return i_init;
}

bool SmarTC_VEML6070::clearInt()
{
    Wire.begin();

    Serial.printf("requestFrom: %04x\n", VEML6070_ADDR_ARA);
    if (0 != Wire.requestFrom(VEML6070_ADDR_ARA, 1))
    {
        // No value is got from this command.
        Serial.println("Error in initialization");
        return false;
    }

    return true;
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

uint16_t SmarTC_VEML6070::getUV()
{
    if (millis() - i_last_uvt > i_itv)
    {
        i_last_uvt = millis();
        i_uv = readUV();
    }

    return i_uv;
}

/************************* Private methods *************************/
bool SmarTC_VEML6070::write()
{
    Wire.begin();

    Serial.printf("Begin Trans: %04x\n", VEML6070_ADDR_CMD);
    Wire.beginTransmission(VEML6070_ADDR_CMD);
    Serial.printf("Write : %04x\n", cmd_buffer.buf);
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

uint16_t SmarTC_VEML6070::readUV()
{
    Serial.printf("requestFrom: %04x\n", VEML6070_ADDR_MSB);
    if (1 != Wire.requestFrom(VEML6070_ADDR_MSB, 1))
    {
        Serial.println("Fail to request MSB UV Value");
        return 0;
    }

    uint16_t uv = Wire.read();
    Serial.printf("uvi: %i\n", uv);
    uv <<= 8;

    Serial.printf("requestFrom: %04x\n", VEML6070_ADDR_LSB);
    if (1 != Wire.requestFrom(VEML6070_ADDR_LSB, 1))
    {
        Serial.println("Fail to request LSB UV Value");
        return 0;
    }

    uv |= Wire.read();
    Serial.printf("uvi2: %i\n", uv);

    return uv;
}