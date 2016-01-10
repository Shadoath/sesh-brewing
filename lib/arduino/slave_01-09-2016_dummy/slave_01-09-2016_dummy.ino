#include <SoftwareSerial.h>

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

String DummyTemperature = "123.45";

SoftwareSerial RS485(RS485RX, RS485TX); // RX, TX

int debug = 0;

int DeviceID = 0;



void setup() { 
  Serial.begin(9600);
  RS485.begin(9600);
  delay(100);
  Serial.println("Connection established at 9600 baud between SLAVE and COMPUTER.");
  Serial.println("Type 'debug1!' to turn on debug output. 'debug0!' to turn off debug output.");
  Serial.println("Type a command terminated by a '!'");
  Serial.println("Type 'help!' for a list of commands.");                                        //For later use when system is working
}




void loop() {
  message = receive_message();
  if( message != "" ) process_rs485_message(message);
  
  ui_message = receive_ui_message();
  if( ui_message != "" ) process_ui_message(ui_message);
}




void process_rs485_message(String message){
  String debugString = "";    
  String newMessage = "";     //used for sending new messages generated form this function

  if( message.length() < 9 ) return;                          //Check for correct message length ( at least "To(3):From(3):Command(1) ..." ) before analyzing

  int sender = message.substring(4,3).toInt();
  
  if( message.substring(0,3).toInt() == DeviceID )
    switch(message[8])
    {
      case 'C':                                                //P for Program mode
        switch(message[9])
        {
          case 'O':                                            //O for Self Organize (auto addressing)
            switch(DeviceID)                                   //check device id (self) to determine what to do
            {
              case 0:
                //coordinate pinging to other arduinos
                //self assign address
                //save address
                //send new address back to PI (254)
                
                break;
              default:
                //wait for ping, then send back self address
                respond_to_ping();
                break;
            }
            break;
        }
        break;
        case 'R':
          switch(message[9]){
            case 'T':
              //send currently read temperature back to sender
              newMessage = 'T' + DummyTemperature;
              debugString = "Sending current temperature reading: " + newMessage;
              send_message(sender, newMessage);
              break;
          }
          break;
    }
  
  if( debug == 1 ) Serial.println(debugString);
}





void respond_to_ping(){
  String sender = "";
  unsigned long now = millis();
  unsigned long start_time = now;
  unsigned long time_out = 3000;                              //3 seconds should be more than sufficient to get each ping out from the new device
  int original_sender;
  
  //wait for the message to arrive, and keep checking until it does or we hit timeout
  while( message.length() == 0 || now-start_time < time_out )
  {
    now = millis();
    message = receive_message();
  }

  if( message.substring(0,3).toInt() == DeviceID && message.substring(8,16) == "Please Come Back" )
  {
    if( DeviceID < 100 ) sender += '0';
    if( DeviceID < 10 ) sender += '0';
    sender += String(DeviceID);
    
    original_sender = message.substring(4,3).toInt();
    send_message(original_sender, sender);
  }
}









/*********************************************************************************************************
* This function interprets the string sent by the user and executes requested commands.
*********************************************************************************************************/
void process_ui_message(String ui_message){
  //check for debug command
  if( ui_message.length() > 5 )                                                                                                   //make sure the length of the full debug command is present, so 'debug' plus one more char
    if( ui_message.substring(0,5) == "debug" && (ui_message.substring(5).toInt() == 0 || ui_message.substring(5).toInt() == 1) )  //make sure the message consists of the debug option and a binary value, then assign it
    {
      debug = ui_message.substring(5).toInt();                                                                                    //the 6th index (0,1,2,3,4,5) of the string is 5, containing the new debug mode char
      Serial.print("Debug: "); Serial.println(debug);    
    }

  send_message(254, ui_message);
}




/*********************************************************************************************************
* This function checks for the "start of text" ascii char <2> and then reads in each char to a string.
* The function breaks out of reading into the string if the "end of text" char <3> is read or the maximum buffer size is reached
* THIS IS A BLOCKING FUNCTION: once characters have been read but no end_ui character is set, it blocks.
*********************************************************************************************************/
String receive_message(){
  char incoming_char;
  int max_size = 64;                                                        //size of serial buffer on UNO
  String message_string = "";
  unsigned long now = 0;
  unsigned long start_read = 0;
  unsigned long read_timout = 500; //ms
  
  
  if( RS485.available() )
  { 
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
      Serial.print(message_string.length()); Serial.print(" chars from rs485: "); Serial.println(message_string);  
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
  int max_size = 64 - 5;                                                  //size of serial buffer on UNO - hash
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

  if( DeviceID < 100 ) sender += '0';
  if( DeviceID < 10 ) sender += '0';
  sender += String(DeviceID);
  
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
  delay(5);
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
