/*
 Name:		Sketch1.ino
 Created:	6/12/2023 9:46:51 AM
 Author:	linhb
*/


#include <DMDESP.h>
#include <fonts/ElektronMart6x8.h>
#include <fonts/Mono5x7.h>
#include <fonts/ElektronMart6x16.h>
#include <fonts/EMKotak5x7.h>
#include "Solar2luna.h"
//SETUP DMD
#define DISPLAYS_WIDE 2 // Kolom Panel
#define DISPLAYS_HIGH 1 // Baris Panel
DMDESP matrix(DISPLAYS_WIDE, DISPLAYS_HIGH);  // Jumlah Panel P10 yang digunakan (KOLOM,BARIS)
converSolar2luna lunaDate;


//----------------------------------------------------------------------
// SETUP

void setup() {
	Serial.begin(115200);
	// DMDESP Setup
	matrix.start();
	matrix.setBrightness(100);
	lunaDate.Solar2Lunar(12, 6, 23);
}



//----------------------------------------------------------------------
// LOOP

void loop() {

	matrix.loop();
	showClock(12, 23, 56, 8, 0);
	//showDay(12, 6, 23, 0, 8);
}

void showClock(uint8_t h, uint8_t m, uint8_t s, uint8_t x, int8_t y) {
	static uint32_t lastUpdate;
	uint32_t now = millis();
	static bool blink;
	if (lastUpdate + 500<= now) {
		lastUpdate = now;
		blink = !blink;
		String dot= blink ? ":" : " ";
		String timeShow= String(h) + dot + String(m) + dot + String(s);
		matrix.setFont(Mono5x7);
		matrix.drawText(x, y,timeShow );
	}
}
//void showDay(uint8_t dd, uint8_t mm, uint8_t yy, uint8_t x, uint8_t y) {
//	String text1 = String(dd) + "/" + String(mm);
//
//	String text2 = String(lunar_dd) + "/" + String(lunar_mm);
//	matrix.setFont(EMKotak5x7);
//	matrix.drawText(x, y, text1 +""+ text2);
//
//}
