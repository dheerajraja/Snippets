 /***************************************************************************************
 * Copyright 2014 Infineon Technologies AG ( www.infineon.com ).                       *
 * All rights reserved.                                                                *
 *                                                                                     *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,            *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,           *
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  IN NO       *
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,                 *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;         *
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY             *
 * WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR            *
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF              *
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                          *
 *                                                                                     *
 ***************************************************************************************/

 
/** @cond INCLUDE_FILE_DESCRIPTION */
    /**
     ** @file IFX_I2C_Defaulthandlers.c
     **
     ** @brief Implementation of callback functions of the I2C-Slave driver
     **
     ** @version 2.0.0.0 
     **
     ** @date 10 Feb. 2015                                           
     **
     ** @author R. Goetze                                                         **
     **/
/** @endcond */

/**
 * @defgroup IFX_I2C_GROUP Inter-Integrated-Circuit Bus Slave Module (I2C-Slave)
 * @{
 */

/*  includes
****************************************************************/
#include <string.h>
#include "IFX_Framework.h"

#if IFX_CFG_I2C || defined(DOXYGEN)

		#include "IFX_I2c_Config.h"
		#include "IFX_I2c.h"
		
#ifdef IFX_CFG_KERNEL
		#include "IFX_Event_Manager.h"
    #include "IFX_Clk_Pwr.h"
#else
		UINT16 i2c_reception_complete;
		UINT16 i2c_transmission_complete;
		UINT16 i2c_error;
		UINT16 i2c_still_transmitting;
		UINT16 initialised;
		UINT32 NORETURN;

    /**
     ** @brief Initializes global I2C statemarkers
     **                                     
     ** @version 2.0.0.0 
     **                                     
     ** @date 10 Feb. 2015 
     **                                     
     ** @param[in] error - the error code found in I2C interrupt service routine 
     **         
     ** @details    When the error check inside the I2C interrupt service routine has detected a problem, the callback function 'DefaultErrorHandler(void)' is automatically started. 
     **            The user is asked to extend this routine and to implement all actions that are required by his specific application upon different 'error' scenarios.
     **/                                       
    void I2c_InitDefaultHandlers(void) 
    {
        ENTER_CRITICAL_SECTION_PRIV();
        i2c_reception_complete = 0;
        i2c_transmission_complete = 0;
        i2c_error = 0;
			  i2c_still_transmitting = 1;
				initialised = 0xABAB;
			  NORETURN = 0xAA;
        LEAVE_CRITICAL_SECTION_PRIV();
	}	

#endif

    /**
     ** @brief a callback function that is automatically invoked inside the I2C interrupt service routine upon an error condition
     **                                     
     ** @version 2.0.0.0 
     **                                     
     ** @date 10 Feb. 2015 
     **                                     
     ** @param[in] error - the error code found in I2C interrupt service routine 
     **         
     ** @details    When the error check inside the I2C interrupt service routine has detected a problem, the callback function 'DefaultErrorHandler(void)' is automatically started. 
     **            The user is asked to extend this routine and to implement all actions that are required by his specific application upon different 'error' scenarios.
     **/                                       
    void I2c_DefaultErrorHandler(void) 
    {
        // I2C Errors happend - return all buffer handles (TRM/REC) to application 
        // as I2C module will be reset (clears buffers and error states) after callback

        // application callback should return to protocol recovery state (e.g. frame repetition)
        #ifdef IFX_CFG_KERNEL	
            IFX_Evm_FireEventFromIsr(IFX_EV_I2C_ERROR);
			
						/* after error state, signal the system that deep sleep is possible */
						IfxRequestDeepSleepMode(IFX_INT_I2C);
        #else
            i2c_error = 1;
        #endif
    }

    

    /**
     ** @brief a callback function that is automatically invoked inside the I2C interrupt service routine upon an unexpected frame start  
     **                                     
     ** @version 2.0.0.0 
     **                                     
     ** @date 10 Feb. 2015 
     **                                     
     ** @param[in] error - the error code found in I2C interrupt service routine 
     **         
     ** @details    When the error check inside the I2C interrupt service routine has detected an unexpected frame start, the callback function 'DefaultFrameStartHandler(void)'  
     **            is automatically started. The user is asked to extend this routine and to implement all actions that are required by his specific application upon this error situation.
     **/                                       
    void I2c_DefaultFrameStartHandler(void) 
    {   // I2C Frame Start ocurred - normally not needed but can be used to detect Host requests
        // while peripheral is muted (during Late-Acknowledgement-ON) - place your code here !!
			#ifdef IFX_CFG_KERNEL	
				/* within I2C frame, signal the system not to enter deep sleep (wakeup-time-to-NVM-Handler > 100µs) */
				IfxRequestHaltMode(IFX_INT_I2C);
			#endif
    }


    

    /**
     ** @brief a callback function that is automatically invoked inside the I2C interrupt service routine upon a transmission buffer underflow. 
     **                                     
     ** @version 2.0.0.0 
     **                                     
     ** @date 10 Feb. 2015 
     **                                     
     ** @param[in] error - the error code found in I2C interrupt service routine 
     **         
     ** @details    When the error check inside the I2C interrupt service routine has detected that all transmission data bytes have already been sent and a further byte transmission is tried, the callback function 'DefaultTrmEndHandler(void)' 
     **            is automatically started. The user is asked to extend this routine and to implement all actions that are required by his specific application upon this situation.
     **/                                       
    void I2c_DefaultTrmEndHandler(void)
    {   // Transmission Frame Ended;
        
        // Call callback into application to return buffer 
        // (ActualSize shows how many bytes were put to I2C_BUF already)
        #ifdef IFX_CFG_KERNEL	
            IFX_Evm_FireEventFromIsr(IFX_EV_I2C_TRM_END);
			
						/* if all data is sent, signal the system that deep sleep is possible */
						IfxRequestDeepSleepMode(IFX_INT_I2C);
        #else
            i2c_transmission_complete = 1;
        #endif     
    }


    

    /**
     ** @brief a callback function that is automatically invoked inside the I2C interrupt service routine upon a receive data buffer overflow.  
     **                                     
     ** @version 2.0.0.0 
     **                                     
     ** @date 10 Feb. 2015 
     **                                     
     ** @param[in] error - the error code found in I2C interrupt service routine 
     **         
     ** @details    When the error check inside the I2C interrupt service routine has detected a receive data buffer overflow, the callback function 'DefaultRecEndHandler(void)' is  
     **            is automatically started. The user is asked to extend this routine and to implement all actions that are required by his specific application upon this situation.
     **/                                       
    void I2c_DefaultRecEndHandler(void)
    {   // Receive Frame Ended

        // call callback into allication to return buffer 
        // (ActualSize shows how many bytes were received)
        #ifdef IFX_CFG_KERNEL	
            IFX_Evm_FireEventFromIsr(IFX_EV_I2C_RCP_END);
			
						/* after successful frame reception, signal the system that deep sleep is possible */
						IfxRequestDeepSleepMode(IFX_INT_I2C);
        #else
            i2c_reception_complete = 1;
        #endif  
    }
        

#endif // #if IFX_CFG_I2C_PORT || defined(DOXYGEN)

/** @} */ // end of IFX_I2C_GROUP


