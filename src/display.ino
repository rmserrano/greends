/*
  display.ino - Display Support
  Derivador de excedentes para ESP32 DEV Kit // Wifi Kit 32

  Inspired in opends+ (https://github.com/iqas/derivador)
  
  Copyright (C) 2020 Pablo Zerón (https://github.com/pablozg/freeds)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

void showOledData(void)
{
  //if (config.flags.debug1 == 4) { INFOV("showOledData()\n"); }
#ifdef OLED
  uint8_t wversion = 0;

  if ((millis() - timers.FlashDisplay) > 1000)
  {
    timers.FlashDisplay = millis();
    Flags.flash = !Flags.flash;
  }

  if (config.wversion == SLAVE_MODE) { wversion = slave.masterMode; }
  else { wversion = config.wversion; }

  if (config.flags.wifi)
  {
    switch (button.screen)
    {
      case 0: // Principal
        display.clear();

        // Texto Columnas
        display.setFont(ArialMT_Plain_10);

        // Columna Izquierda
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        switch (wversion)
        {
          default:
            display.drawString(0, 0, lang._SOLAR_);
            break;
        }

        // Columna Derecha
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        switch (wversion)
        {
          default:
            display.drawString(128, 0, lang._GRID_);
            break;
        }

        // Datos Columnas
        display.setFont(ArialMT_Plain_24);

        // Columna Izquierda
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        switch (wversion)
        {
          default:
            display.drawString(0, 12, (String)(int)inverter.wsolar);
            break;
        }

        // Columna Derecha
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        switch (wversion)
        {
          default:
            display.drawString(128, 12, (String)(int)inverter.wgrid);
            break;
        }

        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        // Display Select Mode
        if (config.wversion == SLAVE_MODE) { display.drawString(69, 0, "SLV"); }
        else { 
          switch (wversion)
          {
            case SOLAX_V2_LOCAL:
              display.drawString(69, 0, "SV2L");
              break;
            case SOLAX_V1:
              display.drawString(69, 0, "SV1");
              break;
            case SOLAX_V2:
              display.drawString(69, 0, "SV2");
              break;
            case MQTT_BROKER:
              display.drawString(69, 0, "MQTT");
              break;
            case GOODWE:
              display.drawString(69, 0, "GDWE");
              break;
            case WIBEEE:
              display.drawString(69, 0, "WIBE");
              break;
            case SHELLY_EM:
              display.drawString(69, 0, "SHLY");
              break;
            case FRONIUS_API:
              display.drawString(69, 0, "FAPI");
              break;
            case ICC_SOLAR:
              display.drawString(69, 0, "ICCS");
              break;
          }
        }

        if (Flags.Updating)
          display.drawString(64, 38, lang._UPDATING_);
        else if (Error.ConexionWifi)
          display.drawString(64, 38, lang._LOSTWIFI_);
        // else if ((!config.flags.pwmEnabled || (!config.flags.pwmMan && (Error.VariacionDatos || Error.RecepcionDatos))) && pwm.invert_pwm <= 1)
        else if ((!config.flags.pwmMan && (Error.VariacionDatos || Error.RecepcionDatos)) && pwm.invert_pwm <= 1)
          display.drawString(64, 38, WiFi.localIP().toString());
        else {
          display.drawProgressBar(0, 38, 127, 12, pwm.pwmValue); // draw the progress bar

          display.setTextAlignment(TEXT_ALIGN_CENTER);
          if (config.flags.pwmEnabled == false) { display.drawString(64, 38, "PWM: OFF"); }
          else {
            display.setColor(INVERSE);
            if (config.wversion == SLAVE_MODE) {
              display.drawString(64, 38, (config.flags.pwmMan ? "PWM: " + String(pwm.pwmValue) + "% (MANUAL)" : "MSTR: " + String(slave.masterPwmValue) + "%" + " PWM: " + String(pwm.pwmValue) + "%"));
            } else {
              display.drawString(64, 38, (config.flags.pwmMan ? "PWM: " + String(pwm.pwmValue) + "% (MANUAL)" : "PWM: " + String(pwm.pwmValue) + "%"));
            }
            display.setColor(WHITE);
          }
        }

        display.setTextAlignment(TEXT_ALIGN_LEFT);
        if (Flags.flash)
        {
          display.drawString(5, 52, (String(Error.RecepcionDatos ? "S" : "S")));
          display.drawString(17, 52, (String(Error.ConexionWifi ? "W" : "W")));
          display.drawString(30, 52, (String(Error.ConexionMqtt ? "M" : "M")));
        }
        else
        {
          display.drawString(5, 52, (String(Error.RecepcionDatos ? "_" : "S")));
          display.drawString(17, 52, (String(Error.ConexionWifi ? "_" : "W")));
          display.drawString(30, 52, (String(Error.ConexionMqtt ? "_" : "M")));

        }
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        display.drawString(128, 52, (lang._RELAY_ + String((digitalRead(PIN_RL1) ? "1 " : "_ ")) + String((digitalRead(PIN_RL2) ? "2 " : "_ ")) + String((digitalRead(PIN_RL3) ? "3 " : "_ ")) + String((digitalRead(PIN_RL4) ? "4 " : "_ "))));

        display.display();
        break;

      case 1: // Strings Info
          button.screen++;
          break;
      
      case 2: // Meters
          button.screen++;
          break;

      case 3: // Wifi Info
          display.clear();
          display.setFont(ArialMT_Plain_10);
          display.setTextAlignment(TEXT_ALIGN_LEFT);    
          display.drawString(0, 0,  ("IP: " + WiFi.localIP().toString()));
          display.drawString(0, 12, ("SSID: " + WiFi.SSID() + " (" + String(WifiGetRssiAsQuality((int8_t)WiFi.RSSI())) + "%)"));
          display.drawString(0, 24, ("Frec. Pwm: " + String((float)config.pwmFrequency / 10000) + "Khz"));
          display.drawString(0, 36, ("PWM: " + String(pwm.pwmValue) + "% (" + String(pwm.invert_pwm) + ")"));
          display.drawString(0, 48, printUptimeOled());
          // if (Flags.ntpTime) {
          //   display.drawString(0, 60, printDateOled());
          // }
          display.display();
          break;

      case 4: // Temperaturas
          display.clear();
          display.setFont(ArialMT_Plain_10);
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.drawString(64, 0, lang._TEMPERATURES_);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0, 12, (lang._INVERTERTEMP_ + String(inverter.temperature) + "ºC"));
          display.drawString(0, 24, (lang._TERMOTEMP_ + String(temperature.temperaturaTermo) + "ºC"));
          display.drawString(0, 36, (lang._TRIACTEMP_ + String(temperature.temperaturaTriac) + "ºC"));
          display.drawString(0, 48, (String(config.nombreSensor) + ": " + String(temperature.temperaturaCustom) + "ºC"));
          display.display();
          break;
      
      case 5: // Build Info
          display.clear();
          display.setFont(ArialMT_Plain_24);
          display.setTextAlignment(TEXT_ALIGN_CENTER);    
          display.drawString(64, 0, "FreeDS");
          display.setFont(ArialMT_Plain_10);
          display.drawString(64, 25, lang._DERIVADOR_);
          display.drawString(64, 40, lang._COMPILATION_);
          display.drawString(64, 50, ("(" + String(compile_date) + ")"));
          display.display();
          break;
    }
  }
#endif
}

void showLogo(String Texto, bool timeDelay)
{
  display.clear();
  //display.flipScreenVertically();
  display.drawFastImage(0, 0, 128, 64, FreeDS);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  int8_t saltoLinea = Texto.indexOf("\n");
  if (saltoLinea == -1){
    display.drawString(87, 45, Texto);
  } else {
    display.drawString(87, 40, Texto.substring(0, saltoLinea));
    display.drawString(87, 50, Texto.substring(saltoLinea + 1, Texto.length()));
  }
  display.display();
  if (timeDelay) { delay(2000); } // Innecesario salvo para mostrar el mensaje ;-)
}