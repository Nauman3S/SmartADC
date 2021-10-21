#include "headers.h" //all misc. headers and functions
#include "webApp.h"  //Captive Portal webpages
#include <FS.h>      //ESP32 File System
IPAddress ipV(192, 168, 4, 1);

TaskHandle_t Task1;

String loadParams(AutoConnectAux &aux, PageArgument &args) //function to load saved settings
{
    (void)(args);
    File param = FlashFS.open(PARAM_FILE, "r");

    if (param)
    {
        Serial.println("load params func");
        aux.loadElement(param);
        Serial.println(param);
        AutoConnectText &networkElm = aux["network"].as<AutoConnectText>();
        AutoConnectText &providerElm = aux["provider"].as<AutoConnectText>();
        AutoConnectText &signalElm = aux["signal"].as<AutoConnectText>();
        AutoConnectText &voltageElm = aux["signal"].as<AutoConnectText>();
        AutoConnectText &capacityElm = aux["capacity"].as<AutoConnectText>();

        networkElm.value = String("Network: ") + network;
        providerElm.value = String("Provider: ") + provider;
        signalElm.value = String("Signal: ") + signal;
        voltageElm.value = String("Voltage: ") + voltage;
        capacityElm.value = String("Capacity: ") + capacity;

        // curSValueElm.value="CurS:7788";
        param.close();
    }
    else
        Serial.println(PARAM_FILE " open failed");
    return String("");
}

String saveParams(AutoConnectAux &aux, PageArgument &args) //save the settings
{
    serverName = args.arg("mqttserver"); //broker
    serverName.trim();

    gatewayID = args.arg("gatewayID");
    gatewayID.trim();

    port = args.arg("port"); //user name
    port.trim();

    nodeID = args.arg("nodeID"); //password
    nodeID.trim();

    apPass = args.arg("apPass"); //ap pass
    apPass.trim();

    settingsPass = args.arg("settingsPass"); //ap pass
    settingsPass.trim();

    hostName = args.arg("hostname");
    hostName.trim();

    ntpAdd = args.arg("ntpAdd");
    ntpAdd.trim();

    apn = args.arg("apn");
    apn.trim();

    sensorEnabled1 = args.arg("period1");
    sensorEnabled1.trim();
    sensorEnabled2 = args.arg("period2");
    sensorEnabled2.trim();
    sensorEnabled3 = args.arg("period3");
    sensorEnabled3.trim();
    sensorEnabled4 = args.arg("period4");
    sensorEnabled4.trim();

    nameS1 = args.arg("nameS1");
    nameS1.trim();
    nameS2 = args.arg("nameS2");
    nameS2.trim();
    nameS3 = args.arg("nameS3");
    nameS3.trim();
    nameS4 = args.arg("nameS4");
    nameS4.trim();

    mulS1 = args.arg("mulS1");
    mulS1.trim();
    mulS2 = args.arg("mulS2");
    mulS2.trim();
    mulS3 = args.arg("mulS3");
    mulS3.trim();
    mulS4 = args.arg("mulS4");
    mulS4.trim();

    // The entered value is owned by AutoConnectAux of /mqtt_setting.
    // To retrieve the elements of /mqtt_setting, it is necessary to get
    // the AutoConnectAux object of /mqtt_setting.
    File param = FlashFS.open(PARAM_FILE, "w");
    portal.aux("/mqtt_setting")->saveElement(param, {"mqttserver", "gatewayID", "port", "nodeID", "hostname", "apPass", "settingsPass", "ntpAdd", "apn", "period1", "period2", "period3", "period4", "nameS1", "nameS2", "nameS3", "nameS4", "mulS1", "mulS2", "mulS3", "mulS4"});
    param.close();

    // Echo back saved parameters to AutoConnectAux page.
    AutoConnectText &echo = aux["parameters"].as<AutoConnectText>();
    echo.value = "Server: " + serverName + "<br>";
    echo.value += "Gateway ID: " + gatewayID + "<br>";
    echo.value += "Port: " + port + "<br>";
    echo.value += "Node ID: " + nodeID + "<br>";
    echo.value += "ESP host name: " + hostName + "<br>";
    echo.value += "AP Password: " + apPass + "<br>";
    echo.value += "Settings Page Password: " + settingsPass + "<br>";
    echo.value += "NTP: " + ntpAdd + "<br>";
    echo.value += "APN: " + apn + "<br>";
    echo.value += "Sensor 1-4 Settings Saved" + apn + "<br>";

    return String("");
}
bool loadAux(const String auxName) //load defaults from data/*.json
{
    bool rc = false;
    Serial.println("load aux func");
    String fn = auxName + ".json";
    File fs = FlashFS.open(fn.c_str(), "r");
    if (fs)
    {
        rc = portal.load(fs);
        fs.close();
    }
    else
        Serial.println("Filesystem open failed: " + fn);
    return rc;
}

void Task1Loop(void *pvParameters) //GPRS
{
    Serial.print("Task1 running on core ");
    Serial.println(xPortGetCoreID());
    setupGPRS();

    for (;;)
    {
        loopGPRS();
        capacity=String(getADC(5));
        if (isMQTTConnected())
        {
            String topicP = gatewayID + String("/") + nodeID;
            provider = getProvider();
            doc["ts"] = String("1234");

            doc["values"][0]["temperautre"] = getTemp("C");
            doc["values"][0]["humidity"] = getHumid();
            doc["values"][0]["batt"] = String(getADC(5));
            if (sensorEnabled1.indexOf(enableC) >= 0)
            {
                doc["values"][0]["custom1"] = String(getADC(1, mulS1));
            }
            if (sensorEnabled2.indexOf(enableC) >= 0)
            {
                doc["values"][0]["custom2"] = String(getADC(2, mulS2));
            }
            if (sensorEnabled3.indexOf(enableC) >= 0)
            {
                doc["values"][0]["custom3"] = String(getADC(3, mulS3));
            }
            if (sensorEnabled4.indexOf(enableC) >= 0)
            {
                doc["values"][0]["custom4"] = String(getADC(4, mulS4));
            }
            doc["values"][0]["network"] = String(network);
            doc["values"][0]["signal"] = getSignalStrength();

            serializeJson(doc, jsonDoc);
            mqtt.publish(topicP.c_str(), jsonDoc);
        }
    }
}

uint8_t inAP = 0;
bool whileCP()
{

    //use this function as a main loop

    if (inAP == 0)
    {
        ledState(AP_MODE);
        inAP = 1;
    }

    loopLEDHandler();
}

void setup() //main setup functions
{
    Serial.begin(115200);

    xTaskCreatePinnedToCore(
        Task1Loop, /* Task function. */
        "Task1",   /* name of task. */
        10000,     /* Stack size of task */
        NULL,      /* parameter of the task */
        1,         /* priority of the task */
        &Task1,    /* Task handle to keep track of created task */
        1);        /* pin task to core 1 */
    delay(500);

    setupDHT22();
    delay(1000);

    if (!MDNS.begin("esp32")) //starting mdns so that user can access webpage using url `esp32.local`(will not work on all devices)
    {
        Serial.println("Error setting up MDNS responder!");
        while (1)
        {
            delay(1000);
        }
    }
#if defined(ARDUINO_ARCH_ESP8266)
    FlashFS.begin();
#elif defined(ARDUINO_ARCH_ESP32)
    FlashFS.begin(true);
#endif
    loadAux(AUX_MQTTSETTING);
    loadAux(AUX_MQTTSAVE);
    AutoConnectAux *setting = portal.aux(AUX_MQTTSETTING);
    if (setting) //get all the settings parameters from setting page on esp32 boot
    {
        Serial.println("Setting loaded");
        PageArgument args;
        AutoConnectAux &mqtt_setting = *setting;
        loadParams(mqtt_setting, args);
        AutoConnectInput &hostnameElm = mqtt_setting["hostname"].as<AutoConnectInput>();
        AutoConnectInput &apPassElm = mqtt_setting["apPass"].as<AutoConnectInput>();
        AutoConnectInput &serverNameElm = mqtt_setting["mqttserver"].as<AutoConnectInput>();
        AutoConnectInput &portElm = mqtt_setting["port"].as<AutoConnectInput>();
        AutoConnectInput &gwIDElm = mqtt_setting["gatewayID"].as<AutoConnectInput>();
        AutoConnectInput &noIDElm = mqtt_setting["nodeID"].as<AutoConnectInput>();
        AutoConnectInput &settingsPassElm = mqtt_setting["settingsPass"].as<AutoConnectInput>();
        AutoConnectInput &ntpAddElm = mqtt_setting["ntpAdd"].as<AutoConnectInput>();
        AutoConnectInput &apnElm = mqtt_setting["apn"].as<AutoConnectInput>();

        AutoConnectInput &nameS1Elm = mqtt_setting["nameS1"].as<AutoConnectInput>();
        AutoConnectInput &nameS2Elm = mqtt_setting["nameS2"].as<AutoConnectInput>();
        AutoConnectInput &nameS3Elm = mqtt_setting["nameS3"].as<AutoConnectInput>();
        AutoConnectInput &nameS4Elm = mqtt_setting["nameS4"].as<AutoConnectInput>();

        AutoConnectInput &mulS1Elm = mqtt_setting["mulS1"].as<AutoConnectInput>();
        AutoConnectInput &mulS2Elm = mqtt_setting["mulS2"].as<AutoConnectInput>();
        AutoConnectInput &mulS3Elm = mqtt_setting["mulS3"].as<AutoConnectInput>();
        AutoConnectInput &mulS4Elm = mqtt_setting["mulS4"].as<AutoConnectInput>();

        AutoConnectRadio &sensorEnabled1Elm = mqtt_setting["period1"].as<AutoConnectRadio>();
        AutoConnectRadio &sensorEnabled2Elm = mqtt_setting["period2"].as<AutoConnectRadio>();
        AutoConnectRadio &sensorEnabled3Elm = mqtt_setting["period3"].as<AutoConnectRadio>();
        AutoConnectRadio &sensorEnabled4Elm = mqtt_setting["period4"].as<AutoConnectRadio>();

        AutoConnectText &networkElm = mqtt_setting["network"].as<AutoConnectText>();
        AutoConnectText &providerElm = mqtt_setting["provider"].as<AutoConnectText>();
        AutoConnectText &signalElm = mqtt_setting["signal"].as<AutoConnectText>();

        AutoConnectText &voltageElm = mqtt_setting["voltage"].as<AutoConnectText>();
        AutoConnectText &capacityElm = mqtt_setting["capacity"].as<AutoConnectText>();
        //vibSValueElm.value="VibS:11";
        serverName = String(serverNameElm.value);
        port = String(portElm.value);
        gatewayID = String(gwIDElm.value);
        nodeID = String(noIDElm.value);
        hostName = String(hostnameElm.value);
        apPass = String(apPassElm.value);
        settingsPass = String(settingsPassElm.value);
        ntpAdd = String(ntpAddElm.value);
        apn = String(apnElm.value);

        nameS1 = String(nameS1Elm.value);
        nameS2 = String(nameS2Elm.value);
        nameS3 = String(nameS3Elm.value);
        nameS4 = String(nameS4Elm.value);

        mulS1 = String(mulS1Elm.value);
        mulS2 = String(mulS2Elm.value);
        mulS3 = String(mulS3Elm.value);
        mulS4 = String(mulS4Elm.value);

        sensorEnabled1 = String(sensorEnabled1Elm.value());
        sensorEnabled2 = String(sensorEnabled2Elm.value());
        sensorEnabled3 = String(sensorEnabled3Elm.value());
        sensorEnabled4 = String(sensorEnabled4Elm.value());

        if (hostnameElm.value.length())
        {
            //hostName=hostName+ String("-") + String(GET_CHIPID(), HEX);
            //;
            //portal.config(hostName.c_str(), apPass.c_str());
            // portal.config(hostName.c_str(), "123456789AP");
            config.apid = hostName + "-" + String(GET_CHIPID(), HEX);
            config.password = apPass;
            config.psk = apPass;
            // portal.config(hostName.c_str(), "123456789AP");
            Serial.println("(from hostELM) hostname set to " + hostName);
        }
        else
        {

            // hostName = String("OEE");;
            // portal.config(hostName.c_str(), "123456789AP");
            config.apid = hostName + "-" + String(GET_CHIPID(), HEX);
            config.password = apPass;
            config.psk = apPass;
            //config.hostName = hostName;//hostnameElm.value+ "-" + String(GET_CHIPID(), HEX);
            // portal.config(hostName.c_str(), "123456789AP");
            Serial.println("hostname set to " + hostName);
        }
        config.homeUri = "/_ac";
        config.apip = ipV;

        portal.on(AUX_MQTTSETTING, loadParams);
        portal.on(AUX_MQTTSAVE, saveParams);
    }
    else
    {
        Serial.println("aux. load error");
    }
    //config.homeUri = "/_ac";
    config.apip = ipV;
    config.autoReconnect = true;
    config.reconnectInterval = 1;
    Serial.print("AP: ");
    Serial.println(hostName);
    Serial.print("Password: ");
    Serial.println(apPass);
    config.title = "Smart ADC Device"; //set title of webapp

    //add different tabs on homepage

    server.on("/", handleRoot);
    // Starts user web site included the AutoConnect portal.

    config.auth = AC_AUTH_DIGEST;
    config.authScope = AC_AUTHSCOPE_PARTIAL;
    config.username = hostName;
    config.password = settingsPass;

    portal.config(config);
    portal.whileCaptivePortal(whileCP);
    portal.onDetect(atDetect);
    portal.load(FPSTR(PAGE_AUTH));

    portal.disableMenu(AC_MENUITEM_DISCONNECT);
    portal.disableMenu(AC_MENUITEM_OPENSSIDS);
    portal.disableMenu(AC_MENUITEM_CONFIGNEW);
    if (portal.begin())
    {
        Serial.println("Started, IP:" + WiFi.localIP().toString());
        ledState(AP_MODE);
    }
    else
    {
        Serial.println("Connection failed.");
        while (true)
        {
            yield();
            delay(100);
        }
    }

    MDNS.addService("http", "tcp", 80);
    mqttConnect(); //start mqtt
}

void loop()
{
    //don't edit this function
    //use whileCP function above as a main loop
    server.handleClient();

    portal.handleRequest();

    if (millis() - lastPub > updateInterval) //publish data to mqtt server
    {
        ledState(ACTIVE_MODE);
        lastPub = millis();
    }
}