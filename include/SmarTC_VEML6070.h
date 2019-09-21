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
 * @defgroup SmarTC_VEML6070 "SmarTC VEML6070"
 *
 * @brief VEML6070 UV Sensor management library over Arduino SDK. Check VEML6070 Datasheet 
 * for detailed information : http://www.vishay.com/docs/84277/veml6070.pdf
 */

/**
 * @file SmarTC_VEML6070.h
 * 
 * @brief Header for VEML6070 UV Sensor management.
 * @ingroup SmarTC_VEML6070
 * 
 * @author Clément TOCHE
 * Contact: ad@toche.fr
 */

/**
 * @class SmarTC_VEML6070
 *
 * @ingroup SmarTC_VEML6070
 *
 * @brief VEML6070 Manager class
 *
 */

#ifndef SMARTC_VEML6070_H
#define SMARTC_VEML6070_H

#include <Arduino.h>

// Slave addresses. From http://www.vishay.com/docs/84310/designingveml6070.pdf (p11)
#define VEML6070_ADDR_ARA (0x18 >> 1)
#define VEML6070_ADDR_CMD (0x70 >> 1)
#define VEML6070_ADDR_LSB (0x71 >> 1)
#define VEML6070_ADDR_MSB (0x73 >> 1)

/**
 * @brief Refresh Time value (see http://www.vishay.com/docs/84277/veml6070.pdf (p8))
 * 
 * As we use a bitfield, we directly interrat with IT0 and IT1 register.
 */
typedef enum veml6070_it
{
    VEML6070_H_T = 0x00, ///< Half Time. Equal 0x00 in full buffer.
    VEML6070_1_T = 0x01, ///< Simple Time. Equal 0x04 in full buffer.
    VEML6070_2_T = 0x02, ///< Double Time. Equal 0x08 in full buffer.
    VEML6070_4_T = 0x03, ///< Quadruple Time. Equal 0x0C in full buffer.
} veml6070_it_e;

// TODO: Consider integration time when requesting a value. Only get value when IT has been reach depending on IT settings and RSet value.
// TODO: Debug trace library.

class SmarTC_VEML6070
{
public:
    /**
    * @brief Construct a new SmarTC_VEML6070 object with default value.
    * Using default internal values :
    * * IT = VEML6070_4_T
    * * RSET = 300
    */
    SmarTC_VEML6070();

    /**
     * @brief Construct a new SmarTC_VEML6070 object by setting Integration Time and RSet value
     * 
     * @param it Integration Time value.
     * @param rset Rset value of the HW design.
     */
    SmarTC_VEML6070(veml6070_it_e it, int rset);

    /**
     * @brief Launch communication with the VEML6070 component and reset his state.
     * 
     * @return true Init is successful
     * @return false Init failed
     */
    bool launch();

    /**
     * @brief Clear interrupt status of the VEML6070
     * 
     * @return true Interruption has been cleared
     * @return false Process failed
     */
    bool clearInt();

    /**
     * @brief 
     * 
     * @param enable true to shutdown the module, false to wakeup
     * 
     * @return true Shutdown request succeed.
     * @return false Shutdown request failed.
     */
    bool shutDown(bool enable);

    /**
     * @brief Read UV Sensor value
     * 
     * @return uint16_t The UV value read from the sensor
     */
    uint16_t readUV();

    /**
     * @brief Set/Unset ACK (i.e. interrupt) feature of the component.
     * 
     * @param active true to active interrupt mode, false to deactive.
     * @param steps false to set 102 steps as threshold value, true to set it to 145 steps.
     * 
     * @return true Command request succeed.
     * @return false Command request failed.
     */
    bool setACK(bool active, bool steps);

    /**
     * @brief Destroy the SmarTC_VEML6070 object
     */
    ~SmarTC_VEML6070();

private:
    veml6070_it_e i_it; ///< Integration time value
    int i_rset;         ///< Rset resistance value in KOhm
    bool i_init;        ///< Does the lib has been

    /**
     * @brief Data buffer definition for command. See http://www.vishay.com/docs/84277/veml6070.pdf (p6)
     */
    typedef union {
        struct
        {
            byte SD : 1;      ///< Standby Device
            byte RES : 1;     ///< Reserved, must always be 1
            byte IT : 2;      ///< Integration time
            byte ACK_THD : 1; ///< Aknowledge threshold settings
            byte ACK : 1;     ///< Aknowledge activity settings
        } bitfield;
        byte buf;
    } cmd_buffer_t;

    cmd_buffer_t cmd_buffer;

    bool write();
};

#endif // SMARTC_VEML6070_H