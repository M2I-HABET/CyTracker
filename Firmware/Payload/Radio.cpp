/**
 * RADIO.ccp holds all functions related the radio port/module infused inside the LoRa FeatherWing
 * development micro controller.
 */

#include <Arduino.h>
#include "Radio.h"
#include "Data.h"
#include "GPS.h"
#include <RH_RF95.h>
#include "Globals.h"

/**
 * Constructor used to reference all other variables & functions.
 */
RADIO::RADIO()
{

}


/**
 * Parses and returns the radio transmission's Time Stamp (ms).
 *    payload         -> 1
 *    mission_control -> 6
 */
float RADIO::get_radio_timestamp(char buf[], String selector)
{
    if(selector == "payload")
    {
        return (Data.Parse(buf, 1));
    }
    else if(selector == "mc")
    {
        return (Data.Parse(buf, 7));
    }
    else if(selector == "recovery")
    {
        return (Data.Parse(buf, 8));
    }
}


/**
 * Parses and returns the radio transmission's altitude.
 */
float RADIO::get_radio_payload_altitude(char buf[])
{
    return (Data.Parse(buf, 2));
}


/**
 * Parses and returns the radio transmission's latitude.
 */
float RADIO::get_radio_payload_latitude(char buf[])
{
    return (Data.Parse(buf, 3)) / 10000.0;
}


/**
 * Parses and returns the radio transmission's longitude.
 */
float RADIO::get_radio_payload_longitude(char buf[])
{
    return (Data.Parse(buf, 4)) / 10000.0;
}


/**
 * Parses and returns the radio transmission's craft Event.
 */
float RADIO::get_radio_payload_event(char buf[])
{
    return (Data.Parse(buf, 5));
}


/**
 * Parses and returns the radio transmission's craft Event.
 */
float RADIO::get_radio_payload_speed(char buf[])
{
    return (Data.Parse(buf, 6));
}


/**
 * Parses and returns the radio transmission's craft Event.
 */
float RADIO::get_radio_recovery_latitude(char buf[])
{
    return (Data.Parse(buf, 9)) / 10000.0;
}


/**
 * Parses and returns the radio transmission's craft Event.
 */
float RADIO::get_radio_recovery_longitude(char buf[])
{
    return (Data.Parse(buf, 10)) / 10000.0;
}


/**
 * Parses and returns the radio transmission's craft Event.
 */
float RADIO::get_radio_node_reset(char buf[])
{
    return (Data.Parse(buf, 11));
}


/**
 * Parses and returns the radio transmission's reset bit.
 */
float RADIO::get_radio_node_id(char buf[])
{
    return (Data.Parse(buf, 12));
}


/**
 * Assigns correct pins to the radio output port. Tests connections and variables.
 */
void RADIO::initialize()
{
    // Assigns pin to have an output singal connection to the LoRa's radio port.
    pinMode(RFM95_RST, OUTPUT);
    // Sends a high signal to the radio port for intialization.
    digitalWrite(RFM95_RST, HIGH);
    // Adjust the LED to be insync with radio trasmission.
    digitalWrite(RFM95_RST, LOW);
    // 10 millisecond delay to allow for radio setup to complete before next instruction.
    delay(10);
    // Turns the radio output high to compelte setup.
    digitalWrite(RFM95_RST, HIGH);
    // Checks for the creation of the radio object and its physical connection attribute.
    if(!rf95.init())
    {
        // If invalid connection, the program will stall and pulse the onbaord led.
        while (1)
        {
            Data.blink_error_led();
        }
    }
    // Checks the radio objects tuned frequency.
    if(!rf95.setFrequency(RF95_FREQ))
    {
        // If invalid connection, the program will stall and pulse the onbaord led.
        while (1)
        {
            Data.blink_error_led();
        }
    }
    // Sets the max power to be used to in the amplification of the signal being sent out.
    rf95.setTxPower(23, false);
}


/**
 * Manages all radio comms either incoming or outgoing.
 */
void RADIO::manager()
{
  // Reads in radio transmission if available.
  radio_receive();
  // Each of the crafts have # seconds to broadcast.
  if(millis() - broadcast_timer > network_node_delay)
  {
  	// Resets the counter. This disables broadcasting again until # seconds have passed.
    broadcast_timer = millis();
    String packet = construct_network_packet();
  	// Sends the transmission via radio.
  	broadcast(packet);
  }
}


/**
 * Constructs the normal network packet.
 */
String RADIO::construct_network_packet()
{
  // Updates the time object to hold the most current operation time.
    payload_ts = millis()/1000.0;
    // Casting all float values to a character array with commas saved in between values
    // so the character array can be parsed when received by another craft.
    String temp = "";
    temp += "$";
    temp += ",";
    temp += payload_ts;
    temp += ",";
    temp += Gps.payload_altitude;
    temp += ",";
    temp += Gps.payload_latitude * 10000;
    temp += ",";
    temp += Gps.payload_longitude * 10000;
    temp += ",";
    temp += Data.payload_event;
    temp += ",";
    temp += Gps.payload_speed;
    temp += ",";
    temp += mission_control_ts;
    temp += ",";
    temp += recovery_ts;
    temp += ",";
    temp += recovery_latitude * 10000;
    temp += ",";
    temp += recovery_longitude * 10000;
    temp += ",";
    temp += Data.node_reset;
    temp += ",";
    temp += node_id;
    temp += ",";
    temp += "$";
    radio_output = "";
    // Copy contents.
    radio_output = temp;
    Serial.print("Radio Out: ");
    Serial.println(radio_output);
    return temp;
}


/**
 * Responsible for sending out messages via the radio antenna.
 */
void RADIO::broadcast(String packet)
{
    // Converts from String to char array.
    char transmission[packet.length()+1];
    packet.toCharArray(transmission, packet.length()+1);
    // Sends message passed in as paramter via antenna.
    rf95.send((const uint8_t*)transmission, sizeof(transmission));
    // Pauses all operations until the micro controll has guaranteed the transmission of the
    // signal.
    rf95.waitPacketSent();
    Data.blink_send_led();
}


/**
 * Responsible for reading in signals over the radio antenna.
 */
void RADIO::radio_receive()
{
    // Checks if radio message has been received.
    if (rf95.available())
    {
      	// Creates a temporary varaible to read in the incoming transmission.
      	uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      	// Gets the length of the above temporary varaible.
      	uint8_t len = sizeof(buf);
        // Reads in the avaiable radio transmission, then checks if it is corrupt or complete.
        if(rf95.recv(buf, &len))
        {
            // Used to display the received data in the GUI.
            radio_input = (char*)buf;
            Data.blink_receive_led();
            // Conversion from uint8_t to string. The purpose of this is to be able to convert to an
            // unsigned char array for parsing.
            String str = (char*)buf;
            char to_parse[str.length()];
            str.toCharArray(to_parse,str.length());
            // Debugging to the Serial Monitor.
            Serial.print("Radio In: ");
            Serial.println(radio_input);

            // Checks for a valid packet. Only parses contents if valid to prevent
            // data corruption.
            if(Radio.validate_checksum())
            {
                // This whole section is comparing the currently held varaibles from the last radio update
                // to that of the newly received signal. Updates the craft's owned variables and copies
                // down the other nodes varaibles. If the timestamp indicates that this craft currently
                // holds the most updated values for another node (ie: LoRa's time stamp is higher than the
                // new signal's), it replaces those variables+

                // Reads in the time stamp for Mission Control's last broadcast.
                float temp_ts = get_radio_timestamp(to_parse, "mc");
                // Compares the currently brought in time stamp to the one stored onboad.
                if(temp_ts > mission_control_ts)
                {
                    // If the incoming signal has more up-to-date versions, we overwrite our saved version with
                    // the new ones.
                    mission_control_ts = temp_ts;
                }
                temp_ts = 0.0;
                // Reads in the time stamp for Recovery's last broadcast.
                temp_ts = get_radio_timestamp(to_parse, "recovery");
                // Compares the currently brought in time stamp to the one stored onboad.
                if(temp_ts > recovery_ts)
                {
                    // If the incoming signal has more up-to-date versions, we overwrite our saved version with
                    // the new ones.
                    recovery_ts = temp_ts;
                    recovery_latitude = get_radio_recovery_latitude(to_parse);
                    recovery_longitude = get_radio_recovery_longitude(to_parse);
                }
                // Reads in the value associated with the reset. 
                received_reset = get_radio_node_reset(to_parse);
                // Reads in Craft ID to see where signal came from.
                received_id = get_radio_node_id(to_parse);
                // Checks for a value of 1 (reset needs to happen).
                if(received_reset)
                {
                    // Check which node reset bit is bound to.
                    // Mission Control.
                    if(0.9 < received_id && received_id < 1.1)
                    {
                        // Mission Control LoRa has powercycled. 
                        // Clear its time stamp variable to ensure that the 
                        // this node continues to pull in new data.
                        mission_control_ts = 0.0;
                    }
                    // Recovery.
                    else if(2.9 < received_id && received_id < 3.1)
                    {
                        // Recovery LoRa has powercycled. 
                        // Clear its time stamp variable to ensure that the 
                        // this node continues to pull in new data.
                        recovery_ts = 0.0;
                    }
                }
            }
        }
	  }
}


/**
 * Responsible for ensuring that a full packet has been received
 * by validating that the packet begins and ends with the correct
 * symbol '$'.
 */
bool RADIO::validate_checksum()
{
    //blink_led_long();
    // Gets the length of the packet. Non-zero indexed.
    int str_length = radio_input.length();
    // Checks for the correct starting and ending symbols.
    if((radio_input.charAt(0) == '$') && (radio_input.charAt(str_length-1) == '$'))
    {
        // If both are detected, valid packet.
        return true;
    }
    else
    {
        // Otherwise, invalid packet. Prevents the system from
        // attempting to parse its contents.
        return false;
    }
}
