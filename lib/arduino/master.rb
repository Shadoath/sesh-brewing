#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#
# When restarting this program, first run: $ echo "18" > /sys/class/gpio/unexport
# This will free up the GPIO pin 24 for this program to use
#
# Run on IRB
# sudo irb -r ./master.rb
# UART functionality:
# This program communicates on the PI B+ using hardware UART on physical GPIO pins 8 (TX) and 10 (RX).
# To do this the rubyserial gem is utilized.
#
# RS485 network functionality:
# Before transmission the program brings physical pin 12 high
# After transmission the program brings physical pin 12 low
# To do this the PiPiper gem is installed as in the instructions at: https://github.com/jwhitehorn/pi_piper
#
# Send the rs485 network "exit" to gracefully get out of the program
#
#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

require 'rubyserial'
require 'pi_piper'
include PiPiper

#params for serial port
$serial_port = Serial.new '/dev/ttyAMA0', 9600
message = String.new
length_string = String.new
$start_of_text = 2
$end_of_text   = 3
$sender_ip     = '254'  #Pi IP
$recipient_id  = 1		#Self IP
$seperator     = ':'

puts "Connection established to rs485 network at 9600 baud"
puts "send 'exit' from another device on the rs485 network to close this program"

#params for GPIO
$enable_pin = PiPiper::Pin.new(:pin => 18, :direction => :out)
$enable_pin.off					#initialize in off state






#function to echo the message
def send(msg, en_pin, $serial_port)
	output_buffer = String.new
	output_buffer << $start_of_text << $recipient_id.to_s << $seperator << $sender_ip << $seperator << msg << $end_of_text

	                                        	#echo recieved message back to sender
        en_pin.on					#enable trasmission on RS485 network
	sleep(0.01)					#allow network to establish "transmit" state
        $serial_port.write(output_buffer)
        en_pin.off					#disable trasmission on RS485 network

	puts "Sent on rs485: " + output_buffer
end






#function to read the serial buffer
def read
		$input_buffer = String.new
	  if 	is_message? $serial_port.getbyte()
		    length_string = $serial_port.read(2)              	#get the next two chars in the buffer, which represent the length of the message
		    msg_length = length_string.to_i                 #convert string to integer for next operation
		    if $serial_port.read(1) == $seperator              	#read in seperator char
	          sleep(0.06)				#delay needed to receive at least 64 chars on hardware buffer, experimentally derrived
	          message = $serial_port.read(msg_length)   	#use the received message length the get the actual message
	          puts message.length.to_s + " characters recieved: " + message
		    else
	      		puts "no seperator received after start character and length value"
		    end

				serial_available = $serial_port.getbyte()		#look for end of text char
				if serial_available == $end_of_text
						puts serial_available			#print end of text char
						send(message, $enable_pin, $serial_port)
						$input_buffer = message
				end
    end
    message
end

def is_message?(message)
	if !serial_available.nil?				#look for any chars on the incoming hardware buffer
    puts serial_available                           #print out extra chars that weren't read in the buffer
  end
	serial_available == $start_of_text
end






#rock the infinite loop, bro
#while true do
#	if read(serialport) == "exit"
#		exit
#	end
#end
