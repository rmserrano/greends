/*
  webserver_handlers.ino - FreeDs webserver handlers
  Derivador de excedentes para ESP32 DEV Kit // Wifi Kit 32
  
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
/*extern "C"
{
#include <base64.h>
}
*/
void handleCnet(AsyncWebServerRequest *request)
{
  strcpy(config.ssid1, request->urlDecode(request->arg("wifi1")).c_str());
  strcpy(config.ssid2, request->urlDecode(request->arg("wifi2")).c_str());
  strcpy(config.pass1, request->urlDecode(request->arg("wifip1")).c_str());
  strcpy(config.pass2, request->urlDecode(request->arg("wifip2")).c_str());

  String host = request->urlDecode(request->arg("host"));
  host.toLowerCase(); // Nos aseguramos que este en minúsculas
  
  if (host != String(config.hostServer))
  {  
    strcpy(config.hostServer, host.c_str());
    MDNS.end();
    MDNS.begin(config.hostServer);
  }

  if (request->hasArg("ip"))
  {
    strcpy(config.ip, request->urlDecode(request->arg("ip")).c_str());
    strcpy(config.gw, request->urlDecode(request->arg("gw")).c_str());
    strcpy(config.mask, request->urlDecode(request->arg("mask")).c_str());
    strcpy(config.dns1, request->urlDecode(request->arg("dns1")).c_str());
    strcpy(config.dns2, request->urlDecode(request->arg("dns2")).c_str());
  }

  if (request->arg("dhcp") == "on")
  {
    config.flags.dhcp = true;
  }
  else
  {
    config.flags.dhcp = false;
  }

  saveEEPROM();
}

void handleConfigMqtt(AsyncWebServerRequest *request)
{
  config.flags.mqtt = false;
  unSuscribeMqtt();
  Error.ConexionMqtt = false;

  if (request->arg("mqttactive") == "on")
  {

    config.flags.mqtt = true;

    config.MQTT_port = request->arg("mqttport").toInt();
    strcpy(config.MQTT_broker, request->urlDecode(request->arg("broker")).c_str());
    strcpy(config.MQTT_user, request->urlDecode(request->arg("mqttuser")).c_str());
    strcpy(config.MQTT_password, request->urlDecode(request->arg("mqttpass")).c_str());
    config.publishMqtt = constrain(request->arg("mqttpublish").toInt(), 1500, 60000);
    Tickers.updatePeriod(6, config.publishMqtt);
    strcpy(config.R01_mqtt, request->urlDecode(request->arg("mqttr1")).c_str());
    strcpy(config.R02_mqtt, request->urlDecode(request->arg("mqttr2")).c_str());
    strcpy(config.R03_mqtt, request->urlDecode(request->arg("mqttr3")).c_str());
    strcpy(config.R04_mqtt, request->urlDecode(request->arg("mqttr4")).c_str());

    switch(config.wversion)
    {
      case MQTT_BROKER:
        strcpy(config.Solax_mqtt, request->urlDecode(request->arg("solax")).c_str());
        strcpy(config.Meter_mqtt, request->urlDecode(request->arg("meter")).c_str());
      break;
      case ICC_SOLAR:
        strcpy(config.SoC_mqtt, request->urlDecode(request->arg("soctopic")).c_str());
      break;
    } 
  }

  config.flags.domoticz = false;
  if (request->arg("domoticzactive") == "on")
  {

    config.flags.domoticz = true;
    config.domoticzIdx[0] = request->arg("idxpwm").toInt();
    config.domoticzIdx[1] = request->arg("idxman").toInt();
    config.domoticzIdx[2] = request->arg("idxoled").toInt();
  }

  saveEEPROM();
  mqttClient.disconnect(true);

  if (config.flags.mqtt) {
    Tickers.enable(2);
    Tickers.enable(6);
  } else {
    Tickers.disable(2);
    Tickers.disable(6);
  }
}

void handleConfig(AsyncWebServerRequest *request)
{
  if (request->arg("offGrid") == "on") {
    config.flags.offGrid = true;
    if (config.flags.offgridVoltage) {
      config.batteryVoltage = request->arg("soc").toFloat();
    } else {
      config.soc = constrain(request->arg("soc").toInt(), 0, 100);
    }
  } else { config.flags.offGrid = false; }

  config.battWatts = constrain(request->arg("battWatts").toInt(), -9999, 9999);

  if (request->arg("changeGridSign") == "on") {
    config.flags.changeGridSign = true;
  } else {
    config.flags.changeGridSign = false;
  }
  
  if (request->hasArg("baudiosmeter")) {
    config.baudiosMeter = constrain(request->arg("baudiosmeter").toInt(), 300, 38400);
    //SerieMeter.updateBaudRate(config.baudiosMeter);
  }
  
  if (request->hasArg("idmeter")) {
    config.idMeter = constrain(request->arg("idmeter").toInt(), 1, 250);
  }
  
  if (request->hasArg("wifis"))
  {
    /*if (config.wversion == SOLAX_V2)
    {
      strcpy(config.ssid_esp01, request->urlDecode(request->arg("wifis")).c_str());
      SerieEsp.printf("SSID: %s\n", config.ssid_esp01);
    }*/
    if (config.wversion == SOLAX_V1 || (config.wversion >= SMA_BOY && config.wversion <= SLAVE_MODE) || (config.wversion >= VICTRON && config.wversion <= SOLAREDGE))
    {
      strcpy(config.sensor_ip, request->urlDecode(request->arg("wifis")).c_str());
      modbustcp = NULL;
      modbusIP.fromString((String)config.sensor_ip);
    }
  }
  
  if (request->urlDecode(request->arg("oldpass")) == String(config.password))
  {
    strcpy(config.password, request->urlDecode(request->arg("newpass")).c_str());
    INFOV("New Password -> %s\n", config.password);
  }

  config.maxErrorTime = constrain(request->arg("maxerrortime").toInt(), 10000, 60000);
  config.getDataTime = constrain(request->arg("getdatatime").toInt(), 250, 60000);
  setGetDataTime();

  config.flags.oledAutoOff = false;
  if (request->arg("autoPowerOff") == "on") {
    timers.OledAutoOff = millis();
    config.flags.oledAutoOff = true;
    config.oledControlTime = constrain(request->arg("autoPowerOffTime").toInt(), 1000, 3000000);
  } else {
    config.flags.oledPower = true;
    turnOffOled();
  }

  config.flags.sensorTemperatura = false;
  Flags.pwmIsWorking = true;
  
  config.modoTemperatura = 0;
  if (request->arg("sensorModoAuto") == "on") { config.modoTemperatura += 1; }
  if (request->arg("sensorModoManual") == "on") { config.modoTemperatura += 2; }
  if (request->arg("sensorTemp") == "on") {
    config.flags.sensorTemperatura = true;
    Tickers.enable(7);
    config.temperaturaEncendido = constrain(request->arg("tempOn").toInt(), 0, 99);
    config.temperaturaApagado = constrain(request->arg("tempOff").toInt(), 0, 99);
    
    int8_t idx = 0;
    if (request->hasArg("termoaddrs")) {
      idx = request->arg("termoaddrs").toInt() - 1;
     if (idx >= 0) { memcpy(config.termoSensorAddress, tempSensorAddress[idx], sizeof config.termoSensorAddress); }
      if (idx < 0) { memset(config.termoSensorAddress, 0, sizeof config.termoSensorAddress); }
    }
    
    if (request->hasArg("triacaddrs")) {
      idx = request->arg("triacaddrs").toInt() - 1; 
      if (idx >= 0) { memcpy(config.triacSensorAddress, tempSensorAddress[idx], sizeof config.triacSensorAddress); }
      if (idx < 0) { memset(config.triacSensorAddress, 0, sizeof config.triacSensorAddress); }
    }

    strcpy(config.nombreSensor, request->urlDecode(request->arg("customSensor")).c_str());

    if (request->hasArg("customaddrs")) {
      idx = request->arg("customaddrs").toInt() - 1; 
      if (idx >= 0) { memcpy(config.customSensorAddress, tempSensorAddress[idx], sizeof config.customSensorAddress); }
      if (idx < 0) { memset(config.customSensorAddress, 0, sizeof config.customSensorAddress); }
    }
    
  } else {
    config.flags.sensorTemperatura = false;
  }
  
  request->arg("alexa") == "on" ? config.flags.alexaControl = true : config.flags.alexaControl = false;
  fauxmo.enable(config.flags.alexaControl);
   
  saveEEPROM();
}

void handleRelay(AsyncWebServerRequest *request)
{
  if (request->arg("pwmactive") == "on") {
    config.flags.pwmEnabled = true;
    config.pwmMin = request->arg("pwmmin").toInt();
    config.pwmMax = request->arg("pwmmax").toInt();
    
    config.R01Min = request->arg("r01min").toInt();
    config.R02Min = request->arg("r02min").toInt();
    config.R03Min = request->arg("r03min").toInt();
    config.R04Min = request->arg("r04min").toInt();

    config.R01PotOn = request->arg("r01poton").toInt();
    config.R02PotOn = request->arg("r02poton").toInt();
    config.R03PotOn = request->arg("r03poton").toInt();
    config.R04PotOn = request->arg("r04poton").toInt();

    config.R01PotOff = request->arg("r01potoff").toInt();
    config.R02PotOff = request->arg("r02potoff").toInt();
    config.R03PotOff = request->arg("r03potoff").toInt();
    config.R04PotOff = request->arg("r04potoff").toInt();
  } else {
    config.flags.pwmEnabled = false;
    down_pwm(true);
  }

  config.attachedLoadWatts = request->arg("loadwatts").toInt();
  config.maxWattsTariff = request->arg("wattstariff").toInt();

  if (request->hasArg("slavepwm")) {
    config.pwmSlaveOn = constrain(request->arg("slavepwm").toInt(), 0, 100);
  }
  
  config.pwmControlTime = constrain(request->arg("looppwm").toInt(), 500, 10000);
  Tickers.updatePeriod(5, config.pwmControlTime);
  config.manualControlPWM = constrain(request->arg("manpwm").toInt(), 0, 100);
  config.autoControlPWM = constrain(request->arg("autopwm").toInt(), 0, 100);
  

  if (request->arg("potpwmactive") == "on") {
    config.flags.potManPwmActive = true;
    config.potManPwm = constrain(request->arg("potmanpwm").toInt(), 0, 9999);
  } else {
    config.flags.potManPwmActive = false;
    Flags.pwmManAuto = false;
  }

  if (request->arg("lowcostactive") == "on") {
    config.flags.dimmerLowCost = true;
  } else {
    config.flags.dimmerLowCost = false;
  }

  if (request->hasArg("maxpwmlowcost")) {
    config.maxPwmLowCost = constrain(request->arg("maxpwmlowcost").toInt(), 1024, 1232);
  }
  
  Flags.timerSet = false;
  if (request->arg("timeractive") == "on") {
    config.flags.timerEnabled = true;
    String n;
    if (request->arg("timerStart") != ""){
      n = request->arg("timerStart");
      n.replace(":", "");
      config.timerStart = atoi(n.c_str());
    }
  
    if (request->arg("timerStop") != ""){
      n = request->arg("timerStop");
      n.replace(":", "");
      config.timerStop = atoi(n.c_str());
    }
  } else { config.flags.timerEnabled = false; }
  
  request->arg("R01_man") == "on" ? config.relaysFlags.R01Man = true : config.relaysFlags.R01Man = false;
  request->arg("R02_man") == "on" ? config.relaysFlags.R02Man = true : config.relaysFlags.R02Man = false;
  request->arg("R03_man") == "on" ? config.relaysFlags.R03Man = true : config.relaysFlags.R03Man = false;
  request->arg("R04_man") == "on" ? config.relaysFlags.R04Man = true : config.relaysFlags.R04Man = false;

  if (config.pwmFrequency != constrain(request->arg("frecpwm").toInt(), 10, 3000)){
    config.pwmFrequency = constrain(request->arg("frecpwm").toInt(), 10, 3000);
    ledcWriteTone(2, config.pwmFrequency);
  }

  relay_control_man(false); // Control de relays

  saveEEPROM();
}

void rebootCause(void)
{
  verbose_print_reset_reason(0);
  verbose_print_reset_reason(1);
}

const char *sendJsonWeb(void)
{
  DynamicJsonDocument jsonValues(1024);

  uint16_t error = 0;
  char tmpString[33];

  if (Error.ConexionWifi)
    error = 0x01;
  if (Error.ConexionMqtt && config.flags.mqtt)
    error |= 0x02;
  if (Error.RecepcionDatos)
    error |= 0x04;
  if (Error.VariacionDatos)
    error |= 0x08;
  if (!config.flags.mqtt && config.wversion == MQTT_BROKER)
    error |= 0x10;
  if (config.flags.sensorTemperatura && Error.temperaturaTermo)
    error |= 0x20;
  if (config.flags.sensorTemperatura && Error.temperaturaTriac)
    error |= 0x40;
  if (config.flags.sensorTemperatura && Error.temperaturaCustom)
    error |= 0x80;

  jsonValues["error"] = error;
  jsonValues["R01"] = digitalRead(PIN_RL1);
  jsonValues["R02"] = digitalRead(PIN_RL2);
  jsonValues["R03"] = digitalRead(PIN_RL3);
  jsonValues["R04"] = digitalRead(PIN_RL4);
  jsonValues["Oled"] = config.flags.oledPower;
  jsonValues["oledBrightness"] = config.oledBrightness;
  jsonValues["POn"] = config.flags.pwmEnabled;
  jsonValues["PwmMan"] = config.flags.pwmMan | Flags.pwmManAuto;
  jsonValues["SenTemp"] = config.flags.sensorTemperatura;
  jsonValues["Msg"] = webMessageResponse;
  jsonValues["pwmfrec"] = config.pwmFrequency;
  jsonValues["pwm"] = pwmValue;
   
  dtostrfd(inverter.currentCalcWatts, 0, tmpString);
  jsonValues["loadCalcWatts"] = tmpString;

  jsonValues["baudiosMeter"] = config.baudiosMeter;
  jsonValues["configwVersion"] = config.wversion;

  if (config.wversion == SLAVE_MODE) {
    jsonValues["wversion"] = masterMode;
  } else {
    jsonValues["wversion"] = config.wversion;
  }

  // Inverter data
  if (webMonitorFields.wsolar) {
    dtostrfd(inverter.wsolar, 2, tmpString);
    jsonValues["wsolar"] = tmpString;
  }

  if (webMonitorFields.wgrid) {
    dtostrfd(inverter.wgrid, 2, tmpString);
    jsonValues["wgrid"] = tmpString;
  }
  if (webMonitorFields.temperature) {
    dtostrfd(inverter.temperature, 2, tmpString);
    jsonValues["invTemp"] = tmpString;
  }
  if (webMonitorFields.batteryWatts) {
    dtostrfd(inverter.batteryWatts, 2, tmpString);
    jsonValues["wbattery"] = tmpString;
  }
  if (webMonitorFields.batterySoC) {
    dtostrfd(inverter.batterySoC, 2, tmpString);
    jsonValues["invSoC"] = tmpString;
  }
  if (webMonitorFields.loadWatts) {
    dtostrfd(inverter.loadWatts, 2, tmpString);
    jsonValues["wload"] = tmpString;
  }
  if (webMonitorFields.wtoday) {
    dtostrfd(inverter.wtoday, 2, tmpString);
    jsonValues["wtoday"] = tmpString;
  }
  if (webMonitorFields.gridv) {
    dtostrfd(inverter.gridv, 2, tmpString);
    jsonValues["gridv"] = tmpString;
  }
  if (webMonitorFields.pv1c) {
    dtostrfd(inverter.pv1c, 2, tmpString);
    jsonValues["pv1c"] = tmpString;
  }
  if (webMonitorFields.pv1v) {
    dtostrfd(inverter.pv1v, 2, tmpString);
    jsonValues["pv1v"] = tmpString;
  }
  if (webMonitorFields.pw1) {
    dtostrfd(inverter.pw1, 2, tmpString);
    jsonValues["pw1"] = tmpString;
  }
  if (webMonitorFields.pv2c) {
    dtostrfd(inverter.pv2c, 2, tmpString);
    jsonValues["pv2c"] = tmpString;
  }
  if (webMonitorFields.pv2v) {
    dtostrfd(inverter.pv2v, 2, tmpString);
    jsonValues["pv2v"] = tmpString;
  }
  if (webMonitorFields.pw2) {
    dtostrfd(inverter.pw2, 2, tmpString);
    jsonValues["pw2"] = tmpString;
  }

  // Meter data
  if (webMonitorFields.voltage) {
    dtostrfd(meter.voltage, 2, tmpString);
    jsonValues["mvoltage"] = tmpString;
  }
  if (webMonitorFields.current) {
    dtostrfd(meter.current, 2, tmpString);
    jsonValues["mcurrent"] = tmpString;
  }
  if (webMonitorFields.powerFactor) {
    dtostrfd(meter.powerFactor, 2, tmpString);
    jsonValues["mpowerFactor"] = tmpString;
  }
  if (webMonitorFields.frequency) {
    dtostrfd(meter.frequency, 2, tmpString);
    jsonValues["mfrequency"] = tmpString;
  }
  if (webMonitorFields.importActive) {
    dtostrfd(meter.importActive, 2, tmpString);
    jsonValues["mimportActive"] = tmpString;
  }
  if (webMonitorFields.exportActive) {
    dtostrfd(meter.exportActive, 2, tmpString);
    jsonValues["mexportActive"] = tmpString;
  }

  if (config.flags.showEnergyMeter && !config.flags.offGrid && Flags.ntpTime) {
    // Energy Import
    dtostrfd(config.KwToday, 3, tmpString);
    jsonValues["KwToday"] = tmpString;
    dtostrfd(config.KwYesterday, 3, tmpString);
    jsonValues["KwYesterday"] = tmpString;
    dtostrfd(config.KwTotal, 3, tmpString);
    jsonValues["KwTotal"] = tmpString;

    // Energy export
    dtostrfd(config.KwExportToday, 3, tmpString);
    jsonValues["KwExportToday"] = tmpString;
    dtostrfd(config.KwExportYesterday, 3, tmpString);
    jsonValues["KwExportYesterday"] = tmpString;
    dtostrfd(config.KwExportTotal, 3, tmpString);
    jsonValues["KwExportTotal"] = tmpString;
  }

  // Temperatures
  dtostrfd(temperaturaTermo, 1, tmpString);
  jsonValues["tempTermo"] = tmpString;
  
  dtostrfd(temperaturaTriac, 1, tmpString);
  jsonValues["tempTriac"] = tmpString;

  dtostrfd(temperaturaCustom, 1, tmpString);
  jsonValues["tempCustom"] = tmpString;

  jsonValues["customSensor"] = config.nombreSensor;

  serializeJson(jsonValues, jsonResponse);

  return jsonResponse;
}

const char *sendMasterData(void)
{
  DynamicJsonDocument jsonValues(768);
  
  jsonValues["wversion"] = config.wversion;
  jsonValues["PwmMaster"] = pwmValue;
  
  char tmpString[33];

  // Inverter data
  if (webMonitorFields.wsolar) {
    dtostrfd(inverter.wsolar, 2, tmpString);
    jsonValues["wsolar"] = tmpString;
  }

  if (webMonitorFields.wgrid) {
    dtostrfd(inverter.wgrid, 2, tmpString);
    jsonValues["wgrid"] = tmpString;
  }
  if (webMonitorFields.temperature) {
    dtostrfd(inverter.temperature, 2, tmpString);
    jsonValues["invTemp"] = tmpString;
  }
  if (webMonitorFields.batteryWatts) {
    dtostrfd(inverter.batteryWatts, 2, tmpString);
    jsonValues["wbattery"] = tmpString;
  }
  if (webMonitorFields.batterySoC) {
    dtostrfd(inverter.batterySoC, 2, tmpString);
    jsonValues["invSoC"] = tmpString;
  }
  if (webMonitorFields.loadWatts) {
    dtostrfd(inverter.loadWatts, 2, tmpString);
    jsonValues["wload"] = tmpString;
  }
  if (webMonitorFields.wtoday) {
    dtostrfd(inverter.wtoday, 2, tmpString);
    jsonValues["wtoday"] = tmpString;
  }
  if (webMonitorFields.gridv) {
    dtostrfd(inverter.gridv, 2, tmpString);
    jsonValues["gridv"] = tmpString;
  }
  if (webMonitorFields.pv1c) {
    dtostrfd(inverter.pv1c, 2, tmpString);
    jsonValues["pv1c"] = tmpString;
  }
  if (webMonitorFields.pv1v) {
    dtostrfd(inverter.pv1v, 2, tmpString);
    jsonValues["pv1v"] = tmpString;
  }
  if (webMonitorFields.pw1) {
    dtostrfd(inverter.pw1, 2, tmpString);
    jsonValues["pw1"] = tmpString;
  }
  if (webMonitorFields.pv2c) {
    dtostrfd(inverter.pv2c, 2, tmpString);
    jsonValues["pv2c"] = tmpString;
  }
  if (webMonitorFields.pv2v) {
    dtostrfd(inverter.pv2v, 2, tmpString);
    jsonValues["pv2v"] = tmpString;
  }
  if (webMonitorFields.pw2) {
    dtostrfd(inverter.pw2, 2, tmpString);
    jsonValues["pw2"] = tmpString;
  }

  // Meter data
  if (webMonitorFields.voltage) {
    dtostrfd(meter.voltage, 2, tmpString);
    jsonValues["mvoltage"] = tmpString;
  }
  if (webMonitorFields.current) {
    dtostrfd(meter.current, 2, tmpString);
    jsonValues["mcurrent"] = tmpString;
  }
  if (webMonitorFields.powerFactor) {
    dtostrfd(meter.powerFactor, 2, tmpString);
    jsonValues["mpowerFactor"] = tmpString;
  }
  if (webMonitorFields.frequency) {
    dtostrfd(meter.frequency, 2, tmpString);
    jsonValues["mfrequency"] = tmpString;
  }
  if (webMonitorFields.importActive) {
    dtostrfd(meter.importActive, 2, tmpString);
    jsonValues["mimportActive"] = tmpString;
  }
  if (webMonitorFields.exportActive) {
    dtostrfd(meter.exportActive, 2, tmpString);
    jsonValues["mexportActive"] = tmpString;
  }

  serializeJson(jsonValues, jsonResponse);

  return jsonResponse;
}

void checkAuth(AsyncWebServerRequest *request)
{
  size_t outputLength;
  char password[30];
  char decoded[30];

  //unsigned char *decoded = base64_decode((const unsigned char *)config.password, strlen(config.password), &outputLength);
  int result = mbedtls_base64_decode((unsigned char *)decoded, 29, &outputLength, (unsigned char *)config.password, strlen(config.password));

  sprintf(password, "%.*s", outputLength, decoded);
 
  // Serial.printf("Decoded Password -> %.*s\n", outputLength, decoded);

  if (!request->authenticate(www_username, (const char *)password)) {
    //free(decoded);
    return request->requestAuthentication();
  }
  //free(decoded);
}

void send_events()
{
  events.send(printUptime(), "uptime");
  events.send(sendJsonWeb(), "jsonweb");
  // Serial.printf("\nEvents loop Time: %lu\n", millis() - tme);
  // tme = millis();
}

void setWebConfig(void)
{
  //////////// PAGINAS EN LA MEMORIA SPPIFS ////////
  
  server.on("/", [](AsyncWebServerRequest *request) {
    checkAuth(request);
    request->send(SPIFFS, "/index.html", "text/html", false, processorFreeDS);
    if (Flags.reboot)
    {
      restartFunction();
    }
  });

  server.on("/Red.html", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    checkAuth(request);
    request->send(SPIFFS, "/Red.html", "text/html", false, processorRed);
  });

  server.on("/Mqtt.html", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    checkAuth(request);
    request->send(SPIFFS, "/Mqtt.html", "text/html", false, processorMqtt);
  });

  server.on("/Config.html", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    checkAuth(request);
    request->send(SPIFFS, "/Config.html", "text/html", false, processorConfig);
  });

  server.on("/Salidas.html", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    checkAuth(request);
    request->send(SPIFFS, "/Salidas.html", "text/html", false, processorSalidas);
  });

  server.on("/Ota.html", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    checkAuth(request);
    request->send(SPIFFS, "/Ota.html", "text/html", false, processorOta);
  });

  server.on("/weblog.html", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    checkAuth(request);
    request->send(SPIFFS, "/weblog.html", "text/html", false, processorOta);
  });

  server.on("/backup.html", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    checkAuth(request);
    request->send(SPIFFS, "/backup.html", "text/html", false, processorOta);
  });

  //////////// JAVASCRIPT EN LA MEMORIA SPPIFS ////////
  
  server.on("/sb-admin-2.min.js", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/sb-admin-2.min.js.jgz", "application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  server.on("/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/bootstrap.bundle.min.js.jgz", "application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  server.on("/jquery-3.5.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/jquery-3.5.1.min.js.jgz", "application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  server.on("/freeds.min.js", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/freeds.min.js.jgz", "application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  //////////// IDIOMAS EN LA MEMORIA SPPIFS ////////

  server.on("/lang-es.js", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/lang-es.js.jgz", "application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  server.on("/lang-en.js", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/lang-en.js.jgz", "application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  server.on("/lang-pt.js", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/lang-pt.js.jgz", "application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });
  
  server.on("/lang-ca.js", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/lang-ca.js.jgz", "application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  server.on("/lang-ga.js", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/lang-ga.js.jgz", "application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  //////////// CSS EN LA MEMORIA SPPIFS ////////
  
  server.on("/sb-admin-2.min.css", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/sb-admin-2.min.css.jgz", "text/css");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  server.on("/all.min.css", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/all.min.css.jgz", "text/css");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  //////////// FUENTES EN LA MEMORIA SPPIFS ////////

  server.on("/fa-solid-900.woff", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/fa-solid-900.woff", "text/plain");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  server.on("/fa-solid-900.woff2", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/fa-solid-900.woff2", "text/plain");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  server.on("/fa-solid-900.ttf", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/fa-solid-900.ttf.jgz", "text/plain");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/favicon.ico", "image/x-icon");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  server.on("/freeds.png", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/freeds.png.jgz", "image/png");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
    request->send(response);
  });

  //////// RESPUESTAS A LAS PULSACIONES DE LOS BOTONES //////////////////

  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    checkAuth(request);
    saveEEPROM();
    webMessageResponse = 2;
    Flags.reboot = true;
    request->redirect("/");
  });

  server.on("/selectversion", HTTP_POST, [](AsyncWebServerRequest *request) {
    //checkAuth(request);
    config.wversion = request->arg("data").toInt();
    processData = false;

    if (config.flags.mqtt && config.wversion != SOLAX_V2_LOCAL && !mqttClient.connected()) { Tickers.enable(2); }
    if (config.flags.mqtt && mqttClient.connected()) {
       if (config.wversion != SOLAX_V2_LOCAL) { unSuscribeMqtt(); }
       if (config.wversion == MQTT_BROKER) { suscribeMqttMeter(); }
    }
    
    setGetDataTime();

    modbustcp = NULL;

    if (config.wversion == SMA_BOY || (config.wversion >= VICTRON && config.wversion <= SOLAREDGE)) {
      modbusIP.fromString((String)config.sensor_ip);
      if (config.wversion == SMA_BOY || (config.wversion >= VICTRON && config.wversion <= WIBEEE_MODBUS)) {
        modbustcp = new esp32ModbusTCP(modbusIP, 502);
      } else { modbustcp = new esp32ModbusTCP(modbusIP, 1502); }
      configModbusTcp();
    }
    
    saveEEPROM();
    memset(&inverter, 0, sizeof(inverter));
    memset(&meter, 0, sizeof(meter));
    Error.VariacionDatos = true;
    defineWebMonitorFields(config.wversion);

    AsyncWebServerResponse *response = request->beginResponse(200);
    response->addHeader("Connection", "close");
    request->send(response);
  });

  server.on("/language", HTTP_POST, [](AsyncWebServerRequest *request) {
    //checkAuth(request);
    strcpy(config.language, request->arg("value").c_str());
    Serial.printf("new language: %s\n", config.language);
    saveEEPROM();
    readLanguages();
    
    AsyncWebServerResponse *response = request->beginResponse(200);
    response->addHeader("Connection", "close");
    request->send(response);
  });

  server.on("/brightness", HTTP_POST, [](AsyncWebServerRequest *request) {
    //checkAuth(request);
    config.oledBrightness = request->arg("data").toInt();
    config.flags.oledPower = true;
    Flags.setBrightness = true;
    AsyncWebServerResponse *response = request->beginResponse(200);
    response->addHeader("Connection", "close");
    request->send(response);
  });

  server.on("/tooglebuttons", HTTP_POST, [](AsyncWebServerRequest *request) {
    //checkAuth(request);
    uint8_t button = request->arg("data").toInt();

    if (config.flags.debug1) {
      INFOV("Comando recibido Nº %i\n", button);
    }

    switch (button)
    {
    case 1: // Activación Manual Relé 1
      Flags.Relay01Man = !Flags.Relay01Man;
      break;
    case 2: // Activación Manual Relé 2
      Flags.Relay02Man = !Flags.Relay02Man;
      break;
    case 3: // Activación Manual Relé 3
      Flags.Relay03Man = !Flags.Relay03Man;
      break;
    case 4: // Activación Manual Relé 4
      Flags.Relay04Man = !Flags.Relay04Man;
      break;
    case 5: // Encender / Apagar OLED
      timers.OledAutoOff = millis();
      config.flags.oledPower = !config.flags.oledPower;
      turnOffOled();
      saveEEPROM();
      break;
    case 6: // Encender / Apagar PWM
      config.flags.pwmEnabled = !config.flags.pwmEnabled;
      Flags.pwmIsWorking = true;
      saveEEPROM();
      break;
    case 7: // Encender / Apagar PWM Manual
      config.flags.pwmMan = !config.flags.pwmMan;
      Flags.pwmIsWorking = true;
      saveEEPROM();
      break;
    }

    if (button > 0 && button < 5)
    {
      relay_control_man(false);
    }
    AsyncWebServerResponse *response = request->beginResponse(200);
    response->addHeader("Connection", "close");
    request->send(response);
  });

  server.on("/masterdata", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
    AsyncWebServerResponse *response = request->beginResponse(200, "text/json", sendMasterData());
    request->send(response);
  });

  server.on("/handlecmnd", HTTP_POST, [](AsyncWebServerRequest *request) {
    
    String comandoRaw = request->arg("webcmnd");
    String comando = comandoRaw.substring(0, comandoRaw.indexOf(" "));
    // uint32_t payload = atoi(comandoRaw.substring(comandoRaw.indexOf(" ") + 1, strlen(comandoRaw.c_str())).c_str());
    String payload = comandoRaw.substring(comandoRaw.indexOf(" ") + 1, strlen(comandoRaw.c_str())).c_str();
    uint32_t value = atoi(payload.c_str());
        
    if (comando == "rebootcause") { rebootCause(); }
    if (comando == "getfreeheap") { INFOV("Free Heap: %d bytes\n", ESP.getFreeHeap()); }
    if (comando == "serial") {
      if (value == 1) { config.flags.serial = true; } else { config.flags.serial = false; }
      saveEEPROM();
    }

    if (comando == "debug") {
      switch (value)
      {
      case 0:
        config.flags.debug1 = false;
        config.flags.debug2 = false;
        config.flags.debug3 = false;
        config.flags.debug4 = false;
        config.flags.debug5 = false;
        break;
      case 1:
        config.flags.debug1 = true; 
        break;
      case 2:
        config.flags.debug2 = true;
        break;
      case 3:
        config.flags.debug3 = true;
        break;
      case 4:
        config.flags.debug4 = true;
        break;
      case 5:
        config.flags.debug5 = true;
        break;
      }
      saveEEPROM();
    }

    if (comando == "weblog") {
      if (value == 1) { config.flags.weblog = true; } else { config.flags.weblog = false; }
      saveEEPROM();
    }

    if (comando == "KwToday") {
      if (value >= 0) { config.KwToday = value / 1000.0; }
    }

    if (comando == "KwTotal") {
      if (value >= 0) { config.KwTotal = value / 1000.0; }
    }

     if (comando == "KwExportToday") {
      if (value >= 0) { config.KwExportToday = value / 1000.0; }
    }

    if (comando == "KwExportTotal") {
      if (value >= 0) { config.KwExportTotal = value / 1000.0; }
    }

    if (comando == "flipScreen") {
      config.flags.flipScreen = !config.flags.flipScreen;
      if(config.flags.flipScreen) { display.flipScreenVertically(); }
      else { display.resetOrientation(); }
      saveEEPROM();
    }

    if (comando == "showEnergyMeter") {
      if (value == 1) { config.flags.showEnergyMeter = true; }
      else { config.flags.showEnergyMeter = false; }
      saveEEPROM();
    }

    /*if (comando == "solaxVersion") {
      SerieEsp.printf("DATAVERSION: %d\n", value);
    }*/

    if (comando == "tzConfig") {
      if (payload != "tzConfig") {
        strcpy(config.tzConfig, payload.c_str());
        if (config.wversion != SOLAX_V2_LOCAL) { configTzTime(config.tzConfig, config.ntpServer); updateLocalTime(); }
        INFOV("Payload: %s\n",payload.c_str());
        saveEEPROM();
      } else {
        INFOV("tzConfig: %s\n", config.tzConfig);
      }
    }

    if (comando == "ntpServer") {
      if (payload != "ntpServer") {
        strcpy(config.ntpServer, payload.c_str());
        if (config.wversion != SOLAX_V2_LOCAL) { configTzTime(config.tzConfig, config.ntpServer); updateLocalTime(); }
        INFOV("Payload: %s\n",payload.c_str());
        saveEEPROM();
      } else {
        INFOV("ntpServer: %s\n", config.ntpServer);
      }
    }

    if (comando == "offgridVoltage") {
      if (value == 1) { config.flags.offgridVoltage = true; }
      else { config.flags.offgridVoltage = false; }
      saveEEPROM();
    }

    if (comando == "calibration") {
      emon1.current(ADC_INPUT, value);  
      saveEEPROM();
    }

    if (comando == "useClamp") {
      if (value == 1) { config.flags.useClamp = true; }
      else { config.flags.useClamp = false; }
      saveEEPROM();
    }

    if (comando == "storeClampValues") {
      // Disable pwm Output
      down_pwm(false);

      // Start with a 5%
      readClampPos = 0;
      writeClampPwm();
  
      // Disable all timers and enable again the essentials timers
      Tickers.disableAll();
      Tickers.enable(0); // Oled
      Tickers.enable(1); // Send Events
      Tickers.enable(7); // Every 1000ms loop
      Tickers.enable(8); // Store Clamp Values
    }

    if (comando == "maxWattsTariff") {
      config.maxWattsTariff = value;
      INFOV("maxWattsTariff: %d\n", config.maxWattsTariff);
      saveEEPROM();
    }

    if (comando == "writeConfig") {
      writeConfigSpiffs("/config.bin");
    }

    if (comando == "readConfig") {
      readConfigSpiffs();
    }

    AsyncWebServerResponse *response = request->beginResponse(200);
    response->addHeader("Connection", "close");
    request->send(response);
  });

  ///////// RESPUESTAS A LAS PÁGINAS DE CONFIGURACIÓN ////////

  server.on("/handleCnet", HTTP_POST, [](AsyncWebServerRequest *request) {
    checkAuth(request);
    handleCnet(request);
    webMessageResponse = 1;
    request->redirect("/");
    if (!config.flags.wifi)
    {
      config.flags.wifi = true;
      restartFunction();
    }
  });

  server.on("/handleConfigMqtt", HTTP_POST, [](AsyncWebServerRequest *request) {
    checkAuth(request);
    handleConfigMqtt(request);
    webMessageResponse = 1;
    request->redirect("/");
  });

  server.on("/handleConfig", HTTP_POST, [](AsyncWebServerRequest *request) {
    checkAuth(request);
    handleConfig(request);
    webMessageResponse = 1;
    request->redirect("/");
  });

  server.on("/handleRelay", HTTP_POST, [](AsyncWebServerRequest *request) {
    checkAuth(request);
    handleRelay(request);
    webMessageResponse = 1;
    request->redirect("/");
  });

  server.on("/factoryDefaults", HTTP_POST, [](AsyncWebServerRequest *request) {
    checkAuth(request);
    request->redirect("/");
    defaultValues();
    restartFunction();
  });

  server.on("/downloadBackup", HTTP_GET, [](AsyncWebServerRequest *request) {
    checkAuth(request);
    char filename[30];
    sprintf(filename, "/config_%s.bin", version);
    writeConfigSpiffs(filename);
    request->send(SPIFFS, filename, "application/octet-stream", true);
    SPIFFS.remove(filename);
  });

  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
      
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      /// Check if extension is .bin
      if (strcmp(get_filename_ext(filename.c_str()), "bin") == 0) {
        INFOV("Update Start: %s\n", filename.c_str());
        Flags.Updating = true;
        config.flags.pwmEnabled = false;
        down_pwm(false);
        Tickers.disableAll();
        mqttClient.disconnect();
        Tickers.enable(0);
        if (filename == "spiffs.bin") {
          SPIFFS.end();
          if(!Update.begin(UPDATE_SIZE_UNKNOWN, 100)) { Update.printError(Serial); }
        } else {
          if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) { Update.printError(Serial); }
        }
      } else {
        Update.end();
        request->send(500);
        Serial.printf("Extensión no compatible\n"); 
      }
    }
    if (Update.hasError()) {
      request->send(500);
      Update.printError(Serial);
      ESP.restart();
    } else {
      if(Update.write(data, len) != len){
        Update.printError(Serial);
      }
    }
    if (final) {
      if(Update.end(true)){
        request->send(200);
        config.flags.pwmEnabled = true;
        saveEEPROM();
        INFOV("Update Success: %uB\n", index+len);
        delay(500);
        ESP.restart();
      } else {
        Update.printError(Serial);
      }
    } });
  
  server.on("/backup", HTTP_POST, [](AsyncWebServerRequest *request) {
      
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if(!index){
    INFOV("UploadStart: %s\n", filename.c_str());
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open("/config.bin", "w");
  }
  if(len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data,len);
  }
  if(final){
    request->send(200);
    INFOV("UploadEnd: %s, size: %d\n", filename.c_str(), index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    readConfigSpiffs();
    saveEEPROM();
    delay(500);
    ESP.restart();
  }
  });

  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId())
    {
      INFOV("Client reconnected! Last message ID that it get is: %lu\n", client->lastId());
    }
    client->send("Hello!", NULL, millis(), 1000);
  });

  webLogs.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId())
    {
      INFOV("Weblog client reconnected! Last message ID that it get is: %lu\n", client->lastId());
    }
    client->send("Hello weblog!", NULL, millis(), 1000);
    Flags.weblogConnected = true;
    sendWeblogStreamTest();
  });

  // attach EventSource
  server.addHandler(&events);
  server.addHandler(&webLogs);

  String mdnsUrl = "http://";
  mdnsUrl += String(config.hostServer);
  mdnsUrl += ".local";
  mdnsUrl.toLowerCase();

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", mdnsUrl);
  
  if (config.flags.wifi) alexaConfig();
  server.begin();
  if (config.flags.wifi) alexaStart();
}

void alexaConfig(void)
{
  // These two callbacks are required for gen1 and gen3 compatibility
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), String((char *)data))) return;
      // Handle any other body request here...
  });
  server.onNotFound([](AsyncWebServerRequest *request) {
      String body = (request->hasParam("body", true)) ? request->getParam("body", true)->value() : String();
      if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), body)) return;
      // Handle not found request here...
  });
}

void alexaStart(void)
{
  fauxmo.setPort(80); // This is required for gen3 devices
  fauxmo.enable(config.flags.alexaControl);

  byte mac[6];
  char tmp[30];
  WiFi.macAddress(mac);

  // PWM ON/OFF
  sprintf(tmp, "Derivador %02X%02X", mac[4], mac[5]);
  fauxmo.addDevice(tmp, ONOFF);
  fauxmo.setState(tmp, config.flags.pwmEnabled, 0);

  // Modo Auto/Manual y valor de modo manual
  sprintf(tmp, "Derivador Manual %02X%02X", mac[4], mac[5]);
  fauxmo.addDevice(tmp, DIMMABLE);
  fauxmo.setState(tmp, config.flags.pwmMan, (uint8_t)((config.manualControlPWM * 254) / 100));

  // Pantalla Derivador
  sprintf(tmp, "Derivador Oled %02X%02X", mac[4], mac[5]);
  fauxmo.addDevice(tmp, DIMMABLE);
  fauxmo.setState(tmp, config.flags.oledPower, (uint8_t)((config.oledBrightness * 254) / 100));
  
  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
             
    INFOV("[ALEXA] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    if (device_id == 0) {
      config.flags.pwmEnabled = state;
    } else if (device_id == 1) {
      config.flags.pwmMan = state;
      config.manualControlPWM = (uint8_t)((value * 100) / 254);
    } else if (device_id == 2) {
      if (config.flags.oledPower != state) {
        config.flags.oledPower = state;
        timers.OledAutoOff = millis();
        turnOffOled();
      }
      if (config.oledBrightness != (uint8_t)((value * 100) / 254)) {
        config.oledBrightness = (uint8_t)((value * 100) / 254);
        Flags.setBrightness = true;
      }
    }
  });
}