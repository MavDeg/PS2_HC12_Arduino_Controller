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
	unsigned int leftX;
	unsigned int leftY;
	unsigned int rightX;
	unsigned int rightY;
};
//digitals
typedef union digitals {
	struct {
		unsigned int l1 : 1;
		unsigned int l2 : 1;
		unsigned int l3 : 1;
		unsigned int r1 : 1;
		unsigned int r2 : 1;
		unsigned int r3 : 1;
		unsigned int up : 1;
		unsigned int dn : 1;
		unsigned int lf : 1;
		unsigned int rt : 1;
		unsigned int tr : 1;
		unsigned int cr : 1;
		unsigned int sq : 1;
		unsigned int cl : 1;
		unsigned int sl : 1;
		unsigned int st : 1;
	}bits;
	unsigned int onoff;
};

struct keys {
	analogs analogKeys;
	digitals digitalKeys;
};

keys keyValues, prevKeyValues;

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
	memset(&prevKeyValues, 0, sizeof(struct keys));  // intialize struct to 0	

													 //HC-12 Setup
	HC12.begin(9600);               // Serial port to HC12
									//pinMode(4, OUTPUT);			//uncomment when sending AT command
									//digitalWrite(4, LOW);
}

void loop()
{
	// PS2X read gamepad values:
	gameController_Reading();
	//delay(50);
	//ps2x.read_gamepad(200, 200);          //read controller and set large motor to spin at 'vibrate' speed

	// HC-12 commands
	sendCommand();
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
		if (ps2x.ButtonPressed(PSB_L1))			keyValues.digitalKeys.bits.l1 = toggleButton(0x0001);
		if (ps2x.ButtonPressed(PSB_L2))			keyValues.digitalKeys.bits.l2 = toggleButton(0x0002);
		if (ps2x.ButtonPressed(PSB_L3))			keyValues.digitalKeys.bits.l3 = toggleButton(0x0004);

		if (ps2x.ButtonPressed(PSB_R1))			keyValues.digitalKeys.bits.r1 = toggleButton(0x0008);
		if (ps2x.ButtonPressed(PSB_R2))			keyValues.digitalKeys.bits.r2 = toggleButton(0x0010);
		if (ps2x.ButtonPressed(PSB_R3))			keyValues.digitalKeys.bits.r3 = toggleButton(0x0020);

		//
		if (ps2x.ButtonPressed(PSB_PAD_UP))		keyValues.digitalKeys.bits.up = toggleButton(0x0040);
		if (ps2x.ButtonPressed(PSB_PAD_DOWN))	keyValues.digitalKeys.bits.dn = toggleButton(0x0080);
		if (ps2x.ButtonPressed(PSB_PAD_LEFT))	keyValues.digitalKeys.bits.lf = toggleButton(0x0100);
		if (ps2x.ButtonPressed(PSB_PAD_RIGHT))	keyValues.digitalKeys.bits.rt = toggleButton(0x0200);

		// Momentary buttons triangle, cross, square, circle
		if (ps2x.ButtonPressed(PSB_TRIANGLE))	keyValues.digitalKeys.bits.tr = toggleButton(0x0400);
		if (ps2x.ButtonReleased(PSB_TRIANGLE))	keyValues.digitalKeys.bits.tr = toggleButton(0x0400);
		if (ps2x.ButtonPressed(PSB_CROSS))		keyValues.digitalKeys.bits.cr = toggleButton(0x0800);
		if (ps2x.ButtonReleased(PSB_CROSS))		keyValues.digitalKeys.bits.cr = toggleButton(0x0800);
		if (ps2x.ButtonPressed(PSB_SQUARE))		keyValues.digitalKeys.bits.sq = toggleButton(0x1000);
		if (ps2x.ButtonReleased(PSB_SQUARE))	keyValues.digitalKeys.bits.sq = toggleButton(0x1000);
		if (ps2x.ButtonPressed(PSB_CIRCLE))		keyValues.digitalKeys.bits.cl = toggleButton(0x2000);
		if (ps2x.ButtonReleased(PSB_CIRCLE))	keyValues.digitalKeys.bits.cl = toggleButton(0x2000);

		if (ps2x.ButtonPressed(PSB_SELECT))		keyValues.digitalKeys.bits.sl = toggleButton(0x4000);
		if (ps2x.ButtonPressed(PSB_START))		keyValues.digitalKeys.bits.st = toggleButton(0x8000);
	}

	//Debug part
	Serial.print("Left Analog X:	"); Serial.print(keyValues.analogKeys.leftX);
	Serial.print("	Left Analog Y:	"); Serial.print(keyValues.analogKeys.leftY);
	Serial.print("	Right Analog X:	"); Serial.print(keyValues.analogKeys.rightX);
	Serial.print("	Right Analog Y:	"); Serial.println(keyValues.analogKeys.rightY);
	Serial.print("	DIGITAL KEYS	"); Serial.println(keyValues.digitalKeys.onoff);
}

int toggleButton(unsigned int mask) {
	if ((keyValues.digitalKeys.onoff & mask) == 0) {
		return 1;
	}
	else {
		return 0;
	}
}

void sendCommand() {

	//if ((keyValues.analogKeys.leftX != prevKeyValues.analogKeys.leftX))		{ prevKeyValues.analogKeys.leftX = keyValues.analogKeys.leftX; 	HC12.write(keyValues.analogKeys.leftX); }   // Send that data to HC-12
	//if ((keyValues.analogKeys.leftY != prevKeyValues.analogKeys.leftY))		{ prevKeyValues.analogKeys.leftY = keyValues.analogKeys.leftY; 	HC12.write(keyValues.analogKeys.leftY); }    // Send that data to HC-12
	//if ((keyValues.analogKeys.rightX != prevKeyValues.analogKeys.rightX))	{ prevKeyValues.analogKeys.rightX = keyValues.analogKeys.rightX; 	HC12.write(keyValues.analogKeys.rightX); }    // Send that data to HC-12
	//if ((keyValues.analogKeys.rightY != prevKeyValues.analogKeys.rightY))	{ prevKeyValues.analogKeys.rightY = keyValues.analogKeys.rightY; 	HC12.write(keyValues.analogKeys.rightY); }   // Send that data to HC-12
	//if ((keyValues.digitalKeys.onoff != prevKeyValues.digitalKeys.onoff))	{ prevKeyValues.digitalKeys.onoff = keyValues.digitalKeys.onoff; 	HC12.write(keyValues.digitalKeys.onoff); }  // Send that data to HC-12

	//option 1
	/*HC12.write(keyValues.analogKeys.leftX);
	HC12.write(keyValues.analogKeys.leftY);
	HC12.write(keyValues.analogKeys.rightX);
	HC12.write(keyValues.analogKeys.rightY);
	HC12.write(keyValues.digitalKeys.onoff);*/

	//option 2: MavDeg" I think it needs to be looped
	HC12.write(keyValues.analogKeys.leftX);
	HC12.write(keyValues.analogKeys.leftY);
}