#include <PS2X_lib.h>			// for PS2
#include <SPI.h>				// for PS2
#include <SoftwareSerial.h>		// for HC-12

/*-------------------Preprocessors-----------------------*/
//PS2X
#define PIN_08_PS2X_DTA  8
#define PIN_10_PS2X_COM  10
#define PIN_11_PS2X_ATT  11
#define PIN_12_PS2X_CLK  12

/*--------Variables/Class Declarations-------------------*/
//PS2X
PS2X ps2x;
int ps2x_ErrorCheck = 0;
char vibrate = 0;

//HC-12
SoftwareSerial HC12(2, 3); // HC-12 TX Pin, HC-12 RX Pin

 //analogs
typedef struct analogs {
	 byte leftX;
	 byte leftY;
	 byte rightX;
	 byte rightY;
};
//digitals
typedef union digitalKeySet1 {
	struct {

		byte up			: 1;
		byte down		: 1;
		byte left		: 1;
		byte right		: 1;
		byte triangle	: 1;
		byte cross		: 1;
		byte square		: 1;
		byte circle		: 1;
	}bits;
	byte onoff;
};
//digitals
typedef union digitalKeySet2 {
	struct {

		byte left1		: 1;
		byte left2		: 1;
		byte left3		: 1;
		byte right1		: 1;
		byte right2		: 1;
		byte right3		: 1;
		byte select		: 1;
		byte start		: 1;
	}bits;
	byte onoff;
};

struct keys {
	analogs analogKeys;
	digitalKeySet1 digiSet1;
	digitalKeySet2 digiSet2;
};

keys keyValues;

void setup()
{
	// enable serial monitor for debugging
	Serial.begin(9600);

	// PS2X setup
	ps2x_ErrorCheck = ps2x.config_gamepad(PIN_12_PS2X_CLK, PIN_10_PS2X_COM,
		PIN_11_PS2X_ATT, PIN_08_PS2X_DTA, false, true);   //setup pins and settings:  GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
	switch (ps2x_ErrorCheck) {
	case 0:
		Serial.println("Found Controller, configured successful");
		Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
		Serial.println("holding L1 or R1 will print out the analog stick values.");
		Serial.println("Go to www.billporter.info for updates and to report bugs.");
		break;
	case 1:
		Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
		break;
	case 2:
		Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
		break;
	case 3:
	default:
		Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
		break;
	}

	memset(&keyValues, 0, sizeof(struct keys));		// intialize struct to 0
	//memset(&prevKeyValues, 0, sizeof(struct keys));  // intialize struct to 0	

	//HC-12 Setup
	HC12.begin(9600);               // Serial port to HC12
									//pinMode(4, OUTPUT);			//uncomment when sending AT command
									//digitalWrite(4, LOW);
}

void loop()
{
	// PS2X read gamepad values:
	gameController_Reading();
	//ps2x.read_gamepad(200, 200);          //read controller and set large motor to spin at 'vibrate' speed

	// HC-12 commands
	txRxCommand();
}

//Read the controllers inputs
void gameController_Reading()
{
	ps2x.read_gamepad(false, false);  //read controller but set rumble motor to off

									  //analog readings
	keyValues.analogKeys.leftX = ps2x.Analog(PSS_LX);	// left analog stick
	keyValues.analogKeys.leftY = ps2x.Analog(PSS_LY);
	keyValues.analogKeys.rightX = ps2x.Analog(PSS_RX);	// right analog stick
	keyValues.analogKeys.rightY = ps2x.Analog(PSS_RY);

	// digital readings
	if (ps2x.NewButtonState()) {
		// use masking to toggle the value

	// digital kesy set 1
		if (ps2x.ButtonPressed(PSB_PAD_UP))		keyValues.digiSet1.bits.up		=	toggleDigitalKeySet(keyValues.digiSet1.onoff, 0x01);
		if (ps2x.ButtonPressed(PSB_PAD_DOWN))	keyValues.digiSet1.bits.down	=	toggleDigitalKeySet(keyValues.digiSet1.onoff, 0x02);
		if (ps2x.ButtonPressed(PSB_PAD_LEFT))	keyValues.digiSet1.bits.left	=	toggleDigitalKeySet(keyValues.digiSet1.onoff, 0x04);
		if (ps2x.ButtonPressed(PSB_PAD_RIGHT))	keyValues.digiSet1.bits.left	=	toggleDigitalKeySet(keyValues.digiSet1.onoff, 0x08);
		
		// Momentary buttons triangle, cross, square, circle
		if (ps2x.ButtonPressed(PSB_TRIANGLE))	keyValues.digiSet1.bits.triangle =	toggleDigitalKeySet(keyValues.digiSet1.onoff, 0x10);
		if (ps2x.ButtonReleased(PSB_TRIANGLE))	keyValues.digiSet1.bits.triangle =	toggleDigitalKeySet(keyValues.digiSet1.onoff, 0x10);
		if (ps2x.ButtonPressed(PSB_CROSS))		keyValues.digiSet1.bits.cross	 =	toggleDigitalKeySet(keyValues.digiSet1.onoff, 0x20);
		if (ps2x.ButtonReleased(PSB_CROSS))		keyValues.digiSet1.bits.cross	 =	toggleDigitalKeySet(keyValues.digiSet1.onoff, 0x20);
		if (ps2x.ButtonPressed(PSB_SQUARE))		keyValues.digiSet1.bits.square	 =  toggleDigitalKeySet(keyValues.digiSet1.onoff, 0x40);
		if (ps2x.ButtonReleased(PSB_SQUARE))	keyValues.digiSet1.bits.square	 =	toggleDigitalKeySet(keyValues.digiSet1.onoff, 0x40);
		if (ps2x.ButtonPressed(PSB_CIRCLE))		keyValues.digiSet1.bits.circle	 =	toggleDigitalKeySet(keyValues.digiSet1.onoff, 0x80);
		if (ps2x.ButtonReleased(PSB_CIRCLE))	keyValues.digiSet1.bits.circle	 =	toggleDigitalKeySet(keyValues.digiSet1.onoff, 0x80);



	// digital kesy set 2

		if (ps2x.ButtonPressed(PSB_L1))			keyValues.digiSet2.bits.left1	 = toggleDigitalKeySet(keyValues.digiSet2.onoff, 0x01);
		if (ps2x.ButtonPressed(PSB_L2))			keyValues.digiSet2.bits.left2	 = toggleDigitalKeySet(keyValues.digiSet2.onoff, 0x02);
		if (ps2x.ButtonPressed(PSB_L3))			keyValues.digiSet2.bits.left3	 = toggleDigitalKeySet(keyValues.digiSet2.onoff, 0x04);

		if (ps2x.ButtonPressed(PSB_R1))			keyValues.digiSet2.bits.right1	 = toggleDigitalKeySet(keyValues.digiSet2.onoff, 0x08);
		if (ps2x.ButtonPressed(PSB_R2))			keyValues.digiSet2.bits.right2	 = toggleDigitalKeySet(keyValues.digiSet2.onoff, 0x10);
		if (ps2x.ButtonPressed(PSB_R3))			keyValues.digiSet2.bits.right3   = toggleDigitalKeySet(keyValues.digiSet2.onoff, 0x20);

		if (ps2x.ButtonPressed(PSB_SELECT))		keyValues.digiSet2.bits.select	 = toggleDigitalKeySet(keyValues.digiSet2.onoff, 0x40);
		if (ps2x.ButtonPressed(PSB_START))		keyValues.digiSet2.bits.start	 = toggleDigitalKeySet(keyValues.digiSet2.onoff, 0x80);
	}

	//Debug part
	Serial.print("Left Analog X:	"); Serial.print(keyValues.analogKeys.leftX);
	Serial.print("	Left Analog Y:	"); Serial.print(keyValues.analogKeys.leftY);
	Serial.print("	Right Analog X:	"); Serial.print(keyValues.analogKeys.rightX);
	Serial.print("	Right Analog Y:	"); Serial.println(keyValues.analogKeys.rightY);
	Serial.print("	DIGITAL KEYS	"); Serial.println(keyValues.digiSet1.onoff);
	Serial.print("	DIGITAL KEYS	"); Serial.println(keyValues.digiSet2.onoff);

}

int toggleDigitalKeySet(byte keys, byte mask) {
	if ((keys & mask) == 0) {
		return 1;
	}
	else {
		return 0;
	}
}

void txRxCommand() {

	//sending: always do as fast as possible
	HC12.write(keyValues.analogKeys.leftX);		delay(1);
	HC12.write(keyValues.analogKeys.leftY);		delay(1);
	HC12.write(keyValues.analogKeys.rightX);	delay(1);
	HC12.write(keyValues.analogKeys.rightY);	delay(1);
	HC12.write(keyValues.digiSet1.onoff);		delay(1);
	HC12.write(keyValues.digiSet2.onoff);		delay(1);

	// receiving: do only every second
	if (HC12.available()) {}
}