#include <EEPROM.h>
#include <DHT.h>
#include <SoftwareSerial.h>

#define DHTPIN A5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
float temperature_C = 0;
float humidity = 0;

#define RS485RX          2
#define RS485TX          3
#define TxControl        4               //RS485 Direction control
#define Transmit         HIGH
#define Receive          LOW

String message;
String ui_message;
char start_of_text = 2;
char end_of_text = 3;
char seperator = 58;    // ':'
char end_ui = 33;   // '!'

SoftwareSerial RS485(RS485RX, RS485TX); // RX, TX

/*********EEPROM Elements*********/
struct structEEPROM
{
    double crc;                                        //Cycle Reduncancy Check - used to check if memory is intact. 
    int Self_Address;
};

union
{
    structEEPROM settings;
    char eeArray[sizeof(structEEPROM)];
} EEPROMData;
/*********************************/

int debug = 1;

unsigned long now = 0;
unsigned long last_read = 0;
unsigned long sample_time = 2000;

int MASTER = 254;





void setup() { 
  Serial.begin(9600);
  
  RS485.begin(9600);
  pinMode(TxControl, OUTPUT);
  digitalWrite(TxControl, Receive);
  
  dht.begin();
  
  Serial.println("Connection established at 9600 baud between SLAVE and COMPUTER.");
  Serial.println("Type 'debug1!' to turn on debug output. 'debug0!' to turn off debug output.");
  Serial.println("Type a command terminated by a '!'");
  Serial.println("Type 'help!' for a list of commands.");                                        //For later use when system is working
  
  readEEPROM();
  if(EEPROMData.settings.crc != 1234.56) setDefaultEEPROM();
}








void loop() {
  now = millis();
  get_temperatureC();
  get_humidity();
  
  message = read_rs485_message();
  if( message != "" ) process_rs485_message(message);
  
  ui_message = receive_ui_message();
  if( ui_message != "" ) process_ui_message(ui_message);
}






void get_temperatureC(){
  if( now-last_read > sample_time ) temperature_C = dht.readTemperature();
}

void get_humidity(){
  if( now-last_read > sample_time )
  {
    humidity = dht.readHumidity();
    last_read = now;
  }
}








void process_rs485_message(String message){
  String debugString = "";    
  String newMessage = "";                                     //used for sending new messages generated form this function to rs485 network

  if( message.length() < 10 ) return;                          //Check for correct message length ( at least "To(3):From(3):Command(1) ..." ) before analyzing

  int sender = message.substring(4,7).toInt();
  Serial.println("string at (14,17):");
  Serial.println(message.substring(14,17));
  
  if( message.substring(0,3).toInt() == EEPROMData.settings.Self_Address || message.substring(11,13) == "CO")
    switch(message[11])
    {
      case 'C':                                                //C for Command
        switch(message[12])
        {
          case 'A':
            //make system active or passive (on or off)
            //look for true/false or 1/0 in rest of message
            break;
          case 'I':
            save_new_self_address( message.substring(14,17).toInt() );
            break;
          case 'S':
            //assign setpoint sent in rest of message
            break;
          case 'O':                                            //O for Self Organize (auto addressing)
            switch(EEPROMData.settings.Self_Address)                               //check device id (self) to determine what to do
            {
              case 0:
                //coordinate pinging to other arduinos
                //self assign address
                //save address
                //send new address back to PI (254)
                save_new_self_address( find_open_slave_address() );
                break;
              default:
                //wait for ping, then send back self address
                wait_for_ping();
                break;
            }
            break;
        }
        break;
        case 'R':                                                 //R for Request
          switch(message[12]){
            case 'T':                                             //T for temperature
              //send currently read temperature back to sender
              newMessage = 'T' + String(temperature_C);
              debugString = "Sending current temperature reading: " + newMessage;
              send_message(sender, newMessage);
              break;
            case 'H':                                             //H for humidity
              //send currently read temperature back to sender
              newMessage = 'H' + String(humidity);
              debugString = "Sending current humidity reading: " + newMessage;
              send_message(sender, newMessage);
              break;
          }
          break;
    }
  
  if( debug == 1 ) Serial.println(debugString);
}








int find_open_slave_address(){
  String received_message = "";
  unsigned long timeout = 1000;
  int i;
  boolean break_out = false;
  
  //order of commands makes a big difference, here
  for(i=1; i<254; i++)
  {
    last_read = millis();                                           //Start timeout timer for each itteration of i
    send_message(i, "Present?");                                    //Send Ping
    
    while( received_message != "Present" )                          //Test for response to Ping
    {
      now = millis();
      if( now-last_read > timeout )                                 //Test for no response within timeout period
      {
        if( debug == 1 )
        {
          Serial.print("Timed-out while waiting for response from device ");
          Serial.println(i);
        }
        break_out = true;                                           //get out of the while loop and use i value for last available device ID on network
        break;
      }
      received_message = read_rs485_message();
      if( received_message.length() == 18 )                         //test for adequate response length
        received_message = received_message.substring(11, 7);       //update received_message 
    }
    
    if( break_out ) break;                                          //get out of FOR loop
  }
  
  if( i == 253 ) return 0;                                                          //network is full of devices, do nothing
  return i;
}


void save_new_self_address(int new_address){
  if( debug == 1 )
  {
    Serial.print("Assigning new Self-Address: ");
    Serial.println(new_address);
  }
  EEPROMData.settings.Self_Address = new_address;
  writeEEPROM();                                     //save to EEPROM
  
  //Send update to MASTER
  send_message(MASTER, "NewAddress:" + String(EEPROMData.settings.Self_Address));
}






/*********************************************************************************************************
* This function waits for the ping from a new slave arduino and responds with it's address.
* It is only called if the EEPROMData.settings.Self_Address is already assigned and therefore not 0.
*
* Edit the "timeout" var based on how long it would take to find the last device on the network.
*
*********************************************************************************************************/
void wait_for_ping(){
  String received_message = ""; 
  unsigned long timeout = 10000;
  
  if( debug == 1 ) Serial.println("Waiting for ping from new device.");
  now = millis();
  last_read = now;
  while( received_message == "" && now-last_read < timeout )
  {
    received_message = read_rs485_message();
    now = millis();
  }
  
  //is it for me?
  //if so, respond
  if( received_message.length() == 19 )
    if( received_message.substring(0,3).toInt() == EEPROMData.settings.Self_Address && received_message.substring(11,8) == "Present?")
    {
      send_message(received_message.substring(4,3).toInt(), "Present");
      if( debug == 1 ) Serial.println("Responding to ping: Present");
      return;
    }
  if( debug == 1 ) Serial.println("Timed-out while waiting for ping.");
}








/*********************************************************************************************************
* This function interprets the string sent by the user and executes requested commands.
*********************************************************************************************************/
void process_ui_message(String ui_message){
  String msg_string = "";
  
  //test for message length before trying to read substrings
  switch(ui_message.length()){
    case 4:
      if( ui_message.substring(0,4) == "help" )
      {
        msg_string += "List of commands:\n";
        msg_string += " help!\n";
        msg_string += " exit!\n";
        msg_string += " debug<1,0>!\n";
        msg_string += " address!\n";
        msg_string += " humidity!\n";
        msg_string += " temperature!\n";
        msg_string += " self organize!";
      }
      if( ui_message.substring(0,4) == "exit" )
      {
        msg_string = "Sent exit command to MASTER device...";
        send_message(254, ui_message);
      }      
      break;
    case 6:
      if( ui_message.substring(0,5) == "debug" && (ui_message.substring(5).toInt() == 0 || ui_message.substring(5).toInt() == 1) )  //make sure the message consists of the debug option and a binary value, then assign it
      {
        debug = ui_message.substring(5).toInt();                                                                                    //the 6th index (0,1,2,3,4,5) of the string is 5, containing the new debug mode char
        msg_string = "Debug: " + String(debug);    
      }else if( ui_message.substring(0,5) == "test:" )
      {
        send_message( ui_message.substring(5).toInt(), "RT");
      }else msg_string = "Invalid input, type 'help!'";
      break;
    case 7:
      if( ui_message.substring(0,7) == "address" )
      {
        msg_string = "Self_Adderss:" + String(EEPROMData.settings.Self_Address);
      }
      break;
    case 8:
      if( ui_message.substring(0,8) == "humidity" )
      {
        msg_string = "Humidity (%): " + String(humidity);
      }
      break;
    case 11:
      if( ui_message.substring(0,11) == "temperature" )
      {
        msg_string = "Temperature (C): " + String(temperature_C);
      }
      break;
    case 13:
      if( ui_message.substring(0,13) == "self organize" )
      {
        if( EEPROMData.settings.Self_Address == 0 )
        {
          save_new_self_address( find_open_slave_address() );
          msg_string = "Established Self_Address as " + String(EEPROMData.settings.Self_Address);
        }else{
          msg_string = "Self_Address already assigned as " + String(EEPROMData.settings.Self_Address);
        }
      }
      break;
  }
  
    
  if( msg_string != "" ) Serial.println(msg_string);
}




/*********************************************************************************************************
* This function checks for the "start of text" ascii char <2> and then reads in each char to a string.
* The function breaks out of reading into the string if the "end of text" char <3> is read or the maximum buffer size is reached
* THIS IS A BLOCKING FUNCTION: once characters have been read but no end_ui character is set, it blocks.
*********************************************************************************************************/
String read_rs485_message(){
  char incoming_char;
  int max_size = 64;                                                        //size of serial buffer on UNO
  String message_string = "";
  unsigned long now = 0;
  unsigned long start_read = 0;
  unsigned long read_timout = 500; //ms
  
  
  if( RS485.available() )
  {
    delay(60);                                                              //time it takes to read in up to 64 chars from the network, derrived experimentally
    if( RS485.read() == start_of_text )
    {
      start_read = millis();                                                //Set start time of string reading
      while( message_string.length() <= max_size )                          //end the while loop if the "end of text" char is read or buffer maximum is reached or reading times out
      {
        now = millis();                                                     //Update last time of string reading
        if( now - start_read > read_timout )                                //Stop reading if timeout is achieved, return empty string
        {
          message_string = "";
          break;
        }
      
        if( RS485.available() > 0 )                                         //Check for available char on serial buffer, read it in and analyze for end_of_text char or add it to the message_string
        {
          incoming_char = RS485.read();
          if( incoming_char == end_of_text ) break;
          else
          {
            message_string += incoming_char;
          } 
        }
      }
    }
    if( debug == 1 )
    {
      Serial.print("Recieved "); Serial.print(message_string.length()); Serial.print(" chars from rs485: "); Serial.println(message_string);  
    }
  }
  return message_string;
}




/*********************************************************************************************************
* This function checks for any characters available on from the serial console and then reads in each char to a string.
* The function breaks out of reading into the string if the "end of text" char <.> is read or the maximum buffer size is reached.
* THIS IS A BLOCKING FUNCTION: once characters have been read but no end_ui character is set, it blocks.
*********************************************************************************************************/
String receive_ui_message(){
  char incoming_char;
  int max_size = 64 - 12;                                                  //size of serial buffer on UNO - hash
  String message_string = "";
  unsigned long now = 0;
  unsigned long start_read = 0;
  unsigned long read_timout = 500; //ms
  
  if( Serial.available() > 0 )
  { 
    start_read = millis();                                                //Set start time of string reading
    while( message_string.length() <= max_size )                          //end the while loop if the "end of text" char is read or buffer maximum is reached
    {
      now = millis();                                                     //Update last time of string reading
      if( now - start_read > read_timout )                                //Stop reading if timeout is achieved, return empty string
      {
        message_string = "";
        break;
      }
        
      if( Serial.available() > 0 )
      {
        incoming_char = Serial.read();
        if( incoming_char == end_ui) break;
        else
        {
          message_string += incoming_char;
        }
      } 
    }
    if( debug == 1 )
    {
      Serial.print(message_string.length()); Serial.print(" chars from console: "); Serial.println(message_string);
    }
  } 
  return message_string;
}




/*********************************************************************************************************
* This function outputs the message, msg, preceded by a start-text char and followed by an end-text char
* Since this will be used on an RS485 network, the enable-transmission line is also controlled.
* --->msg must be smaller than 64 chars
* --->Pi (receiving device) assumes message length will be two chars long, so put a zero in front if it's <10
*********************************************************************************************************/
void send_message(int recipient, String msg){
  String recip = "";
  String sender = "";
  int int_msg_length = msg.length();
  String msg_length = "";
  String output_buffer = "";

  if( recipient < 100 ) recip += '0';
  if( recipient < 10 ) recip += '0';
  recip += String(recipient);

  if( EEPROMData.settings.Self_Address < 100 ) sender += '0';
  if( EEPROMData.settings.Self_Address < 10 ) sender += '0';
  sender += String(EEPROMData.settings.Self_Address);
  
  if(int_msg_length < 10) msg_length += "0";
  msg_length += String(int_msg_length);
  

  output_buffer += start_of_text;
  output_buffer += recip;
  output_buffer += seperator;
  output_buffer += sender;
  output_buffer += seperator;
  output_buffer += msg_length;
  output_buffer += seperator;
  output_buffer += msg;
  output_buffer += end_of_text;
  
  digitalWrite(TxControl, Transmit);
  delay(5);
  RS485.print(output_buffer);
  delay(10);
  digitalWrite(TxControl, Receive);
  
  if(debug == 1)
  {
    Serial.print("Text Sent:  "); Serial.print(output_buffer);
    Serial.print("  ASCII Sent: "); 
    for(int k=0; k<output_buffer.length(); k++)
    {
      Serial.print( int(output_buffer[k]) );
      Serial.print(" ");
    }
    Serial.println();
  }
}











/******************* EEPROM STUFF *******************/
void readEEPROM(){
    for(int i=0; i<sizeof(structEEPROM); i++) EEPROMData.eeArray[i] = EEPROM.read(i);
}

void writeEEPROM(){
    for(int i=0; i<sizeof(structEEPROM); i++) EEPROM.write(i, EEPROMData.eeArray[i]);
}

void setDefaultEEPROM(){
    EEPROMData.settings.crc = 1234.56 ;              //Cycle Reduncancy Check - used to check if memory is intact. 
    EEPROMData.settings.Self_Address = 0;
    
    writeEEPROM();
    readEEPROM();
}
