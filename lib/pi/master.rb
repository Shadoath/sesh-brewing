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
$message = String.new
length_string = String.new
$start_of_text = 2.chr
$end_of_text   = 3.chr
$device_id     = 254    #Pi IP (SELF)
$seperator     = ':'

$arduinos      = []
puts "Connection established to rs485 network at 9600 baud"
puts "send 'exit' from another device on the rs485 network to close this program"

#params for GPIO
$enable_pin = PiPiper::Pin.new(:pin => 18, :direction => :out)
$enable_pin.off					#initialize in off state






#function to echo the message
def send(recipient, msg)
	output_buffer = String.new
	output_buffer << $start_of_text << to_id_string(recipient) << $seperator << to_id_string($device_id) << $seperator << to_msg_length(msg) << $seperator << msg << $end_of_text

	#echo recieved message back to sender
	$enable_pin.on					  #enable trasmission on RS485 network
	sleep(0.01)					#allow network to establish "transmit" state
	$serial_port.write(output_buffer)
	$enable_pin.off					#disable trasmission on RS485 network

	puts "Sent on rs485: " + output_buffer
end

def to_id_string(id)
	id_string = String.new
	if id < 100 
		id_string << "0"
	end
	if id < 10
		id_string << "0"
	end
	id_string << id.to_s
end

def to_msg_length(msg)
	msg.length < 10 ? "0" + msg.length.to_s : msg.length.to_s
end

#def get_arduinos
#	i = 0
#	while (i< 254)
#		send("RA")
#		puts "getting:"+i+ " "+ read
#		i++
#	end
#end


#function to read the serial buffer
def read
	$message = ""
	hash = String.new						#this is where the address and length data is stored
	if is_message? $serial_port.getbyte()				#getbyte() returns the first byte if it exists or nil if nothing exists
		sleep(0.06)						#delay needed to receive at least 64 chars on hardware buffer, experimentally derrived

		hash = $serial_port.read(10)				#read in all the hash data
			puts hash
		recipient = hash[0..2].to_i
		sender = hash[4..6].to_i
		message_length = hash[8..9].to_i
		$serial_port.getbyte()					#read in seperator and clear it from the buffer

		$message = $serial_port.read(message_length)		#read in the message
		analyze_message($message)				#this function only prints stuff out, at this point
	end
	$message
end

def is_message?(serial_available)
	serial_available.to_i.chr == $start_of_text			#return true or false - must convert char "2" to and integer and then to ascii by .to_i.chr
end

def analyze_message(message)
	puts message.length.to_s + " characters recieved: " + message
	#TODO
	# get Arduino and update values
	#
end

def return_message(message)
	if $serial_port.getbyte() == $end_of_text #Check end char and return
		send($message)
	end
end





#rock the infinite loop, bro
#while true do
#	if read == "exit"
#		exit
#	end
#end
